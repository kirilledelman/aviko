/*
 
	Editable text input field

	Usage:

		var textField = App.scene.addChild( 'ui/input' );
		textField.text = "Some text";

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js

	Events:
		'change' - when field changes (as you type)
		'editStart' - when control begin text edit
		'editEnd' - when control ended text edit
		'accept' - on blur, if contents changed
		'cancel' - on blur, if contents didn't change
		'copy' - if text is copied via Ctrl+C or Ctrl+X ( into App.clipboard )
		'paste' - some text was pasted from UI.copiedText
		'selectionChanged' - when selection changes ( event parameter is selected text )
 
*/

include( './ui' );
(function( go ) {

	// inner props
	var ui = new UI(), tc, rt, bg, shp;
	var tabEnabled = false;
	var disabled = false, disabledCanFocus = true;
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
	var autocomplete = null, autocompleteParam = undefined;
	var autocompleteReplaceStart = -1;
	var newLinesRequireShift = true;
	var minValue = -Infinity;
	var maxValue = Infinity;
	var step = 1.0;
	var constructing = true;
	var canScrollUnfocused = false;
	var alwaysShowSelection = false;
	var allowed = null, pattern = null;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) input contents
		'text': { get: function (){ return rt.text; }, set: function( t ){
			var pt = rt.text;
			var ps0 = rt.selectionStart, ps1 = rt.selectionEnd;
			rt.text = t;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
			if ( pt != t && !constructing ) go.fire( 'change', go.value );
			if ( ps0 != ps1 ) go.fire( 'selectionChanged' );
		}  },

		// (String) or (Number) depending on if .numeric is set
		'value': {
			get: function (){
				var v = rt.text;
				if ( numeric ) {
					v = integer ? parseInt( rt.text ) : parseFloat( rt.text );
					if ( isNaN ( v ) && resetText ) v = ( integer ? parseInt( resetText ) : parseFloat( resetText ) );
					v = Math.max( minValue, Math.min( maxValue, v ) );
				}
				return v;
			},
			set: function( v ){
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
			}
		},

		// (Number) current width of the control
		'width': { get: function (){ return ui.width; }, set: function( w ){ ui.width = w; go.scrollCaretToView(); }  },

		// (Number) current height of the control
		'height': { get: function (){ return ui.height; }, set: function( h ){ ui.height = h; go.scrollCaretToView(); }  },

		// (Boolean) requires user to press Enter (or 'accept' controller button) to begin editing
		'acceptToEdit': { get: function (){ return acceptToEdit; }, set: function( ae ){
			acceptToEdit = ae;
			ui.focusable = !disabled || acceptToEdit;
		}  },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		'cancelToBlur': { get: function (){ return cancelToBlur; }, set: function( cb ){ cancelToBlur = cb; }  },

		// (Boolean) clicking outside control will blur the control
		'blurOnClickOutside': { get: function (){ return blurOnClickOutside; }, set: function( cb ){ blurOnClickOutside = cb; }  },

		// (Boolean) show selection even when not editing
		'alwaysShowSelection': { get: function (){ return alwaysShowSelection; }, set: function( s ){ alwaysShowSelection = s; }  },

		// (Boolean) allows mousewheel to scroll field without being focused in it
		'canScrollUnfocused': { get: function (){ return canScrollUnfocused; }, set: function( u ){ canScrollUnfocused = u; }  },

		// (Boolean) in multiline input require shift|alt|ctrl|meta to make new lines with Enter key
		'newLinesRequireShift': { get: function (){ return newLinesRequireShift; }, set: function( u ){ newLinesRequireShift = u; }  },

		// (Number) current position of caret ( from 0 to text.positionLength() )
		'caretPosition': { get: function (){ return rt.caretPosition; }, set: function( p ){ rt.caretPosition = p; go.scrollCaretToView(); }  },

		// (Boolean) turns display of caret and selection on and off (used by focus system)
		'editing': { get: function (){ return editing; }, set: function( e ){
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
					// go.async( cancelAutocomplete, 0.25 );
					cancelAutocomplete();
					if ( numeric ) go.value = go.value;
					go.fire( rt.text == resetText ? 'cancel' : 'accept', rt.text );
					go.fire( 'editEnd', go.value );
					if ( !alwaysShowSelection ) rt.showSelection = false;
				}
			}
		}  },

		// (string) font name
		'font': { get: function (){ return rt.font; }, set: function( f ){ rt.font = f; go.scrollCaretToView(); }  },

		// (string) optional bold font name (improves rendering)
		'boldFont': { get: function (){ return rt.boldFont; }, set: function( f ){ rt.boldFont = f; go.scrollCaretToView(); }  },

		// (string) optional italic font name (improves rendering)
		'italicFont': { get: function (){ return rt.italicFont; }, set: function( f ){ rt.italicFont = f; go.scrollCaretToView(); }  },

		// (string) optional bold+italic font name (improves rendering)
		'boldItalicFont': { get: function (){ return rt.boldItalicFont; }, set: function( f ){ rt.boldItalicFont = f; go.scrollCaretToView(); }  },

		// (Number) font size
		'size': { get: function (){ return rt.size; }, set: function( s ){
			var nl = go.numLines;
			rt.size = s;
			ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
			if ( rt.multiLine ) {
				go.numLines = nl;
			} else {
				ui.height = ui.minHeight;
			}
		}  },

		// (Boolean) only accepts numbers
		'numeric': { get: function (){ return numeric; }, set: function( n ){
			numeric = n;
			if ( n ) {
				rt.multiLine = false;
				go.value = go.text; // update value
			}
		}  },

		// (Number) minimum value for numeric input
		'min': { get: function (){ return minValue; }, set: function( v ){ minValue = v; }  },

		// (Number) maximum value for numeric input
		'max': { get: function (){ return maxValue; }, set: function( v ){ maxValue = v; }  },

		// (Number) step by which to increment/decrement value, when using arrow keys in control
		'step': { get: function (){ return step; }, set: function( v ){ step = v; }  },

		// (Boolean) maximum value for numeric input
		'integer': { get: function (){ return integer; }, set: function( v ){ integer = v; }  },

		// (Function) - callback function that returns an object with two properties:
		// {
		//      suggestions: Array of suggestions to display in a popup, or null to hide popup, e.g. [ { text: "suggestion 1", value: "replace value" } ... ]
		//      replaceStart: position in current field text, from which an accepted suggestion will replace text up to current caret
		// }
		// (String) - alternatively, name of the function in UI.base namespace ( e.g. "autocompleteObjectProperty" for UI.base.autocompleteObjectProperty )
		//  see UI.base.autocompleteObjectProperty for example
		'autocomplete': { get: function (){ return autocomplete; }, set: function( v ){ autocomplete = v; } },

		// (*) an extra parameter used by autocomplete function.
		// object property lookup uses this a "this" target
		// filename autocomplete uses this as an array for allowed file extensions
		'autocompleteParam': { get: function (){ return autocompleteParam; }, set: function( v ){ autocompleteParam = v; } },

		// (RegExp) allow typing only these characters. Regular expression against which to compare incoming character, e.g. /[0-9a-z]/i
		'allowed': { get: function (){ return allowed; }, set: function( a ){ allowed = a; }  },

		// (RegExp) only allow the text that matches this RegExp e.g. /^[0-9]{0,4}$/
		'pattern': { get: function (){ return pattern; }, set: function( p ){ pattern = p; }  },

		// (Boolean) should text be antialiased
		'antialias': { get: function (){ return rt.antialias; }, set: function( a ){ rt.antialias = a; }  },

		// (Boolean) word wrapping at control width enabled for multiline field
		'wrap': { get: function (){ return rt.wrap; }, set: function( w ){ rt.wrap = w; go.scrollCaretToView(); }  },

		// (Number) extra spacing between characters
		'characterSpacing': { get: function (){ return rt.characterSpacing; }, set: function( v ){ rt.characterSpacing = v; go.scrollCaretToView(); }  },

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		'background': { get: function (){ return background; }, set: function( b ){
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
			go.requestLayout( 'background' );
		}  },

		// (Number) corner roundness when background is solid color
		'cornerRadius': { get: function (){ return shp.radius; }, set: function( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		}  },

		// (Number) outline thickness when background is solid color
		'lineThickness': { get: function (){ return shp.lineThickness; }, set: function( b ){
			shp.lineThickness = b;
		}  },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		'outlineColor': { get: function (){ return shp.outlineColor; }, set: function( c ){
			shp.outlineColor = (c === false ? '00000000' : c );
		}  },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		'filled': { get: function (){ return shp.filled; }, set: function( v ){ shp.filled = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		'slice': { get: function (){ return bg.slice; }, set: function( v ){ bg.slice = v; }  },

		// (Number) texture slice top
		'sliceTop': { get: function (){ return bg.sliceTop; }, set: function( v ){ bg.sliceTop = v; }, serialized: false  },

		// (Number) texture slice right
		'sliceRight': { get: function (){ return bg.sliceRight; }, set: function( v ){ bg.sliceRight = v; }, serialized: false },

		// (Number) texture slice bottom
		'sliceBottom': { get: function (){ return bg.sliceBottom; }, set: function( v ){ bg.sliceBottom = v; }, serialized: false },

		// (Number) texture slice left
		'sliceLeft': { get: function (){ return bg.sliceLeft; }, set: function( v ){ bg.sliceLeft = v; }, serialized: false },

		// (Boolean) multiple line input
		'multiLine': { get: function (){ return rt.multiLine; }, set: function( v ){
			rt.multiLine = v;
			go.requestLayout( 'multiLine' );
			go.scrollCaretToView(); }  },

		// (Number) gets or sets number of visible lines in multiline control
		'numLines': {
			get: function (){ return rt.height / rt.lineHeight; },
			set: function( v ) {
				if ( v != 1 ) rt.multiLine = true;
				ui.minHeight = ui.padTop + ui.padBottom + v * rt.lineHeight;
				go.scrollCaretToView();
			}, serialized: false
		},

		// (Number) scrollable height
		'scrollHeight': { get: function (){ return rt.scrollHeight; } },

		// (Number) scrollable width
		'scrollWidth': { get: function (){ return rt.scrollHeight; } },

		// (Number) vertical scroll offset
		'scrollTop': { get: function(){ return rt.scrollTop; }, set: function ( v ){ rt.scrollTop = v; }, serialized: false },

		// (Number) horizontal scroll offset
		'scrollLeft': { get: function (){ return rt.scrollLeft; }, set: function( v ){ rt.scrollLeft = v; }, serialized: false },

		// (Number) scrollable width
		'scrollWidth': { get: function (){ return rt.scrollHeight; } },

		// (Boolean) auto grow / shrink vertically multiline field
		'autoGrow': {
			get: function (){ return autoGrow; },
			set: function( v ){
				rt.multiLine = rt.multiLine || v;
				autoGrow = v;
				go.requestLayout( 'autoGrow' );
			}
		},

		// (Number) multiLine line spacing
		'lineSpacing': { get: function (){ return rt.lineSpacing; }, set: function( v ){
			rt.lineSpacing = v;
			go.requestLayout( 'lineSpacing' );
		}  },

		// (Number) returns line height - font size + line spacing
		'lineHeight': { get: function (){ return rt.lineHeight; } },

		// (Boolean) font bold
		'bold': { get: function(){ return rt.bold; }, set: function ( v ){ rt.bold = v; } },

		// (Boolean) font italic
		'italic': { get: function (){ return rt.italic; }, set: function( v ){ rt.italic = v; }  },

		// (Boolean) allow text selection
		'selectable': { get: function (){ return selectable; }, set: function( v ){ selectable = v; rt.showSelection = v; if ( !v ) rt.selectionStart = rt.selectionEnd; }  },

		// (Boolean) select all text when control is first focused
		'selectAllOnFocus': { get: function (){ return selectAllOnFocus; }, set: function( b ){ selectAllOnFocus = b; }  },

		// (Boolean) allow typing in Tab character (as opposed to tabbing to another UI control)
		'tabEnabled': { get: function (){ return tabEnabled; }, set: function( v ){ tabEnabled = v; }  },

		// (Boolean) input disabled
		'disabled': { get: function (){ return disabled; }, set: function( v ){
			 disabled = ui.disabled = v; // ui.disabled is used in sharedProps state = 'auto' check
			 if ( v && editing ) go.editing = false;
			 go.state = 'auto';
			 go.requestLayout( 'disabled' );
		 }  },

		// (Boolean) input is currently in scrolling mode
		'scrolling': { get: function (){ return scrolling; }, set: function( v ){
			 scrolling = v;
			 go.state = 'scrolling';
		 }  },

		// (Boolean) enable display ^code formatting (while not editing)
		'formatting': { get: function (){ return formatting; }, set: function( v ){ formatting = v; rt.formatting = ( formatting && !ui.focused ); }  },

		// (Boolean) if using ^code formatting, each new line will auto-reset color, bold, etc.
		'newLinesResetFormatting': { get: function (){ return rt.newLinesResetFormatting; }, set: function( p ){ rt.newLinesResetFormatting = p; }  },

		// (Number) or (Color) text color
		'color': { get: function (){ return rt.textColor; }, set: function( v ){ rt.textColor = v; }  },

		// (Number) or (Color) text selection color
		'selectionColor': { get: function (){ return rt.selectionColor; }, set: function( v ){ rt.selectionColor = v; }  },

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
	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

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
	rt.autoSize = false;
	rt.showSelection = true;
	tc.render = rt;
	tc.serializeable = false;
	go.addChild( tc );

	// UI
	ui.autoMoveFocus = false;
	ui.layoutType = Layout.Anchors;
	ui.minWidth = 10; ui.minHeight = 10;
	ui.focusable = true;
	go.ui = ui;


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
	}

	// rollover / rollout - just redispatch on object
	ui.mouseOver = ui.mouseOut = function ( x, y, wx, wy ) {
		stopAllEvents();
		go.fire( currentEventName(), x, y, wx, wy );
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
		var rx = /[\s.@'"\/,\^=]/; // characters that function as "stops" or word boundary
		while( i > 0 && i < txt.length ) {
			if ( txt.substr( i, 1 ).match( rx ) ) break;
			i += direction;
		}
		if ( i != 0 && direction < 0 ) i++;
		return i;
	}

	// helper for autocomplete - searches backwards from startIndex until finding a character
	// that doesn't match 'acceptableCharactersRegex' param,
	// returns substring if it matches acceptableExpressionRegex, or empty string
	go.findExpression = function ( startIndex, acceptableCharactersRegex, acceptableExpressionRegex ) {
		// word boundary
		var txt = rt.text;
		var i = Math.min( startIndex, txt.length - 1 );
		var m;
		while( i >= 0 ) {
			m = txt.substr( i, 1 ).match( acceptableCharactersRegex );
			if ( !m || !m.length ) break;
			i--;
		}
		i++;
		var expr = txt.substr( i, startIndex - i + 1 );
		if ( acceptableExpressionRegex && !expr.match( acceptableExpressionRegex ) ) return ''; // ignore invalid
		return expr;
	}

	// helper - returns current line
	function getCurrentLine() {
		var txt = rt.text;
		var cp = rt.caretPosition;
		var i = cp;
		var res = '';
		// go back to beginning of line
		while ( i >= 0 ) {
			if ( txt.substr( i, 1 ) == "\n" ) {
				i++;
				break;
			}
			i--;
		}
		if ( i < 0 ) i = 0;
		return txt.substr( i, cp - i );
	}

	// helper - returns all whitespace in front of current line
	function getLineWhitespacePrefix() {
		var txt = rt.text;
		var cp = rt.caretPosition - 1;
		var i = cp;
		var res = '';
		// go back to beginning of line first
		while ( i >= 0 ) {
			if ( txt.substr( i, 1 ) == "\n" ) {
				i++;
				break;
			}
			i--;
		}
		if ( i < 0 ) i = 0;
		// starting from i, collect whitespace
		var m;
		do {
			m = txt.substr( i, 1 );
			if ( m.match( /\s/ ) ) res += m;
			else break;
			i++;
		} while ( i <= cp );
		return res;
	}
	
	// autocomplete popup handler
	function autocompleteCheck( accept )  {

		var suggestions = [];

		// accept selected item in popup
		if ( accept ) {

			if ( go.popup && go.popup.selectedIndex >= 0 )
			suggestions = [ go.popup.items[ go.popup.selectedIndex ] ];

		// call callback to get suggestions and replace start index
		} else {

			var temp = autocomplete( this );
			suggestions = temp.suggestions;
			autocompleteReplaceStart = temp.replaceStart;

		}

		// multiple suggestions? - show popup
		if ( suggestions.length > 1 ) {

			// new popup
			if ( !go.popup ) {
				go.popup = new GameObject( './popup-menu', {
					preferredDirection: 'up',
					noFocus: true,
					selected: function( item ) {
						autocompleteCheck( true );
					},
				} );
				go.popup.style = go.baseStyle.popupMenu;
			}

			// sort suggestions
			suggestions.sort( function ( a, b ) { return a.text < b.text ? -1 : 1; } );

			// only update if items changed
			var items = suggestions;
			var ni = go.popup.items.length;

			// definitely changed
			if ( ni != items.length ) {
				go.popup.selectedIndex = -1;
				go.popup.items = items;

			// check one by one
			} else {
				for ( var i = 0; i < ni; i++ ) {
					// items are different
					if ( go.popup.items[ i ].value != items[ i ].value ) {
						go.popup.selectedIndex = -1;
						go.popup.items = items;
						break;
					// item label only is different (submatch)
					} else if ( go.popup.items[ i ].text != items[ i ].text ) {
						go.popup.container.getChild( i ).text = items[ i ].text;
					}
				}
			}

			// update popup position
			var gp = go.localToGlobal( rt.caretX, rt.caretY );
			go.popup.setTransform( gp.x, gp.y );

		// single suggestion? - append selected text
		} else if ( suggestions.length == 1 ) {

			// close popup
			if ( go.popup ) { go.popup.parent = null; go.popup = null; }

			// insert suggestion
			var sugg = suggestions[ 0 ].value;
			rt.text = rt.text.substr( 0, autocompleteReplaceStart ) + sugg + rt.text.substr( rt.caretPosition );

			// if accepting
			if ( accept ) {

				// clear selection, set cursor to end
				rt.selectionStart = rt.selectionEnd = rt.caretPosition = autocompleteReplaceStart + sugg.length;
				go.focus(); go.editing = true; // keep editing

			// if replace start is where cursor is
			} else if ( autocompleteReplaceStart == rt.caretPosition ) {
				// select part that differs
				rt.selectionEnd = ( rt.selectionStart = rt.caretPosition ) + sugg.length;
			}

			go.fire( 'change', rt.text );
			// go.debounce( 'autocomplete', autocompleteCheck, 0.5 );

		// no matches, remove popup
		} else {
			if ( go.popup ) { go.popup.parent = null; go.popup = null; }
		}
	}

	// cancels autocomplete
	function cancelAutocomplete() {
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
		if ( autocomplete && typeof( key ) === 'string' ) {
			go.debounce( 'autocomplete', autocompleteCheck, 0.5 );
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
			        autocompleteCheck( true );
				    stopEvent();
			    } else if ( selectable && rt.selectionStart == rt.caretPosition && rt.selectionStart < rt.selectionEnd ) {
				    rt.caretPosition = rt.selectionEnd;
				    rt.selectionStart = rt.selectionEnd = 0;
					if ( autocomplete ) go.debounce( 'autocomplete', autocompleteCheck, 0.5 );
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
				        autocompleteCheck( true );
				        return;
			        } else if ( rt.multiLine && ( !newLinesRequireShift || ( shift || alt || ctrl || meta ) ) ) {
				        if ( tabEnabled ) {
					        var post = '';
					        // code style
					        if ( autocomplete ){
						        // if last character was {, add extra tab
						        var lastChar = txt.substr( caretPosition - 1, 1 )
						        if ( lastChar == '{' || lastChar == '[' || lastChar == '(' ) post = '\t';
					        }
					        ui.keyPress( "\n" + getLineWhitespacePrefix() + post );
				        } else {
					        ui.keyPress( "\n" ); // newline in multiline box
				        }
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
				    if ( autocomplete ) {
					    if ( go.popup ) {
						    cancelAutocomplete();
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
			    if ( autocomplete ) cancelAutocomplete();
	            break;

	        case Key.Delete:
	            if ( editing ) ui.keyPress( -1, 1 );
			    if ( autocomplete ) cancelAutocomplete();
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
			    if ( autocomplete ) cancelAutocomplete();
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
			    if ( autocomplete ) cancelAutocomplete();
	            break;

			case Key.Up:
			case Key.Down:
				if ( !editing ) break;

				// navigate autocomplete popup
				if ( autocomplete && go.popup ) {
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
			    if ( autocomplete ) cancelAutocomplete();
				break;

			case Key.A:
			    // select all
				if ( selectable && ( meta || ctrl ) ) {
					rt.selectionStart = 0;
					rt.caretPosition = rt.selectionEnd = rt.text.positionLength();
				    if ( autocomplete ) cancelAutocomplete();
				}
                break;

            case Key.C:
		    case Key.X:
			    // copy or cut
				if ( ( meta || ctrl ) && rt.selectionStart != rt.selectionEnd && selectable ) {
					var ss = txt.positionToIndex( rt.selectionStart );
					var se = txt.positionToIndex( rt.selectionEnd );
					var copiedText = txt.substr( ss, se - ss );
					App.clipboard = copiedText;
					if ( key == Key.X ) ui.keyPress( -1, 1 );
					go.fire( 'copy', copiedText );
				    if ( autocomplete ) cancelAutocomplete();
				}
                break;

		    case Key.V:
			    var copiedText;
			    if ( ( meta || ctrl ) && ( copiedText = App.clipboard ) && copiedText.length ){
				    if ( autocomplete ) cancelAutocomplete();
					ui.keyPress( copiedText );
				    go.fire( 'paste', copiedText );
			    }
			    break;

		    case Key.RightBracket:
		    case Key.Zero:

			    // code style } ) ]
			    if ( autocomplete && tabEnabled && editing && ( key !== Key.Zero || shift ) ) {

				    // if current line is all whitespace with tab on end?
				    var line = getCurrentLine();
				    if ( line.match( /^(\s*)\t$/g ) ) {
					    // delete one character before inserting }
					    ui.keyPress( -1, -1 );
			            cancelAutocomplete();
					    selStart = selEnd = caretPosition = rt.caretPosition;
						haveSelection = false;
				    }
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
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.textfield );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;

})(this);






