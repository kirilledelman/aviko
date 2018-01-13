/*
 
 Text input field
 
 usage:
 var textField = app.scene.addChild( 'ui/input' );
 textField.text = "Some text";

 look at mappedProps in source below for additional properties


 
 
*/

include( './style' );
(function(go) {

	// background
	var bg = new RenderSprite( UI.style.texturesFolder + UI.style.input );
	bg.slice = UI.style.inputSlice;

	// text container
	var tc = new GameObject();
	var rt = new RenderText( UI.style.font, UI.style.baseSize );
	tc.render = rt; tc.z = 1;
	rt.textColor = UI.style.textColor;
	rt.selectionColor = UI.style.selectionColor;

	// UI
	var ui = new UI();
	ui.autoMoveFocus = false;
	ui.minWidth = UI.style.inputPadding[ 3 ] + UI.style.inputPadding[ 1 ];
	ui.minHeight = UI.style.baseSize + UI.style.inputPadding[ 0 ] + UI.style.inputPadding[ 2 ];
	ui.layoutType = Layout.Anchors;
	ui.focusable = true;

	// inner props
	var tabEnabled = false;
	var disabled = false;
	var selectable = true;
	var resetText = "";
	var formatting = false;
	var mouseDownTime = null;

	// create properties on gameObject
	var mappedProps = [
		[ 'text',   function (){ return rt.text; }, function( t ){ rt.text = t; rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0; } ],
		[ 'width',  function (){ return rt.width; }, function ( w ){ ui.width = w; } ],
		[ 'height',  function (){ return rt.height; }, function ( h ){ ui.height = h; } ],
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],
		[ 'wrap',  function (){ return rt.wrap; }, function ( w ){ rt.wrap = w; } ],
		[ 'multiLine',  function (){ return rt.multiLine; }, function ( v ){ rt.multiLine = v; go.fire( 'layout' ); } ],
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; } ],
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; } ],
		[ 'selectable',  function (){ return selectable; }, function ( v ){ selectable = v; rt.showSelection = v; if ( !v ) rt.selectionStart = rt.selectionEnd; } ],
		[ 'tabEnabled',  function (){ return tabEnabled; }, function ( v ){ tabEnabled = v; } ],
		[ 'disabled',  function (){ return disabled; },
		 function ( v ){
			ui.focusable = !(disabled = v);
			if ( v && ui.focused ) ui.blur();
			bg.texture = UI.style.texturesFolder + (disabled ? UI.style.inputDisabled : (ui.focused ? UI.style.inputFocus : UI.style.input));
			rt.color0 = v ? UI.style.disabledTextColor : UI.style.textColor;
			go.fire( 'layout' );
		 } ],
		[ 'formatting',  function (){ return formatting; }, function ( v ){ formatting = v; rt.formatting = ( formatting && !rt.focused ); } ],
		[ 'textColor',  function (){ return rt.textColor; }, function ( v ){ rt.textColor = v; } ],
		[ 'selectionColor',  function (){ return rt.selectionColor; }, function ( v ){ rt.selectionColor = v; } ],
		[ 'color0',  function (){ return rt.color0; }, function ( v ){ rt.color0 = v; } ],
		[ 'color1',  function (){ return rt.color1; }, function ( v ){ rt.color1 = v; } ],
		[ 'color2',  function (){ return rt.color2; }, function ( v ){ rt.color2 = v; } ],
		[ 'color3',  function (){ return rt.color3; }, function ( v ){ rt.color3 = v; } ],
		[ 'color4',  function (){ return rt.color4; }, function ( v ){ rt.color4 = v; } ],
		[ 'color5',  function (){ return rt.color5; }, function ( v ){ rt.color5 = v; } ],
		[ 'color6',  function (){ return rt.color6; }, function ( v ){ rt.color6 = v; } ],
		[ 'color7',  function (){ return rt.color7; }, function ( v ){ rt.color7 = v; } ],
		[ 'color8',  function (){ return rt.color8; }, function ( v ){ rt.color8 = v; } ],
		[ 'color9',  function (){ return rt.color9; }, function ( v ){ rt.color9 = v; } ],
	];
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined)
		} );
	}

	// components are added after component is awake
	go.awake = function () {
		go.ui = ui;
		go.render = bg;
		go.addChild( tc );
	};

	// layout
	ui.layout = function ( x, y, w, h ) {
		go.setTransform( x, y );
		if ( !rt.multiLine ) h = UI.style.baseSize + UI.style.inputPadding[ 0 ] + UI.style.inputPadding[ 2 ];
		bg.width = w; bg.height = h;
		tc.setTransform( UI.style.inputPadding[ 0 ], UI.style.inputPadding[ 3 ] );
		rt.width = w - (UI.style.inputPadding[ 1 ] + UI.style.inputPadding[ 3 ]);
		rt.height = Math.max( rt.lineHeight, h - (UI.style.inputPadding[ 0 ] + UI.style.inputPadding[ 2 ]) );
	}

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// show caret
	    if ( newFocus == ui ) {
	        rt.showCaret = true;
		    rt.showSelection = selectable;
		    rt.formatting = false;
			bg.texture = UI.style.texturesFolder + UI.style.inputFocus;
		    resetText = rt.text;
	    // hide
	    } else {
		    go.scrollCaretToView();
	        rt.showCaret = rt.showSelection = ui.dragSelect = false;
		    rt.formatting = formatting;
		    rt.scrollLeft = 0;
			bg.texture = UI.style.texturesFolder + (disabled ? UI.style.inputDisabled : UI.style.input);
	    }
		go.fire( 'layout' );
	}

	// scrolling
	ui.mouseWheel = function ( wy, wx ) {
		// ignore if not focused
		if ( !rt.multiLine && !ui.focused ) return;

		// scroll vertically
		if ( wy != 0 && rt.scrollHeight > rt.height ) {
			rt.scrollTop =
			Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.scrollTop - wy ) ); // wy * rt.lineHeight
		}
		// scroll horizontally
		if ( wx != 0 && rt.scrollWidth > rt.width ){
			rt.scrollLeft =
			Math.max( 0, Math.min( rt.scrollWidth - rt.width, rt.scrollLeft + wx ) ); // 0.1 * wx * rt.lineHeight
		}
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

	// down
	ui.mouseDown = function ( btn, x, y ) {
		// ignore
		if ( disabled ) return;

		// offset by padding
		x -= UI.style.inputPadding[ 3 ]; y -= UI.style.inputPadding[ 0 ];

		// shift + down
		if ( ( input.get( Key.LeftShift ) || input.get( Key.RightShift ) ) && selectable ) {
			rt.selectionStart = Math.min( rt.selectionStart, rt.caretPosition );
			rt.selectionEnd = rt.caretPositionAt( x, y );
        } else {
			// set cursor
		    rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPositionAt( x, y );
		}
		// focus
	    ui.focus();

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

	// release
	ui.mouseUp = ui.mouseUpOutside = function ( btn, x, y ) {
		ui.dragSelect = null;
		input.off( 'mouseMove', ui.mouseMoveGlobal );
	};

	// mouse move while holding down
	ui.mouseMoveGlobal = function ( x, y ) {
		// dragging selection
		if ( ui.dragSelect ) {
			x -= UI.style.inputPadding[ 3 ]; y -= UI.style.inputPadding[ 0 ];
			var p = go.globalToLocal( x, y );
			var selEnd = rt.caretPositionAt( p.x, p.y );
			if ( selEnd >= 0 ) {
				rt.caretPosition = rt.selectionEnd = selEnd;
			}
		}
	}

	// key press
	ui.keyPress = function ( key, direction ) {
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

		// if multi-line text box, dispatch change event as user types
		if ( rt.multiLine ) go.fire( 'change', rt.text );

	}

	// key down
	ui.keyDown = function ( code, shift, ctrl, alt, meta ) {
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
			    }
				break;

	        case Key.Return:
		        if ( rt.multiLine ) {
			        ui.keyPress( "\n" );
		        } else {
			        if ( txt != resetText ) go.fire( 'change', txt );
			        ui.blur();
		        }
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

		    case Key.Escape:
		        // if single line text field, reset text
				if ( !rt.multiLine ) rt.text = resetText;
				ui.blur();
	            break;

			case Key.A:
			    // select all
				if ( meta || ctrl ) {
					rt.selectionStart = 0;
					rt.caretPosition = rt.selectionEnd = rt.text.positionLength();
				}
                break;

            case Key.C:
		    case Key.X:
			    // copy or cut
				if ( ( meta || ctrl ) && rt.selectionStart != rt.selectionEnd ) {
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

	// makes sure caret is inside visible area
	go.scrollCaretToView = function (){
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
	
})(this);







