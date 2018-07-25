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
	
	// API properties
	UI.base.checkboxPrototype = UI.base.checkboxPrototype || {
	
	 	__proto__: UI.base.componentPrototype,

		// (Boolean) space between icon and label
		get checked(){ return this.__checked; },
		set checked( v ){
			if ( this.__checked != v ) {
				if ( this.__group ) {
					if ( v ) {
						// uncheck other
						for ( var i = 0; i < this.__group.length; i++ ) {
							if ( this.__group[ i ] == this ) continue;
							this.__group[ i ].checked = false;
						}
					}
				}
				this.__checked = v;
				if ( this.__checkbox.image ) this.__checkbox.image.opacity = v ? 1 : 0;
				this.fire( 'change', this.__checked );
			}
		},

		// (String) text on button
		get text(){ return this.__label.text; },
		set text( v ){
			this.__label.text = v;
			this.__label.active = !!v;
		},

		// (GameObject) instance of 'ui/text.js' used as label
		get label(){ return this.__label; },

		// (GameObject) instance of 'ui/button.js' used as checkbox body
		get checkbox(){ return this.__checkbox; },

		// (Boolean) input disabled
		get disabled(){ return this.__disabled; },
		set disabled( v ){
			this.ui.disabled = this.__disabled = v;
			this.ui.focusable = !v || this.__disabledCanFocus;
			if ( v && this.ui.focused ) this.ui.blur();
			this.__label.opacity = v ? 0.6 : 1;
			this.state = 'auto';
			this.requestLayout( 'disabled' );
		},

		// (Boolean) whether control is focusable when it's disabled
		get disabledCanFocus (){ return this.__disabledCanFocus; },
		set disabledCanFocus( f ){
				this.__disabledCanFocus = f;
				this.ui.focusable = this.__disabledCanFocus || !this.__disabled;
		},

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		get cancelToBlur(){ return this.__cancelToBlur; },
		set cancelToBlur( cb ){ this.__cancelToBlur = cb; },

		// (Array) or null - makes checkbox act as a radio button in a group. Array must be all the checkboxes in a group. One will always be selected
		get group(){ return this.__group; },
		set group( v ){ this.__group = v; },

		__focusChanged: function ( newFocus ) {
			var go = this.gameObject;
			// focused
		    if ( newFocus == this ) {
			    go.scrollIntoView();
		    }
		    go.state = 'auto';
			go.fire( 'focusChanged', newFocus );
		},
	
		__navigation: function ( name, value ) {
	
			var go = this.gameObject;
			stopAllEvents();
	
			// enter = click
			if ( name == 'accept' ) {
	
				// simulated click
				this.fire( 'click', 1, 0, 0, go.x, go.y );
	
				// animate down / up
				go.state = 'down';
				go.async( function() { go.state = 'auto'; }, 0.1 );
	
			// escape = blur
			} else if ( name == 'cancel' ) {
	
				if ( go.__cancelToBlur ) ui.blur();
	
			// directional - move focus
			} else {
				var dx = 0, dy = 0;
				if ( name == 'horizontal' ) dx = value;
				else dy = value;
				ui.moveFocus( dx, dy );
			}
	
		},
	
		__click: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled || btn != 1 ) return;
			if ( go.__group && go.__checked ) return; // can't uncheck in a group
			this.focus();
			go.fire( 'click', btn, x, y, wx, wy );
			go.checked = !go.__checked;
		},
	
		__mouseDown: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled || btn != 1 ) return;
			go.state = 'down';
			// redispatch on gameObject
			go.fire( 'mouseDown', btn, x, y, wx, wy );
		},
	
		__mouseUp: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled || btn != 1 ) return;
			go.fire( currentEventName(), btn, x, y, wx, wy );
			go.state = 'auto';
		},
	
		__keyDown: function ( code, shift ) {
			// handle tab key
		    switch ( code ) {
			    case Key.Tab:
				    this.moveFocus( shift ? -1 : 1 );
				    stopEvent();
				    break;
		    }
		},
	
		__mouseOverOut: function ( x, y, wx, wy ) {
			this.gameObject.state = 'auto';
			this.gameObject.fire( currentEventName(), x, y, wx, wy );
		}
		
	};

	// init
	go.name = "Checkbox";
	go.ui = new UI( {
		autoMoveFocus: true,
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Center,
		fitChildren: true,
		wrapEnabled: false,
		focusable: true,
		focusChanged: UI.base.checkboxPrototype.__focusChanged,
		navigation: UI.base.checkboxPrototype.__navigation,
		click: UI.base.checkboxPrototype.__click,
		mouseDown: UI.base.checkboxPrototype.__mouseDown,
		mouseUp: UI.base.checkboxPrototype.__mouseUp,
		mouseUpOutside: UI.base.checkboxPrototype.__mouseUp,
		keyDown: UI.base.checkboxPrototype.__keyDown,
		mouseOver: UI.base.checkboxPrototype.__mouseOverOut,
    });
	go.__checkbox = go.addChild( './button', {
		name: "Checkbox",
		slice: 0,
		pad: 0,
		margin: 0,
		layoutAlignX: LayoutAlign.Center,
		layoutAlignY: LayoutAlign.Center,
		states: {},
		disabled: true,
		disabledCanFocus: false,
		fitChildren: false,
		serializeable: false,
		eventMask: [ 'mouseDown', 'click', 'navigation', 'mouseUp' ]
	});
	go.__checkbox.image.pad = go.__checkbox.image.margin = 0;
	go.__checkbox.image.opacity = 0;
	go.__checkbox.image.mode = 'icon';
	go.__checkbox.label.active = false;
	go.__label = go.addChild( './text', {
		name: "Label",
		wrap: true,
		flex: 1,
		active: false,
		serializeable: false,
	});
	go.__disabled = false;
	go.__disabledCanFocus = true;
	go.__checked = undefined;
	go.__cancelToBlur = false;
	go.__group = null;
	go.__proto__ = UI.base.checkboxPrototype;
	go.init();

	// add property-list inspectable info
	UI.base.addInspectables( go, 'Checkbox',
	[ 'text', 'checked', 'disabled', 'cancelToBlur', 'disabledCanFocus', 'group', 'style' ],
	null, 1 );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.checkbox );
	UI.base.applyProperties( go, go.__baseStyle );
	go.state = 'auto';

})(this);
