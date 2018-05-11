/*

	Draggable / resizable container

	Usage:

		var w = App.scene.addChild( 'ui/window' );
		w.title = "Hello";
		w.background = 0x666666;
		w.layoutType = Layout.Vertical;
		w.addChild( .. );

	padding on top is reserved for window header

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'closed' - dispatched after close button is clicked (.active has been set to false)


*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), bg, shp, background = false;
	var header, titleText, closeButton;
	var draggable = true, resizable = true;
	var dragOffsetX, dragOffsetY, dragging = false;
	var constructing = true;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (ui/panel) reference to window header
		[ 'header',  function (){ return header; } ],

		// (ui/text) reference to window title
		[ 'titleText',  function (){ return titleText; } ],

		// (ui/button) reference to window close button
		[ 'closeButton',  function (){ return closeButton; } ],

		// (String) window title
		[ 'title',  function (){ return titleText.text; }, function ( t ){ titleText.text = t; }   ],

		// (Boolean) window can be dragged
		[ 'draggable',  function (){ return draggable; }, function ( d ){ draggable = d; } ],

		// (Boolean) window can be resized
		[ 'resizable',  function (){ return resizable; }, function ( r ){ resizable = r; } ],

		// (String) or (Color) or (Number) or (Image) or (null|false) - set background to sprite, or solid color, or nothing
		[ 'background',  function (){ return background; }, function ( v ){
			background = v;
			if ( v === false || v === null || v === undefined ){
				go.render = null;
			} else if ( typeof( v ) == 'string' ) {
				bg.image = null;
				bg.texture = v;
				go.render = bg;
			} else if ( typeof( v ) == 'object' && v.constructor == Image ){
				bg.image = v;
				bg.texture = null;
				go.render = bg;
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

		// (Number) outline thickness when background is solid color
		[ 'lineThickness',  function (){ return shp.lineThickness; }, function ( b ){
			shp.lineThickness = b;
		} ],

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		[ 'outlineColor',  function (){ return shp.outlineColor; }, function ( c ){
			shp.outlineColor = (c === false ? '00000000' : c );
		} ],

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		[ 'filled',  function (){ return shp.filled; }, function ( v ){ shp.filled = v; } ],

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

	// api

	go[ 'close' ] = function () {
		go.active = false;
		go.fire( 'closed' );
	}

	// create components

	// set name
	if ( !go.name ) go.name = "Window";

	// sprite bg
	bg = new RenderSprite( background );
	go.render = bg;

	// solid color background
	shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});

	// header
	header = go.addChild( './panel', {
		fixedPosition: true,
		x: 0, y: 0,
		layoutType: Layout.Horizontal,
		layoutAlignY: LayoutAlign.Center,
		fitChildren: false,
	});

	// title
	titleText = header.addChild( './text', {
		flex: 1
	} );

	// close button
	closeButton = header.addChild( './button', {
		click: go[ 'close' ],
		focusGroup: 'window'
	} );


	// UI
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = ui.padLeft + ui.padRight;
	ui.height = ui.minHeight = ui.padTop + ui.padBottom;
	ui.layoutType = Layout.Vertical;
	ui.fitChildren = false;
	ui.focusable = false;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {
		shp.resize( w, h );
		bg.resize( w, h );
		// resize title
		header.width = w ;
		ui.padTop = header.height + header.marginBottom;
	}


	// dragging window
	var dragCallback = function ( x, y ) {
		if ( dragging == 'window' ) {
			go.setTransform( x - dragOffsetX, y - dragOffsetY );
		} else if ( dragging == 'width' ) {
			this.width = Math.max( this.minWidth, x - go.x );
		} else if ( dragging == 'height' ) {
			this.height = Math.max( this.minHeight, y - go.y );
		} else if ( dragging == 'widthheight' ) {
			this.width = Math.max( this.minWidth, x - go.x );
			this.height = Math.max( this.minHeight, y - go.y );
		}
	}.bind( ui );

	var mouseUpCallback = function ( btn, x, y ) {
		if ( dragging ) {
			Input.off( 'mouseMove', dragCallback );
			Input.off( 'mouseUp', dragCallback );
			go.eventMask.length = 0;
			go.opacity = 1;
			dragging = false;
		}
		stopAllEvents();
	}.bind( ui );

	// start/stop drag
	ui.mouseDown = function ( btn, x, y ) {
		if ( dragging ) return;
		dragOffsetX = x; dragOffsetY = y;
		stopAllEvents();
		if ( y < header.height ) {
			dragging = 'window';
		} else {
			if ( x >= this.width - this.padRight ) {
				dragging = 'width';
			}
			if ( y >= this.height - this.padBottom ) {
				dragging = ( dragging ? dragging : '' ) + 'height';
			}
			if ( !dragging ) return;
		}
		go.eventMask = [ 'mouseUp', 'mouseMove' ];
		Input.on( 'mouseMove', dragCallback );
		Input.on( 'mouseUp', mouseUpCallback );
		go.opacity = 0.7;
	}

	// apply defaults
	go.baseStyle = Object.create( UI.style.window );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);
