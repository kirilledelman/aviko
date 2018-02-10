/*

	Scrollbar

	Usage:
		var sb = app.scene.addChild( 'ui/scrollbar' );
		sb.orientation = 'vertical';
		sb.totalSize = 100;
		sb.handleSize = 20;
		sb.scroll = function ( scrollPos ) {
			// called when scrolled
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
	var handle, handleBackground, hbg, hshp;
	var position = 0;
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
			UI.base.applyDefaults( go, UI.style.scrollbar[ orientation ] );
			go.fireLate( 'layout' );
		} ],

		// (Number) - position of the handle - 0 to (totalSize - handleSize)
		[ 'position',  function (){ return position; }, function ( p ){
			if ( p != position ) {
				position = Math.max( 0, Math.min( p, totalSize - handleSize ) );
				go.fireLate( 'layout' );
			}
		}],

		// (Number) - total size of scrollable content (note that actual size of scrollbar is set with width/height, or anchors)
		[ 'totalSize',  function (){ return totalSize; }, function ( v ) {
			if ( v != totalSize ) {
				totalSize = v;
				handle.active = (v > 0);
				go.fireLate( 'layout' );
			}
		}],

		// (Number) - the size of visible "window" into content (less than totalSize)
		[ 'handleSize',  function (){ return totalSize; }, function ( v ){
			if ( v != handleSize ) {
				handleSize = v;
				go.fireLate( 'layout' );
			}
		}],

		// (String) or (Color) or (Number) or (null|false)- scrollbar background set to sprite, or solid color, or nothing
		[ 'background',  function (){ return background; }, function ( v ){
			background = v;
			if ( typeof( v ) == 'string' ) {
				bg.texture = v;
				go.render = bg;
			} else if ( v === false || v === null || v === undefined ){
				go.render = null;
			} else {
				shp.color = v;
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

		// (String) or (Color) or (Number) or (null|false) - set draggable handle background to sprite, or solid color, or nothing
		[ 'handleBackground',  function (){ return handleBackground; }, function ( v ){
			handleBackground = v;
			if ( typeof( v ) == 'string' ) {
				hbg.texture = v;
				handle.render = hbg;
			} else if ( v === false || v === null || v === undefined ){
				handle.render = null;
			} else {
				hshp.color = v;
				handle.render = hshp;
			}
		}],

		// (Number) handle corner roundness when background is solid color
		[ 'handleCornerRadius',  function (){ return hshp.radius; }, function ( b ){
			hshp.radius = b;
			hshp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		} ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - handle background texture slice
		[ 'handleSlice',  function (){ return hbg.slice; }, function ( v ){ hbg.slice = v; } ],

		// (Number) handle texture slice top
		[ 'handleSliceTop',  function (){ return hbg.sliceTop; }, function ( v ){ hbg.sliceTop = v; }, true ],

		// (Number) handle texture slice right
		[ 'handleSliceRight',  function (){ return hbg.sliceRight; }, function ( v ){ hbg.sliceRight = v; }, true ],

		// (Number) handle texture slice bottom
		[ 'handleSliceBottom',  function (){ return hbg.sliceBottom; }, function ( v ){ hbg.sliceBottom = v; }, true ],

		// (Number) handle texture slice left
		[ 'handleSliceLeft',  function (){ return hbg.sliceLeft; }, function ( v ){ hbg.sliceLeft = v; }, true ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

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
	handle = new GameObject( { name: "Scrollbar.Handle" } );
	handle.ui = new UI();
	handle.ui.focusable = false;
	handle.serialized = false;
	go.addChild( handle );

	// handle background
	hbg = new RenderSprite();
	hshp = new RenderShape( Shape.Rectangle );
	hshp.radius = 0;
	hshp.filled = true; hshp.centered = false;

	// UI
	ui.layoutType = Layout.None;
	ui.fitChildren = false;
	ui.focusable = false;
	ui.minWidth = ui.minHeight = 8;
	go.ui = ui;

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
				handle.ui.width = w - ui.padLeft - ui.padRight;
				handle.ui.height = Math.round( ( handleSize / totalSize ) * availSize );
				handle.y = ui.padTop + Math.round( ( position / totalSize ) * availSize );
			} else {
				var availSize = w - ui.padLeft - ui.padRight;
				handle.y = ui.padTop;
				handle.ui.height = h - ui.padTop - ui.padBottom;
				handle.ui.width = Math.round( ( handleSize / totalSize ) * availSize );
				handle.x = ui.padLeft + Math.round( ( position / totalSize ) * availSize );
			}
		}
	}

	// move handle while dragging
	handle.ui.mouseMoveGlobal = function ( x, y ) {
		if ( dragging ) {
			var lp = go.globalToLocal( x, y );
			if ( orientation == 'vertical' ) {
				var availSize = ui.height - ui.padTop - ui.padBottom;
				var hy = Math.max( ui.padTop,
	                        Math.min( ui.padTop + availSize - handle.render.height,
	                                  lp.y - grabY ) );
				go.position = totalSize * ( (hy - ui.padTop) / availSize );
				go.fire( 'scroll', go.position );
			} else {
				var availSize = ui.width - ui.padLeft - ui.padRight;
				var hx = Math.max( ui.padLeft,
	                        Math.min( ui.padLeft + availSize - handle.render.width,
	                                  lp.x - grabX ) );
				go.position = totalSize * ( (hx - ui.padLeft) / availSize );
				go.fire( 'scroll', go.position );
			}

		}
	}

	// mouse down on handle
	handle.ui.mouseDown = function ( btn, x, y, wx, wy ) {
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

	// wheel - just dispatch 'scroll' on gameObject
	ui.mouseWheel = function ( wy, wx ) {
		if ( orientation == 'vertical' ) {
			go.fire( 'scroll', position - wy );
		} else {
			go.fire( 'scroll', position + wx );
		}
	}

	// down on scroll pane
	ui.mouseDown = function( btn, x, y, wx, wy ) {
		if ( dragging ) return;

		// try to center handle on point
		handle.ui.fire( 'mouseDown', btn, handle.render.width * 0.5, handle.render.height * 0.5, wx, wy );

		// do first mousemove
		handle.ui.mouseMoveGlobal( wx, wy );
	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.scrollbar );

	// apply orientation
	go.orientation = orientation;

})(this);
