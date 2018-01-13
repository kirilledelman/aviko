#include "RenderTextBehavior.hpp"
#include "FontResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderTextBehavior::RenderTextBehavior( ScriptArguments* args ) : RenderTextBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RenderTextBehavior>( this );
	
	// add defaults
	RenderBehavior::AddDefaults();
	
	// create background object
	Color* color = new Color( NULL );
	color->SetInts( 0, 0, 0, 0 );
	script.SetProperty( "backgroundColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create text color object
	color = new Color( NULL );
	script.SetProperty( "textColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create text selection color object
	color = new Color( NULL );
	color->SetInt( 0x0070B0, false );
	script.SetProperty( "selectionColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create ^0 - ^9 colors
	int defaultColors[ 10 ] = { 0x0, 0x3333FF, 0xFF3333, 0xFF33FF, 0x33FF33, 0x33FFFF, 0xFFFF33, 0xFFFFFF, 0xAAAAAA, 0x666666 };
	for ( int i = 0; i < 10; i++ ) {
		colors[ i ] = new Color( NULL );
		colors[ i ]->SetInt( defaultColors[ i ], false );
		static char clrProp[6];
		sprintf( clrProp, "color%d", i ) ;
		script.SetProperty( clrProp, ArgValue( colors[ i ]->scriptObject ), this->scriptObject );
	}
	
	// with arguments
	if ( args ) {
		/// load font
		string fname = app.defaultFontName, txt = "";
		int size = this->fontSize;
		if ( args->ReadArguments( 1, TypeString, &txt, TypeString, &fname, TypeInt, &size ) ) {
			this->text = txt;
			SetFont( fname.c_str(), size );
			return;
		}
	}
	// default
	SetFont( app.defaultFontName.c_str(), this->fontSize );
	
	// set default width, height
	this->width = this->fontSize * 10;
	if ( this->fontResource && this->fontResource->font ) {
		this->height = TTF_FontLineSkip( this->fontResource->font ) + this->lineSpacing;
	}
	
}

// init
RenderTextBehavior::RenderTextBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &RenderTextBehavior::Render );
	
	// can render
	this->isRenderBehavior = true;
	
}

/// destructor
RenderTextBehavior::~RenderTextBehavior() {
	
	// release surface
	if ( this->surface ) {
		GPU_FreeImage( this->surface );
		this->surface = NULL;
	}
	
	// release resource
	if ( this->fontResource ) this->fontResource->AdjustUseCount( -1 );
	
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void RenderTextBehavior::InitClass() {
	
	// register class
	script.RegisterClass<RenderTextBehavior>( "RenderBehavior" );
	
	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "TextAlign", constants );
	script.SetProperty( "Left", ArgValue( 0 ), constants );
	script.SetProperty( "Right", ArgValue( 1 ), constants );
	script.SetProperty( "Center", ArgValue( 2 ), constants );
	script.FreezeObject( constants );
	
	// properties
	
	script.AddProperty<RenderTextBehavior>
	( "backgroundColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		return ArgValue(((RenderTextBehavior*) b)->backgroundColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->backgroundColor = other;
		} else {
			rs->backgroundColor->Set( val );
		}
		rs->_dirty = true;
		return rs->backgroundColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "textColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		return ArgValue(((RenderTextBehavior*) b)->textColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->textColor = other;
		} else {
			rs->textColor->Set( val );
		}
		rs->_dirty = true;
		return rs->textColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "selectionColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		return ArgValue(((RenderTextBehavior*) b)->selectionColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->selectionColor = other;
		} else {
			rs->selectionColor->Set( val );
		}
		rs->_dirty = true;
		return rs->selectionColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color0",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 0 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 0 ] = other;
		} else {
			rs->colors[ 0 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 0 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color1",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 1 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 1 ] = other;
		} else {
			rs->colors[ 1 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 1 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color2",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 2 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 2 ] = other;
		} else {
			rs->colors[ 2 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 2 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color3",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 3 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 3 ] = other;
		} else {
			rs->colors[ 3 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 3 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color4",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 4 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 4 ] = other;
		} else {
			rs->colors[ 4 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 4 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color5",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 5 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 5 ] = other;
		} else {
			rs->colors[ 5 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 5 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color6",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 6 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 6 ] = other;
		} else {
			rs->colors[ 6 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 6 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color7",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 7 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 7 ] = other;
		} else {
			rs->colors[ 7 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 7 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color8",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 8 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 8 ] = other;
		} else {
			rs->colors[ 8 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 8 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "color9",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b; rs->_dirty = true;
		return ArgValue(rs->colors[ 9 ]->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->colors[ 9 ] = other;
		} else {
			rs->colors[ 9 ]->Set( val );
		}
		rs->_dirty = true;
		return rs->colors[ 9 ]->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "font",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		return ArgValue( ( (RenderTextBehavior*) b )->fontName.c_str() );
	 } ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		// if clearing
		if ( val.isNull() || val.type != TypeString ) {
			// clear previous
			if ( rs->fontResource ) rs->fontResource->AdjustUseCount( -1 );
			rs->fontResource = NULL;
			rs->fontName = "";
		} else if ( val.type == TypeString ) {
			rs->SetFont( val.value.stringValue->c_str(), rs->fontSize );
		}
		return val;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "size", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->fontSize; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->SetFont( rs->fontName.c_str(), val );
		return rs->fontSize;
	 }));
	
	script.AddProperty<RenderTextBehavior>
	( "outline", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->outlineWidth; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->outlineWidth = max( 0, min( 16, val ) );
		rs->ClearGlyphs();
		rs->_dirty = true;
		return rs->outlineWidth;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "text", //
	 static_cast<ScriptStringCallback>([](void *b, string val ){ return ((RenderTextBehavior*) b)->text; }),
	 static_cast<ScriptStringCallback>([](void *b, string val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		if ( rs->text.compare( val ) != 0 ) {
			rs->text = val;
			rs->_dirty = true;
		}
		return val;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "width",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->width; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->width = max( 0, val );
		rs->_dirty = true;
		return rs->width;
	 }));
	 
	script.AddProperty<RenderTextBehavior>
	( "height",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->height; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->height = max( 0, val );
		rs->_dirty = true;
		return rs->height;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "dirty",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->_dirty; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->_dirty = val;
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "bold",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->bold; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->bold = val;
		rs->_dirty = true;
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "italic",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->italic; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->italic = val;
		rs->_dirty = true;
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "antialias",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->antialias; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->antialias = val;
		rs->ClearGlyphs();
		rs->_dirty = true;
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "align", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->align; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->align = max( 0, min( 2, val ) );
		rs->_dirty = true;
		return rs->align;
	}));

    script.AddProperty<RenderTextBehavior>
    ( "wrap",
     static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->wrap; }),
     static_cast<ScriptBoolCallback>([](void *b, bool val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->wrap = val;
        rs->_dirty = true;
        return val;
    }) );
	
	script.AddProperty<RenderTextBehavior>
	( "multiLine",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->multiLine; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->multiLine = val;
		rs->_dirty = true;
		return val;
	}) );

    script.AddProperty<RenderTextBehavior>
    ( "scrollLeft",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->scrollLeft; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->scrollLeft = val;
        rs->_dirty = true;
        return val;
    }) );
	
	script.AddProperty<RenderTextBehavior>
	( "scrollTop",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->scrollTop; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->scrollTop = val;
		rs->_dirty = true;
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "scrollWidth",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->scrollWidth; }));
	 
	script.AddProperty<RenderTextBehavior>
	( "scrollHeight",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->scrollHeight; }));
	
	script.AddProperty<RenderTextBehavior>
	( "numLines",
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( rs->_dirty ) rs->Repaint();
		return rs->lines.size();
	 }));
	
	script.AddProperty<RenderTextBehavior>
	( "characterSpacing",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderTextBehavior*) b)->characterSpacing; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->characterSpacing = val;
		rs->_dirty = true;
		return rs->characterSpacing;
	 }) );

	script.AddProperty<RenderTextBehavior>
	( "lineSpacing",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderTextBehavior*) b)->lineSpacing; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->lineSpacing = val;
		rs->_dirty = true;
		return rs->lineSpacing;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "lineHeight",
	 static_cast<ScriptIntCallback>([](void* b, int val ) {
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		if ( !rs->fontResource || !rs->fontResource->font ) return 0;
		return (int) ceil( TTF_FontLineSkip( rs->fontResource->font ) + rs->lineSpacing );
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "formatting",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->formatting; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->formatting = val;
		rs->_dirty = true;
		return rs->formatting;
	}) );
	
    script.AddProperty<RenderTextBehavior>
    ( "showCaret",
     static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->showCaret; }),
     static_cast<ScriptBoolCallback>([](void *b, bool val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->showCaret = val;
        rs->_dirty = true;
        return val;
    }) );
    
    script.AddProperty<RenderTextBehavior>
    ( "showSelection",
     static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->showSelection; }),
     static_cast<ScriptBoolCallback>([](void *b, bool val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->showSelection = val;
        rs->_dirty = true;
        return val;
    }) );
    
    script.AddProperty<RenderTextBehavior>
    ( "caretPosition",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->caretPosition; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->caretPosition = max( 0, min( StringPositionLength( rs->text.c_str() ), val ) );
        rs->_dirty = true;
        return val;
    }) );

	script.AddProperty<RenderTextBehavior>
	( "caretLine",
	 static_cast<ScriptIntCallback>([](void* b, int val ) {
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		if ( rs->_dirty ) rs->Repaint();
		return rs->caretLine;
	}));

	script.AddProperty<RenderTextBehavior>
	( "caretX",
	 static_cast<ScriptFloatCallback>([](void* b, float val ) {
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		if ( rs->_dirty ) rs->Repaint();
		return rs->caretX;
	}));

	script.AddProperty<RenderTextBehavior>
	( "caretY",
	 static_cast<ScriptFloatCallback>([](void* b, float val ) {
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		if ( rs->_dirty ) rs->Repaint();
		return rs->caretY;
	}));
	
    script.AddProperty<RenderTextBehavior>
    ( "selectionStart",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->selectionStart; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->selectionStart = max( 0, val );
        rs->_dirty = true;
        return val;
    }) );
    
    script.AddProperty<RenderTextBehavior>
    ( "selectionEnd",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->selectionEnd; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        RenderTextBehavior* rs = ((RenderTextBehavior*) b);
        rs->selectionEnd = max( 0, val );
        rs->_dirty = true;
        return val;
    }) );
	
	// functions
	
    script.DefineFunction<RenderTextBehavior>
    ( "caretPositionAt", //
     static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
        RenderTextBehavior* rs = (RenderTextBehavior*) obj;
        float x = 0, y = 0;
        if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ){
            script.ReportError( "usage: caretPositionAt( Float localX, Float localY )" );
            return false;
        }
        sa.ReturnInt( rs->GetCaretPositionAt( x, y ) );
        return true;
    }));
    
	script.DefineFunction<RenderTextBehavior>
	( "repaint", //
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		RenderTextBehavior* rs = (RenderTextBehavior*) obj;
		rs->Repaint();
		return true;
	}));
}


/* MARK:	-				Font
 -------------------------------------------------------------------- */


/// clears prerendered glyphs on font change
void RenderTextBehavior::ClearGlyphs() {
	this->glyphs[ 0 ].clear();
	this->glyphs[ 1 ].clear();
	this->glyphs[ 2 ].clear();
	this->glyphs[ 3 ].clear();
	this->lines.clear();
	this->width = this->height = 0;
}

/// sets font
bool RenderTextBehavior::SetFont( const char* face, int size ) {

	// trim
	size = max( 1, min( 512, size ) );
	
	// load font
	FontResource* fnt = NULL;
	if ( face != NULL ) {
		static char buf[ 128 ];
		sprintf( buf, "%s,%d", face, size );
		fnt = app.fontManager.Get( buf );
		// make sure it exists
		if ( fnt->error == ERROR_NONE ) {
			fnt->AdjustUseCount( 1 );
		} else return false;
	}

	// clear previous
	if ( this->fontResource ) this->fontResource->AdjustUseCount( -1 );
	
	// set new
	this->fontResource = fnt;
	this->fontName = face;
	this->fontSize = size;
	this->_dirty = true;
	this->ClearGlyphs();
	TTF_SetFontKerning( fnt->font, 1 );
	return true;
	
}


/* MARK:	-				UI
 -------------------------------------------------------------------- */


// returns sprite bounding box
GPU_Rect RenderTextBehavior::GetBounds() {
	GPU_Rect rect;
	rect.w = this->width;
	rect.h = this->height;
	rect.x = -this->width * this->pivotX;
	rect.y = -this->height * this->pivotY;
	return rect;
}

int RenderTextBehavior::GetCaretPositionAt( float localX, float localY ) {

    // update
    if ( this->_dirty ) Repaint();
	
	// adjust coord
	localX += this->width * pivotX;
	localY += this->height * pivotY;
	
    // locate character
	int nc = 0;
    size_t lineIndex = 0;
	size_t totalLines = lines.size();
    int lineHeight = ceil( TTF_FontLineSkip( this->fontResource->font ) + this->lineSpacing );
    RenderTextLine* currentLine = NULL;
    while ( lineIndex < totalLines ) {
        currentLine = &this->lines[ lineIndex ];
        // on line
        if ( currentLine->y <= localY && currentLine->y + lineHeight > localY ) {
			nc = (int) currentLine->characters.size();
			// if line is blank
			if ( nc == 0 || !currentLine->characters[ 0 ].value ) {
				return currentLine->firstCharacterPos;
			} else {
				// if before current line
				if ( currentLine->x > localX ) {
					// return before first char
					return (int) currentLine->characters[ 0 ].pos;
				// after current line
				} else if( localX > currentLine->x + currentLine->width ) {
					return (int) currentLine->characters[ nc - 1 ].pos + 1;
				}
				// check each character
				for( size_t i = 0; i < nc; i++ ){
					RenderTextCharacter* character = &currentLine->characters[ i ];
					float x0 = character->x + currentLine->x;
					float x1 = x0 + character->width;
					float w = ( x1 - x0 ) * 0.5;
					if ( localX >= x0 && localX <= x1 ) {
						return ( localX < x0 + w ? (int) character->pos : (int) ( character->pos + 1 ) );
					}
				}
			}
        }
        lineIndex++;
    }
    return -1;
}



/* MARK:	-				Render
 -------------------------------------------------------------------- */


RenderTextBehavior::GlyphInfo* RenderTextBehavior::GetGlyph(Uint16 c, bool b, bool i) {
	
	// if there's no such glyph, return placeholder
	if ( c >= 32 && !TTF_GlyphIsProvided( this->fontResource->font, c ) ) return GetGlyph( '?', b, i );

	// try to find it first
	size_t style = ( b ? TTF_STYLE_BOLD : 0 ) | ( i ? TTF_STYLE_ITALIC : 0 );
	unordered_map<Uint16, GlyphInfo>::iterator it = this->glyphs[ style ].find( c );
	GlyphInfo* gi = ( it == glyphs[ style ].end() ? NULL : &it->second );
	if ( gi ) return gi;
	
	// create in place
	gi = &this->glyphs[ style ][ c ];
	
	// if printable character
	if ( c >= 32 && TTF_GlyphMetrics( this->fontResource->font, c, &gi->minX, &gi->maxX, &gi->minY, &gi->maxY, &gi->advance ) == 0 ) {
		// otherwise, render glyph
		TTF_SetFontStyle( this->fontResource->font, (int) style );
		TTF_SetFontOutline( this->fontResource->font, this->outlineWidth );
		TTF_SetFontHinting( this->fontResource->font, TTF_HINTING_LIGHT );
		static SDL_Color white = { 255, 255, 255, 255 };
		gi->surface = antialias ? TTF_RenderGlyph_Blended( this->fontResource->font, c, white ) :
								  TTF_RenderGlyph_Solid( this->fontResource->font, c, white );
	}
	
	// done
	return gi;
}

/// redraws surface
void RenderTextBehavior::Repaint() {
	
	this->_dirty = false;
	
	// clear old image
	if ( this->surface ) {
		GPU_FreeImage( this->surface );
		this->surface = NULL;
	}
	
	if ( this->fontResource ) {
		// redraw all lines
		SDL_Surface* textSurface = NULL;
		this->lines.clear();
		RenderTextLine* currentLine = NULL;
		
		// for each character
		const unsigned char *current = (unsigned char*) this->text.c_str();
		Uint16 character = 0;
		size_t characterPos = 0;
		SDL_Color currentColor = this->textColor->rgba;
		scrollWidth = scrollHeight = 0;
		bool currentBold = this->bold;
		bool currentItalic = this->italic;
		
		// for tabbing
		GlyphInfo* spaceGlyph = GetGlyph( ' ', false, false );
		float tabWidth = spaceGlyph->advance * this->tabSpaces;
		
		// start new line
		lines.emplace_back();
		currentLine = &lines.back();
		while ( *current != 0 ) {
			// decode utf-8
			if ( (*current & 0x80) != 0 ) {
				if ( (*current & 0xE0) == 0xC0 ) {
					character = (*current & 0x1F) << 6;
					character |= (*(current + 1) & 0x3F);
					current += 1;
				} else if ( (*current & 0xF0) == 0xE0 ) {
					character = (*current & 0xF) << 12;
					character |= (*(current + 1) & 0x3F) << 6;
					character |= (*(current + 2) & 0x3F);
					current += 2;
				} else if ( (*current & 0xF8) == 0xF0 ) {
					character = (*current & 0x7) << 18;
					character |= (*(current + 1) & 0x3F) << 12;
					character |= (*(current + 2) & 0x3F) << 6;
					character |= (*(current + 3) & 0x3F);
					current += 3;
				} else if ( (*current & 0xFC) == 0xF8 ) {
					character = (*current & 0x3) << 24;
					character |= (*(current + 1) & 0x3F) << 18;
					character |= (*(current + 2) & 0x3F) << 12;
					character |= (*(current + 3) & 0x3F) << 6;
					character |= (*(current + 4) & 0x3F);
					current += 4;
				} else if ( (*current & 0xFE) == 0xFC ) {
					character = (*current & 0x1) << 30;
					character |= (*(current + 1) & 0x3F) << 24;
					character |= (*(current + 2) & 0x3F) << 18;
					character |= (*(current + 3) & 0x3F) << 12;
					character |= (*(current + 4) & 0x3F) << 6;
					character |= (*(current + 5) & 0x3F);
					current += 5;
				}
			// ascii
			} else {
				// just use value
				character = *current;
			}
			
			// get previous character
			RenderTextCharacter* previousCharacter = NULL;
			if ( currentLine->characters.size() ) previousCharacter = &currentLine->characters.back();
			
			// if character is newline
			if ( character == '\n' && this->multiLine ) {
				
				// start new line
				lines.emplace_back();
				currentLine = &lines.back();
				currentLine->firstCharacterPos = (int) characterPos;
				previousCharacter = NULL;
				
			// character is special sequence
			} else if ( character == '^' && this->formatting ){
				
				// check next character
				Uint16 nextChar = *(current + 1);
				bool skipTwo = false, skipOne = false;
				
				// code
				if ( nextChar == 'B' ) {
					currentBold = true;
					skipTwo = true;
				} else if ( nextChar == 'I' ) {
					currentItalic = true;
					skipTwo = true;
				} else if ( nextChar == 'b' ) {
					currentBold = false;
					skipTwo = true;
				} else if ( nextChar == 'i' ) {
					currentItalic = false;
					skipTwo = true;
				} else if ( nextChar == 'n' || nextChar == 'N' ) {
					currentItalic = currentBold = false;
					skipTwo = true;
				} else if ( nextChar == 'c' ) {
					currentColor = this->textColor->rgba;
					skipTwo = true;
				} else if ( nextChar >= '0' && nextChar <= '9' ) {
					currentColor = this->colors[ nextChar - '0' ]->rgba;
					skipTwo = true;
				} else if ( nextChar == '^' ) {
					skipOne = true;
				}
				
				// constrol code accepted
				RenderTextCharacter empty;
				if ( skipOne || skipTwo ) {
					empty.x = previousCharacter ?
						( previousCharacter->x +
						 previousCharacter->glyphInfo->advance + this->characterSpacing ) : 0;
					empty.value = character;
					empty.glyphInfo = GetGlyph( '\n', currentBold, currentItalic );
					empty.pos = characterPos;
					currentLine->characters.push_back( empty );
					current++;
					characterPos++;
					
				}
				if ( skipTwo ) {
					empty.value = nextChar;
					empty.pos = characterPos + 1;
					empty.color = currentColor;
					currentLine->characters.push_back( empty );
					current++;
					characterPos++;
					continue;
				}
			}
			
			// get glyph
			GlyphInfo* glyph = GetGlyph( character, currentBold, currentItalic );
			
			// check if line width will exceed max line width
			if ( currentLine->width + glyph->advance >= this->width && this->wrap && this->multiLine ) {
				// start new line
				lines.emplace_back();
				currentLine = &lines.back();
				currentLine->firstCharacterPos = (int) characterPos;
				previousCharacter = NULL;
			}
			
			// add new character
			RenderTextCharacter thisCharacter;
			thisCharacter.color = currentColor;
			thisCharacter.glyphInfo = glyph;
			thisCharacter.value = character;
			thisCharacter.width = glyph->advance + this->characterSpacing;
			thisCharacter.pos = characterPos;
			if ( previousCharacter ) {
				thisCharacter.x = glyph->minX + previousCharacter->x + previousCharacter->width;
			} else {
				thisCharacter.x = glyph->minX;
			}
			
			// if tab, jump to next pos
			if ( character == '\t' ){
				thisCharacter.width = tabWidth - fmod( currentLine->width, tabWidth );
			}
			
			// adjust line width
			currentLine->width = thisCharacter.x + thisCharacter.width;
			
			// measure
			this->scrollWidth = max( this->scrollWidth, (int) currentLine->width );
			
			// advance
			currentLine->characters.push_back( thisCharacter );
			current++;
			characterPos++;
			
		}
		
		// set up
		characterPos = 0;
		int lineHeight = ceil( TTF_FontLineSkip( this->fontResource->font ) + this->lineSpacing );
		size_t lineIndex = 0;
		size_t totalLines = (int) lines.size();
		this->scrollHeight = (int) totalLines * lineHeight;
		float x = 0, y = -scrollTop;
		SDL_Rect rect;
		int selStart = this->selectionStart;
		int selEnd = this->selectionEnd;
		if ( selStart > selEnd ) { int tmp = selStart; selStart = selEnd; selEnd = tmp; }
		
		// create surface
		if ( width <= 0 || height <= 0 ) return;
		textSurface = SDL_CreateRGBSurfaceWithFormat( 0, width, height, 32, SDL_PIXELFORMAT_RGBA32 );
		if ( !textSurface ) return;
		
		SDL_Rect temp = { 0, 0, width, height };
		Uint32 bgColorVal = SDL_MapRGBA( textSurface->format, backgroundColor->rgba.r, backgroundColor->rgba.g, backgroundColor->rgba.b, backgroundColor->rgba.a );
		SDL_FillRect( textSurface, &temp, bgColorVal );
		
		// for each line
		SDL_Color selColor = this->selectionColor->rgba;
		Uint32 selColorVal = SDL_MapRGBA( textSurface->format, selColor.r, selColor.g, selColor.b, selColor.a );
		while ( lineIndex < totalLines ) {
			currentLine = &this->lines[ lineIndex ];
            
			// left aligned?
			if ( this->align == 0 ) {
				x = 0;
			// right aligned?
			} else if ( this->align == 1 ) {
				x = width - currentLine->width - ( this->showCaret ? 2 : 0 );
			// center
			} else {
				x = floor( width - currentLine->width ) * 0.5;
			}
			
            x -= scrollLeft;
            currentLine->x = x;
            currentLine->y = y;
			
			// if line is empty, and we need to draw caret, add empty character
			if ( showCaret && currentLine->characters.size() == 0 ){
				RenderTextCharacter thisCharacter;
				thisCharacter.glyphInfo = GetGlyph( '\n', false, false );
				thisCharacter.value = 0;
				thisCharacter.pos = this->caretPosition;
				thisCharacter.x = 0;
				thisCharacter.color = currentColor;
				currentLine->characters.push_back( thisCharacter );
			}
			
			bool drawCurrentLine = ( y >= -lineHeight && y < height );
			
			// for each character
			for ( size_t i = 0, nc = currentLine->characters.size(); i < nc; i++ ) {
				RenderTextCharacter* character = &currentLine->characters[ i ];
                
                // coords
                rect.x = x + character->x;
                rect.y = y;

				// selection
				if ( this->showSelection && drawCurrentLine && selStart != selEnd && characterPos >= selStart && characterPos < selEnd ) {
					SDL_Rect selRect;
					selRect.x = rect.x;
					selRect.y = y - 1;
					selRect.h = lineHeight + 1;
					if ( i < nc - 1 ) {
						selRect.w = currentLine->characters[ i + 1 ].x - character->x;
					} else {
						selRect.w = character->width;
					}
					
					SDL_FillRect( textSurface, &selRect, selColorVal );
				}
				
                // caret
				bool drawCaret = false;
				SDL_Rect caretRect;
				if ( character->pos == this->caretPosition - 1 ) {
					// after current character?
					caretRect.x = x + character->x + character->width;
					caretX = caretRect.x - width * pivotX;
					caretY = y - height * pivotY;
					caretLine = (int) lineIndex;
					drawCaret = this->showCaret && drawCurrentLine;
				} else if ( character->pos == 0 && this->caretPosition == 0 && lineIndex == 0 ) {
					caretRect.x = x + character->x;
					caretX = caretRect.x - width * pivotX;
					caretY = y - height * pivotY;
					caretLine = (int) lineIndex;
					drawCaret = this->showCaret && drawCurrentLine;
				}
				
				// do draw caret
				if ( drawCaret ) {
					caretRect.y = y;
					caretRect.h = lineHeight;
					caretRect.w = max( 1.0f, this->fontSize * 0.1f );
					SDL_FillRect( textSurface, &caretRect, SDL_MapRGB(textSurface->format, character->color.r, character->color.g, character->color.b ) );
				}
				
				// draw
				if ( drawCurrentLine && character->glyphInfo ) {
					// set color
					SDL_SetSurfaceColorMod( character->glyphInfo->surface, character->color.r, character->color.g, character->color.b );
					// draw
					SDL_UpperBlit(character->glyphInfo->surface, NULL, textSurface, &rect );
				}
				
				// next character
				if ( character->value ) characterPos++;
			}
			
			// new line
			y += lineHeight;
			lineIndex++;
		}
		
		// convert to GPU_Image
		this->surface = GPU_CopyImageFromSurface( textSurface );
		SDL_FreeSurface( textSurface );
		
		this->surface->anchor_x = this->surface->anchor_y = 0; // reset
		GPU_SetImageFilter( this->surface, GPU_FILTER_NEAREST );
		GPU_SetSnapMode( this->surface, GPU_SNAP_NONE );
		this->surfaceRect.w = this->surface->base_w;
		this->surfaceRect.h = this->surface->base_h;
		
	}
	
}

/// render callback
void RenderTextBehavior::Render( RenderTextBehavior* behavior, GPU_Target* target, Event* event ) {

	// set color
	SDL_Color color = behavior->color->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0.0 ) return;

	// repaint if needed
	if ( behavior->_dirty ) behavior->Repaint();
	
	// exit if no surface
	if ( !behavior->surface ) return;
	
	// set params
	if ( behavior->blendMode <= GPU_BLEND_NORMAL_FACTOR_ALPHA ) {
		// normal mode
		GPU_SetBlendMode( behavior->surface, (GPU_BlendPresetEnum) behavior->blendMode );
		// special mode
	} else if ( behavior->blendMode == GPU_BLEND_CUT_ALPHA ) {
		// cut alpha
		GPU_SetBlendFunction( behavior->surface,  GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetBlendEquation( behavior->surface, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	}
	behavior->surface->color = color;
	
	// set shader
	behavior->SelectShader( true );
	
	// draw
	float x = -behavior->surface->base_w * behavior->pivotX,
		  y = -behavior->surface->base_h * behavior->pivotY;
	
	GPU_Blit( behavior->surface, &behavior->surfaceRect, target, x, y );
	
}

