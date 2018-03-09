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

	// inner props
	var ui = new UI(), tc, rt;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) text
		[ 'text',   function (){ return rt.text; }, function( t ){
			if ( !rt.wrap || !ui.width ) {
				rt.autoResize = true;
				rt.text = t;
				rt.measure();
				ui.width = rt.width + ui.padLeft + ui.padRight;
				ui.height = rt.height + ui.padTop + ui.padTop;
				rt.autoResize = false;
			} else {
				rt.text = t;
				ui.requestLayout( 'text' );
			}

		} ],

		// (Number) current width of the control
		[ 'width',  function (){ return ui.width; }, function ( w ){ ui.width = w; } ],

		// (Number) current height of the control
		[ 'height',  function (){ return ui.height; }, function ( h ){ ui.height = h; } ],

		// (string) font name
		[ 'font',  function (){ return rt.font; }, function ( f ){ rt.font = f; ui.requestLayout( 'font' ); } ],

		// (Number) font size
		[ 'size',  function (){ return rt.size; }, function ( s ){ rt.size = s; ui.requestLayout( 'size' ); } ],

		// (Boolean) should text be antialiased
		[ 'antialias',  function (){ return rt.antialias; }, function ( a ){ rt.antialias = a; } ],

		// (Number) - Align.Left | Align.Center | Align.Right - text alignment
		[ 'align',  function (){ return rt.align; }, function ( w ){ rt.align = w; } ],

		// (Number) extra spacing between characters
		[ 'characterSpacing',  function (){ return rt.characterSpacing; }, function ( v ){ rt.characterSpacing = v; } ],

		// (Number) multiLine line spacing
		[ 'lineSpacing',  function (){ return rt.lineSpacing; }, function ( v ){ rt.lineSpacing = v; ui.requestLayout( 'lineSpacing' ); } ],

		// (Number) returns line height - font size + line spacing
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],

		// (Boolean) font bold
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; ui.requestLayout( 'bold' ); } ],

		// (Boolean) font italic
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; ui.requestLayout( 'italic' ); } ],

		// (Boolean) automatically wrap text
		[ 'wrap',  function (){ return rt.wrap; }, function ( v ){ rt.wrap = v; ui.requestLayout( 'wrap' ); } ],

		// (Boolean) enable display ^code formatting (while not editing)
		[ 'formatting',  function (){ return rt.formatting; }, function ( v ){ rt.formatting = v; ui.requestLayout( 'formatting' ); } ],

		// (Number) or (Color) text color
		[ 'color',  function (){ return rt.textColor; }, function ( v ){ rt.textColor = v; } ],

		// (Number) or (Color) ^0 color
		[ 'color0',  function (){ return rt.color0; }, function ( v ){ rt.color0 = v; } ],

		// (Number) or (Color) ^1 color
		[ 'color1',  function (){ return rt.color1; }, function ( v ){ rt.color1 = v; } ],

		// (Number) or (Color) ^2 color
		[ 'color2',  function (){ return rt.color2; }, function ( v ){ rt.color2 = v; } ],

		// (Number) or (Color) ^3 color
		[ 'color3',  function (){ return rt.color3; }, function ( v ){ rt.color3 = v; } ],

		// (Number) or (Color) ^4 color
		[ 'color4',  function (){ return rt.color4; }, function ( v ){ rt.color4 = v; } ],

		// (Number) or (Color) ^5 color
		[ 'color5',  function (){ return rt.color5; }, function ( v ){ rt.color5 = v; } ],

		// (Number) or (Color) ^6 color
		[ 'color6',  function (){ return rt.color6; }, function ( v ){ rt.color6 = v; } ],

		// (Number) or (Color) ^7 color
		[ 'color7',  function (){ return rt.color7; }, function ( v ){ rt.color7 = v; } ],

		// (Number) or (Color) ^8 color
		[ 'color8',  function (){ return rt.color8; }, function ( v ){ rt.color8 = v; } ],

		// (Number) or (Color) ^9 color
		[ 'color9',  function (){ return rt.color9; }, function ( v ){ rt.color9 = v; } ],

		// (Integer) characters to skip rendering from beginning of string
		[ 'revealStart',  function (){ return rt.revealStart; }, function ( s ){ rt.revealStart = s; } ],

		// (Integer) characters to skip rendering from beginning of string
		[ 'revealEnd',  function (){ return rt.revealEnd; }, function ( s ){ rt.revealEnd = s; } ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "Text";

	// text container
	tc = new GameObject();
	rt = new RenderText();
	rt.autoResize = false;
	rt.multiLine = true;
	rt.wrap = false;
	tc.render = rt;
	tc.serialized = false;

	// UI
	ui.layoutType = Layout.Anchors;
	ui.minWidth = 10; ui.minHeight = 10;
	ui.focusable = false;
	go.ui = ui;

	// children are added after component is awake,
	// because component's component-children may be overwritten on unserialize
	go.awake = function () {
		go.addChild( tc );
	};

	// layout components
	ui.layout = function ( w, h ) {
		tc.setTransform( ui.padLeft, ui.padTop );
		rt.width = w - ( ui.padLeft + ui.padRight );
		rt.measure();
		rt.height = rt.numLines * rt.lineHeight;
		ui.minHeight = rt.height + ui.padTop + ui.padBottom;
	}

	// apply defaults
	UI.base.applyProperties( go, UI.style.text );

})(this);