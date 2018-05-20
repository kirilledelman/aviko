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
	var disabled = false;
	var cancelToBlur = false;
	var constructing = true;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) text on button
		[ 'text',  function (){ return label.text; }, function ( v ){
			label.text = v;
			label.active = !!v;
			ui.requestLayout( 'text' );
		} ],

		// (String) or null - texture on icon
		[ 'icon',  function (){ return image.texture; }, function ( v ){
			image.texture = v;
			image.active = !!v;
		} ],

		// (GameObject) instance of 'ui/text.js' used as label
		[ 'label',  function (){ return label; } ],

		// (GameObject) instance of 'ui/image.js' used as icon
		[ 'image',  function (){ return image; } ],

		// (Number) space between icon and label
		[ 'gap',  function (){ return ui.spacingX; }, function ( v ){ ui.spacingX = v; } ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			 ui.disabled = disabled = v;
			 ui.focusable = !v;
			 if ( v && ui.focused ) ui.blur();
			 go.state = 'auto';
			 // label.opacity = v ? 0.6 : 1;
		 } ],

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		[ 'cancelToBlur',  function (){ return cancelToBlur; }, function ( cb ){ cancelToBlur = cb; } ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
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
		} ],

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

	// icons
	image = go.addChild( './image', {
		name: "Icon",
		mode: 'icon',
		active: false
	}, 0 );

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
	}

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

	// navigation event
	ui.navigation = function ( name, value ) {

		stopAllEvents();

		// enter = click
		if ( name == 'accept' ) {

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
		if ( disabled || btn != 1 ) return;
		stopAllEvents();
		if ( ui.focusable ) ui.focus();
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
	ui.mouseUp = /* ui.mouseUpOutside = */ function ( btn, x, y, wx, wy ) {
		if ( disabled || btn != 1 ) return;
		go.state = 'auto';
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

	// apply defaults
	go.baseStyle = Object.create( UI.style.button );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;
})(this);
