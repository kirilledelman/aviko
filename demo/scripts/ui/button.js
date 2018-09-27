/*

	Button

	Usage:

		var btn = App.scene.addChild( 'ui/button', {
			text: "Click me!",
			click: function () {
				log( "Clicked:", this.text );
			}
		} );

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

	// API properties
	UI.base.buttonPrototype = UI.base.buttonPrototype || {

		__proto__: UI.base.componentPrototype,
		
		// (String) text on button
		get text(){ return this.__label.text; },
		set text( v ){
			this.__label.text = v;
			this.__label.active = !!v;
			this.ui.requestLayout( 'text' );
		},

		// (String) or null - texture on icon
		get icon(){ return this.__image ? this.__image.render.texture : ''; },
		set icon( v ){
			if ( v ) {
				if ( !this.__image ) this.__makeImage();
				this.__image.render.texture = v;
				this.__image.ui.minWidth = this.__image.render.originalWidth;
				this.__image.ui.minHeight = this.__image.render.originalHeight;
				this.__image.active = true;
			} else if ( this.__image ) {
				this.__image.active = false;
			}
		},

		// (GameObject) instance of 'ui/text.js' used as label
		get label(){ return this.__label; },

		// (GameObject) object used as icon
		get image(){ return this.__image ? this.__image : this.__makeImage(); },

		// (Boolean) disabled
		get disabled(){ return this.__disabled; },
	    set disabled( v ){
			 this.ui.disabled = this.__disabled = v;
			 this.ui.focusable = !v || this.__disabledCanFocus;
			 if ( v && this.ui.focused ) this.ui.blur();
			 this.state = 'auto';
	    },

		// (Boolean) whether control is focusable when it's disabled
		get disabledCanFocus(){ return this.__disabledCanFocus; },
		set disabledCanFocus( f ){
			this.__disabledCanFocus = f;
			this.ui.focusable = this.__disabledCanFocus || !this.__disabled;
		},

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		get cancelToBlur(){ return this.__cancelToBlur; }, set cancelToBlur( cb ){ this.__cancelToBlur = cb; },

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		get background(){ return this.__background; },
		set background( b ){
			this.__background = b;
			if ( b === null || b === false ) {
				this.render = null;
			} else if ( typeof( b ) == 'string' ) {
				this.__bg.texture = b;
				this.__bg.resize( this.ui.width, this.ui.height );
				this.render = this.__bg;
			} else {
				this.__shp.color = b;
				this.render = this.__shp;
			}
		},

		// (Number) corner roundness when background is solid color
		get cornerRadius(){ return this.__shp.radius; },
		set cornerRadius( b ){
			this.__shp.radius = b;
			this.__shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		},

		// (Number) outline thickness when background is solid color
		get lineThickness(){ return this.__shp.lineThickness; }, set lineThickness( b ){ this.__shp.lineThickness = b; },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		get outlineColor(){ return this.__shp.outlineColor; }, set outlineColor( c ){ this.__shp.outlineColor = (c === false ? '00000000' : c ); },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		get filled(){ return this.__shp.filled; }, set filled( v ){ this.__shp.filled = v; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		get slice(){ return this.__bg.slice; }, set slice( v ){ this.__bg.slice = v; },

		// (Number) texture slice top
		get sliceTop(){ return this.__bg.sliceTop; }, set sliceTop( v ){ this.__bg.sliceTop = v; },

		// (Number) texture slice right
		get sliceRight(){ return this.__bg.sliceRight; }, set sliceRight( v ){ this.__bg.sliceRight = v; },

		// (Number) texture slice bottom
		get sliceBottom(){ return this.__bg.sliceBottom; }, set sliceBottom( v ){ this.__bg.sliceBottom = v; },

		// (Number) texture slice left
		get sliceLeft(){ return this.__bg.sliceLeft; }, set sliceLeft( v ){ this.__bg.sliceLeft = v; },
		
		__layout: function( w, h, r ) {
			var go = this.gameObject;
			go.__shp.resize( w, h );
			go.__bg.resize( w, h );
			go.__label.ui.maxWidth = w - ( this.padLeft + this.padRight )
				- ( go.__image ? ( this.spacingX + go.__image.ui.width + go.__image.ui.marginLeft + go.__image.ui.marginRight ) : 0 );
			go.fire( 'layout', w, h );
		},
	
		__focusChanged: function ( newFocus, oldFocus ) {
			var go = this.gameObject;
			var inst = go.__;
		    if ( newFocus == this ) go.scrollIntoView();
		    go.state = 'auto';
			go.fire( 'focusChanged', newFocus );
		},
	
		__navigation: function ( name, value ) {
			var go = this.gameObject;
			stopAllEvents();
	
			// same as click or enter
			if ( name == 'accept' && !go.__disabled ) {
	
				// animate down / up
				go.state = 'down';
				go.debounce( 'up', function() { this.state = 'auto'; }, 0.1 );
	
				// simulated click
				this.fire( 'click', 1, 0, 0, go.x, go.y );
	
			// same as escape
			} else if ( name == 'cancel' ) {
	
				if ( cancelToBlur ) this.blur();
	
			}
	
			// rethrow
			go.fire( 'navigation', name, value );
	
		},
	
		__click: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( this.focusable ) this.async( this.focus );
			stopAllEvents();
			if ( go.__disabled ) return;
			go.fire( 'click', btn, x, y, wx, wy );
		},
	
		__mouseDown: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled ) return;
			stopAllEvents();
			go.state = 'down';
			go.fire( 'mouseDown', btn, x, y, wx, wy );
		},
	
		__mouseUp: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			go.state = 'auto';
			if ( go.__disabled ) return;
			go.fire( currentEventName(), btn, x, y, wx, wy );
		},
	
		__keyDown: function ( code, shift ) {
			// handle default keys (regardless of controller config)
		    switch ( code ) {
			    case Key.Tab:
				    this.moveFocus( shift ? -1 : 1 );
				    break;
			    case Key.Enter:
				    this.navigation( 'accept', 1 );
				    break;
			    case Key.Escape:
				    this.navigation( 'cancel', 1 );
				    break;
			    default:
			    	return;
		    }
		    stopEvent();
		},
	
		__mouseOverOut: function ( x, y, wx, wy ) {
			var go = this.gameObject;
			go.state = 'auto';
			go.fire( currentEventName(), x, y, wx, wy );
		},
	
		__makeImage: function (){
			this.__image = this.addChild(
				new GameObject({
					ui: new UI(),
					render: new RenderSprite( { pivotX: 0, pivotY: 0 } ),
					serializeable: false,
				}), 0 );
			return this.__image;
		}

	};
	
	// initialize
	go.name = "Button";
	go.ui = new UI( {
		autoMoveFocus: true,
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Center,
		layoutAlignY: LayoutAlign.Center,
		fitChildren: true,
		focusable: true,
		layout: UI.base.buttonPrototype.__layout,
		navigation: UI.base.buttonPrototype.__navigation,
		focusChanged: UI.base.buttonPrototype.__focusChanged,
		mouseDown: UI.base.buttonPrototype.__mouseDown,
		click: UI.base.buttonPrototype.__click,
		mouseUp: UI.base.buttonPrototype.__mouseUp,
		mouseUpOutside: UI.base.buttonPrototype.__mouseUp,
		mouseOver: UI.base.buttonPrototype.__mouseOverOut,
		mouseOut: UI.base.buttonPrototype.__mouseOverOut,
		keyDown: UI.base.buttonPrototype.__keyDown,
    } );
	go.__bg = go.render = new RenderSprite();
	go.__shp = new RenderShape( Shape.Rectangle, {
			radius: 0,
			filled: true,
			centered: false
		});
	go.__background = false;
	go.__label = go.addChild( './text', {
		name: "Label",
		wrap: false,
		active: false,
		autoSize: true,
		serializeable: false,
	}, 1 );
	go.__image = null;
	go.__disabled = false;
	go.__disabledCanFocus = true;
	go.__cancelToBlur = false;
	go.__proto__ = UI.base.buttonPrototype;
	go.__init();
	go.serializeMask.push( 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' );
	
	// add property-list inspectable info
	UI.base.addInspectables( go, 'Button',
		[ 'text', 'icon', 'disabled', 'cancelToBlur', 'disabledCanFocus', 'style' ],
		{ 'icon': { autocomplete: 'texture' } }, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.button );
	UI.base.applyProperties( go, go.__baseStyle );
	go.state = 'auto';
	
})(this);
