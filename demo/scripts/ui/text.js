/*

	Text box

	Usage:

		var label = App.scene.addChild( 'ui/input' );
		label.text = "Some text\nSecond line";

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js

	setting .wrap = false will auto-size control to size of text, otherwise
	width should be set manually, or be laid out by a container.

 */

include( './ui' );
(function(go) {
	
	// API properties
	UI.base.textPrototype = UI.base.textPrototype || {

		__proto__: UI.base.componentPrototype,

		// (String) text
		get text(){ return this.__rt.text; }, set text( t ){ this.__rt.text = t; this.ui.requestLayout(); },

		// (string) font name
		get font(){ return this.__rt.font; }, set font( f ){ this.__rt.font = f; this.ui.requestLayout( 'font' ); },

		// (string) optional bold font name (improves rendering)
		get boldFont(){ return this.__rt.boldFont; }, set boldFont( f ){ this.__rt.boldFont = f; this.ui.requestLayout( 'boldFont' ); },

		// (string) optional italic font name (improves rendering)
		get italicFont(){ return this.__rt.italicFont; }, set italicFont( f ){ this.__rt.italicFont = f; this.ui.requestLayout( 'italicFont' ); },

		// (string) optional bold+italic font name (improves rendering)
		get boldItalicFont(){ return this.__rt.boldItalicFont; }, set boldItalicFont( f ){ this.__rt.boldItalicFont = f; this.ui.requestLayout( 'boldItalicFont' ); },

		// (Number) font size
		get size(){ return this.__rt.size; }, set size( s ){ this.__rt.size = s; this.ui.requestLayout( 'size' ); },

		// (Boolean) should text be antialiased
		get antialias(){ return this.__rt.antialias; }, set antialias( a ){ this.__rt.antialias = a; },

		// (Number) - Align.Left | Align.Center | Align.Right - text alignment
		get align(){ return this.__rt.align; }, set align( w ){ this.__rt.align = w; },

		// (Number) extra spacing between characters
		get characterSpacing(){ return this.__rt.characterSpacing; }, set characterSpacing( v ){ this.__rt.characterSpacing = v; this.ui.requestLayout( 'characterSpacing' ); },

		// (Number) multiLine line spacing
		get lineSpacing(){ return this.__rt.lineSpacing; }, set lineSpacing( v ){ this.__rt.lineSpacing = v; this.ui.requestLayout( 'lineSpacing' ); },

		// (Number) returns line height - font size + line spacing
		get lineHeight(){ return this.__rt.lineHeight; },

		// (Boolean) font bold
		get bold(){ return this.__rt.bold; }, set bold( v ){ this.__rt.bold = v; this.ui.requestLayout( 'bold' ); },

		// (Boolean) font italic
		get italic(){ return this.__rt.italic; }, set italic( v ){ this.__rt.italic = v; this.ui.requestLayout( 'italic' ); },

		// (Boolean) automatically wrap text
		get wrap(){ return this.__rt.wrap; }, set wrap( v ){ this.__rt.wrap = v; if ( v ) this.__rt.multiLine = true; this.ui.requestLayout( 'wrap' ); },

		// (Boolean) make text size to fit text up to maxWidth
		get autoSize(){ return this.__autoResize; }, set autoSize( v ){ this.__rt.autoSize = this.__autoResize = v; this.ui.requestLayout( 'autoSize' ); },

		// (Boolean) enable display ^code formatting (while not editing)
		get formatting(){ return this.__rt.formatting; }, set formatting( v ){ this.__rt.formatting = v; this.ui.requestLayout( 'formatting' ); },

		// (Number) or (Color) text color
		get color(){ return this.__rt.textColor; }, set color( v ){ this.__rt.textColor = v; },

		// (Array) of (Color) for ^0-^9 color formatting
		get colors(){ return this.__rt.colors; }, set colors( v ){ this.__rt.colors = v; },

		// (Color) | (Number) .addColor property of current render component
		get addColor(){ return this.__rt.addColor; }, set addColor( v ){ this.__rt.addColor = v; },

		// (Number) or (Color) background color
		get backgroundColor(){ return this.__rt.backgroundColor; }, set backgroundColor( v ){ this.__rt.backgroundColor = v; },

		// (Integer) characters to skip rendering from beginning of string
		get revealStart(){ return this.__rt.revealStart; }, set revealStart( s ){ this.__rt.revealStart = s; },

		// (Integer) characters to skip rendering from beginning of string
		get revealEnd(){ return this.__rt.revealEnd; }, set revealEnd( s ){ this.__rt.revealEnd = s; },

		// (Number) or (Color) pixel shader outline color
		get outlineColor(){ return this.__rt.outlineColor; }, set outlineColor( v ){ this.__rt.outlineColor = v; },

		// (Number) thickness of pixel shader outline
		get outlineRadius(){ return this.__rt.outlineRadius; }, set outlineRadius( v ){ this.__rt.outlineRadius = v; },

		// (Number) X offset of pixel shader outline
		get outlineOffsetX(){ return this.__rt.outlineOffsetX; }, set outlineOffsetX( v ){ this.__rt.outlineOffsetX = v; },

		// (Number) X offset of pixel shader outline
		get outlineOffsetY(){ return this.__rt.outlineOffsetY; }, set outlineOffsetY( v ){ this.__rt.outlineOffsetY = v; },

		// (RenderText) returns reference to RenderText object
		get renderText(){ return this.__rt; },
		
		// (Number) transform origin - sets both x and y
		get pivot(){ return this.__rt.pivotX; }, set pivot( v ){ this.__rt.pivotX = this.__rt.pivotY = v; },
		
		// (Number) transform origin X
		get pivotX(){ return this.__rt.pivotX; }, set pivotX( v ){ this.__rt.pivotX = v; },

		// (Number) transform origin Y
		get pivotY(){ return this.__rt.pivotY; }, set pivotY( v ){ this.__rt.pivotY = v; },
		
		__updateSize: function() {
			if ( this.__autoResize ) {
				this.__rt.width = ( this.ui.maxWidth ? this.ui.maxWidth : 99999 ) - (this.ui.padLeft + this.ui.padRight);
				this.__rt.measure();
				this.ui.minWidth = this.__rt.width + this.ui.padLeft + this.ui.padRight;
			} else {
				this.__rt.autoSize = true;
				if ( this.__rt.wrap ) {
					this.__rt.width = this.ui.width - (this.ui.padLeft + this.ui.padRight);
					this.__rt.measure();
					this.ui.minWidth = this.__rt.size * 2 + (this.ui.padLeft + this.ui.padRight);
				} else {
					this.__rt.measure();
					this.ui.minWidth = this.__rt.width + this.ui.padLeft + this.ui.padRight;
				}
			}
			this.ui.minHeight = this.__rt.height + (this.ui.padTop + this.ui.padBottom);
			this.__rt.width = Math.max( this.ui.width - (this.ui.padLeft + this.ui.padRight), this.ui.minWidth );
			this.__rt.autoSize = this.__autoResize;
		},
	
		__layout: function ( w, h ) {
			this.gameObject.__tc.setTransform( this.padLeft, this.padTop );
			this.gameObject.__updateSize();
		},
	
		__mouseOverOut: function ( x, y, wx, wy ) {
			this.gameObject.fire( currentEventName(), x, y, wx, wy );
		}

	};
	
	// initialize
	go.name = "Text";
	go.ui = new UI( {
		layoutType: Layout.None,
		minWidth: 10,
		minHeight: 10,
		focusable: false,
		layout: UI.base.textPrototype.__layout,
		mouseOver: UI.base.textPrototype.__mouseOverOut,
		mouseOut: UI.base.textPrototype.__mouseOverOut,
	} );
	go.__tc = go.addChild( { serializeable: false }),
	go.__rt = go.__tc.render = new RenderText( {
		autoSize: false,
		multiLine: true,
		wrap: false,
	} );
	go.__proto__ = UI.base.textPrototype;
	go.__init();
	go.serializeMask.push( 'pivot' );

	// add property-list inspectable info
	UI.base.addInspectables( go, "Text",
    [ 'text', 'size', 'font', 'boldFont', 'italicFont', 'boldItalicFont',
	    'align', 'bold', 'italic', 'wrap', 'formatting',
	    'color', 'backgroundColor', 'outlineColor', 'outlineRadius', 'outlineOffsetX', 'outlineOffsetY',
	    'antialias', 'characterSpacing', 'lineSpacing' ],
    {
        'size': { min: 1, max: 128, step: 1, integer: true },
	    'font': { autocomplete: 'file', autocompleteParam: 'fonts;ttf', liveUpdate: false },
	    'boldFont': { autocomplete: 'file', autocompleteParam: 'fonts;ttf', liveUpdate: false },
	    'italicFont': { autocomplete: 'file', autocompleteParam: 'fonts;ttf', liveUpdate: false },
	    'boldItalicFont': { autocomplete: 'file', autocompleteParam: 'fonts;ttf', liveUpdate: false },
	    'align': { enum: [ { text: "Left", value: TextAlign.Left }, { text: "Center", value: TextAlign.Center }, { text: "Right", value: TextAlign.Right },  ] },
	    'outlineRadius': { min: 0, max: 16, integer: true },
	    'color': { inline: true }, 'backgroundColor': { inline: true }, 'outlineColor': { inline: true },
    }, 1 );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.text );
	UI.base.applyProperties( go, go.__baseStyle );
	go.__updateSize();
	
})(this);