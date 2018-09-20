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

	// API properties
	UI.base.textfieldPrototype = UI.base.textfieldPrototype || {

		__proto__: UI.base.componentPrototype,
		
		// (String) input contents
		get text(){ return this.__rt.text; },
		set text( t ){
			var rt = this.__rt;
			var pt = rt.text;
			var ps0 = rt.selectionStart, ps1 = rt.selectionEnd;
			rt.text = t;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
			if ( pt != t /*&& !constructing*/ ) this.fire( 'change', this.value );
			if ( ps0 != ps1 ) this.fire( 'selectionChanged' );
		},

		// (String) or (Number) depending on if .numeric is set
		get value(){
			var rt = this.__rt;
			var v = rt.text;
			if ( this.__numeric ) {
				v = this.__integer ? parseInt( rt.text ) : parseFloat( rt.text );
				if ( isNaN ( v ) && this.__resetText ) v = ( this.__integer ? parseInt( this.__resetText ) : parseFloat( this.__resetText ) );
				v = Math.max( this.__minValue, Math.min( this.__maxValue, v ) );
			}
			return v;
		},
		set value( v ){
			var rt = this.__rt;
			if ( this.__numeric ) {
				if ( this.__integer ) v = Math.round( v );
				v = Math.min( this.__maxValue, Math.max( this.__minValue, v ) );
				if ( !this.__integer ) {
					// rounding
					v = Math.round( parseFloat( v.toFixed( 3 ) ) * 10000 ) * 0.0001;
					v = v.toFixed( 3 ).replace( /(0+)$/, '' ).replace( /\.$/, '' ); // delete trailing zeros, and .
				}
			}
			rt.text = v;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
		},

		// (Number) current width of the control
		get width(){ return this.ui.width; }, set width( w ){ this.ui.width = w; this.__scrollCaretToView(); },

		// (Number) current height of the control
		get height(){ return this.ui.height; }, set height( h ){ this.ui.height = h; this.__scrollCaretToView(); },

		// (Boolean) requires user to press Enter (or 'accept' controller button) to begin editing
		get acceptToEdit(){ return this.__acceptToEdit; },
		set acceptToEdit( ae ){
			this.__acceptToEdit = ae;
			this.ui.focusable = !this.__disabled || ae;
		},

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		get cancelToBlur(){ return this.__cancelToBlur; }, set cancelToBlur( cb ){ this.__cancelToBlur = cb; },

		// (Boolean) clicking outside control will blur the control
		get blurOnClickOutside(){ return this.__blurOnClickOutside; }, set blurOnClickOutside( cb ){ this.__blurOnClickOutside = cb; },

		// (Boolean) show selection even when not editing
		get alwaysShowSelection(){ return this.__alwaysShowSelection; }, set alwaysShowSelection( s ){ this.__alwaysShowSelection = s; },

		// (Boolean) allows mousewheel to scroll field without being focused in it
		get canScrollUnfocused(){ return this.__canScrollUnfocused; }, set canScrollUnfocused( u ){ this.__canScrollUnfocused = u; },

		// (Boolean) in multiline input require shift|alt|ctrl|meta to make new lines with Enter key
		get newLinesRequireShift(){ return this.__newLinesRequireShift; }, set newLinesRequireShift( u ){ this.__newLinesRequireShift = u; },

		// (Number) current position of caret ( from 0 to text.positionLength() )
		get caretPosition(){ return this.__rt.caretPosition; }, set caretPosition( p ){ this.__rt.caretPosition = p; this.__scrollCaretToView(); },

		// (Boolean) turns display of caret and selection on and off (used by focus system)
		get editing(){ return this.__editing; },
		set editing( e ){
			var rt = this.__rt;
			if ( e != this.__editing ) {
				this.__editing = e;
				if ( e ) {
					// first time editing
					if ( !this.__touched ) {
						// set caret to end
						rt.caretPosition = rt.text.positionLength();
						this.__touched = true;
					}
					rt.showCaret = rt.showSelection = true;
				    rt.formatting = false;
					this.__resetText = rt.text;
					if ( this.__selectAllOnFocus && this.__selectable ) {
						rt.selectionStart = 0; rt.selectionEnd = rt.text.positionLength();
				    }
					this.fire( 'editStart' );
				} else {
					rt.showCaret = this.ui.dragSelect = false;
				    rt.formatting = this.__formatting;
				    rt.scrollLeft = 0;
					this.__cancelAutocomplete();
					if ( this.__numeric ) this.value = this.value;
					this.fire( rt.text == this.__resetText ? 'cancel' : 'accept', rt.text );
					this.fire( 'editEnd', this.value );
					if ( !this.__alwaysShowSelection ) rt.showSelection = false;

				}
			}
		},

		// (string) font name
		get font(){ return this.__rt.font; }, set font( f ){ this.__rt.font = f; this.__scrollCaretToView(); },

		// (string) optional bold font name (improves rendering)
		get boldFont(){ return this.__rt.boldFont; }, set boldFont( f ){ this.__rt.boldFont = f; this.__scrollCaretToView(); },

		// (string) optional italic font name (improves rendering)
		get italicFont(){ return this.__rt.italicFont; }, set italicFont( f ){ this.__rt.italicFont = f; this.__scrollCaretToView(); },

		// (string) optional bold+italic font name (improves rendering)
		get boldItalicFont(){ return this.__rt.boldItalicFont; }, set boldItalicFont( f ){ this.__rt.boldItalicFont = f; this.__scrollCaretToView(); },

		// (Number) font size
		get size(){ return this.__rt.size; },
		set size( s ){
			var nl = this.numLines;
			this.__rt.size = s;
			this.ui.minHeight = this.__rt.lineHeight + this.ui.padTop + this.ui.padBottom;
			if ( this.__rt.multiLine ) {
				this.numLines = nl;
			} else {
				this.ui.height = this.ui.minHeight;
			}
		},

		// (Boolean) only accepts numbers
		get numeric(){ return this.__numeric; },
		set numeric( n ){
			this.__numeric = n;
			if ( n ) {
				this.__rt.multiLine = false;
				this.value = this.text; // update value
			}
		},

		// (Number) minimum value for numeric input
		get min(){ return this.__minValue; }, set min( v ){ this.__minValue = v; },

		// (Number) maximum value for numeric input
		get max(){ return this.__maxValue; }, set max( v ){ this.__maxValue = v; },

		// (Number) step by which to increment/decrement value, when using arrow keys in control
		get step(){ return this.__step; }, set step( v ){ this.__step = v; },

		// (Boolean) maximum value for numeric input
		get integer(){ return this.__integer; }, set integer( v ){ this.__integer = v; },

		// (Function) - callback function that returns an object with two properties:
		// {
		//      suggestions: Array of suggestions to display in a popup, or null to hide popup, e.g. [ { text: "suggestion 1", value: "replace value" } ... ]
		//      replaceStart: position in current field text, from which an accepted suggestion will replace text up to current caret
		// }
		// (String) - alternatively, name of the function in UI.base namespace ( e.g. "autocompleteObjectProperty" for UI.base.autocompleteObjectProperty )
		//  see UI.base.autocompleteObjectProperty for example
		get autocomplete(){ return this.__autocomplete; }, set autocomplete( v ){ this.__autocomplete = v; },

		// (*) an extra parameter used by autocomplete function.
		// object property lookup uses this a "this" target
		// filename autocomplete uses this as an array for allowed file extensions
		get autocompleteParam(){ return this.__autocompleteParam; }, set autocompleteParam( v ){ this.__autocompleteParam = v; },

		// (RegExp) allow typing only these characters. Regular expression against which to compare incoming character, e.g. /[0-9a-z]/i
		get allowed(){ return this.__allowed; }, set allowed( a ){ this.__allowed = a; },

		// (RegExp) only allow the text that matches this RegExp e.g. /^[0-9]{0,4}$/
		get pattern(){ return this.__pattern; }, set pattern( p ){ this.__pattern = p; },

		// (Boolean) should text be antialiased
		get antialias(){ return this.__rt.antialias; }, set antialias( a ){ this.__rt.antialias = a; },

		// (Boolean) word wrapping at control width enabled for multiline field
		get wrap(){ return this.__rt.wrap; }, set wrap( w ){ this.__rt.wrap = w; this.__scrollCaretToView(); },

		// (Number) extra spacing between characters
		get characterSpacing(){ return this.__rt.characterSpacing; }, set characterSpacing( v ){ this.__rt.characterSpacing = v; this.__scrollCaretToView(); },

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		get background(){ return this.__background; },
		set background( b ){
			this.__background = b;
			// set look
			if ( b === null || b === false ) {
				this.render = null;
			} else if ( typeof( b ) == 'string' ) {
				this.__bg.texture = b;
				this.render = this.__bg;
			} else {
				this.__shp.color = b;
				this.render = this.__shp;
			}
			this.requestLayout( 'background' );
		},

		// (Number) corner roundness when background is solid color
		get cornerRadius(){ return this.__shp.radius; }, set cornerRadius( b ){
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

		// (Boolean) multiple line input
		get multiLine(){ return this.__rt.multiLine; }, set multiLine( v ){
			this.__rt.multiLine = v;
			this.requestLayout( 'multiLine' );
			this.__scrollCaretToView(); },

		// (Number) gets or sets number of visible lines in multiline control
		get numLines (){ return this.__rt.height / this.__rt.lineHeight; },
		set numLines( v ) {
			if ( v != 1 ) this.__rt.multiLine = true;
			this.ui.minHeight = this.ui.padTop + this.ui.padBottom + v * this.__rt.lineHeight;
			this.__scrollCaretToView();
		},

		// (Number) scrollable height
		get scrollHeight(){ return this.__rt.scrollHeight; },

		// (Number) scrollable width
		get scrollWidth(){ return this.__rt.scrollWidth; },

		// (Number) vertical scroll offset
		get scrollTop(){ return this.__rt.scrollTop; }, set scrollTop( v ){ this.__rt.scrollTop = v; },

		// (Number) horizontal scroll offset
		get scrollLeft(){ return this.__rt.scrollLeft; }, set scrollLeft( v ){ this.__rt.scrollLeft = v; },
		
		// (Boolean) auto grow / shrink vertically multiline field
		get autoGrow(){ return this.__autoGrow; },
		set autoGrow( v ){
			this.__rt.multiLine = this.__rt.multiLine || v;
			this.__autoGrow = v;
			this.requestLayout( 'autoGrow' );
		},

		// (Number) multiLine line spacing
		get lineSpacing(){ return this.__rt.lineSpacing; },
		set lineSpacing( v ){
			this.__rt.lineSpacing = v;
			this.requestLayout( 'lineSpacing' );
		},

		// (Number) returns line height - font size + line spacing
		get lineHeight(){ return this.__rt.lineHeight; },

		// (Boolean) font bold
		get bold(){ return this.__rt.bold; }, set bold( v ){ this.__rt.bold = v; },

		// (Boolean) font italic
		get italic(){ return this.__rt.italic; }, set italic( v ){ this.__rt.italic = v; },

		// (Boolean) allow text selection
		get selectable(){ return this.__selectable; }, set selectable( v ){ this.__selectable = v; this.__rt.showSelection = v; if ( !v ) this.__rt.selectionStart = this.__rt.selectionEnd; },

		// (Boolean) select all text when control is first focused
		get selectAllOnFocus(){ return this.__selectAllOnFocus; }, set selectAllOnFocus( b ){ this.__selectAllOnFocus = b; },

		// (Boolean) allow typing in Tab character (as opposed to tabbing to another UI control)
		get tabEnabled(){ return this.__tabEnabled; }, set tabEnabled( v ){ this.__tabEnabled = v; },

		// (Boolean) input disabled
		get disabled(){ return this.__disabled; },
		set disabled( v ){
			 this.__disabled = this.ui.disabled = v; // ui.disabled is used in sharedProps state = 'auto' check
			 if ( v && this.__editing ) this.editing = false;
			 this.state = 'auto';
			 this.requestLayout( 'disabled' );
		 },

		// (Boolean) input is currently in scrolling mode
		get scrolling(){ return this.__scrolling; },
		set scrolling( v ){
			 this.__scrolling = v;
			 this.state = 'scrolling';
		 },

		// (Boolean) enable display ^code formatting (while not editing)
		get formatting(){ return this.__formatting; }, set formatting( v ){ this.__formatting = v; this.__rt.formatting = ( this.__formatting && !this.ui.focused ); },

		// (Number) or (Color) text color
		get color(){ return this.__rt.textColor; }, set color( v ){ this.__rt.textColor = v; },

		// (Number) or (Color) text selection color
		get selectionColor(){ return this.__rt.selectionColor; }, set selectionColor( v ){ this.__rt.selectionColor = v; },

		// (Number) or (Color) ^0 color
		get color0(){ return this.__rt.color0; }, set color0( v ){ this.__rt.color0 = v; },

		// (Number) or (Color) ^1 color
		get color1(){ return this.__rt.color1; }, set color1( v ){ this.__rt.color1 = v; },

		// (Number) or (Color) ^2 color
		get color2(){ return this.__rt.color2; }, set color2( v ){ this.__rt.color2 = v; },

		// (Number) or (Color) ^3 color
		get color3(){ return this.__rt.color3; }, set color3( v ){ this.__rt.color3 = v; },

		// (Number) or (Color) ^4 color
		get color4(){ return this.__rt.color4; }, set color4( v ){ this.__rt.color4 = v; },

		// (Number) or (Color) ^5 color
		get color5(){ return this.__rt.color5; }, set color5( v ){ this.__rt.color5 = v; },

		// (Number) or (Color) ^6 color
		get color6(){ return this.__rt.color6; }, set color6( v ){ this.__rt.color6 = v; },

		// (Number) or (Color) ^7 color
		get color7(){ return this.__rt.color7; }, set color7( v ){ this.__rt.color7 = v; },

		// (Number) or (Color) ^8 color
		get color8(){ return this.__rt.color8; }, set color8( v ){ this.__rt.color8 = v; },

		// (Number) or (Color) ^9 color
		get color9(){ return this.__rt.color9; }, set color9( v ){ this.__rt.color9 = v; },
		
		//
		
		// set input focus to the control. forceEdit param = true, will begin editing even if acceptToEdit = true
		focus: function ( forceEdit ) {
			this.ui.focus();
			if ( forceEdit && !this.__disabled ) this.editing = true;
		},

		blur: function () { this.ui.blur(); },
	
		// scroll to bottom of text
		scrollToBottom: function () { this.__rt.scrollLeft = 0; this.__rt.scrollTop = (this.__rt.scrollHeight > this.__rt.height ? (this.__rt.scrollHeight - this.__rt.height) : 0); },
		
		// layout components
		__layout: function ( w, h ) {
			var go = this.gameObject;
			if ( go.__autoGrow ) {
				this.minHeight = Math.min( this.maxHeight ? this.maxHeight : 999999, go.__rt.lineHeight * go.__rt.numLines + this.padTop + this.padBottom );
			} else {
				this.minHeight = go.__rt.lineHeight + this.padTop + this.padBottom;
			}
			go.__bg.resize( w, h );
			go.__shp.resize( w, h );
			go.__tc.setTransform( this.padLeft, this.padTop );
			go.__rt.resize( w - ( this.padLeft + this.padRight ), Math.max( go.__rt.lineHeight, h - ( this.padTop + this.padBottom ) ) );
			go.__scrollCaretToView();
		},
	
		// focus changed
		__focusChanged: function ( newFocus ) {
			var go = this.gameObject;
			// focused
		    if ( newFocus == this ) {
			    this.autoMoveFocus = false;
		        if ( !go.__disabled ) {
			        go.editing = !go.__acceptToEdit;
			        go.state = 'focus';
		        }
			    go.scrollIntoView();
		        go.__checkClickOutside = go.__proto__.__checkClickOutside.bind( go );
			    Input.on( 'mouseDown', go.__checkClickOutside );
		    // blurred
		    } else {
			    this.autoMoveFocus = go.__acceptToEdit;
		        go.editing = false;
			    go.scrolling = false;
			    go.state = 'auto';
			    Input.off( 'mouseDown', go.__checkClickOutside );
		    }
		},
	
		// rollover / rollout - just redispatch on object
		__mouseOverOut: function ( x, y, wx, wy ) {
			stopAllEvents();
			this.gameObject.fire( currentEventName(), x, y, wx, wy );
		},
	
		// navigation event
		__navigation: function ( name, value ) {
			var go = this.gameObject;
			stopAllEvents();
	
			// editing
			if ( go.__editing ) {
	
				if ( name == 'accept' && !go.__rt.multiLine ) go.editing = false;
				else if ( name == 'cancel' ) {
					// blur, or stop editing
					if ( go.__cancelToBlur ) this.blur();
					else go.editing = false;
				} else if ( name == 'vertical' ) {
					this.keyDown( value < 0 ? Key.Up : Key.Down );
				}
	
			// reading
			} else if ( go.__scrolling ) {
	
				if ( name == 'cancel' || name == 'accept' ) {
	
					// turn off scrolling mode
					go.scrolling = false;
	
				} else {
	
					// scroll
					if ( name == 'vertical' && go.__rt.scrollHeight > go.__rt.height ) {
						go.__rt.scrollTop =
						Math.max( 0, Math.min( go.__rt.scrollHeight - go.__rt.height, go.__rt.scrollTop + value * go.__rt.lineHeight ) );
					}
					// scroll horizontally
					if ( name == 'horizontal' && go.__rt.scrollWidth > go.__rt.width ){
						go.__rt.scrollLeft =
						Math.max( 0, Math.min( go.__rt.scrollWidth - go.__rt.width, go.__rt.scrollLeft + value * go.__rt.lineHeight  ) );
					}
	
				}
				
			// focused but not editing
			} else {
	
				// enter = begin editing, or scrolling
				if ( name == 'accept' ) {
	
					if ( go.__acceptToEdit && !go.__disabled ) go.editing = true;
					else if ( go.__disabled && (go.__rt.scrollHeight > go.__rt.height || go.__rt.scrollWidth > go.__rt.width) ) go.scrolling = true;
	
				// escape = blur
				} else if ( name == 'cancel' ) {
	
					if ( go.__cancelToBlur ) this.blur();
	
				// directional - move focus
				} else {
					var dx = 0, dy = 0;
					if ( name == 'horizontal' ) dx = value;
					else dy = value;
					this.moveFocus( dx, dy );
				}
	
			}
		},
	
		// scrolling
		__mouseWheel: function ( wy, wx ) {
			var go = this.gameObject;
			// if numeric and focused
			if ( go.__numeric && wy && this.focused && !go.__disabled ){
				var val = wy < 0 ? -go.__step : go.__step;
				go.value += (Input.get( Key.LeftShift ) ? 10 : 1 ) * val;
				go.fire( 'change', go.value );
				go.__rt.selectionStart = 0;
				go.__rt.caretPosition = go.__rt.selectionEnd = go.__rt.text.positionLength(); // select all
				stopEvent();
				return;
			}
	
			var canScroll = (go.__rt.scrollHeight > go.__rt.height || go.__rt.scrollWidth > go.__rt.width);
	
			// ignore if not focused
			if ( !canScroll || ( !go.__canScrollUnfocused && !this.focused ) ) return;
	
			var st = go.__rt.scrollTop, sl = go.__rt.scrollLeft;
	
			// scroll vertically
			if ( wy != 0 && go.__rt.scrollHeight > go.__rt.height ) {
				go.__rt.scrollTop =
				Math.max( 0, Math.min( go.__rt.scrollHeight - go.__rt.height, go.__rt.scrollTop - wy ) );
			}
			// scroll horizontally
			if ( wx != 0 && go.__rt.scrollWidth > go.__rt.width ){
				go.__rt.scrollLeft =
				Math.max( 0, Math.min( go.__rt.scrollWidth - go.__rt.width, go.__rt.scrollLeft + wx ) );
			}
	
			// stop event if scrolled
			if ( sl != go.__rt.scrollLeft || st != go.__rt.scrollTop ) stopEvent();
		},
	
		// helper for selection
		__findWordBoundaryIndex: function ( txt, startIndex, direction ) {
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
		},
	
		// helper for autocomplete - searches backwards from startIndex until finding a character
		// that doesn't match 'acceptableCharactersRegex' param,
		// returns substring if it matches acceptableExpressionRegex, or empty string
		findExpression: function ( startIndex, acceptableCharactersRegex, acceptableExpressionRegex ) {
			// word boundary
			var txt = this.__rt.text;
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
		},
	
		// helper - returns current line
		__getCurrentLine: function() {
			var txt = this.__rt.text;
			var cp = this.__rt.caretPosition;
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
		},
	
		// helper - returns all whitespace in front of current line
		__getLineWhitespacePrefix: function () {
			var txt = this.__rt.text;
			var cp = this.__rt.caretPosition - 1;
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
		},
		
		// autocomplete popup handler
		__autocompleteCheck: function ( accept )  {
	
			var suggestions = [];
			var rt = this.__rt;
			
			// accept selected item in popup
			if ( accept ) {
				
				if ( this.__popup && this.__popup.selectedIndex >= 0 )
				suggestions = [ this.__popup.items[ this.__popup.selectedIndex ] ];
	
			// call callback to get suggestions and replace start index
			} else {
	
				var temp = this.__autocomplete( this );
				suggestions = temp.suggestions;
				this.__autocompleteReplaceStart = temp.replaceStart;
	
			}
	
			// multiple suggestions? - show popup
			if ( suggestions.length > 1 ) {
	
				// new popup
				if ( !this.__popup ) {
					this.__popup = new GameObject( './popup-menu', {
						preferredDirection: 'up',
						noFocus: true,
						selected: function( item ) {
							this.__autocompleteCheck( true );
						}.bind( this ),
					} );
					this.__popup.style = this.__baseStyle.popupMenu;
				}
	
				// sort suggestions
				suggestions.sort( function ( a, b ) { return a.text < b.text ? -1 : 1; } );
	
				// only update if items changed
				var items = suggestions;
				var ni = this.__popup.items.length;
	
				// definitely changed
				if ( ni != items.length ) {
					this.__popup.selectedIndex = -1;
					this.__popup.items = items;
	
				// check one by one
				} else {
					for ( var i = 0; i < ni; i++ ) {
						// items are different
						if ( this.__popup.items[ i ].value != items[ i ].value || this.__popup.items[ i ].text != items[ i ].text ) {
							this.__popup.selectedIndex = -1;
							this.__popup.items = items;
							break;
						}
					}
				}
	
				// update popup position
				var gp = this.localToGlobal( rt.caretX, rt.caretY );
				this.__popup.setTransform( gp.x, gp.y );
	
			// single suggestion? - append selected text
			} else if ( suggestions.length == 1 ) {
	
				// close popup
				if ( this.__popup ) { this.__popup.parent = null; this.__popup = null; }
	
				// insert suggestion
				var sugg = suggestions[ 0 ].value;
				rt.text = rt.text.substr( 0, this.__autocompleteReplaceStart ) + sugg + rt.text.substr( rt.caretPosition );
	
				// if accepting
				if ( accept ) {
	
					// clear selection, set cursor to end
					rt.selectionStart = rt.selectionEnd = rt.caretPosition = this.__autocompleteReplaceStart + sugg.length;
					this.focus(); this.editing = true; // keep editing
	
				// if replace start is where cursor is
				} else if ( this.__autocompleteReplaceStart == rt.caretPosition ) {
					// select part that differs
					rt.selectionEnd = ( rt.selectionStart = rt.caretPosition ) + sugg.length;
					this.__scrollCaretToView();
				}
	
				this.fire( 'change', rt.text );
	
			// no matches, remove popup
			} else {
				if ( this.__popup ) { this.__popup.parent = null; this.__popup = null; }
			}
		},
	
		// cancels autocomplete
		__cancelAutocomplete: function () {
			this.cancelDebouncer( 'autocomplete' );
			if ( this.__popup ) { this.__popup.parent = null; this.__popup = null; }
		},
	
		// mouse down
		__mouseDown: function ( btn, x, y ) {
	
			var go = this.gameObject;
			var rt = go.__rt;
			
			if ( btn != 1 ) return;
	
			// offset by padding
			x -= this.padLeft; y -= this.padTop;
	
			// shift + down
			if ( ( Input.get( Key.LeftShift ) || Input.get( Key.RightShift ) ) && go.__selectable ) {
				rt.selectionStart = Math.min( rt.selectionStart, rt.caretPosition );
				rt.selectionEnd = rt.caretPositionAt( x, y );
	        } else {
				// set cursor
			    rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPositionAt( x, y );
			}
	
			// mouse always starts editing
			go.focus();
			if ( !go.__disabled ) go.editing = true;
	
			// double clicked word
			var newMouseDownTime = new Date();
			if ( go.__mouseDownTime && ( newMouseDownTime.getTime() - go.__mouseDownTime.getTime() < 300 ) ) {
				var pos = rt.text.positionToIndex( rt.caretPosition );
				var left = go.__findWordBoundaryIndex( rt.text, pos, -1 );
				var right = go.__findWordBoundaryIndex( rt.text, pos, 1 );
				if ( left != right ){
					rt.selectionStart = rt.text.indexToPosition( left );
					rt.caretPosition = rt.selectionEnd = rt.text.indexToPosition( right );
				}
				go.__mouseDownTime = null;
				return;
			}
			go.__mouseDownTime = newMouseDownTime;
	
			// start selection drag
			if ( go.__selectable ) {
				this.dragSelect = true;
				go.__mouseMoveGlobal = go.__proto__.__mouseMoveGlobal.bind( this );
				Input.on( 'mouseMove', go.__mouseMoveGlobal );
			}
	
			stopAllEvents();
		},
	
		__mouseUp: function ( btn, x, y ) {
			if ( btn != 1 ) return;
			this.dragSelect = null;
			Input.off( 'mouseMove', this.gameObject.__mouseMoveGlobal );
		},
	
		// mouse move while holding down
		__mouseMoveGlobal: function ( x, y ) {
			var go = this.gameObject;
			// dragging selection
			if ( this.dragSelect ) {
				x -= this.padLeft; y -= this.padTop;
				var p = go.globalToLocal( x, y );
				var selEnd = go.__rt.caretPositionAt( p.x, p.y );
				if ( selEnd >= 0 ) {
					go.__rt.caretPosition = go.__rt.selectionEnd = selEnd;
				}
			}
		},
	
		// key press
		__keyPress: function ( key, direction ) {
			
			var go = this.gameObject;
			var rt = go.__rt;
			
			// if not editing
			if ( !go.__editing ) {
				// if it's a character, start editing
				if ( !go.__disabled && typeof( key ) == 'string' ) go.editing = true;
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
					if ( go.__pattern && !txt.match( go.__pattern ) ) return;
					rt.text = txt;
					caretPosition = rt.selectionStart;
				}
	
				// backspace
				else if ( direction < 0 ) {
					if ( caretPosition <= 0 || caretIndex <= 0 ) return;
					var prevCaretIndex = rt.text.positionToIndex( caretPosition - 1 );
					txt = rt.text.substr( 0, prevCaretIndex ) + rt.text.substr( caretIndex );
					if ( go.__pattern && !txt.match( go.__pattern ) ) return;
					rt.text = txt;
					caretPosition--;
				}
	
			// normal character
		    } else {
			    // filters
			    if ( go.__allowed ) {
				    var match = key.match( go.__allowed );
				    if ( !match || !match.length ) return;
			    }
			    if ( go.__numeric ) {
				    if ( go.__integer && !key.match( /\d/ ) ) return;
				    else if ( !key.match( /[0-9.\-]/ ) ) return;
					// test new value
					if ( replacingSelection ) {
						txt = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
					} else {
						txt = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
					}
				    if ( txt == '.' || txt == '-' ) txt += '0';
				    if ( (go.__integer && isNaN( parseInt( txt ) ) ) || isNaN( parseFloat( txt ) ) ) return;
				    if ( go.__pattern && !txt.match( go.__pattern ) ) return;
	
			    }
	
				// selection
				if ( replacingSelection ) {
					txt = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
					caretPosition = rt.selectionStart + key.positionLength();
				} else {
					txt = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
					caretPosition += key.positionLength();
				}
				if ( go.__pattern && !txt.match( go.__pattern ) ) return;
				rt.text = txt;
		    }
	
			// update caret position
		    rt.caretPosition = caretPosition;
			rt.selectionEnd = rt.selectionStart;
	
			// autogrow
			if ( go.__autoGrow ) go.debounce( 'autoGrow', go.__checkAutoGrow );
	
			// autocomplete
			if ( go.__autocomplete && typeof( key ) === 'string' ) {
				go.debounce( 'autocomplete', go.__autocompleteCheck, 0.5 );
			}
	
			// make sure caret is in view
			go.__scrollCaretToView();
	
			// change
			go.fire( 'change', go.value );
	
		},
	
		// key down
		__keyDown: function ( key, shift, ctrl, alt, meta ) {
	
			var go = this.gameObject;
			var rt = go.__rt;
			
		    // ready
			this.dragSelect = false;
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
			if ( go.__autoGrow ) go.debounce( 'autoGrow', go.__checkAutoGrow );
	
			// key
		    switch ( key ) {
			    case Key.Tab:
				    // complete word
				    if ( go.__popup ) {
				        go.__autocompleteCheck( true );
					    stopEvent();
				    } else if ( go.__selectable && rt.selectionStart == rt.caretPosition && rt.selectionStart < rt.selectionEnd ) {
					    rt.caretPosition = rt.selectionEnd;
					    rt.selectionStart = rt.selectionEnd = 0;
						if ( go.__autocomplete ) go.debounce( 'autocomplete', go.__autocompleteCheck, 0.5 );
				    // tab character
		            } else if ( go.__tabEnabled ) {
				        this.keyPress( "\t" );
				    // move focus
		            } else {
				        this.moveFocus( shift ? -1 : 1 );
				    }
					break;
	
		        case Key.Return:
			        if ( go.__editing ) {
				        if ( go.__popup ) {
					        go.__autocompleteCheck( true );
					        return;
				        } else if ( rt.multiLine && ( !go.__newLinesRequireShift || ( shift || alt || ctrl || meta ) ) ) {
					        if ( go.__tabEnabled ) {
						        var post = '';
						        // code style
						        if ( go.__autocomplete ){
							        // if last character was {, add extra tab
							        var lastChar = txt.substr( caretPosition - 1, 1 )
							        if ( lastChar == '{' || lastChar == '[' || lastChar == '(' ) post = '\t';
						        }
						        this.keyPress( "\n" + go.__getLineWhitespacePrefix() + post );
					        } else {
						        this.keyPress( "\n" ); // newline in multiline box
					        }
					        return;
				        } else {
					        // text changed?
					        if ( txt != go.__resetText ) go.fire( 'change', go.value );
					        go.editing = false;
				        }
			        } else {
				        this.navigation( 'accept' );
			        }
			        stopEvent();
		            break;
	
			    case Key.Escape:
				    if ( go.__editing ) {
					    if ( go.__autocomplete ) {
						    if ( go.__popup ) {
							    go.__cancelAutocomplete();
							    stopAllEvents();
							    go.editing = true;
							    return;
						    } else {
							    go.cancelDebouncer( 'autocomplete' );
						    }
					    }
					    // if single line text field, reset text
					    if ( !rt.multiLine ) {
						    rt.text = go.__resetText;
						    rt.caretPosition = go.__resetText.positionLength();
					    }
				    }
				    this.navigation( 'cancel' );
		            break;
	
		        case Key.Backspace:
		            if ( go.__editing ) this.keyPress( -1, -1 );
				    if ( go.__autocomplete ) go.__cancelAutocomplete();
		            break;
	
		        case Key.Delete:
		            if ( go.__editing ) this.keyPress( -1, 1 );
				    if ( go.__autocomplete ) go.__cancelAutocomplete();
		            break;
	
		        case Key.Left:
				case Key.Right:
					var increment = ( key == Key.Left ? -1 : 1 );
					// skip word?
					if ( ctrl || alt || meta ) {
						// find either next, or previous word
						increment = txt.indexToPosition( go.__findWordBoundaryIndex( txt, txt.positionToIndex( caretPosition ), increment ) ) - caretPosition;
					}
					// expand selection
					if ( shift && go.__selectable ) {
						if ( key == Key.Left && caretPosition > 0 ) {
							if ( !haveSelection ) rt.selectionEnd = caretPosition;
							rt.caretPosition = rt.selectionStart = caretPosition + increment;
						} else if ( key == Key.Right && caretPosition < txt.positionLength() ){
							if ( !haveSelection ) rt.selectionStart = caretPosition;
							rt.selectionEnd = ( rt.caretPosition += increment );
						}
					// move caret
					} else if ( go.__editing ) {
						if ( haveSelection ) {
							rt.caretPosition = ( key == Key.Left ? rt.selectionStart : rt.selectionEnd );
						} else rt.caretPosition += increment;
						rt.selectionStart = rt.selectionEnd;
					}
					go.__scrollCaretToView();
				    if ( go.__autocomplete ) go.__cancelAutocomplete();
		            break;
	
				case Key.Home:
				case Key.End:
					// expand selection
					if ( shift && go.__selectable ) {
						if ( key == Key.Home && caretPosition > 0 ) {
							if ( !haveSelection ) rt.selectionEnd = caretPosition;
							rt.caretPosition = rt.selectionStart = rt.caretPositionAt ( -9999, rt.caretY );
						} else if ( key == Key.End && caretPosition < txt.positionLength() ){
							if ( !haveSelection ) rt.selectionStart = caretPosition;
							rt.selectionEnd = rt.caretPositionAt ( 9999, rt.caretY );
						}
					// move caret
					} else if ( go.__editing ) {
						if ( !haveSelection ) rt.caretPosition = rt.caretPositionAt ( ( key == Key.Home ? -9999 : 9999 ), rt.caretY );
						rt.selectionStart = rt.selectionEnd;
					}
					go.__scrollCaretToView();
				    if ( go.__autocomplete ) go.__cancelAutocomplete();
		            break;
	
				case Key.Up:
				case Key.Down:
					if ( !go.__editing ) break;
	
					// navigate autocomplete popup
					if ( go.__autocomplete && go.__popup ) {
						if ( key == Key.Up ) {
							if ( go.__popup.selectedIndex <= 0 ) go.__popup.selectedIndex = go.__popup.items.length - 1;
							else go.__popup.selectedIndex--;
						} else {
							if ( go.__popup.selectedIndex == -1 || go.__popup.selectedIndex == go.__popup.items.length - 1 ) go.__popup.selectedIndex = 0;
							else go.__popup.selectedIndex++;
						}
						stopEvent();
						break;
					}
	
					// numeric
					if ( go.__numeric ) {
						go.value += ( key == Key.Up ? go.__step : -go.__step ) * ( shift ? 10 : 1 );
						rt.selectionStart = 0; rt.caretPosition = rt.selectionEnd = rt.text.positionLength(); // select all
						go.fire( 'change', go.value );
						stopAllEvents();
						return;
					}
	
					// starting selection
					if ( shift && go.__selectable ) {
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
					if ( shift && go.__selectable ) {
						if ( key == Key.Up ) rt.selectionStart = rt.caretPosition;
						else rt.selectionEnd = rt.caretPosition;
					}
					go.__scrollCaretToView();
				    if ( go.__autocomplete ) go.__cancelAutocomplete();
					break;
	
				case Key.A:
				    // select all
					if ( go.__selectable && ( meta || ctrl ) ) {
						rt.selectionStart = 0;
						rt.caretPosition = rt.selectionEnd = rt.text.positionLength();
					    if ( go.__autocomplete ) go.__cancelAutocomplete();
					}
	                break;
	
	            case Key.C:
			    case Key.X:
				    // copy or cut
					if ( ( meta || ctrl ) && rt.selectionStart != rt.selectionEnd && go.__selectable ) {
						var ss = txt.positionToIndex( rt.selectionStart );
						var se = txt.positionToIndex( rt.selectionEnd );
						var copiedText = txt.substr( ss, se - ss );
						if ( rt.formatting ) {
							// strip formatting
							copiedText = copiedText.replace( /(\^\^)/g, '^' ).replace( /(\^[biBI0-9c])/g, '' );
						}
						App.clipboard = copiedText;
						if ( key == Key.X ) this.keyPress( -1, 1 );
						go.fire( 'copy', copiedText );
					    if ( go.__autocomplete ) go.__cancelAutocomplete();
					}
	                break;
	
			    case Key.V:
				    var copiedText;
				    if ( ( meta || ctrl ) && ( copiedText = App.clipboard ) && copiedText.length ){
					    if ( go.__autocomplete ) go.__cancelAutocomplete();
						this.keyPress( copiedText );
					    go.fire( 'paste', copiedText );
				    }
				    break;
	
			    case Key.RightBracket:
			    case Key.Zero:
	
				    // code style } ) ]
				    if ( go.__autocomplete && go.__tabEnabled && go.__editing && ( key !== Key.Zero || shift ) ) {
	
					    // if current line is all whitespace with tab on end?
					    var line = go.__getCurrentLine();
					    if ( line.match( /^(\s*)\t$/g ) ) {
						    // delete one character before inserting }
						    this.keyPress( -1, -1 );
				            go.__cancelAutocomplete();
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
			if ( go.__popup ) {
				var gp = go.localToGlobal( rt.caretX, rt.caretY );
				go.__popup.setTransform( gp.x, gp.y );
			}
		},
	
		// checks if clicked outside
		__checkClickOutside: function ( btn, x, y ) {
			if ( blurOnClickOutside && this.ui.focused && !this.__popup ) {
				var lp = this.globalToLocal( x, y );
				if ( lp.x < 0 || lp.x > this.width || lp.y < 0 || lp.y > this.height ) {
					this.blur();
				}
			}
		},
	
		// auto resize text box vertically with text
		__checkAutoGrow: function () {
			if ( this.__autoGrow && this.__rt.multiLine ) {
				var h = this.ui.height;
				this.ui.height = this.ui.minHeight = this.__rt.lineHeight * this.__rt.numLines + this.ui.padTop + this.ui.padBottom;
				if ( h != this.ui.height ) this.async( go.scrollIntoView );
			}
		},
	
		// makes sure caret is inside visible area
		__scrollCaretToView: function (){
	
			var ui = this.ui;
			var rt = this.__rt;
			
			// only when focused
			if ( !ui.focused || !this.__editing) return;
	
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
			this.scrollIntoView();
		},
	
		// called from "focusChanged" to scroll this component into view
		scrollIntoView: function() {
			if ( this.__editing ) {
				// scroll to caret, if editing
				UI.base.componentPrototype.scrollIntoView.call( this, this.__rt.caretX, this.__rt.caretY ); // ,1 + ui.padLeft + ui.padRight, rt.lineHeight + ui.padTop + ui.padBottom );
			} else {
				UI.base.componentPrototype.scrollIntoView.call( this );
			}
		}
	};
	
	// set name
	go.name = "Textfield";
	go.ui = new UI({
		autoMoveFocus: false,
		layoutType: Layout.Anchors,
		minWidth: 10,
		minHeight: 10,
		focusable: true,
		layout: UI.base.textfieldPrototype.__layout,
		focusChanged: UI.base.textfieldPrototype.__focusChanged,
		mouseOver: UI.base.textfieldPrototype.__mouseOverOut,
		navigation: UI.base.textfieldPrototype.__navigation,
		mouseWheel: UI.base.textfieldPrototype.__mouseWheel,
		mouseDown: UI.base.textfieldPrototype.__mouseDown,
		mouseUp: UI.base.textfieldPrototype.__mouseUp,
		keyDown: UI.base.textfieldPrototype.__keyDown,
		keyPress: UI.base.textfieldPrototype.__keyPress,
    });
	go.__bg = go.render = new RenderSprite();
	go.__shp = new RenderShape( {
        shape: Shape.Rectangle,
        radius: 0,
        centered: false, filled: true
	} );
	go.__tc = new GameObject( { serializeable: false } );
	go.__rt = go.__tc.render = new RenderText( {
		autoSize: false,
		showSelection: true
	} );
	go.addChild( go.__tc );
	go.__tabEnabled = false;
	go.__disabled = false;
	go.__disabledCanFocus = true;
	go.__selectable = true;
	go.__resetText = "";
	go.__formatting = false;
	go.__mouseDownTime = null;
	go.__selectAllOnFocus = false;
	go.__acceptToEdit = false;
	go.__cancelToBlur = false;
	go.__blurOnClickOutside = true;
	go.__editing = false;
	go.__scrolling = false;
	go.__touched = false;
	go.__background = false;
	go.__autoGrow = false;
	go.__numeric = false;
	go.__integer = false;
	go.__autocomplete = null;
	go.__autocompleteParam = undefined;
	go.__autocompleteReplaceStart = -1;
	go.__newLinesRequireShift = true;
	go.__minValue = -Infinity;
	go.__maxValue = Infinity;
	go.__step = 1.0;
	go.__canScrollUnfocused = false;
	go.__alwaysShowSelection = false;
	go.__allowed = null;
	go.__pattern = null;
	go.__proto__ = UI.base.textfieldPrototype;
	go.__init();
	go.serializeMask.push( 'numLines', 'value', 'scrollTop', 'scrollLeft', 'focus', 'blur', 'scrollToBottom', 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' );

	// add property-list inspectable info
	UI.base.addInspectables( go, "TextField",
    [ 'text', 'numeric', 'integer', 'min', 'max', 'step', 'autoGrow', 'multiLine',
	  'acceptToEdit', 'cancelToBlur', 'blurOnClickOutside', 'alwaysShowSelection', 'canScrollUnfocused',
	  'newLinesRequireShift' ],
    {}, 1 );
	UI.base.addInspectables( go, "Text",
    [   'size', 'font', 'boldFont', 'italicFont', 'boldItalicFont',
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
    }, 2 );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.textfield );
	UI.base.applyProperties( go, go.__baseStyle );
	go.state = 'auto';
	
})(this);






