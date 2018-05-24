/*

	Checkbox (or radio button)

	Usage:

		var chk = App.scene.addChild( 'ui/checkbox' );
		chk.text = "Option";
		chk.change = function ( val ) {
			log( "Checked:", val );
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
	var ui = new UI();
	var checkbox;
	var label;
	var disabled = false, disabledCanFocus = true;
	var checked = undefined;
	var cancelToBlur = false;
	var group = null;
	var constructing = true;
	go.serializeMask = [ 'ui', 'render', 'children' ];

	// API properties
	var mappedProps = {

		// (Boolean) space between icon and label
		'checked': {
			get: function (){ return checked; },
			set: function( v ){
				if ( checked != v ) {
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
					if ( checkbox.image ) checkbox.image.opacity = checked ? 1 : 0;
				}
			}, enumerable: true, configurable: true
		},

		// (String) text on button
		'text': {
			get: function (){ return label.text; },
			set: function( v ){
				label.text = v;
				label.active = !!v;
			}, enumerable: true, configurable: true
		},

		// (GameObject) instance of 'ui/text.js' used as label
		'label': { get: function (){ return label; }, enumerable: true, configurable: true },

		// (GameObject) instance of 'ui/button.js' used as checkbox body
		'checkbox': { get: function (){ return checkbox; }, enumerable: true, configurable: true },

		// (Boolean) input disabled
		'disabled': {
			get: function (){ return disabled; },
			set: function( v ){
				ui.disabled = disabled = v;
				ui.focusable = !v || disabledCanFocus;
				if ( v && ui.focused ) ui.blur();
				go.state = 'disabled';
				label.opacity = v ? 0.6 : 1;
				go.dispatch( 'layout' );
			}, enumerable: true, configurable: true
		},

		// (Boolean) whether control is focusable when it's disabled
		'disabledCanFocus': { get: function (){ return disabledCanFocus; }, set: function ( f ){ disabledCanFocus = f; } },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		'cancelToBlur': { get: function (){ return cancelToBlur; }, set: function( cb ){ cancelToBlur = cb; }, enumerable: true, configurable: true },

		// (Array) or null - makes checkbox act as a radio button in a group. Array must be all the checkboxes in a group. One will always be selected
		'group': { get: function (){ return group; }, set: function( v ){ group = v; }, enumerable: true, configurable: true },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "Checkbox";

	// check box
	checkbox = go.addChild( './button', {
		name: "Checkbox",
		slice: 0,
		pad: 0,
		margin: 0,
		layoutAlignX: LayoutAlign.Center,
		layoutAlignY: LayoutAlign.Center,
		states: {},
		disabled: true,
		focusable: false,
		fitChildren: false,
	});
	checkbox.image.pad = checkbox.image.margin = 0;
	checkbox.label.active = false;
	checkbox.image.opacity = 0;
	checkbox.image.mode = 'icon';
	checkbox.label.active = false;

	// label
	label = go.addChild( './text', {
		name: "Label",
		wrap: true,
		flex: 1,
		active: false
	});

	// UI
	ui.autoMoveFocus = true;
	ui.layoutType = Layout.Horizontal;
	ui.layoutAlignX = LayoutAlign.Start;
	ui.layoutAlignY = LayoutAlign.Center;
	ui.fitChildren = true;
	ui.wrapEnabled = false;
	ui.focusable = true;
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

	// navigation event
	ui.navigation = function ( name, value ) {

		stopAllEvents();

		// enter = click
		if ( name == 'accept' ) {

			// simulated click
			ui.fire( 'click', 0, 0, 0, go.x, go.y );

			// animate down / up
			go.state = 'down';
			go.async( function() { go.state = 'auto'; }, 0.1 );

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
		if ( disabled || btn != 1 ) return;
		if ( group && checked ) return; // can't uncheck in a group
		ui.focus();
		go.fire( 'click', btn, x, y, wx, wy );
		go.checked = !checked;
	}

	// mouse down/up state
	ui.mouseDown = function ( btn, x, y, wx, wy ) {
		if ( disabled || btn != 1 ) return;
		go.state = 'down';
		// redispatch on gameObject
		go.fire( 'mouseDown', btn, x, y, wx, wy );
	}

	// up
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y, wx, wy ) {
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
		go.state = 'auto';
		go.fire( currentEventName(), x, y, wx, wy );
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.checkbox );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;

})(this);
