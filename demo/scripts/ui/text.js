/*

	Text box

	Usage:
		var textField = app.scene.addChild( 'ui/input' );
		textField.text = "Some text";

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js


 */

include( './ui' );
(function(go) {

	// inner props
	var ui = new UI(), tc, rt;
	var autoGrow = false;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) input contents
		[ 'text',   function (){ return rt.text; }, function( t ){ rt.text = t; go.fire( 'layout' ); } ],

		// (Number) current width of the control
		[ 'width',  function (){ return ui.width; }, function ( w ){ ui.width = w; } ],

		// (Number) current height of the control
		[ 'height',  function (){ return ui.height; }, function ( h ){ ui.height = h; } ],

		// (string) font name
		[ 'font',  function (){ return rt.font; }, function ( f ){ rt.font = f; } ],

		// (Number) font size
		[ 'size',  function (){ return rt.size; }, function ( s ){
			rt.size = s;
			ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
			if ( !rt.multiLine ) ui.height = ui.minHeight;
			go.fire( 'layout' );
		} ],

		// (Number) - Align.Left | Align.Center | Align.Right - text alignment
		[ 'align',  function (){ return rt.align; }, function ( w ){ rt.align = w; } ],

		// (Boolean) word wrapping at control width enabled for multiline field
		[ 'wrap',  function (){ return rt.wrap; }, function ( w ){ rt.wrap = w; } ],

		// (Number) extra spacing between characters
		[ 'characterSpacing',  function (){ return rt.characterSpacing; }, function ( v ){ rt.characterSpacing = v; } ],

		// (Boolean) multiple line input
		[ 'multiLine',  function (){ return rt.multiLine; }, function ( v ){ rt.multiLine = v; go.fire( 'layout' ); } ],

		// (Number) gets or sets number of visible lines in multiline control
		[ 'numLines',  function (){ return rt.height / rt.lineHeight; }, function ( v ) {
			if ( v != 1 ) rt.multiLine = true;
			ui.minHeight = ui.padTop + ui.padBottom + v * rt.lineHeight;
		}, true ],

		// (Boolean) auto grow / shrink vertically multiline field
		[ 'autoGrow',  function (){ return autoGrow; }, function ( v ){
			rt.multiLine = rt.multiLine || v;
			autoGrow = v;
			go.fire( 'layout' );
		} ],

		// (Number) multiLine line spacing
		[ 'lineSpacing',  function (){ return rt.lineSpacing; }, function ( v ){ rt.lineSpacing = v; } ],

		// (Number) returns line height - font size + line spacing
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],

		// (Boolean) font bold
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; } ],

		// (Boolean) font italic
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; } ],

		// (Boolean) enable display ^code formatting (while not editing)
		[ 'formatting',  function (){ return rt.formatting; }, function ( v ){ rt.formatting = v; } ],

		// (Number) or (Color) text color
		[ 'textColor',  function (){ return rt.textColor; }, function ( v ){ rt.textColor = v; } ],

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
	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

	// create components

	// text container
	tc = new GameObject();
	rt = new RenderText();
	tc.render = rt;
	tc.serialized = false;

	// UI
	ui.layoutType = Layout.Anchors;
	ui.minWidth = 10; ui.minHeight = 10;
	ui.focusable = false;
	go.ui = ui;

	// children are added after component is awake,
	// because component's children may be overwritten on unserialize
	go.awake = function () {
		go.addChild( tc );
	};

	// layout components
	ui.layout = function ( x, y, w, h ) {
		go.setTransform( x, y );
		ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
		if ( rt.multiLine ) {
			if ( autoGrow ) ui.minHeight = ui.height = h = rt.lineHeight * rt.numLines + ui.padTop + ui.padBottom;
		} else {
			h = ui.minHeight;
		}
		tc.setTransform( ui.padLeft, ui.padTop );
		rt.width = w - ( ui.padLeft + ui.padRight );
		rt.height = Math.max( rt.lineHeight, h - ( ui.padTop + ui.padBottom ) );
	}

	// scrolling
	ui.mouseWheel = function ( wy, wx ) {

		var st = rt.scrollTop, sl = rt.scrollLeft;

		// scroll vertically
		if ( wy != 0 && rt.scrollHeight > rt.height ) {
			rt.scrollTop =
			Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.scrollTop - wy ) );
		}
		// scroll horizontally
		if ( wx != 0 && rt.scrollWidth > rt.width ){
			rt.scrollLeft =
			Math.max( 0, Math.min( rt.scrollWidth - rt.width, rt.scrollLeft + wx ) );
		}

		// stop event if scrolled
		if ( sl != rt.scrollLeft || st != rt.scrollTop ) stopEvent();
	}

	// auto resize text box vertically with text
	go.checkAutoGrow = function () {
		if ( autoGrow && rt.multiLine ) {
			var h = ui.height;
			ui.height = ui.minHeight = rt.lineHeight * rt.numLines + ui.padTop + ui.padBottom;
			if ( h != ui.height ) go.async( go.scrollIntoView );
		}
	}

	// apply defaults
	if ( UI.style && UI.style.text ) for ( var p in UI.style.text ) go[ p ] = UI.style.text[ p ];

})(this);