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
	var modalBackground;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (ui/panel) reference to window header
		'header': { get: function (){ return header; } },

		// (ui/text) reference to window title
		'titleText': { get: function (){ return titleText; } },

		// (ui/button) reference to window close button
		'closeButton': { get: function (){ return closeButton; } },

		// (String) window title
		'title': { get: function (){ return titleText.text; }, set: function( t ){ titleText.text = t; } },

		// (Boolean) window can be dragged
		'draggable': { get: function (){ return draggable; }, set: function( d ){ draggable = d; } },

		// (Boolean) window can be resized
		'resizable': { get: function (){ return resizable; }, set: function( r ){ resizable = r; } },

		// (Boolean) window blocks UI events outside its bounds
		'modal': {
			get: function (){ return !!modalBackground; },
			set: function( m ){
				if ( m && !modalBackground ) {
					modalBackground = new GameObject( './panel', {
						width: App.windowWidth,
						height: App.windowHeight,
						style: go.baseStyle.modalBackground
					} );
					modalBackground.ui.mouseMove = modalBackgroundCallback;
					modalBackground.ui.mouseOver = modalBackgroundCallback;
					modalBackground.ui.mouseOut = modalBackgroundCallback;
					modalBackground.ui.mouseDown = modalBackgroundCallback;
					modalBackground.ui.mouseUp = modalBackgroundCallback;
					modalBackground.ui.mouseWheel = modalBackgroundCallback;
					modalBackground.ui.click = modalBackgroundCallback;
					if ( go.parent ) {
						go.parent.addChild( modalBackground, go.parent.children.indexOf( go ) );
					}
					draggable = resizable = false;
				} else if ( modalBackground && !m ) {
					modalBackground.parent = null;
					modalBackground = null;
				}
			}
		},

		// (String) or (Color) or (Number) or (Image) or (null|false) - set background to sprite, or solid color, or nothing
		'background': {
			get: function (){ return background; },
			set: function( v ){
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
			}
		},

		// (Number) corner roundness when background is solid color
		'cornerRadius': { get: function (){ return shp.radius; }, set: function( b ){ shp.radius = b; shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle; } },

		// (Number) outline thickness when background is solid color
		'lineThickness': { get: function (){ return shp.lineThickness; }, set: function( b ){ shp.lineThickness = b; } },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		'outlineColor': { get: function (){ return shp.outlineColor; }, set: function( c ){ shp.outlineColor = (c === false ? '00000000' : c ); } },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		'filled': { get: function (){ return shp.filled; }, set: function( v ){ shp.filled = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		'slice': { get: function (){ return bg.slice; }, set: function( v ){ bg.slice = v; }  },

		// (Number) texture slice top
		'sliceTop': { get: function (){ return bg.sliceTop; }, set: function( v ){ bg.sliceTop = v; }, serialized: false  },

		// (Number) texture slice right
		'sliceRight': { get: function (){ return bg.sliceRight; }, set: function( v ){ bg.sliceRight = v; }, serialized: false },

		// (Number) texture slice bottom
		'sliceBottom': { get: function (){ return bg.sliceBottom; }, set: function( v ){ bg.sliceBottom = v; }, serialized: false },

		// (Number) texture slice left
		'sliceLeft': { get: function (){ return bg.sliceLeft; }, set: function( v ){ bg.sliceLeft = v; }, serialized: false },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

	// api

	go[ 'close' ] = function () {
		if ( modalBackground ) {
			go.parent = null;
		} else {
			go.active = false;
		}
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
		// modal is centered
		if ( modalBackground ) {
			modalBackground.resize( App.windowWidth, App.windowHeight );
			go.setTransform( ( App.windowWidth - w ) * 0.5, ( App.windowHeight - h ) * 0.5 );
		}
	}

	function modalBackgroundCallback() {
		// log( "modalBackgroundCallback", currentEventName() );
		stopAllEvents();
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
		stopAllEvents();
	}.bind( ui );

	var mouseUpCallback = function ( btn, x, y ) {
		if ( dragging ) {
			Input.mouseMove = null;
			Input.mouseUp = null;
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
		if ( draggable && y < header.height ) {
			dragging = 'window';
		} else if ( resizable ) {
			if ( x >= this.width - this.padRight ) {
				dragging = 'width';
			}
			if ( y >= this.height - this.padBottom ) {
				dragging = ( dragging ? dragging : '' ) + 'height';
			}
			if ( !dragging ) return;
		} else return;
		go.eventMask = [ 'mouseUp', 'mouseMove' ];
		Input.mouseMove = dragCallback;
		Input.mouseUp = mouseUpCallback;
		go.opacity = 0.7;
	}

	// add modal background under window if present
	go.added = function () {
		if ( modalBackground ) {
			go.parent.addChild( modalBackground, go.parent.children.indexOf( go ) );
		}
	}

	// remove modal background if present
	go.removed = function () {
		if ( modalBackground ) modalBackground.parent = null;
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.window );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);
