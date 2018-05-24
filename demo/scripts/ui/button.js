/*

	Button

	Usage:

		var btn = App.scene.addChild( 'ui/button' );
		btn.text = "Click me";
		btn.click = function () {
			log( "Clicked:", this.text );
		}

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js

	Events:
		'click' - button clicked or activated with 'accept' event from controller
		'focusChanged' - when control focus is set or cleared (same as UI event)
		'navigation' - UI navigation event
		'mouseOver' - mouse rolled over button
		'mouseOut' - mouse left button
		'mouseDown' - pressed down
		'mouseUp' - mouse released
		'mouseUpOutside' - mouse released outside button

 */

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), bg, shp, background = false;
	var label, image;
	var disabled = false, disabledCanFocus = true;
	var cancelToBlur = false;
	var constructing = true;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) text on button
		'text': {
			get: function (){ return label.text; },
			set: function( v ){
				label.text = v;
				label.active = !!v;
				ui.requestLayout( 'text' );
			} 
		},

		// (String) or null - texture on icon
		'icon': {
			get: function (){ return image ? image.texture : ''; },
			set: function( v ){
				if ( v ) {
					if ( !image ) makeImage();
					image.render.texture = v;
					image.ui.minWidth = image.render.originalWidth;
					image.ui.minHeight = image.render.originalHeight;
					image.active = true;
				} else if ( image ) {
					image.active = false;
				}
			}
		},

		// (GameObject) instance of 'ui/text.js' used as label
		'label': { get: function (){ return label; } },

		// (GameObject) instance of 'ui/image.js' used as icon
		'image': { get: function (){ return image ? image : makeImage(); } },

		// (Number) space between icon and label
		'gap': { get: function (){ return ui.spacingX; }, set: function ( v ){ ui.spacingX = v; } },

		// (Boolean) input disabled
		'disabled': {
			get: function (){ return disabled; },
		    set: function ( v ){
				 ui.disabled = disabled = v;
				 ui.focusable = !v || disabledCanFocus;
				 if ( v && ui.focused ) ui.blur();
				 go.state = 'auto';
		    }
		},

		// (Boolean) whether control is focusable when it's disabled
		'disabledCanFocus': { get: function (){ return disabledCanFocus; }, set: function ( f ){ disabledCanFocus = f; } },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		'cancelToBlur': { get: function (){ return cancelToBlur; }, set: function ( cb ){ cancelToBlur = cb; } },

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		'background': {
			get: function (){ return background; },
			set: function( b ){
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
			}
		},

		// (Number) corner roundness when background is solid color
		'cornerRadius': {
			get: function (){ return shp.radius; },
			set: function( b ){
				shp.radius = b;
				shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
			} 
		},

		// (Number) outline thickness when background is solid color
		'lineThickness': { get: function (){ return shp.lineThickness; }, set: function( b ){
			shp.lineThickness = b;
		} },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		'outlineColor': { get: function (){ return shp.outlineColor; }, set: function( c ){
			shp.outlineColor = (c === false ? '00000000' : c );
		} },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		'filled': { get: function (){ return shp.filled; }, set: function( v ){ shp.filled = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		'slice': { get: function (){ return bg.slice; }, set: function( v ){ bg.slice = v; } },

		// (Number) texture slice top
		'sliceTop': { get: function (){ return bg.sliceTop; }, set: function( v ){ bg.sliceTop = v; }, serialized: false },

		// (Number) texture slice right
		'sliceRight': { get: function (){ return bg.sliceRight; }, set: function( v ){ bg.sliceRight = v; }, serialized: false },

		// (Number) texture slice bottom
		'sliceBottom': { get: function (){ return bg.sliceBottom; }, set: function( v ){ bg.sliceBottom = v; }, serialized: false },

		// (Number) texture slice left
		'sliceLeft': { get: function (){ return bg.sliceLeft; }, set: function( v ){ bg.sliceLeft = v; }, serialized: false },

	};

	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );
	UI.base.addInspectables( go, 'UI', [ 'text', 'icon', 'disabled', 'cancelToBlur', 'style' ] );

	// create components

	// set name
	if ( !go.name ) go.name = "Button";

	// sprite background
	bg = new RenderSprite( background );
	go.render = bg;

	// solid color background
	shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});

	// label
	label = go.addChild( 'ui/text', {
		name: "Label",
		wrap: false,
		active: false
	}, 1 );

	// UI
	ui.autoMoveFocus = true;
	ui.layoutType = Layout.Horizontal;
	ui.layoutAlignX = LayoutAlign.Center;
	ui.layoutAlignY = LayoutAlign.Center;
	ui.fitChildren = true;
	ui.focusable = true;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h, r ) {
		shp.resize( w, h );
		bg.resize( w, h );
		label.ui.maxWidth = w - ( ui.padLeft + ui.padRight ) - ( image ? ( ui.spacingX + image.ui.width + image.ui.marginLeft + image.ui.marginRight ) : 0 );
	}

	// focus changed
	ui.focusChanged = function ( newFocus, oldFocus ) {
		// focused
	    if ( newFocus == ui ) {
		    go.scrollIntoView();
	    }
	    go.state = 'auto';
		go.fire( 'focusChanged', newFocus );
	}

	// navigation event
	ui.navigation = function ( name, value ) {

		stopAllEvents();

		// enter = click
		if ( name == 'accept' && !disabled ) {

			// simulated click
			ui.fire( 'click', 1, 0, 0, go.x, go.y );

			// animate down / up
			go.state = 'down';
			go.async( function() { go.state = 'auto'; }, 0.1 );

		// escape = blur
		} else if ( name == 'cancel' ) {

			if ( cancelToBlur ) ui.blur();

		}

		// rethrow
		go.fire( 'navigation', name, value );

	}

	// click - forward to gameObject
	ui.click = function ( btn, x, y, wx, wy ) {
		stopAllEvents();
		if ( ui.focusable ) ui.focus();
		if ( disabled || btn != 1 ) return;
		go.fire( 'click', btn, x, y, wx, wy );
	}

	// mouse down/up state
	ui.mouseDown = function ( btn, x, y, wx, wy ) {
		if ( disabled || btn != 1 ) return;
		stopAllEvents();
		go.state = 'down';
		// forward to gameObject
		go.fire( 'mouseDown', btn, x, y, wx, wy );
	}

	// up
	ui.mouseUp = function ( btn, x, y, wx, wy ) {
		go.state = 'auto';
		if ( disabled || btn != 1 ) return;
		go.fire( currentEventName(), btn, x, y, wx, wy );
	}

	// keyboard
	ui.keyDown = function ( code, shift ) {
		// handle tab key
	    switch ( code ) {
		    case Key.Tab:
			    ui.moveFocus( shift ? -1 : 1 );
			    stopEvent();
			    break;
	    }
	}

	// rollover / rollout
	ui.mouseOver = ui.mouseOut = function ( x, y, wx, wy ) {
		stopAllEvents();
		go.state = 'auto';
		go.fire( currentEventName(), x, y, wx, wy );
	}

	function makeImage(){
		image = go.addChild( new GameObject({
			ui: new UI(),
			render: new RenderSprite( { pivotX: 0, pivotY: 0 } )
		} ), 0);
		return image;
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.button );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;
})(this);
