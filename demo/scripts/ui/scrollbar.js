/*

	Scrollbar

	Usage:

		var sb = App.scene.addChild( 'ui/scrollbar' );
		sb.orientation = 'vertical';
		sb.totalSize = 100;
		sb.handleSize = 20;
		sb.scroll = function ( scrollPos ) {
			log( "Scrollbar position changed:", scrollPos );
		}

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'scroll' - when scrollbar is scrolled by dragging handle

*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), bg, shp, background;
	var handle;
	var cancelToBlur = false;
	var disabled = false;
	var position = 0;
	var discrete = false;
	var totalSize = 100;
	var handleSize = 20;
	var orientation = 'vertical';
	var dragging = false, grabX = 0, grabY = 0;
	go.serializeMask = { 'ui':1, 'render':1, 'children': 1 };

	// API properties
	var mappedProps = [

		// (String) 'horizontal' or 'vertical' - scrollbar orientation
		[ 'orientation',  function (){ return orientation; }, function ( o ){
			if ( o != 'vertical' && o != 'horizontal' ) return;
			orientation = o;
			// apply defaults
			UI.base.applyProperties( go, UI.style.scrollbar[ orientation ] );
			go.dispatchLate( 'layout' );
		} ],

		// (Number) - position of the handle - 0 to (totalSize - handleSize)
		[ 'position',
			function (){
				if ( discrete ) return Math.round( position / handleSize ) * handleSize;
				else return position;
			},
			function ( p ){
				if ( p != position ) {
					position = Math.max( 0, Math.min( p, totalSize - handleSize ) );
					if ( discrete ) position = Math.round( position / handleSize ) * handleSize;
					go.dispatchLate( 'layout' );
				}
			}
		],

		// (Number) - total size of scrollable content (note that actual size of scrollbar is set with width/height, or anchors)
		[ 'totalSize',  function (){ return totalSize; }, function ( v ) {
			if ( v != totalSize ) {
				totalSize = v;
				handle.active = (v > 0);
				go.dispatchLate( 'layout' );
			}
		}],

		// (Number) - the size of visible "window" into content (less than totalSize)
		[ 'handleSize',  function (){ return totalSize; }, function ( v ){
			if ( v != handleSize ) {
				handleSize = v;
				go.dispatchLate( 'layout' );
			}
		}],

		// (Boolean) values can only be changed in increments of handleSize (used for scrollbars used as paginators)
		[ 'discrete',  function (){ return discrete; }, function ( cb ){ discrete = cb; } ],

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		[ 'cancelToBlur',  function (){ return cancelToBlur; }, function ( cb ){ cancelToBlur = cb; } ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			 disabled = v;
			 ui.focusable = !v;
			 if ( v && ui.focused ) ui.blur();
			 go.state = 'disabled';
		 } ],

		// (GameObject) - reference to scroll handle
		[ 'handle',  function (){ return handle; } ],

		// (String) or (Color) or (Number) or (null|false)- scrollbar background set to sprite, or solid color, or nothing
		[ 'background',  function (){ return background; }, function ( b ){
			background = b;
			if ( b === null || b === false ) {
				go.render = null;
			} else if ( typeof( b ) == 'string' ) {
				bg.texture = b;
				bg.resize( ui.width, ui.height );
				go.render = bg;
			} else {
				shp.color = b;
				go.render = shp;
			}
		}],

		// (Number) corner roundness when background is solid color
		[ 'cornerRadius',  function (){ return shp.radius; }, function ( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		} ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		[ 'slice',  function (){ return bg.slice; }, function ( v ){ bg.slice = v; } ],

		// (Number) texture slice top
		[ 'sliceTop',  function (){ return bg.sliceTop; }, function ( v ){ bg.sliceTop = v; }, true ],

		// (Number) texture slice right
		[ 'sliceRight',  function (){ return bg.sliceRight; }, function ( v ){ bg.sliceRight = v; }, true ],

		// (Number) texture slice bottom
		[ 'sliceBottom',  function (){ return bg.sliceBottom; }, function ( v ){ bg.sliceBottom = v; }, true ],

		// (Number) texture slice left
		[ 'sliceLeft',  function (){ return bg.sliceLeft; }, function ( v ){ bg.sliceLeft = v; }, true ],



	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "Scrollbar";

	// background
	bg = new RenderSprite();
	shp = new RenderShape( Shape.Rectangle );
	shp.radius = 0;
	shp.filled = true; shp.centered = false;
	go.render = bg;

	// handle
	handle = go.addChild( './button', {
		name: "Scrollbar.Handle",
		disabled: true,
		focusable: false,
		layoutType: Layout.Anchors,
		states: {}
	} );

	// UI
	ui.layoutType = Layout.None;
	ui.fitChildren = false;
	ui.autoMoveFocus = false;
	ui.focusable = true;
	ui.minWidth = ui.minHeight = 8;
	go.ui = ui;

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// focused
	    if ( newFocus == ui ) {
		    go.scrollIntoView();
		    go.state = 'focus';
	    } else {
		    go.state = 'auto';
	    }
		go.fire( 'focusChanged', newFocus );
	}

	// lay out components
	ui.layout = function( w, h ) {
		w = Math.max( w, ui.padLeft + ui.padRight );
		h = Math.max( h, ui.padTop + ui.padBottom );
		go.setTransform( x, y );
		shp.width = bg.width = w;
		shp.height = bg.height = h;

		// hide handle if no scrolling needed
		handle.active = handleSize < totalSize;

		// position handle
		if ( totalSize > 0 && handle.active ) {
			if ( orientation == 'vertical' ) {
				var availSize = h - ui.padTop - ui.padBottom;
				handle.x = ui.padLeft;
				handle.width = w - ui.padLeft - ui.padRight;
				handle.height = Math.round( ( handleSize / totalSize ) * availSize );
				handle.y = ui.padTop + Math.round( ( position / totalSize ) * availSize );
			} else {
				var availSize = w - ui.padLeft - ui.padRight;
				handle.y = ui.padTop;
				handle.height = h - ui.padTop - ui.padBottom;
				handle.width = Math.round( ( handleSize / totalSize ) * availSize );
				handle.x = ui.padLeft + Math.round( ( position / totalSize ) * availSize );
			}
		}
	}

	// move handle while dragging
	handle.ui.mouseMoveGlobal = function ( x, y ) {
		if ( dragging ) {
			var lp = go.globalToLocal( x, y, true );
			var newPos = prevPos = go.position;
			if ( orientation == 'vertical' ) {
				var availSize = ui.height - ui.padTop - ui.padBottom;
				var hy = Math.max( ui.padTop,
	                        Math.min( ui.padTop + availSize - handle.render.height,
	                                  lp.y - grabY ) );
				go.position = totalSize * ( (hy - ui.padTop) / availSize );
			} else {
				var availSize = ui.width - ui.padLeft - ui.padRight;
				var hx = Math.max( ui.padLeft,
	                        Math.min( ui.padLeft + availSize - handle.render.width,
	                                  lp.x - grabX ) );
				go.position = totalSize * ( (hx - ui.padLeft) / availSize );
			}
			newPos = go.position;
			if ( prevPos != newPos ) go.fire( 'scroll', newPos );
		}
	}

	// mouse down on handle
	handle.ui.mouseDown = function ( btn, x, y, wx, wy ) {
		stopEvent();
		if ( disabled ) return;

		ui.focus();

		// position of mouse on handle
		grabX = x; grabY = y;

		// start drag
		dragging = true;
		Input.on( 'mouseMove', handle.ui.mouseMoveGlobal );
		Input.on( 'mouseUp', handle.ui.mouseUpGlobal );
	}

	// mouse up
	handle.ui.mouseUpGlobal = function ( btn, x, y ) {
		if ( dragging ) {
			dragging = false;
			Input.off( 'mouseMove', handle.ui.mouseMoveGlobal );
			Input.off( 'mouseUp', handle.ui.mouseUpGlobal );
		}
	}

	// wheel - dispatch 'scroll' on gameObject
	ui.mouseWheel = function ( wy, wx ) {
		stopEvent();
		if ( disabled ) return;
		var s = 0;
		if ( orientation == 'horizontal' ) {
			s = ( wx == 0 ? wy : wx );
			if ( discrete ) s = s > 0 ? handleSize : -handleSize;
		} else {
			s = -wy;
			if ( discrete ) s = s > 0 ? handleSize : -handleSize;
		}
		var prevPos = go.position;
		go.position += s;
		var newPos = go.position;
		if ( newPos != prevPos ) go.fire( 'scroll', newPos );
	}

	// down on scroll pane
	ui.mouseDown = function( btn, x, y, wx, wy ) {
		if ( disabled ) return;
		if ( disabled || dragging ) return;
		ui.focus();
		go.state = 'down';

		// try to center handle on point
		handle.ui.fire( 'mouseDown', btn, handle.render.width * 0.5, handle.render.height * 0.5, wx, wy );

		// do first mousemove
		handle.ui.mouseMoveGlobal( wx, wy );
	}

	// up
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y, wx, wy ) {
		if ( disabled ) return;
		stopAllEvents();
		go.state = 'auto';
		go.fire( currentEventName(), btn, x, y, wx, wy );
	}

	// rollover / rollout
	ui.mouseOver = ui.mouseOut = function ( x, y, wx, wy ) {
		go.state = 'auto';
		go.fire( currentEventName(), x, y, wx, wy );
	}

	//
	ui.navigation = function ( name, value ) {

		var dir = ( value > 0 ? 1 : -1 );
		var prevPos = position;

		// pressed direction in the same axis as is orientation
		if ( name == orientation ) {
			if ( orientation == 'horizontal' ) {
				ui.mouseWheel( 0, dir * handleSize );
				if ( prevPos == position ) ui.moveFocus( dir, 0 );
			} else {
				ui.mouseWheel( -dir * handleSize, 0 );
				if ( prevPos == position ) ui.moveFocus( 0, dir );
			}

		// blur
		} else if ( name == 'cancel' && cancelToBlur ) {

			ui.blur();

		// accept
		} else if ( name == 'accept' ) {

			// cycle scroll
			if ( position < ( totalSize - handleSize ) ) {

				go.position = position + handleSize;

			} else {

				go.position = 0;

			}


		} else if ( name == 'horizontal' ) {

			ui.moveFocus( dir, 0 );

		} else {

			ui.moveFocus( 0, dir );

		}

	}

	// apply defaults
	UI.base.applyProperties( go, UI.style.scrollbar );

	// apply orientation
	go.orientation = orientation;
	go.state = 'auto';

})(this);
