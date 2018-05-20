/*
 
	Editable text input field

	Usage:

		var textField = App.scene.addChild( 'ui/input' );
		textField.text = "Some text";

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js

	Events:
		'change' - when field changes (as you type)
		'focusChanged' - when control focus is set or cleared (same as UI event)
		'editStart' - when control begin text edit
		'editEnd' - when control ended text edit
		'accept' - on blur, if contents changed
		'cancel' - on blur, if contents didn't change
		'copy' - if text is copied via Ctrl+C or Ctrl+X ( into UI.clipboard )
		'paste' - some text was pasted from UI.clipboard
		'selectionChanged' - when selection changes ( event parameter is selected text )
 
*/

include( './ui' );
(function( go ) {

	// inner props
	var ui = new UI(), tc, rt, bg, shp;
	var tabEnabled = false;
	var disabled = false;
	var selectable = true;
	var resetText = "";
	var formatting = false;
	var mouseDownTime = null;
	var selectAllOnFocus = false;
	var acceptToEdit = false;
	var cancelToBlur = false;
	var blurOnClickOutside = true;
	var editing = false;
	var scrolling = false;
	var touched = false;
	var background;
	var autoGrow = false;
	var numeric = false;
	var integer = false;
	var code = false;
	var newLinesRequireShift = true;
	var minValue = -Infinity;
	var maxValue = Infinity;
	var step = 1.0;
	var constructing = true;
	var canScrollUnfocused = false;
	var alwaysShowSelection = false;
	var allowed = null, pattern = null;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) input contents
		[ 'text',   function (){ return rt.text; }, function( t ){
			var pt = rt.text;
			var ps0 = rt.selectionStart, ps1 = rt.selectionEnd;
			rt.text = t;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
			if ( pt != t && !constructing ) go.fire( 'change', go.value );
			if ( ps0 != ps1 ) go.fire( 'selectionChanged' );
		} ],

		// (String) or (Number) depending on if .numeric is set
		[ 'value',   function (){
			var v = rt.text;
			if ( numeric ) {
				v = integer ? parseInt( rt.text ) : parseFloat( rt.text );
				if ( isNaN ( v ) && resetText ) v = ( integer ? parseInt( resetText ) : parseFloat( resetText ) );
				v = Math.max( minValue, Math.min( maxValue, v ) );
			}
			return v;
		}, function( v ){
			if ( numeric ) {
				if ( integer ) v = Math.round( v );
				v = Math.min( maxValue, Math.max( minValue, v ) );
				if ( !integer ) {
					// rounding
					v = Math.round( parseFloat( v.toFixed( 3 ) ) * 10000 ) * 0.0001;
					v = v.toFixed( 3 ).replace( /(0+)$/, '' ).replace( /\.$/, '' ); // delete trailing zeros, and .
				}
			}
			rt.text = v;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
		} ],

		// (Number) current width of the control
		[ 'width',  function (){ return ui.width; }, function ( w ){ ui.width = w; go.scrollCaretToView(); } ],

		// (Number) current height of the control
		[ 'height',  function (){ return ui.height; }, function ( h ){ ui.height = h; go.scrollCaretToView(); } ],

		// (Boolean) requires user to press Enter (or 'accept' controller button) to begin editing
		[ 'acceptToEdit',  function (){ return acceptToEdit; }, function ( ae ){
			acceptToEdit = ae;
			ui.focusable = !disabled || acceptToEdit;
		} ],

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		[ 'cancelToBlur',  function (){ return cancelToBlur; }, function ( cb ){ cancelToBlur = cb; } ],

		// (Boolean) clicking outside control will blur the control
		[ 'blurOnClickOutside',  function (){ return blurOnClickOutside; }, function ( cb ){ blurOnClickOutside = cb; } ],

		// (Boolean) show selection even when not editing
		[ 'alwaysShowSelection',  function (){ return alwaysShowSelection; }, function ( s ){ alwaysShowSelection = s; } ],

		// (Boolean) allows mousewheel to scroll field without being focused in it
		[ 'canScrollUnfocused',  function (){ return canScrollUnfocused; }, function ( u ){ canScrollUnfocused = u; } ],

		// (Boolean) in multiline input require shift|alt|ctrl|meta to make new lines with Enter key
		[ 'newLinesRequireShift',  function (){ return newLinesRequireShift; }, function ( u ){ newLinesRequireShift = u; } ],

		// (Number) current position of caret ( from 0 to text.positionLength() )
		[ 'caretPosition',  function (){ return rt.caretPosition; }, function ( p ){ rt.caretPosition = p; go.scrollCaretToView(); } ],

		// (Boolean) turns display of caret and selection on and off (used by focus system)
		[ 'editing',  function (){ return editing; }, function ( e ){
			if ( e != editing ) {
				editing = e;
				if ( e ) {
					// first time editing
					if ( !touched ) {
						// set caret to end
						rt.caretPosition = rt.text.positionLength();
						touched = true;
					}
					rt.showCaret = rt.showSelection = true;
				    rt.formatting = false;
					resetText = rt.text;
					if ( selectAllOnFocus && selectable ) {
						rt.selectionStart = 0; rt.selectionEnd = rt.text.positionLength();
				    }
					go.fire( 'editStart' );
				} else {
					rt.showCaret = ui.dragSelect = false;
				    rt.formatting = formatting;
				    rt.scrollLeft = 0;
					if ( code ) go.async( go.cancelAutocomplete, 0.5 );
					if ( numeric ) go.value = go.value;
					go.fire( rt.text == resetText ? 'cancel' : 'accept', rt.text );
					go.fire( 'editEnd', go.value );
					if ( !alwaysShowSelection ) rt.showSelection = false;
				}
			}
		} ],

		// (string) font name
		[ 'font',  function (){ return rt.font; }, function ( f ){ rt.font = f; go.scrollCaretToView(); } ],

		// (string) optional bold font name (improves rendering)
		[ 'boldFont',  function (){ return rt.boldFont; }, function ( f ){ rt.boldFont = f; go.scrollCaretToView(); } ],

		// (string) optional italic font name (improves rendering)
		[ 'italicFont',  function (){ return rt.italicFont; }, function ( f ){ rt.italicFont = f; go.scrollCaretToView(); } ],

		// (string) optional bold+italic font name (improves rendering)
		[ 'boldItalicFont',  function (){ return rt.boldItalicFont; }, function ( f ){ rt.boldItalicFont = f; go.scrollCaretToView(); } ],

		// (Number) font size
		[ 'size',  function (){ return rt.size; }, function ( s ){
			var nl = go.numLines;
			rt.size = s;
			ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
			if ( rt.multiLine ) {
				go.numLines = nl;
			} else {
				ui.height = ui.minHeight;
			}
		} ],

		// (Boolean) only accepts numbers
		[ 'numeric',  function (){ return numeric; }, function ( n ){
			numeric = n;
			if ( n ) {
				rt.multiLine = false;
				go.value = go.text; // update value
			}
		} ],

		// (Number) minimum value for numeric input
		[ 'min',  function (){ return minValue; }, function ( v ){ minValue = v; } ],

		// (Number) maximum value for numeric input
		[ 'max',  function (){ return maxValue; }, function ( v ){ maxValue = v; } ],

		// (Number) step by which to increment/decrement value, when using arrow keys in control
		[ 'step',  function (){ return step; }, function ( v ){ step = v; } ],

		// (Boolean) maximum value for numeric input
		[ 'integer',  function (){ return integer; }, function ( v ){ integer = v; } ],

		// (Object) | (Boolean) set to an Object to enable code editor mode
		// object will be used to resolve "this", when typing "this."
		// set to false to disable
		[ 'code',  function (){ return code; }, function ( v ){
			code = v;
		} ],

		// (RegExp) allow typing only these characters. Regular expression against which to compare incoming character, e.g. /[0-9a-z]/i
		[ 'allowed',  function (){ return allowed; }, function ( a ){ allowed = a; } ],

		// (RegExp) only allow the text that matches this RegExp e.g. /^[0-9]{0,4}$/
		[ 'pattern',  function (){ return pattern; }, function ( p ){ pattern = p; } ],

		// (Boolean) should text be antialiased
		[ 'antialias',  function (){ return rt.antialias; }, function ( a ){ rt.antialias = a; } ],

		// (Boolean) word wrapping at control width enabled for multiline field
		[ 'wrap',  function (){ return rt.wrap; }, function ( w ){ rt.wrap = w; go.scrollCaretToView(); } ],

		// (Number) extra spacing between characters
		[ 'characterSpacing',  function (){ return rt.characterSpacing; }, function ( v ){ rt.characterSpacing = v; go.scrollCaretToView(); } ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		[ 'background',  function (){ return background; }, function ( b ){
			background = b;
			// set look
			if ( b === null || b === false ) {
				go.render = null;
			} else if ( typeof( b ) == 'string' ) {
				bg.texture = b;
				go.render = bg;
			} else {
				shp.color = b;
				go.render = shp;
			}
			go.dispatch( 'layout' );
		} ],

		// (Number) corner roundness when background is solid color
		[ 'cornerRadius',  function (){ return shp.radius; }, function ( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		} ],

		// (Number) outline thickness when background is solid color
		[ 'lineThickness',  function (){ return shp.lineThickness; }, function ( b ){
			shp.lineThickness = b;
		} ],

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		[ 'outlineColor',  function (){ return shp.outlineColor; }, function ( c ){
			shp.outlineColor = (c === false ? '00000000' : c );
		} ],

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		[ 'filled',  function (){ return shp.filled; }, function ( v ){ shp.filled = v; } ],

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

		// (Boolean) multiple line input
		[ 'multiLine',  function (){ return rt.multiLine; }, function ( v ){
			rt.multiLine = v;
			go.dispatch( 'layout' );
			go.scrollCaretToView(); } ],

		// (Number) gets or sets number of visible lines in multiline control
		[ 'numLines',  function (){ return rt.height / rt.lineHeight; }, function ( v ) {
			if ( v != 1 ) rt.multiLine = true;
			ui.minHeight = ui.padTop + ui.padBottom + v * rt.lineHeight;
			go.scrollCaretToView();
		}, true ],

		// (Number) scrollable height
		[ 'scrollHeight',  function (){ return rt.scrollHeight; } ],

		// (Number) scrollable width
		[ 'scrollWidth',  function (){ return rt.scrollHeight; } ],

		// (Number) vertical scroll offset
		[ 'scrollTop',  function (){ return rt.scrollTop; }, function ( v ){ rt.scrollTop = v; }, true ],

		// (Number) horizontal scroll offset
		[ 'scrollLeft',  function (){ return rt.scrollLeft; }, function ( v ){ rt.scrollLeft = v; }, true ],

		// (Number) scrollable width
		[ 'scrollWidth',  function (){ return rt.scrollHeight; } ],

		// (Boolean) auto grow / shrink vertically multiline field
		[ 'autoGrow',  function (){ return autoGrow; }, function ( v ){
			rt.multiLine = rt.multiLine || v;
			autoGrow = v;
			go.dispatch( 'layout' );
			go.scrollCaretToView();
		} ],

		// (Number) multiLine line spacing
		[ 'lineSpacing',  function (){ return rt.lineSpacing; }, function ( v ){
			rt.lineSpacing = v;
			go.dispatch( 'layout' );
			go.scrollCaretToView();
		} ],

		// (Number) returns line height - font size + line spacing
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],

		// (Boolean) font bold
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; } ],

		// (Boolean) font italic
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; } ],

		// (Boolean) allow text selection
		[ 'selectable',  function (){ return selectable; }, function ( v ){ selectable = v; rt.showSelection = v; if ( !v ) rt.selectionStart = rt.selectionEnd; } ],

		// (Boolean) select all text when control is first focused
		[ 'selectAllOnFocus',  function (){ return selectAllOnFocus; }, function ( b ){ selectAllOnFocus = b; } ],

		// (Boolean) allow typing in Tab character (as opposed to tabbing to another UI control)
		[ 'tabEnabled',  function (){ return tabEnabled; }, function ( v ){ tabEnabled = v; } ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			 disabled = v;
			 // ui.disabled = v || acceptToEdit; //?
			 if ( v && editing ) go.editing = false;
			 go.state = 'auto';
			 go.dispatch( 'layout' );
		 } ],

		// (Boolean) input is currently in scrolling mode
		[ 'scrolling',  function (){ return scrolling; },
		 function ( v ){
			 scrolling = v;
			 go.state = 'scrolling';
		 } ],

		// (Boolean) enable display ^code formatting (while not editing)
		[ 'formatting',  function (){ return formatting; }, function ( v ){ formatting = v; rt.formatting = ( formatting && !ui.focused ); } ],

		// (Number) or (Color) text color
		[ 'color',  function (){ return rt.textColor; }, function ( v ){ rt.textColor = v; } ],

		// (Number) or (Color) text selection color
		[ 'selectionColor',  function (){ return rt.selectionColor; }, function ( v ){ rt.selectionColor = v; } ],

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
	UI.base.addMappedProperties( go, mappedProps );

	// API functions

	// set input focus to the control. forceEdit param = true, will begin editing even if acceptToEdit = true
	go[ 'focus' ] = function ( forceEdit ) { ui.focus(); if ( forceEdit && !disabled ) go.editing = true; }

	// remove input focus from control
	go[ 'blur' ] = function () { ui.blur(); }

	go[ 'scrollToBottom' ] = function () { rt.scrollLeft = 0; rt.scrollTop = (rt.scrollHeight > rt.height ? (rt.scrollHeight - rt.height) : 0); }

	// create components

	// set name
	if ( !go.name ) go.name = "Textfield";

	// background
	bg = new RenderSprite();
	shp = new RenderShape( Shape.Rectangle );
	shp.radius = 0; shp.centered = false;
	shp.filled = true;
	go.render = bg;

	// text container
	tc = new GameObject();
	rt = new RenderText();
	rt.autoResize = false;
	rt.showSelection = true;
	tc.render = rt;
	tc.serialized = false;

	// UI
	ui.autoMoveFocus = false;
	ui.layoutType = Layout.Anchors;
	ui.minWidth = 10; ui.minHeight = 10;
	ui.focusable = true;
	go.ui = ui;

	// children are added after component is awake,
	// because component's children may be overwritten on unserialize
	go.awake = function () {
		go.addChild( tc );
	};

	// layout components
	ui.layout = function ( w, h ) {
		if ( autoGrow ) {
			ui.minHeight = Math.min( ui.maxHeight ? ui.maxHeight : 999999, rt.lineHeight * rt.numLines + ui.padTop + ui.padBottom );
		} else {
			ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
		}
		bg.resize( w, h );
		shp.resize( w, h );
		tc.setTransform( ui.padLeft, ui.padTop );
		rt.resize( w - ( ui.padLeft + ui.padRight ), Math.max( rt.lineHeight, h - ( ui.padTop + ui.padBottom ) ) );
		go.scrollCaretToView();
	}

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// focused
	    if ( newFocus == ui ) {
		    ui.autoMoveFocus = false;
	        if ( !disabled ) {
		        go.editing = !acceptToEdit;
		        go.state = 'focus';
	        }
		    go.scrollIntoView();
		    Input.on( 'mouseDown', go.checkClickOutside );
	    // blurred
	    } else {
		    ui.autoMoveFocus = acceptToEdit;
	        go.editing = false;
		    go.scrolling = false;
		    go.state = 'auto';
		    Input.off( 'mouseDown', go.checkClickOutside );
	    }
		go.fire( 'focusChanged', newFocus );
	}

	// navigation event
	ui.navigation = function ( name, value ) {

		stopAllEvents();

		// editing
		if ( editing ) {

			if ( name == 'accept' && !rt.multiLine ) go.editing = false;
			else if ( name == 'cancel' ) {
				// blur, or stop editing
				if ( cancelToBlur ) ui.blur();
				else go.editing = false;
			}

		// reading
		} else if ( scrolling ) {

			if ( name == 'cancel' || name == 'accept' ) {

				// turn off scrolling mode
				go.scrolling = false;

			} else {

				// scroll
				if ( name == 'vertical' && rt.scrollHeight > rt.height ) {
					rt.scrollTop =
					Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.scrollTop + value * rt.lineHeight ) );
				}
				// scroll horizontally
				if ( name == 'horizontal' && rt.scrollWidth > rt.width ){
					rt.scrollLeft =
					Math.max( 0, Math.min( rt.scrollWidth - rt.width, rt.scrollLeft + value * rt.lineHeight  ) );
				}

			}


		// focused but not editing
		} else {

			// enter = begin editing, or scrolling
			if ( name == 'accept' ) {

				if ( acceptToEdit && !disabled ) go.editing = true;
				else if ( disabled && (rt.scrollHeight > rt.height || rt.scrollWidth > rt.width) ) go.scrolling = true;

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
	}

	// scrolling
	ui.mouseWheel = function ( wy, wx ) {

		// if numeric and focused
		if ( numeric && wy && ui.focused && !disabled ){
			var val = wy < 0 ? -step : step;
			go.value += (Input.get( Key.LeftShift ) ? 10 : 1 ) * val;
			go.fire( 'change', go.value );
			rt.selectionStart = 0; rt.caretPosition = rt.selectionEnd = rt.text.positionLength(); // select all
			stopEvent();
			return;
		}

		var canScroll = (rt.scrollHeight > rt.height || rt.scrollWidth > rt.width);

		// ignore if not focused
		if ( !canScroll || ( !canScrollUnfocused && !ui.focused ) ) return;

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

	// helper for selection
	function findWordBoundaryIndex( txt, startIndex, direction ) {
		// new lines act as hard boundaries
		if (( direction > 0 && txt.substr( direction, 1 ) == "\n" ) ||
			( direction < 0 && startIndex > 0 && txt.substr( startIndex - 1, 1 ) == "\n" ) ) return startIndex + direction;

		// find either next, or previous word boundary
		var i = startIndex + direction;
		if ( direction < 0 ) i--;
		var rx = /[\s.@'"\^=]/; // characters that function as "stops" or word boundary
		while( i > 0 && i < txt.length ) {
			if ( txt.substr( i, 1 ).match( rx ) ) break;
			i += direction;
		}
		if ( i != 0 && direction < 0 ) i++;
		return i;
	}

	// helper for code - searches backwards to beginning of expression that can be evaluated for
	// autocomplete
	function findExpression( startIndex ) {
		// word boundary
		var i = startIndex;
		var rx = /[^a-z0-9_$.]/i; // characters that can be part of lookupable expression
		var txt = rt.text;
		while( i >= 0 ) {
			if ( txt.substr( i, 1 ).match( rx ) ) break;
			i--;
		}
		i++;
		var expr = txt.substr( i, startIndex - i + 1 );
		if ( expr.match( /^[.0-9]/ ) || expr.length < 2 ) return ''; // ignore invalid
		return expr;
	}

	//
	function autocomplete( accept ) {

		var expr = findExpression( rt.caretPosition );
		var exprLen = expr.length;
		var lastPeriod = -1;
		var suggestions = [];
		if ( accept ) {

			// accept selected item in popup
			lastPeriod = expr.lastIndexOf( '.' );
			if ( go.popup && go.popup.selectedIndex >= 0 )
			suggestions = [ go.popup.items[ go.popup.selectedIndex ] ];

		} else {

			// ends with . - show all available properties
			if ( expr.substr( -1 ) == '.' ) {
				lastPeriod = expr.length - 1;
				var obj = eval( expr.substr( 0, exprLen - 1 ) );
				if ( typeof( obj ) !== 'undefined' && obj !== null && !( typeof( obj ) === 'object' && obj.constructor.name.indexOf( 'Error' ) >= 0 ) ) {
					// add all properties
					for ( var p in obj ) suggestions.push( p );
				}
				// object, ends with partially completed property name - suggest matches
			} else if ( ( lastPeriod = expr.lastIndexOf( '.' ) ) > 0 ) {
				var obj = eval( expr.substr( 0, lastPeriod ) );
				if ( typeof( obj ) !== 'undefined' && obj !== null && !( typeof( obj ) === 'object' && obj.constructor.name.indexOf( 'Error' ) >= 0 ) ) {
					// all matching beginning of property name
					var prop = expr.substr( lastPeriod + 1 );
					var propLen = prop.length;
					for ( var p in obj ) {
						if ( p.substr( 0, propLen ) === prop ) suggestions.push( p );
					}
				}
				// doesn't have . in it yet - predict object from global
			} else if ( exprLen ) {
				// all matching beginning of line
				for ( var p in global ) {
					if ( p.substr( 0, exprLen ) === expr ) suggestions.push( p );
				}
			}
		}

		// multiple - show popup
		if ( suggestions.length > 1 ) {
			// new one
			if ( !go.popup ) {
				go.popup = new GameObject( './popup-menu', {
					preferredDirection: 'up',
					noFocus: true,
					selected: function( item ) {
						autocomplete( true );
					}
				} );
			}
			// update
			var items = suggestions;
			// only update if items changed
			var ni = go.popup.items.length;
			if ( ni != items.length ) {
				go.popup.items = items;
			} else {
				for ( var i = 0; i < ni; i++ ) {
					if ( go.popup.items[ i ] != items[ i ] ) {
						go.popup.items = items;
						break;
					}
				}
			}
			// update position
			var gp = go.localToGlobal( rt.caretX, rt.caretY );
			go.popup.setTransform( gp.x, gp.y );

		// single suggestion - append selected text
		} else if ( suggestions.length == 1 ) {
			var sugg = suggestions[ 0 ];
			if ( lastPeriod > 0 ) {
				sugg = sugg.substr( exprLen - lastPeriod - 1 );
			} else {
				sugg = sugg.substr( exprLen );
			}
			rt.text = rt.text.substr( 0, rt.caretPosition ) + sugg + rt.text.substr( rt.caretPosition );
			if ( go.popup ) { go.popup.parent = null; go.popup = null; }
			if ( accept ) {
				rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPosition + sugg.length;
				go.focus(); go.editing = true; // keep editing
			} else {
				rt.selectionEnd = ( rt.selectionStart = rt.caretPosition ) + sugg.length;
			}
		// no matches, remove popup
		} else {
			if ( go.popup ) { go.popup.parent = null; go.popup = null; }
		}
	}

	go.cancelAutocomplete = function() {
		go.cancelDebouncer( 'autocomplete' );
		if ( go.popup ) { go.popup.parent = null; go.popup = null; }
	}

	// mouse down
	ui.mouseDown = function ( btn, x, y ) {

		if ( btn != 1 ) return;

		// offset by padding
		x -= ui.padLeft; y -= ui.padTop;

		// shift + down
		if ( ( Input.get( Key.LeftShift ) || Input.get( Key.RightShift ) ) && selectable ) {
			rt.selectionStart = Math.min( rt.selectionStart, rt.caretPosition );
			rt.selectionEnd = rt.caretPositionAt( x, y );
        } else {
			// set cursor
		    rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPositionAt( x, y );
		}

		// mouse always starts editing
		go.focus();
		if ( !disabled ) go.editing = true;

		// double clicked word
		var newMouseDownTime = new Date();
		if ( mouseDownTime && ( newMouseDownTime.getTime() - mouseDownTime.getTime() < 300 ) ) {
			var pos = rt.text.positionToIndex( rt.caretPosition );
			var left = findWordBoundaryIndex( rt.text, pos, -1 );
			var right = findWordBoundaryIndex( rt.text, pos, 1 );
			if ( left != right ){
				rt.selectionStart = rt.text.indexToPosition( left );
				rt.caretPosition = rt.selectionEnd = rt.text.indexToPosition( right );
			}
			mouseDownTime = null;
			return;
		}
		mouseDownTime = newMouseDownTime;

		// start selection drag
		if ( selectable ) {
			ui.dragSelect = true;
			Input.on( 'mouseMove', ui.mouseMoveGlobal );
		}

		stopAllEvents();
	}

	// mouse release
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y ) {
		if ( btn != 1 ) return;
		ui.dragSelect = null;
		Input.off( 'mouseMove', ui.mouseMoveGlobal );
		//stopEvent();
	};

	// mouse move while holding down
	ui.mouseMoveGlobal = function ( x, y ) {
		// dragging selection
		if ( ui.dragSelect ) {
			x -= ui.padLeft; y -= ui.padTop;
			var p = go.globalToLocal( x, y );
			var selEnd = rt.caretPositionAt( p.x, p.y );
			if ( selEnd >= 0 ) {
				rt.caretPosition = rt.selectionEnd = selEnd;
			}
		}
	}

	// key press
	ui.keyPress = function ( key, direction ) {

		// if not editing
		if ( !editing ) {
			// if it's a character, start editing
			if ( !disabled && typeof( key ) == 'string' ) go.editing = true;
			else return;
		}

		// get ready
		var caretPosition = rt.caretPosition;
	    var caretIndex = rt.text.positionToIndex( caretPosition );
		var replacingSelection = ( rt.selectionStart != rt.selectionEnd );
		if ( replacingSelection && rt.selectionStart > rt.selectionEnd ) {
			var tmp = rt.selectionStart;
			rt.selectionStart = rt.selectionEnd;
			rt.selectionEnd = tmp;
		}
		var selStartIndex = rt.text.positionToIndex( rt.selectionStart );
		var selEndIndex = rt.text.positionToIndex( rt.selectionEnd );
	    var txt = rt.text;

		// delete character
	    if ( key == -1 ) {

			// delete selection
			if ( replacingSelection ) {
				txt = rt.text.substr( 0, selStartIndex ) + rt.text.substr( selEndIndex );
				if ( pattern && !txt.match( pattern ) ) return;
				rt.text = txt;
				caretPosition = rt.selectionStart;
			}

			// backspace
			else if ( direction < 0 ) {
				if ( caretPosition <= 0 || caretIndex <= 0 ) return;
				var prevCaretIndex = rt.text.positionToIndex( caretPosition - 1 );
				txt = rt.text.substr( 0, prevCaretIndex ) + rt.text.substr( caretIndex );
				if ( pattern && !txt.match( pattern ) ) return;
				rt.text = txt;
				caretPosition--;
			}

		// normal character
	    } else {
		    // filters
		    if ( allowed ) {
			    var match = key.match( allowed );
			    if ( !match || !match.length ) return;
		    }
		    if ( numeric ) {
			    if ( integer && !key.match( /\d/ ) ) return;
			    else if ( !key.match( /[0-9.\-]/ ) ) return;
				// test new value
				if ( replacingSelection ) {
					txt = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
				} else {
					txt = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
				}
			    if ( txt == '.' || txt == '-' ) txt += '0';
			    if ( (integer && isNaN( parseInt( txt ) ) ) || isNaN( parseFloat( txt ) ) ) return;
			    if ( pattern && !txt.match( pattern ) ) return;

		    }

			// selection
			if ( replacingSelection ) {
				txt = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
				caretPosition = rt.selectionStart + key.positionLength();
			} else {
				txt = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
				caretPosition += key.positionLength();
			}
			if ( pattern && !txt.match( pattern ) ) return;
			rt.text = txt;
	    }

		// update caret position
	    rt.caretPosition = caretPosition;
		rt.selectionEnd = rt.selectionStart;

		// autogrow
		if ( autoGrow ) go.debounce( 'autoGrow', go.checkAutoGrow );

		// autocomplete
		if ( code && typeof( key ) === 'string' ) {
			go.debounce( 'autocomplete', autocomplete, 1 );
		}

		// make sure caret is in view
		go.scrollCaretToView();

		// change
		go.fire( 'change', go.value );

	}

	// key down
	ui.keyDown = function ( key, shift, ctrl, alt, meta ) {

	    // ready
		ui.dragSelect = false;
		var txt = rt.text;
		var caretPosition = rt.caretPosition;
		var haveSelection = ( rt.selectionStart != rt.selectionEnd );
		var selStart = rt.selectionStart, selEnd = rt.selectionEnd;
		if ( haveSelection && rt.selectionStart > rt.selectionEnd ) {
			var tmp = rt.selectionStart;
			rt.selectionStart = rt.selectionEnd;
			rt.selectionEnd = tmp;
		}

		// autogrow
		if ( autoGrow ) go.debounce( 'autoGrow', go.checkAutoGrow );

		// key
	    switch ( key ) {
		    case Key.Tab:
			    // complete word
			    if ( go.popup ) {
			        autocomplete( true );
			    } else if ( selectable && rt.selectionStart == rt.caretPosition && rt.selectionStart < rt.selectionEnd ) {
				    rt.caretPosition = rt.selectionEnd;
				    rt.selectionStart = rt.selectionEnd = 0;
				    go.cancelAutocomplete();
			    // tab character
	            } else if ( tabEnabled ) {
			        ui.keyPress( "\t" );
			    // move focus
	            } else {
			        ui.moveFocus( shift ? -1 : 1 );
			    }
				break;

	        case Key.Return:
		        if ( editing ) {
			        if ( go.popup ) {
				        autocomplete( true );
				        return;
			        } else if ( rt.multiLine && ( !newLinesRequireShift || ( shift || alt || ctrl || meta ) ) ) {
				        // TODO - if code, count tabs at beginning of this line and add them to here
				        // TODO - maybe add .lineAtCaret property to RenderText?

				        ui.keyPress( "\n" ); // newline in multiline box
				        return;
			        } else {
				        // text changed?
				        if ( txt != resetText ) go.fire( 'change', go.value );
				        go.editing = false;
			        }
		        } else {
			        ui.navigation( 'accept' );
		        }
		        stopEvent();
	            break;

		    case Key.Escape:
			    if ( editing ) {
				    if ( code ) {
					    if ( go.popup ) {
						    go.cancelAutocomplete();
						    stopAllEvents();
						    go.editing = true;
						    return;
					    } else {
						    go.cancelDebouncer( 'autocomplete' );
					    }
				    }
				    // if single line text field, reset text
				    if ( !rt.multiLine ) {
					    rt.text = resetText;
					    rt.caretPosition = resetText.positionLength();
				    }
			    }
			    ui.navigation( 'cancel' );
	            break;

	        case Key.Backspace:
	            if ( editing ) ui.keyPress( -1, -1 );
			    if ( code ) go.cancelAutocomplete();
	            break;

	        case Key.Delete:
	            if ( editing ) ui.keyPress( -1, 1 );
			    if ( code ) go.cancelAutocomplete();
	            break;

	        case Key.Left:
			case Key.Right:
				var increment = ( key == Key.Left ? -1 : 1 );
				// skip word?
				if ( ctrl || alt || meta ) {
					// find either next, or previous word
					increment = txt.indexToPosition( findWordBoundaryIndex( txt, txt.positionToIndex( caretPosition ), increment ) ) - caretPosition;
				}
				// expand selection
				if ( shift && selectable ) {
					if ( key == Key.Left && caretPosition > 0 ) {
						if ( !haveSelection ) rt.selectionEnd = caretPosition;
						rt.caretPosition = rt.selectionStart = caretPosition + increment;
					} else if ( key == Key.Right && caretPosition < txt.positionLength() ){
						if ( !haveSelection ) rt.selectionStart = caretPosition;
						rt.selectionEnd = ( rt.caretPosition += increment );
					}
				// move caret
				} else if ( editing ) {
					if ( haveSelection ) {
						rt.caretPosition = ( key == Key.Left ? rt.selectionStart : rt.selectionEnd );
					} else rt.caretPosition += increment;
					rt.selectionStart = rt.selectionEnd;
				}
				go.scrollCaretToView();
			    if ( code ) go.cancelAutocomplete();
	            break;

			case Key.Home:
			case Key.End:
				// expand selection
				if ( shift && selectable ) {
					if ( key == Key.Home && caretPosition > 0 ) {
						if ( !haveSelection ) rt.selectionEnd = caretPosition;
						rt.caretPosition = rt.selectionStart = rt.caretPositionAt ( -9999, rt.caretY );
					} else if ( key == Key.End && caretPosition < txt.positionLength() ){
						if ( !haveSelection ) rt.selectionStart = caretPosition;
						rt.selectionEnd = rt.caretPositionAt ( 9999, rt.caretY );
					}
				// move caret
				} else if ( editing ) {
					if ( !haveSelection ) rt.caretPosition = rt.caretPositionAt ( ( key == Key.Home ? -9999 : 9999 ), rt.caretY );
					rt.selectionStart = rt.selectionEnd;
				}
				go.scrollCaretToView();
			    if ( code ) go.cancelAutocomplete();
	            break;

			case Key.Up:
			case Key.Down:
				if ( !editing ) break;

				// navigate autocomplete popup
				if ( code && go.popup ) {
					if ( key == Key.Up ) {
						if ( go.popup.selectedIndex <= 0 ) go.popup.selectedIndex = go.popup.items.length - 1;
						else go.popup.selectedIndex--;
					} else {
						if ( go.popup.selectedIndex == -1 || go.popup.selectedIndex == go.popup.items.length - 1 ) go.popup.selectedIndex = 0;
						else go.popup.selectedIndex++;
					}
					stopEvent();
					break;
				}

				// numeric
				if ( numeric ) {
					go.value += ( key == Key.Up ? step : -step ) * ( shift ? 10 : 1 );
					rt.selectionStart = 0; rt.caretPosition = rt.selectionEnd = rt.text.positionLength(); // select all
					go.fire( 'change', go.value );
					return;
				}

				// starting selection
				if ( shift && selectable ) {
					if ( !haveSelection ) rt.selectionEnd = rt.selectionStart = caretPosition;
				// move caret - clear selection
				} else {
					rt.selectionStart = rt.selectionEnd;
				}
				// common
				if ( rt.caretLine == 0 && key == Key.Up ) {
					rt.caretPosition = 0;
				} else if ( rt.caretLine >= rt.numLines - 1 && key == Key.Down ) {
					rt.caretPosition = txt.positionLength();
				} else {
					rt.caretPosition = rt.caretPositionAt ( rt.caretX, rt.caretY + ( key == Key.Up ? -1 : 1 ) * rt.lineHeight );
				}
				// selecting
				if ( shift && selectable ) {
					if ( key == Key.Up ) rt.selectionStart = rt.caretPosition;
					else rt.selectionEnd = rt.caretPosition;
				}
				go.scrollCaretToView();
			    if ( code ) go.cancelAutocomplete();
				break;

			case Key.A:
			    // select all
				if ( selectable && ( meta || ctrl ) ) {
					rt.selectionStart = 0;
					rt.caretPosition = rt.selectionEnd = rt.text.positionLength();
				    if ( code ) go.cancelAutocomplete();
				}
                break;

            case Key.C:
		    case Key.X:
			    // copy or cut
				if ( ( meta || ctrl ) && rt.selectionStart != rt.selectionEnd && selectable ) {
					var ss = txt.positionToIndex( rt.selectionStart );
					var se = txt.positionToIndex( rt.selectionEnd );
					UI.clipboard = txt.substr( ss, se - ss );
					if ( key == Key.X ) ui.keyPress( -1, 1 );
					go.fire( 'copy', UI.clipboard );
				    if ( code ) go.cancelAutocomplete();
				}
                break;

		    case Key.V:
			    if ( typeof( UI.clipboard ) === 'string' ){
				    if ( code ) go.cancelAutocomplete();
					ui.keyPress( UI.clipboard );
				    go.fire( 'paste', UI.clipboard );
			    }
			    break;
	    }

		if ( selStart != rt.selectionStart || selEnd != rt.selectionEnd ) {
			txt = rt.text;
			selStart = txt.positionToIndex( rt.selectionStart );
			selEnd = txt.positionToIndex( rt.selectionEnd );
			go.fire( 'selectionChanged', txt.substr( selStart, selEnd - selStart ) );
		}

		// redispatch keydown on object
		go.fire( 'keyDown', key, shift, ctrl, alt, meta );

		// update position
		if ( go.popup ) {
			var gp = go.localToGlobal( rt.caretX, rt.caretY );
			go.popup.setTransform( gp.x, gp.y );
		}
	}

	// checks if clicked outside
	go.checkClickOutside = function ( btn, x, y ) {
		if ( blurOnClickOutside && ui.focused && !go.popup ) {
			var lp = go.globalToLocal( x, y );
			if ( lp.x < 0 || lp.x > go.width || lp.y < 0 || lp.y > go.height ) {
				go.blur();
			}
		}
	}

	// auto resize text box vertically with text
	go.checkAutoGrow = function () {
		if ( autoGrow && rt.multiLine ) {
			var h = ui.height;
			ui.height = ui.minHeight = rt.lineHeight * rt.numLines + ui.padTop + ui.padBottom;
			if ( h != ui.height ) go.async( go.scrollIntoView );
		}
	}

	// makes sure caret is inside visible area
	go.scrollCaretToView = function (){

		// only when focused
		if ( !ui.focused || !editing) return;

		// vertical
		if ( rt.caretY < 0 ) rt.scrollTop += rt.caretY;
		else if ( rt.caretY > rt.height - rt.lineHeight ) {
			rt.scrollTop = Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.caretLine * rt.lineHeight ) );
		}
		// horizontal
		if ( rt.caretX < 0 ) {
			rt.scrollLeft += rt.caretX;
		} else if ( rt.caretX >= rt.width ){
			rt.scrollLeft += rt.caretX - rt.width + 8;
		}
		// truncate scroll pos
		rt.scrollLeft = Math.max( 0, Math.min( rt.scrollWidth - rt.width + 8, rt.scrollLeft ));
		rt.scrollTop = Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.scrollTop ));
		// if autogrow, scroll box into view
		go.scrollIntoView();
	}

	// called from "focusChanged" to scroll this component into view
	go._scrollIntoView = go.scrollIntoView;
	go[ 'scrollIntoView' ] = function() {
		if ( editing ) {
			// scroll to caret, if editing
			go._scrollIntoView( rt.caretX, rt.caretY ); // ,1 + ui.padLeft + ui.padRight, rt.lineHeight + ui.padTop + ui.padBottom );
		} else {
			go._scrollIntoView();
		}
	}

	// apply defaults
	go.baseStyle = Object.create( UI.style.textfield );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;

})(this);






