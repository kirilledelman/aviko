/*
 
Text input field

Usage:
	var textField = app.scene.addChild( 'ui/input' );
	textField.text = "Some text";

look at mappedProps in source code below for additional properties

Events:
	'change' - when field changes (as you type)
	'focus' - when control is focused
	'blur' - when control lost focus
	'editStart' - when control begin text edit
	'editEnd' - when control ended text edit
	'accept' - on blur, if contents changed
	'cancel' - on blur, if contents didn't change
	'copy' - if text is copied via Ctrl+C or Ctrl+X ( into UI.clipboard )
	'paste' - some text was pasted from UI.clipboard
	'selectionChanged' - when selection changes ( event parameter is selected text )
 
*/

include( './ui' );
(function(go) {

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
	var cancelToBlur = true;
	var editing = false;
	var touched = false;
	var offBackground = false;
	var focusBackground= false;
	var disabledBackground= false;
	go.serializeMask = {};

	// API functions

	// set input focus to the control. forceEdit param = true, will begin editing even if acceptToEdit = true
	go[ 'focus' ] = function ( forceEdit ) { ui.focus(); if ( forceEdit && !disabled ) go.editing = true; }

	// remove input focus from control
	go[ 'blur' ] = function () { ui.blur(); }

	// API properties
	var mappedProps = [

		// (string) input contents
		[ 'text',   function (){ return rt.text; }, function( t ){
			var pt = rt.text;
			var ps0 = rt.selectionStart, ps1 = rt.selectionEnd;
			rt.text = t;
			rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0;
			if ( pt != t ) go.fire( 'change' );
			if ( ps0 != ps1 ) go.fire( 'selectionChanged' );
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
					rt.showCaret = true;
				    rt.showSelection = selectable;
				    rt.formatting = false;
					resetText = rt.text;
					if ( selectAllOnFocus && selectable ) {
					    rt.selectionStart = 0; rt.selectionEnd = rt.text.positionLength();
				    }
					go.fire( 'editStart' );
				} else {
					rt.showCaret = rt.showSelection = ui.dragSelect = false;
				    rt.formatting = formatting;
				    rt.scrollLeft = 0;
					go.fire( rt.text == resetText ? 'cancel' : 'accept', rt.text );
					go.fire( 'editEnd' );
				}
			}
		} ],

		// (string) font name
		[ 'font',  function (){ return rt.font; }, function ( f ){
			rt.font = f;
			go.scrollCaretToView();
		} ],

		// (Number) font size
		[ 'size',  function (){ return rt.size; }, function ( s ){
			rt.size = s;
			ui.minHeight = rt.lineHeight + ui.padTop + ui.padBottom;
			if ( !rt.multiLine ) ui.height = ui.minHeight;
			if ( go.scene ) go.scene.dispatchLate( 'layout' );
		} ],

		// (Boolean) returns or sets whether control has focus
		[ 'wrap',  function (){ return rt.wrap; }, function ( w ){ rt.wrap = w; go.scrollCaretToView(); } ],

		// (Number) extra spacing between characters
		[ 'characterSpacing',  function (){ return rt.characterSpacing; }, function ( v ){ rt.characterSpacing = v; go.scrollCaretToView(); } ],

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

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		[ 'background',  function (){ return offBackground; }, function ( b ){
			offBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		[ 'focusBackground',  function (){ return focusBackground; }, function ( b ){
			focusBackground = b;
			go.updateBackground();
		} ],

		// (String) or (Color) or (Number) or (Boolean) - texture or solid color to display for background
		[ 'disabledBackground',  function (){ return disabledBackground; }, function ( b ){
			disabledBackground = b;
			go.updateBackground();
		} ],

		// (Number) corner roundness when background is solid color
		[ 'cornerRadius',  function (){ return shp.radius; }, function ( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
			go.updateBackground();
		} ],

		// (Boolean) multiple line input
		[ 'multiLine',  function (){ return rt.multiLine; }, function ( v ){ rt.multiLine = v; go.fire( 'layout' ); go.scrollCaretToView(); } ],

		// (Number) multiLine line spacing
		[ 'lineSpacing',  function (){ return rt.lineSpacing; }, function ( v ){ rt.lineSpacing = v; go.scrollCaretToView(); } ],

		// (Number) returns line height - font size + line spacing
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],

		// (Boolean) font bold
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; } ],

		// (Boolean) font italic
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; } ],

		// (Boolean) allow text selection
		[ 'selectable',  function (){ return selectable; }, function ( v ){ selectable = v; rt.showSelection = v; if ( !v ) rt.selectionStart = rt.selectionEnd; } ],

		// (Boolean) display background or not
		[ 'selectAllOnFocus',  function (){ return selectAllOnFocus; }, function ( b ){ selectAllOnFocus = b; } ],

		// (Boolean) allow typing in Tab character (as opposed to tabbing to another UI control)
		[ 'tabEnabled',  function (){ return tabEnabled; }, function ( v ){ tabEnabled = v; } ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			 ui.focusable = !(disabled = v) || acceptToEdit;
			 if ( v && ui.focused ) ui.blur();
			 go.updateBackground();
			 tc.opacity = v ? 0.6 : 1;
			 go.fire( 'layout' );
		 } ],

		// (Boolean) enable display ^code formatting (while not editing)
		[ 'formatting',  function (){ return formatting; }, function ( v ){ formatting = v; rt.formatting = ( formatting && !ui.focused ); } ],

		// (Number) or (Color) text color
		[ 'textColor',  function (){ return rt.textColor; }, function ( v ){ rt.textColor = v; } ],

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
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

	// create components

	// background
	bg = new RenderSprite( offBackground );
	shp = new RenderShape( Shape.Rectangle );
	shp.radius = 0; shp.centered = false;
	shp.filled = true;

	// text container
	tc = new GameObject();
	rt = new RenderText();
	tc.render = rt; tc.z = 1;

	// UI
	ui.autoMoveFocus = false;
	ui.layoutType = Layout.Anchors;
	ui.focusable = true;

	// don't serialize components/properties
	go.serializeMask = { 'ui':1, 'render':1 };
	tc.serialized = false;

	// components are added after component is awake
	go.awake = function () {
		go.ui = ui;
		go.render = bg;
		go.addChild( tc );
	};

	// layout components
	ui.layout = function ( x, y, w, h ) {
		go.setTransform( x, y );
		if ( !rt.multiLine ) ui.minHeight = h = rt.lineHeight + ui.padTop + ui.padBottom;
		bg.width = shp.width = w; bg.height = shp.height = h;
		tc.setTransform( ui.padLeft, ui.padTop );
		rt.width = w - ( ui.padLeft + ui.padRight );
		rt.height = Math.max( rt.lineHeight, h - ( ui.padTop + ui.padBottom ) );
	}

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// focused
	    if ( newFocus == ui ) {
			go.fire( 'focus' );
		    ui.autoMoveFocus = false;
	        go.editing = !acceptToEdit;
		    go.updateBackground();
		    go.scrollIntoView();
	    // blurred
	    } else {
			go.fire( 'blur' );
		    ui.autoMoveFocus = acceptToEdit;
	        go.editing = false;
		    go.updateBackground();
	    }
		go.fire( 'layout' );
	}

	// navigation event
	ui.navigation = function ( name, value ) {

		stopEvent();

		// editing
		if ( editing ) {

			if ( name == 'accept' && !rt.multiLine ) go.editing = false;
			else if ( name == 'cancel' ) {
				// blur, or stop editing
				if( cancelToBlur ) ui.blur();
				else go.editing = false;
			}

		// focused but not editing
		} else {

			// enter = begin editing
			if ( name == 'accept' ) {

				if ( acceptToEdit && !disabled ) go.editing = true;

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
		// ignore if not focused
		if ( !rt.multiLine && !ui.focused ) return;

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

	// mouse down
	ui.mouseDown = function ( btn, x, y ) {
		// focus
	    ui.focus();

		// ignore if disabled
		if ( disabled ) return;

		// mouse always starts editing
		go.editing = true;

		// offset by padding
		x -= ui.padLeft; y -= ui.padTop;

		// shift + down
		if ( ( input.get( Key.LeftShift ) || input.get( Key.RightShift ) ) && selectable ) {
			rt.selectionStart = Math.min( rt.selectionStart, rt.caretPosition );
			rt.selectionEnd = rt.caretPositionAt( x, y );
        } else {
			// set cursor
		    rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPositionAt( x, y );
		}

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
			input.on( 'mouseMove', ui.mouseMoveGlobal );
		}
	}

	// mouse release
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y ) {
		ui.dragSelect = null;
		input.off( 'mouseMove', ui.mouseMoveGlobal );
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

		// if not editing, ignore
		if ( !editing ) return;

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

		// delete character
	    if ( key == -1 ) {

			// delete selection
			if ( replacingSelection ) {
				rt.text = rt.text.substr( 0, selStartIndex ) + rt.text.substr( selEndIndex );
				caretPosition = rt.selectionStart;
			}

			// backspace
			else if ( direction < 0 ) {
				if ( caretPosition <= 0 || caretIndex <= 0 ) return;
				var prevCaretIndex = rt.text.positionToIndex( caretPosition - 1 );
				rt.text = rt.text.substr( 0, prevCaretIndex ) + rt.text.substr( caretIndex );
				caretPosition--;
			}

		// normal character
	    } else {
			// selection
			if ( replacingSelection ) {
				rt.text = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
				caretPosition = rt.selectionStart + key.positionLength();
			} else {
				rt.text = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
				caretPosition += key.positionLength();
			}
	    }
		// update caret position
	    rt.caretPosition = caretPosition;
		rt.selectionEnd = rt.selectionStart;

		// make sure caret is in view
		go.scrollCaretToView();

		// change
		go.fire( 'change' );
	}

	// key down
	ui.keyDown = function ( code, shift, ctrl, alt, meta ) {

		// if not editing, ignore all except Tab
		if ( !editing && code != Key.Tab ) code = 0;

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

		// key
	    switch ( code ) {
		    case Key.Tab:
			    if ( tabEnabled ) {
			        ui.keyPress( "\t" );
	            } else {
			        ui.moveFocus( shift ? -1 : 1 );
				    stopEvent();
			    }
				break;

	        case Key.Return:
		        if ( rt.multiLine ) {
			        ui.keyPress( "\n" ); // newline in multiline box
		        } else {
			        // text changed?
			        if ( txt != resetText ) go.fire( 'change' );
		        }
	            break;

		    case Key.Escape:
		        // if single line text field, reset text
				if ( !rt.multiLine ) { rt.text = resetText; rt.caretPosition = resetText.positionLength(); }
	            break;

	        case Key.Backspace:
	            ui.keyPress( -1, -1 );
	            break;

	        case Key.Delete:
	            ui.keyPress( -1, 1 );
	            break;

	        case Key.Left:
			case Key.Right:
				var increment = ( code == Key.Left ? -1 : 1 );
				// skip word?
				if ( ctrl || alt || meta ) {
					// find either next, or previous word
					increment = txt.indexToPosition( findWordBoundaryIndex( txt, txt.positionToIndex( caretPosition ), increment ) ) - caretPosition;
				}
				// expand selection
				if ( shift && selectable ) {
					if ( code == Key.Left && caretPosition > 0 ) {
						if ( !haveSelection ) rt.selectionEnd = caretPosition;
						rt.caretPosition = rt.selectionStart = caretPosition + increment;
					} else if ( code == Key.Right && caretPosition < txt.positionLength() ){
						if ( !haveSelection ) rt.selectionStart = caretPosition;
						rt.selectionEnd = ( rt.caretPosition += increment );
					}
				// move caret
				} else {
					if ( !haveSelection ) rt.caretPosition += increment;
					rt.selectionStart = rt.selectionEnd;
				}
				go.scrollCaretToView();
	            break;

			case Key.Home:
			case Key.End:
				// expand selection
				if ( shift && selectable ) {
					if ( code == Key.Home && caretPosition > 0 ) {
						if ( !haveSelection ) rt.selectionEnd = caretPosition;
						rt.caretPosition = rt.selectionStart = rt.caretPositionAt ( -9999, rt.caretY );
					} else if ( code == Key.End && caretPosition < txt.positionLength() ){
						if ( !haveSelection ) rt.selectionStart = caretPosition;
						rt.selectionEnd = rt.caretPositionAt ( 9999, rt.caretY );
					}
				// move caret
				} else {
					if ( !haveSelection ) rt.caretPosition = rt.caretPositionAt ( ( code == Key.Home ? -9999 : 9999 ), rt.caretY );
					rt.selectionStart = rt.selectionEnd;
				}
				go.scrollCaretToView();
	            break;

			case Key.Up:
			case Key.Down:
				// starting selection
				if ( shift && selectable ) {
					if ( !haveSelection ) rt.selectionEnd = rt.selectionStart = caretPosition;
				// move caret - clear selection
				} else {
					rt.selectionStart = rt.selectionEnd;
				}
				// common
				if ( rt.caretLine == 0 && code == Key.Up ) {
					rt.caretPosition = 0;
				} else if ( rt.caretLine >= rt.numLines - 1 && code == Key.Down ) {
					rt.caretPosition = txt.positionLength();
				} else {
					rt.caretPosition = rt.caretPositionAt ( rt.caretX, rt.caretY + ( code == Key.Up ? -1 : 1 ) * rt.lineHeight );
				}
				// selecting
				if ( shift && selectable ) {
					if ( code == Key.Up ) rt.selectionStart = rt.caretPosition;
					else rt.selectionEnd = rt.caretPosition;
				}
				go.scrollCaretToView();
				break;

			case Key.A:
			    // select all
				if ( selectable && ( meta || ctrl ) ) {
					rt.selectionStart = 0;
					rt.caretPosition = rt.selectionEnd = rt.text.positionLength();
				}
                break;

            case Key.C:
		    case Key.X:
			    // copy or cut
				if ( ( meta || ctrl ) && rt.selectionStart != rt.selectionEnd && selectable ) {
					var ss = txt.positionToIndex( rt.selectionStart );
					var se = txt.positionToIndex( rt.selectionEnd );
					UI.clipboard = txt.substr( ss, se - ss );
					if ( code == Key.X ) ui.keyPress( -1, 1 );
					go.fire( 'copy', UI.clipboard );
				}
                break;

		    case Key.V:
			    if ( typeof( UI.clipboard ) == 'string' ){
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
	}

	// sets current background based on state
	go.updateBackground = function () {

		// determine state
		var prop;
		if ( ui.focused ) {
			prop = focusBackground;
		} else if ( disabled ) {
			prop = disabledBackground;
		} else {
			prop = offBackground;
		}

		// set look
		if ( prop === null || prop === false ) {
			go.render = null;
		} else if ( typeof( prop ) == 'string' ) {
			bg.texture = prop;
			go.render = bg;
		} else {
			shp.color = prop;
			go.render = shp;
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
	}

	// apply defaults
	if ( UI.style && UI.style.input ) for ( var p in UI.style.input ) go[ p ] = UI.style.input[ p ];

})(this);







