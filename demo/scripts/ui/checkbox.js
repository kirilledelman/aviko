/*

	Checkbox (or radio button)

	Usage:
		var chk = app.scene.addChild( 'ui/checkbox' );
		btn.text = "Option";
		btn.change = function ( val ) {
		}

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js

	Events:
		'change' - checked state changed
		'focusChanged' - when control focus is set or cleared (same as UI event)

 */

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), container, bg, shp, background = false;
	var label, image;
	var disabled = false;
	var checked = false;
	var down = false;
	var cancelToBlur = true;
	var offBackground = false;
	var focusBackground = false;
	var overBackground = false;
	var disabledBackground = false;
	var downBackground = false;
	var group = null;
	var constructing = true;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (Boolean) space between icon and label
		[ 'checked', function (){ return checked; }, function ( v ){
			if ( group ) {
				if ( v ) {
					// uncheck other
					for ( var i = 0; i < group.length; i++ ) {
						if ( group[ i ] == go ) continue;
						group[ i ].checked = false;
					}
				}
			}
			checked = v;
			if ( !constructing ) go.fire( 'change', checked );
			image.opacity = checked ? 1 : 0;

		} ],

		// (String) text on button
		[ 'text',  function (){ return label.text; }, function ( v ){
			label.text = v;
			label.active = !!v;
		} ],

		// (String) or null - texture on icon
		[ 'icon',  function (){ return image.texture; }, function ( v ){
			image.texture = v;
			go.image.active = !!v;
		} ],

		// (GameObject) instance of 'ui/text.js' used as label
		[ 'label',  function (){ return label; } ],

		// (GameObject) instance of 'ui/image.js' used as icon
		[ 'image',  function (){ return image; } ],

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

		// (Array) or null - makes checkbox act as a radio button in a group. Array must be all the checkboxes in a group. One will always be selected
		[ 'group',  function (){ return group; }, function ( v ){ group = v; } ],

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
	if ( !go.name ) go.name = "Checkbox";

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

	// check mark
	image = container.addChild( './image', {
		name: "Check",
		mode: 'icon',
		opacity: 0
	});

	// label
	label = container.addChild( './text', {
		name: "Label",
		wrap: false
	});

	// background
	bg = new RenderSprite( background );
	container.render = bg;
	container.ui.layout = function ( w, h ) {
		shp.resize( h, h );
		bg.resize( h, h ); // square
	}

	// UI
	ui.autoMoveFocus = true;
	ui.layoutType = Layout.Vertical;
	ui.layoutAlign = LayoutAlign.Stretch;
	ui.fitChildren = true;
	ui.focusable = true;
	go.ui = ui;

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
			var dx = 0, dy = 0;
			if ( name == 'horizontal' ) dx = value;
			else dy = value;
			ui.moveFocus( dx, dy );
		}

	}

	// click - dispatch on gameObject
	ui.click = function ( btn, x, y, wx, wy ) {
		if ( disabled ) return;
		if ( group && checked ) return; // can't uncheck in a group
		ui.focus();
		go.fire( 'click', btn, x, y, wx, wy );
		go.checked = !checked;
	}

	// mouse down/up state
	ui.mouseDown = function ( btn, x, y, wx, wy ) {
		if ( disabled ) return;
		down = true; go.updateBackground();
		// redispatch on gameObject
		go.fire( 'mouseDown', btn, x, y, wx, wy );
	}

	// up
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y, wx, wy ) {
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
			container.render = null;
		} else if ( typeof( prop ) == 'string' ) {
			bg.texture = prop;
			bg.resize( shp.height, shp.height );
			container.render = bg;
		} else {
			shp.color = prop;
			container.render = shp;
		}

	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.checkbox );
	constructing = false;

})(this);
