//

this.render = new RenderText();
this.render.maxWidth = 150;
this.render.wrap = false;
this.render.maxLines = 4;
//this.render.align = 0;
//this.render.centered = true;

Object.defineProperty( this, "text", {
get: function (){ return this.text; }.bind( this.render ),
set: function ( val ) { this.text = val; this.caretPosition = this.selectionStart = this.selectionEnd = 0; }.bind( this.render )
} );

Object.defineProperty( this, "width", {
get: function (){ return this.maxWidth; }.bind( this.render ),
set: function ( val ) { this.maxWidth = val; }.bind( this.render )
} );


this.ui = new UI();
this.ui.focusChanged = function ( newFocus ) {
    if ( newFocus == this ) {
        this.gameObject.render.showCaret = true;
		this.gameObject.render.showSelection = true;
    } else {
        this.gameObject.render.showCaret = false;
		this.gameObject.render.showSelection = false;
		this.dragSelect = false;
    }
}

this.ui.mouseWheel = function ( wy, wx ) {
	var render = this.gameObject.render;
	wx = wx < 0 ? -1 : 1; wy = wy < 0 ? -1 : 1;
	if ( wy != 0 && render.numLines > render.maxLines ) {
		render.lineOffset =
		Math.max( 0, Math.min( render.numLines - render.maxLines, render.lineOffset - wy ) );
	}
	if ( wx != 0 && render.width > render.maxWidth ){
		log ( render.width, render.maxWidth, render.horizontalOffset, wx );
		render.horizontalOffset =
		Math.max( 0, Math.min( render.width - render.maxWidth, render.horizontalOffset + 0.1 * wx * render.size ) );
	}
}

this.ui.mouseDown = function ( btn, x, y ) {
	// set cursor
    this.gameObject.render.selectionStart =
		this.gameObject.render.selectionEnd =
		this.gameObject.render.caretPosition =
		this.gameObject.render.caretPositionAt( x, y );
	
	// focus
    this.focus();
	
	// start selection drag
	this.dragSelect = true;
	input.on( 'mouseMove', this.mouseMoveGlobal );
}

this.ui.mouseUp = this.ui.mouseUpOutside = function () {
	this.dragSelect = null;
	input.off( 'mouseMove', this.mouseMoveGlobal );
};

this.ui.mouseMoveGlobal = function ( x, y ) {
	// dragging
	if ( this.dragSelect ) {
		var render = this.gameObject.render;
		var p = this.gameObject.globalToLocal( x, y );
		var selEnd = this.gameObject.render.caretPositionAt( p.x, p.y );
		if ( selEnd >= 0 ) render.selectionEnd = selEnd;
	}
}.bind( this.ui );

this.ui.keyPress = function ( key, direction ) {
	// get ready
	var render = this.gameObject.render;
	var caretPosition = render.caretPosition;
    var caretIndex = render.text.positionToIndex( caretPosition );
	var replacingSelection = ( render.selectionStart != render.selectionEnd );
	if ( replacingSelection && render.selectionStart > render.selectionEnd ) {
		var tmp = render.selectionStart;
		render.selectionStart = render.selectionEnd;
		render.selectionEnd = tmp;
	}
	var selStartIndex = render.text.positionToIndex( render.selectionStart );
	var selEndIndex = render.text.positionToIndex( render.selectionEnd );
	// non-input
    if ( key == -1 ) {
		
		// delete selection
		if ( replacingSelection ) {
			render.text = render.text.substr( 0, selStartIndex ) + render.text.substr( selEndIndex );
			caretPosition = render.selectionStart;
		}
		
		// backspace
		else if ( direction < 0 ) {
			if ( caretPosition <= 0 || caretIndex <= 0 ) return;
			var prevCaretIndex = render.text.positionToIndex( caretPosition - 1 );
			render.text = render.text.substr( 0, prevCaretIndex ) + render.text.substr( caretIndex );
			caretPosition--;
		}
	// input
    } else {
		// selection
		if ( replacingSelection ) {
			render.text = render.text.substr( 0, selStartIndex ) + key + render.text.substr( selEndIndex );
			caretPosition = render.selectionStart + 1;
		} else {
			render.text = render.text.substr( 0, caretIndex ) + key + render.text.substr( caretIndex );
			caretPosition++;
		}
    }
	// update caret position
    render.caretPosition = caretPosition;
	render.selectionEnd = render.selectionStart;
	
	// make sure caret is in view
	this.autoScroll();
}

this.ui.keyDown = function ( code, shift, ctrl, alt ) {
	this.dragSelect = false;
	var render = this.gameObject.render;
	var caretPosition = render.caretPosition;
	var caretIndex = render.text.positionToIndex( caretPosition );
	var haveSelection = ( render.selectionStart != render.selectionEnd );
	if ( haveSelection && render.selectionStart > render.selectionEnd ) {
		var tmp = render.selectionStart;
		render.selectionStart = render.selectionEnd;
		render.selectionEnd = tmp;
	}
	
    switch ( code ) {
		case KEY_TAB:
			this.keyPress( "\t" );
			break;
			
        case KEY_RETURN:
            this.keyPress( "\n" );
            break;
			
        case KEY_BACKSPACE:
            this.keyPress( -1, -1 );
            break;
			
        case KEY_DELETE:
            this.keyPress( -1, 1 );
            break;
			
        case KEY_LEFT:
		case KEY_RIGHT:
			// expand selection
			if ( shift ) {
				if ( code == KEY_LEFT && caretPosition > 0 ) {
					if ( !haveSelection ) render.selectionEnd = caretPosition;
					render.caretPosition = render.selectionStart = caretPosition - 1;
				} else if ( code == KEY_RIGHT && caretPosition < render.text.positionLength() ){
					if ( !haveSelection ) render.selectionStart = caretPosition;
					render.selectionEnd = ++render.caretPosition;
				}
			// move caret
			} else {
				if ( !haveSelection ) render.caretPosition += ( code == KEY_LEFT ? -1 : 1 );
				render.selectionStart = render.selectionEnd;
			}
			this.autoScroll();
            break;
			
		case KEY_UP:
		case KEY_DOWN:
			// starting selection
			if ( shift ) {
				if ( !haveSelection ) render.selectionEnd = render.selectionStart = caretPosition;
			// move caret - clear selection
			} else {
				render.selectionStart = render.selectionEnd;
			}
			// common
			if ( render.caretLine == 0 && code == KEY_UP ) {
				render.caretPosition = 0;
			} else if ( render.caretLine >= render.numLines - 1 && code == KEY_DOWN ) {
				render.caretPosition = render.text.positionLength();
			} else {
				render.caretPosition = render.caretPositionAt ( render.caretX, render.caretY + ( code == KEY_UP ? -1 : 1 ) * render.lineHeight );
			}
			// selecting
			if ( shift ) {
				if ( code == KEY_UP ) render.selectionStart = render.caretPosition;
				else render.selectionEnd = render.caretPosition;
			}
			this.autoScroll();
			break;

        case KEY_ESCAPE:
            this.blur();
            break;
    }
    
}

this.ui.autoScroll = function (){
	var render = this.gameObject.render;
	
	// vertical
	if ( render.caretLine < render.lineOffset ) render.lineOffset = render.caretLine;
	else if ( render.caretLine > render.lineOffset + render.maxLines - 1 || render.caretLine < render.numLines - 1 ) {
		render.lineOffset = Math.max( 0, Math.min( render.numLines - render.maxLines,
												 render.caretLine - render.maxLines + 1 ) );
	}
	
	// horizontal
	if ( render.caretX < 0 ) {
		render.horizontalOffset += render.caretX;
	} else if ( render.caretX > render.maxWidth ){
		render.horizontalOffset += render.caretX - render.maxWidth;
	}
}
