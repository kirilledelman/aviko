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
	var ui = new UI(), container, bg, shp, background = false;
	var label, image;
	var disabled = false;
	var down = false;
	var cancelToBlur = true;
	var offBackground = false;
	var focusBackground = false;
	var overBackground = false;
	var disabledBackground = false;
	var downBackground = false;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) text on button
		[ 'text',  function (){ return label.text; }, function ( v ){
			go.label.text = v;
			go.label.active = !!v;
		} ],

		// (String) or null - texture on icon
		[ 'icon',  function (){ return image.texture; }, function ( v ){
			go.image.texture = v;
			go.image.active = !!v;
		} ],

		// (GameObject) instance of 'ui/text.js' used as label
		[ 'label',  function (){
			if ( !label ) {
				label = container.addChild( './text', {
					name: "Label",
					wrap: false,
					active: false
				}, 1 );
			}
			return label;
		} ],

		// (GameObject) instance of 'ui/image.js' used as icon
		[ 'image',  function (){
			if ( !image ) {
				image = container.addChild( './image', {
					name: "Icon",
					mode: 'icon',
					active: false
				}, 0 );
			}
			return image;
		} ],

		// (Number) space between icon and label
		[ 'gap',  function (){ return container.ui.spacingX; }, function ( v ){ container.ui.spacingX = v; } ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			 disabled = v;
			 ui.focusable = !v;
			 if ( v && ui.focused ) ui.blur();
			 go.updateBackground();
			 label.opacity = v ? 0.6 : 1;
			 go.dispatch( 'layout' );
		 } ],

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		[ 'cancelToBlur',  function (){ return cancelToBlur; }, function ( cb ){ cancelToBlur = cb; } ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		[ 'background',  function (){ return offBackground; }, function ( b ){
			offBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background when focused
		[ 'focusBackground',  function (){ return focusBackground; }, function ( b ){
			focusBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background when focused
		[ 'overBackground',  function (){ return overBackground; }, function ( b ){
			overBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background when disabled
		[ 'disabledBackground',  function (){ return disabledBackground; }, function ( b ){
			disabledBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background when button is pressed down
		[ 'downBackground',  function (){ return downBackground; }, function ( b ){
			downBackground = b;
			go.updateBackground();
		} ],

		// (Number) corner roundness when background is solid color
		[ 'cornerRadius',  function (){ return shp.radius; }, function ( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
			go.updateBackground();
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

	// container
	container = go.addChild();
	container.name = "Button.Container";
	container.ui = new UI( {
		spacingX: 4,
		layoutType: Layout.Horizontal,
		layoutAlign: LayoutAlign.Center,
		fitChildren: true,
		flex: 1,
	} );
	container.serialized = false;

	// UI
	ui.autoMoveFocus = true;
	ui.layoutType = Layout.Vertical;
	ui.layoutAlign = LayoutAlign.Center;
	ui.fitChildren = true;
	ui.focusable = true;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {
		shp.resize( w, h );
		bg.resize( w, h );
	}

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// focused
	    if ( newFocus == ui ) {
		    go.scrollIntoView();
	    }
	    go.updateBackground();
		go.fire( 'focusChanged', newFocus );
	}

	// navigation event
	ui.navigation = function ( name, value ) {

		stopAllEvents();

		// enter = click
		if ( name == 'accept' ) {

			// simulated click
			ui.fire( 'click', 0, 0, 0, go.x, go.y );

			// animate down / up
			down = true; go.updateBackground();
			go.async( function() { down = false; go.updateBackground(); }, 0.1 );

		// escape = blur
		} else if ( name == 'cancel' ) {

			if ( cancelToBlur ) ui.blur();

		// directional - move focus
		} else {
			/*var dx = 0, dy = 0;
			if ( name == 'horizontal' ) dx = value;
			else dy = value;
			ui.moveFocus( dx, dy );*/
		}

		// rethrow
		go.fire( 'navigation', name, value );

	}

	// click - forward to gameObject
	ui.click = function ( btn, x, y, wx, wy ) {
		stopAllEvents();
		if ( disabled ) return;
		ui.focus();
		go.fire( 'click', btn, x, y, wx, wy );
	}

	// mouse down/up state
	ui.mouseDown = function ( btn, x, y, wx, wy ) {
		stopAllEvents();
		if ( disabled ) return;
		down = true; go.updateBackground();
		// forward to gameObject
		go.fire( 'mouseDown', btn, x, y, wx, wy );
	}

	// up
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y, wx, wy ) {
		stopAllEvents();
		if ( disabled ) return;
		down = false; go.updateBackground();
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
		go.updateBackground();
		go.fire( currentEventName(), x, y, wx, wy );
	}

	// sets current background based on state
	go.updateBackground = function () {

		// determine state
		var prop;
		if ( down ) {
			prop = downBackground;
		} else if ( ui.focused ) {
			prop = focusBackground;
		} else if ( disabled ) {
			prop = disabledBackground;
		} else if ( ui.over ) {
			prop = overBackground;
		} else {
			prop = offBackground;
		}

		// set look
		if ( prop === null || prop === false ) {
			go.render = null;
		} else if ( typeof( prop ) == 'string' ) {
			bg.texture = prop;
			bg.resize( ui.width, ui.height );
			go.render = bg;
		} else {
			shp.color = prop;
			go.render = shp;
		}

	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.button );

})(this);
