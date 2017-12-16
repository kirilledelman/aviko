// include Style.js
include( 'UI/Style' );

/**
@param {GameObject} gameObject
@return {GameObject}
*/
(function TextInput( gameObject ) {
	// create components
	var rt = gameObject.render = new RenderText();
	var ui = gameObject.ui = new UI();
	var tabEnabled = false;
	var disabled = false;
	var selectable = true;
	var resetText = "";
	var formatting = false;

	// focus rectangle
	var focusRect = new GameObject();
	var fr = focusRect.render = new RenderShape( SHAPE_RECTANGLE );
	fr.lineWidth = 1;
	fr.stipple = 0.5;
	fr.x = rt.width + 4; fr.y = rt.height + 4;
	focusRect.active = fr.centered = false;
	focusRect.x = -2; focusRect.y = -2;
	gameObject.addChild( focusRect );

	// create properties on gameObject
	var mappedProps = [
		[ 'text',   function (){ return rt.text; }, function( t ){ rt.text = t; rt.caretPosition = rt.selectionStart = rt.selectionEnd = 0; } ],
		[ 'width',  function (){ return rt.width; }, function ( w ){ fr.x = w + 4; rt.width = w; } ],
		[ 'height',  function (){ return rt.height; }, function ( h ){ fr.y = h + 4; rt.height = h; rt.multiLine = ( h > rt.lineHeight ); } ],
		[ 'lineHeight',  function (){ return rt.lineHeight; } ],
		[ 'wrap',  function (){ return rt.wrap; }, function ( w ){ rt.wrap = w; } ],
		[ 'multiLine',  function (){ return rt.multiLine; }, function ( v ){ rt.multiLine = v; } ],
		[ 'bold',  function (){ return rt.bold; }, function ( v ){ rt.bold = v; } ],
		[ 'italic',  function (){ return rt.italic; }, function ( v ){ rt.italic = v; } ],
		[ 'selectable',  function (){ return selectable; }, function ( v ){ selectable = v; rt.showSelection = v; if ( !v ) rt.selectionStart = rt.selectionEnd; } ],
		[ 'tabEnabled',  function (){ return tabEnabled; }, function ( v ){ tabEnabled = v; } ],
		[ 'disabled',  function (){ return disabled; }, function ( v ){ ui.focusable = !(disabled = v); if ( v && ui.focused ) ui.blur(); } ],
		[ 'formatting',  function (){ return formatting; }, function ( v ){ formatting = v; rt.formatting = ( formatting && !rt.focused ); } ],
		[ 'backgroundColor',   function (){ return rt.backgroundColor; },  function ( v ){ rt.backgroundColor = v; } ],
		[ 'color',   function (){ return rt.color; },  function ( v ){ fr.color = rt.color = v; } ],
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
		Object.defineProperty( gameObject, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ]
		} );
	}

	// focus changed
	ui.focusChanged = function ( newFocus ) {
		// show caret
	    if ( newFocus == ui ) {
	        rt.showCaret = true;
		    rt.showSelection = selectable;
		    rt.formatting = false;
		    focusRect.active = true;
		    resetText = rt.text;
	    // hide
	    } else {
		    gameObject.scrollCaretToView();
	        rt.showCaret = rt.showSelection = ui.dragSelect = false;
		    rt.formatting = formatting;
		    rt.scrollLeft = 0;
		    focusRect.active = false;
	    }
	}

	// scrolling
	ui.mouseWheel = function ( wy, wx ) {
		// ignore if not focused
		if ( !rt.multiLine && !ui.focused ) return;

		wx = wx < 0 ? -1 : 1; wy = wy < 0 ? -1 : 1;
		// scroll vertically
		if ( wy != 0 && rt.scrollHeight > rt.height ) {
			rt.scrollTop =
				Math.max( 0, Math.min( rt.scrollHeight - rt.height, rt.scrollTop - wy * rt.lineHeight ) );
		}
		// scroll horizontally
		if ( wx != 0 && rt.scrollWidth > rt.width ){
			rt.scrollLeft =
				Math.max( 0, Math.min( rt.scrollWidth - rt.width, rt.scrollLeft + 0.1 * wx * rt.lineHeight ) );
		}

		// stop other scroll handlers
		stopEvent();
	}

	// down
	ui.mouseDown = function ( btn, x, y ) {
		//
		if ( disabled ) return;
		// shift + down
		if ( ( input.get( KEY_LSHIFT ) || input.get( KEY_RSHIFT ) ) && selectable ) {
			rt.selectionStart = rt.caretPosition;
			rt.selectionEnd = rt.caretPositionAt( x, y );
        } else {
			// set cursor
		    rt.selectionStart = rt.selectionEnd = rt.caretPosition = rt.caretPositionAt( x, y );
		}
		// focus
	    ui.focus();

		// start selection drag
		if ( selectable ) {
			ui.dragSelect = true;
			input.on( 'mouseMove', ui.mouseMoveGlobal );
		}

		// stop other event handlers
		stopEvent();
	}

	// release
	ui.mouseUp = ui.mouseUpOutside = function () {
		ui.dragSelect = null;
		input.off( 'mouseMove', ui.mouseMoveGlobal );
	};

	// mouse move while holding down
	ui.mouseMoveGlobal = function ( x, y ) {
		// dragging selection
		if ( ui.dragSelect ) {
			var p = gameObject.globalToLocal( x, y );
			var selEnd = rt.caretPositionAt( p.x, p.y );
			if ( selEnd >= 0 ) rt.selectionEnd = selEnd;

			// stop other event handlers
			stopEvent();
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
		// input
	    } else {
			// selection
			if ( replacingSelection ) {
				rt.text = rt.text.substr( 0, selStartIndex ) + key + rt.text.substr( selEndIndex );
				caretPosition = rt.selectionStart + 1;
			} else {
				rt.text = rt.text.substr( 0, caretIndex ) + key + rt.text.substr( caretIndex );
				caretPosition++;
			}
	    }
		// update caret position
	    rt.caretPosition = caretPosition;
		rt.selectionEnd = rt.selectionStart;

		// make sure caret is in view
		gameObject.scrollCaretToView();

		// if multi-line text box, dispatch change event as user types
		if ( rt.multiLine ) gameObject.fire( 'change', rt.text );

	}

	ui.keyDown = function ( code, shift, ctrl, alt ) {
		// ready
		ui.dragSelect = false;
		var caretPosition = rt.caretPosition;
		var haveSelection = ( rt.selectionStart != rt.selectionEnd );
		if ( haveSelection && rt.selectionStart > rt.selectionEnd ) {
			var tmp = rt.selectionStart;
			rt.selectionStart = rt.selectionEnd;
			rt.selectionEnd = tmp;
		}
		// key
	    switch ( code ) {
		    case KEY_TAB:
			    if ( tabEnabled ) {
			        ui.keyPress( "\t" );
	            } else {
			        ui.navigate( shift ? -1 : 1 );
			    }
				break;

	        case KEY_RETURN:
		        if ( rt.multiLine ) {
			        ui.keyPress( "\n" );
		        } else {
			        if ( rt.text != resetText ) gameObject.fire( 'change', rt.text );
			        ui.blur();
		        }
	            break;

	        case KEY_BACKSPACE:
	            ui.keyPress( -1, -1 );
	            break;

	        case KEY_DELETE:
	            ui.keyPress( -1, 1 );
	            break;

	        case KEY_LEFT:
			case KEY_RIGHT:
				// expand selection
				if ( shift && selectable ) {
					if ( code == KEY_LEFT && caretPosition > 0 ) {
						if ( !haveSelection ) rt.selectionEnd = caretPosition;
						rt.caretPosition = rt.selectionStart = caretPosition - 1;
					} else if ( code == KEY_RIGHT && caretPosition < rt.text.positionLength() ){
						if ( !haveSelection ) rt.selectionStart = caretPosition;
						rt.selectionEnd = ++rt.caretPosition;
					}
				// move caret
				} else {
					if ( !haveSelection ) rt.caretPosition += ( code == KEY_LEFT ? -1 : 1 );
					rt.selectionStart = rt.selectionEnd;
				}
				gameObject.scrollCaretToView();
	            break;

			case KEY_UP:
			case KEY_DOWN:
				// starting selection
				if ( shift && selectable ) {
					if ( !haveSelection ) rt.selectionEnd = rt.selectionStart = caretPosition;
				// move caret - clear selection
				} else {
					rt.selectionStart = rt.selectionEnd;
				}
				// common
				if ( rt.caretLine == 0 && code == KEY_UP ) {
					rt.caretPosition = 0;
				} else if ( rt.caretLine >= rt.numLines - 1 && code == KEY_DOWN ) {
					rt.caretPosition = rt.text.positionLength();
				} else {
					rt.caretPosition = rt.caretPositionAt ( rt.caretX, rt.caretY + ( code == KEY_UP ? -1 : 1 ) * rt.lineHeight );
				}
				// selecting
				if ( shift && selectable ) {
					if ( code == KEY_UP ) rt.selectionStart = rt.caretPosition;
					else rt.selectionEnd = rt.caretPosition;
				}
				gameObject.scrollCaretToView();
				break;

		    case KEY_ESCAPE:
		        // if single line text field, reset text
		        if ( !rt.multiLine ) rt.text = resetText;
	            ui.blur();
	            break;
	    }

	}

	// makes sure caret is inside visible area
	gameObject.scrollCaretToView = function (){
		// vertical
		if ( rt.caretY < rt.scrollTop ) rt.scrollTop = rt.caretY;
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
	
})( this );







