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
	var constructing = true;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) text
		[ 'text',   function (){ return rt.text; }, function( t ){
			rt.text = t;
			ui.requestLayout();
		} ],

		// (string) font name
		[ 'font',  function (){ return rt.font; }, function ( f ){ rt.font = f; ui.requestLayout( 'font' ); } ],

		// (string) optional bold font name (improves rendering)
		[ 'boldFont',  function (){ return rt.boldFont; }, function ( f ){ rt.boldFont = f; ui.requestLayout( 'boldFont' ); } ],

		// (string) optional italic font name (improves rendering)
		[ 'italicFont',  function (){ return rt.italicFont; }, function ( f ){ rt.italicFont = f; ui.requestLayout( 'italicFont' ); } ],

		// (string) optional bold+italic font name (improves rendering)
		[ 'boldItalicFont',  function (){ return rt.boldItalicFont; }, function ( f ){ rt.boldItalicFont = f; ui.requestLayout( 'boldItalicFont' ); } ],

		// (Number) font size
		[ 'size',  function (){ return rt.size; }, function ( s ){ rt.size = s; ui.requestLayout( 'size' ); } ],

		// (Boolean) should text be antialiased
		[ 'antialias',  function (){ return rt.antialias; }, function ( a ){ rt.antialias = a; } ],

		// (Number) - Align.Left | Align.Center | Align.Right - text alignment
		[ 'align',  function (){ return rt.align; }, function ( w ){ rt.align = w; } ],

		// (Number) extra spacing between characters
		[ 'characterSpacing',  function (){ return rt.characterSpacing; }, function ( v ){ rt.characterSpacing = v; ui.requestLayout( 'characterSpacing' ); } ],

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

		// (Number) or (Color) background color
		[ 'backgroundColor',  function (){ return rt.backgroundColor; }, function ( v ){ rt.backgroundColor = v; } ],

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
	tc = go.addChild();
	rt = new RenderText();
	rt.autoResize = false;
	rt.multiLine = true;
	rt.wrap = false;
	tc.render = rt;
	tc.serialized = false;

	// UI
	ui.layoutType = Layout.None;
	ui.minWidth = 10; ui.minHeight = 10;
	ui.focusable = false;
	go.ui = ui;

	// immediate
	go.updateSize = function() {
		rt.autoResize = true;
		if ( rt.wrap ) {
			rt.width = ui.width - (ui.padLeft + ui.padRight);
			rt.measure();
			ui.minWidth = rt.size * 2 + (ui.padLeft + ui.padRight);
		} else {
			rt.measure();
			ui.minWidth = rt.width + ui.padLeft + ui.padRight;
		}
		ui.minHeight = rt.height + (ui.padTop + ui.padBottom);
		rt.width = ui.width - (ui.padLeft + ui.padRight);
		rt.autoResize = false;
	}

	// layout components
	ui.layout = function ( w, h ) {
		tc.setTransform( ui.padLeft, ui.padTop );
		go.updateSize();
	}

	// apply defaults
	go.baseStyle = Object.create( UI.style.text );
	UI.base.applyProperties( go, go.baseStyle );
	go.updateSize();
	constructing = false;

})(this);