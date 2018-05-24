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
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) text
		'text': { get: function (){ return rt.text; }, set: function( t ){ rt.text = t; ui.requestLayout();} },

		// (string) font name
		'font': { get: function (){ return rt.font; }, set: function( f ){ rt.font = f; ui.requestLayout( 'font' ); }  },

		// (string) optional bold font name (improves rendering)
		'boldFont': { get: function (){ return rt.boldFont; }, set: function( f ){ rt.boldFont = f; ui.requestLayout( 'boldFont' ); }  },

		// (string) optional italic font name (improves rendering)
		'italicFont': { get: function (){ return rt.italicFont; }, set: function( f ){ rt.italicFont = f; ui.requestLayout( 'italicFont' ); }  },

		// (string) optional bold+italic font name (improves rendering)
		'boldItalicFont': { get: function (){ return rt.boldItalicFont; }, set: function( f ){ rt.boldItalicFont = f; ui.requestLayout( 'boldItalicFont' ); }  },

		// (Number) font size
		'size': { get: function (){ return rt.size; }, set: function( s ){ rt.size = s; ui.requestLayout( 'size' ); }  },

		// (Boolean) should text be antialiased
		'antialias': { get: function (){ return rt.antialias; }, set: function( a ){ rt.antialias = a; }  },

		// (Number) - Align.Left | Align.Center | Align.Right - text alignment
		'align': { get: function (){ return rt.align; }, set: function( w ){ rt.align = w; }  },

		// (Number) extra spacing between characters
		'characterSpacing': { get: function (){ return rt.characterSpacing; }, set: function( v ){ rt.characterSpacing = v; ui.requestLayout( 'characterSpacing' ); }  },

		// (Number) multiLine line spacing
		'lineSpacing': { get: function (){ return rt.lineSpacing; }, set: function( v ){ rt.lineSpacing = v; ui.requestLayout( 'lineSpacing' ); }  },

		// (Number) returns line height - font size + line spacing
		'lineHeight': { get: function (){ return rt.lineHeight; } },

		// (Boolean) font bold
		'bold': { get: function (){ return rt.bold; }, set: function( v ){ rt.bold = v; ui.requestLayout( 'bold' ); }  },

		// (Boolean) font italic
		'italic': { get: function (){ return rt.italic; }, set: function( v ){ rt.italic = v; ui.requestLayout( 'italic' ); }  },

		// (Boolean) automatically wrap text
		'wrap': { get: function (){ return rt.wrap; }, set: function( v ){ rt.wrap = v; ui.requestLayout( 'wrap' ); }  },

		// (Boolean) enable display ^code formatting (while not editing)
		'formatting': { get: function (){ return rt.formatting; }, set: function( v ){ rt.formatting = v; ui.requestLayout( 'formatting' ); }  },

		// (Number) or (Color) text color
		'color': { get: function (){ return rt.textColor; }, set: function( v ){ rt.textColor = v; }  },

		// (Number) or (Color) ^0 color
		'color0': { get: function (){ return rt.color0; }, set: function( v ){ rt.color0 = v; }  },

		// (Number) or (Color) ^1 color
		'color1': { get: function (){ return rt.color1; }, set: function( v ){ rt.color1 = v; }  },

		// (Number) or (Color) ^2 color
		'color2': { get: function (){ return rt.color2; }, set: function( v ){ rt.color2 = v; }  },

		// (Number) or (Color) ^3 color
		'color3': { get: function (){ return rt.color3; }, set: function( v ){ rt.color3 = v; }  },

		// (Number) or (Color) ^4 color
		'color4': { get: function (){ return rt.color4; }, set: function( v ){ rt.color4 = v; }  },

		// (Number) or (Color) ^5 color
		'color5': { get: function (){ return rt.color5; }, set: function( v ){ rt.color5 = v; }  },

		// (Number) or (Color) ^6 color
		'color6': { get: function (){ return rt.color6; }, set: function( v ){ rt.color6 = v; }  },

		// (Number) or (Color) ^7 color
		'color7': { get: function (){ return rt.color7; }, set: function( v ){ rt.color7 = v; }  },

		// (Number) or (Color) ^8 color
		'color8': { get: function (){ return rt.color8; }, set: function( v ){ rt.color8 = v; }  },

		// (Number) or (Color) ^9 color
		'color9': { get: function (){ return rt.color9; }, set: function( v ){ rt.color9 = v; }  },

		// (Number) or (Color) background color
		'backgroundColor': { get: function (){ return rt.backgroundColor; }, set: function( v ){ rt.backgroundColor = v; }  },

		// (Integer) characters to skip rendering from beginning of string
		'revealStart': { get: function (){ return rt.revealStart; }, set: function( s ){ rt.revealStart = s; }  },

		// (Integer) characters to skip rendering from beginning of string
		'revealEnd': { get: function (){ return rt.revealEnd; }, set: function( s ){ rt.revealEnd = s; }  },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

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
	tc.serializeable = false;

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
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.text );
	UI.base.applyProperties( go, go.baseStyle );
	go.updateSize();
	constructing = false;

})(this);