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
	
	colorUpdated = static_cast<ColorCallback>([this](Color* c){
		this->_dirty = true;
	});
	colorsUpdated = static_cast<TypedVectorCallback>([this](TypedVector* cv){
		this->_dirty = true;
	});
	
	// create background object
	Color* color = new Color( NULL );
	color->SetInts( 255, 255, 255, 0 );
	color->callback = colorUpdated;
	script.SetProperty( "backgroundColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create text color object
	color = new Color( NULL );
	color->callback = colorUpdated;
	script.SetProperty( "textColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create text selection color objects
	color = new Color( NULL );
	color->callback = colorUpdated;
	color->SetInt( 0x0070B0, false );
	script.SetProperty( "selectionColor", ArgValue( color->scriptObject ), this->scriptObject );
	color = new Color( NULL );
	color->callback = colorUpdated;
	color->SetInt( 0xFFFFFF, false );
	script.SetProperty( "selectionTextColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create ^0 - ^9 colors
	ArgValue cv( "Color" );
	this->colors = new TypedVector( NULL );
	this->colors->InitWithType( cv );
	int defaultColors[ 10 ] = { 0x0, 0x3333FF, 0xFF3333, 0xFF33FF, 0x33FF33, 0x33FFFF, 0xFFFF33, 0xFFFFFF, 0xAAAAAA, 0x666666 };
	for ( int i = 0; i < 10; i++ ) {
		Color* c = new Color( NULL );
		c->callback = colorUpdated;
		c->SetInt( defaultColors[ i ], false );
		ArgValue co( c->scriptObject );
		this->colors->PushElement( co );
	}
	this->colors->lockedSize = this->colors->lockedType = true;
	this->colors->callback = colorsUpdated;
	this->colors->setCallback = RenderTextBehavior::ColorsSetItem;
	
	// create effect color object
	color = new Color( NULL );
	color->callback = colorUpdated;
	color->SetInts( 0, 0, 0, 255 );
	script.SetProperty( "outlineColor", ArgValue( color->scriptObject ), this->scriptObject );

	// default fonts
	script.SetProperty( "font", ArgValue( "Roboto" ), this->scriptObject );
	script.SetProperty( "boldFont", ArgValue( "RobotoBold" ), this->scriptObject );

	// with arguments
	if ( args ) {
		/// load font
		string fname = app.defaultFontName;
		int size = this->fontSize;
		void* initObj = NULL;
		if ( args->ReadArguments( 1, TypeString, &fname, TypeInt, &size ) ) {
			SetFont( fname.c_str(), size, false, false );
			return;
		} else if ( args->ReadArguments( 1, TypeObject, &initObj ) ) {
			script.CopyProperties( initObj, this->scriptObject );
			return;
		}
	}
	
	// default
	SetFont( app.defaultFontName.c_str(), this->fontSize, false, false );
	
	// set default width, height
	if ( !this->width ) this->width = this->fontSize * 10;
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
		GPU_FreeTarget( this->surface->target );
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
	( "selectionTextColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		return ArgValue(((RenderTextBehavior*) b)->selectionTextColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		if ( val.type == TypeObject ) { // replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->selectionTextColor = other;
		} else {
			rs->selectionTextColor->Set( val );
		}
		rs->_dirty = true;
		return rs->selectionTextColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "colors",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		return ArgValue(rs->colors->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		rs->colors->Set( val );
		rs->_dirty = true;
		return rs->colors->scriptObject;
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
			rs->_dirty = true;
			rs->ClearGlyphs();
		} else if ( val.type == TypeString ) {
			rs->SetFont( val.value.stringValue->c_str(), rs->fontSize, false, false );
		}
		return val;
	}));

	script.AddProperty<RenderTextBehavior>
	( "boldFont",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		return ArgValue( ( (RenderTextBehavior*) b )->fontBoldName.c_str() );
	} ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		// if clearing
		if ( val.isNull() || val.type != TypeString ) {
			// clear previous
			if ( rs->fontBoldResource ) rs->fontBoldResource->AdjustUseCount( -1 );
			rs->fontBoldResource = NULL;
			rs->fontBoldName = "";
			rs->_dirty = true;
			rs->ClearGlyphs();
		} else if ( val.type == TypeString ) {
			rs->SetFont( val.value.stringValue->c_str(), rs->fontSize, true, false );
		}
		return val;
	}));

	script.AddProperty<RenderTextBehavior>
	( "italicFont",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		return ArgValue( ( (RenderTextBehavior*) b )->fontItalicName.c_str() );
	} ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		// if clearing
		if ( val.isNull() || val.type != TypeString ) {
			// clear previous
			if ( rs->fontItalicResource ) rs->fontItalicResource->AdjustUseCount( -1 );
			rs->fontItalicResource = NULL;
			rs->fontItalicName = "";
			rs->_dirty = true;
			rs->ClearGlyphs();
		} else if ( val.type == TypeString ) {
			rs->SetFont( val.value.stringValue->c_str(), rs->fontSize, false, true );
		}
		return val;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "boldItalicFont",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		return ArgValue( ( (RenderTextBehavior*) b )->fontBoldItalicName.c_str() );
	} ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderTextBehavior* rs = (RenderTextBehavior*) b;
		// if clearing
		if ( val.isNull() || val.type != TypeString ) {
			// clear previous
			if ( rs->fontBoldItalicResource ) rs->fontBoldItalicResource->AdjustUseCount( -1 );
			rs->fontBoldItalicResource = NULL;
			rs->fontBoldItalicName = "";
			rs->_dirty = true;
			rs->ClearGlyphs();
		} else if ( val.type == TypeString ) {
			rs->SetFont( val.value.stringValue->c_str(), rs->fontSize, true, true );
		}
		return val;
	}));
	
	script.AddProperty<RenderTextBehavior>
	( "size", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->fontSize; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		// reload all font variants
		rs->fontSize = max( 4, val );
		if ( rs->fontResource && rs->fontResource->size != val ) rs->SetFont( rs->fontName.c_str(), val, false, false );
		if ( rs->fontBoldResource && rs->fontBoldResource->size != val ) rs->SetFont( rs->fontBoldName.c_str(), val, true, false );
		if ( rs->fontItalicResource && rs->fontItalicResource->size != val ) rs->SetFont( rs->fontItalicName.c_str(), val, false, true );
		if ( rs->fontBoldItalicResource && rs->fontBoldItalicResource->size != val ) rs->SetFont( rs->fontBoldItalicName.c_str(), val, true, true );
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
	( "outlineColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderBehavior*) b)->outlineColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderBehavior* rs = (RenderBehavior*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->outlineColor = other;
		} else {
			rs->outlineColor->Set( val );
		}
		return rs->outlineColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineOffsetX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineOffsetX != val ) {
			rs->outlineOffsetX = val;
			rs->UpdateTexturePad();
		}
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineOffsetY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineOffsetY != val ) {
			rs->outlineOffsetY = val;
			rs->UpdateTexturePad();
		}
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineRadius",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineRadius; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineRadius != val ) {
			rs->outlineRadius = fmax( -16, fmin( val, 16 ) );
			rs->UpdateTexturePad();
		}
		return val;
	}) );
	
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
	( "pivot", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotX = ((RenderBehavior*) b)->pivotY = val ); }),
	 PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<RenderTextBehavior>
	( "pivotX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotX = val ); }) );
	
	script.AddProperty<RenderTextBehavior>
	( "pivotY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotY = val ); }) );
	
	script.AddProperty<RenderTextBehavior>
	( "autoSize",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderTextBehavior*) b)->autoResize; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->autoResize = val;
		rs->_dirty = true;
		return val;
	}) );
	
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
		rs->Repaint();
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "revealStart", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->revealStart; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->revealStart = max( 0, val );
		rs->_dirty = true;
		return rs->revealStart;
	}));

	script.AddProperty<RenderTextBehavior>
	( "revealEnd", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->revealEnd; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderTextBehavior* rs = ((RenderTextBehavior*) b);
		rs->revealEnd = max( 0, val );
		rs->_dirty = true;
		return rs->revealEnd;
	}));
	
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

	script.AddProperty<RenderTextBehavior>
	( "outlineColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderBehavior*) b)->outlineColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderBehavior* rs = (RenderBehavior*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->outlineColor = other;
		} else {
			rs->outlineColor->Set( val );
		}
		return rs->outlineColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineOffsetX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineOffsetX != val ) {
			rs->outlineOffsetX = val;
			rs->UpdateTexturePad();
		}
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineOffsetY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineOffsetY != val ) {
			rs->outlineOffsetY = val;
			rs->UpdateTexturePad();
		}
		return val;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "outlineRadius",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineRadius; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderBehavior *rs = (RenderBehavior*) b;
		if ( rs->outlineRadius != val ) {
			rs->outlineRadius = fmax( -16, fmin( val, 16 ) );
			rs->UpdateTexturePad();
		}
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
	( "measure", //
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		RenderTextBehavior* rs = (RenderTextBehavior*) obj;
		rs->Repaint( true );
		return true;
	}));
	
	script.DefineFunction<RenderTextBehavior>
	( "resize", // setSize( Number width, Number height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		RenderTextBehavior* self = (RenderTextBehavior*) obj;
		int w = 0, h = 0;
		if ( !sa.ReadArguments( 2, TypeInt, &w, TypeInt, &h ) ) {
			script.ReportError( "usage: resize( Number width, Number height )" );
			return false;
		}
		self->Resize( w, h );
		return true;
	}));
}

void RenderTextBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	
	// colors
	protectedObjects.push_back( &this->colors->scriptObject );
	protectedObjects.push_back( &this->textColor->scriptObject );
	protectedObjects.push_back( &this->selectionColor->scriptObject );
	protectedObjects.push_back( &this->selectionTextColor->scriptObject );
	protectedObjects.push_back( &this->backgroundColor->scriptObject );
	
	// call super
	RenderBehavior::TraceProtectedObjects( protectedObjects );
	
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
}

/// sets font
bool RenderTextBehavior::SetFont( const char* face, int size, bool b, bool i ) {

	if ( face ) {
		// trim
		size = max( 1, min( 512, size ) );
		
		// load font
		FontResource* fnt = NULL;
		static char buf[ 128 ];
		sprintf( buf, "%s,%d", face, size );
		fnt = app.fontManager.Get( buf );
		// make sure it exists
		if ( fnt->error == ERROR_NONE ) {
			fnt->AdjustUseCount( 1 );
		} else return false;

		// set appropriate font resource/name
		TTF_SetFontKerning( fnt->font, 1 );
		FontResource** thisResource = &this->fontResource;
		string* thisFontName = &this->fontName;
		
		if ( b && i ) {
			thisResource = &this->fontBoldItalicResource;
			thisFontName = &this->fontBoldItalicName;
		} else if ( b ) {
			thisResource = &this->fontBoldResource;
			thisFontName = &this->fontBoldName;
		} else if ( i ) {
			thisResource = &this->fontItalicResource;
			thisFontName = &this->fontItalicName;
		}
		
		// clear previous
		if ( *thisResource != NULL ) (*thisResource)->AdjustUseCount( -1 );
		
		// set new
		(*thisResource) = fnt;
		(*thisFontName) = face;
	}
	
	this->fontSize = max( 4, size );
	this->_dirty = true;
	this->ClearGlyphs();
	return true;
	
}


/* MARK:	-				UI
 -------------------------------------------------------------------- */


// returns sprite bounding box
GPU_Rect RenderTextBehavior::GetBounds() {
	GPU_Rect rect;
	rect.w = this->width;
	rect.h = this->height;
	rect.x = -this->width * (this->pivotX <= 1 ? this->pivotX : ( this->pivotX / (float) this->width ) );
	rect.y = -this->height * (this->pivotY <= 1 ? this->pivotY : ( this->pivotY / (float) this->height ) );
	return rect;
}

int RenderTextBehavior::GetCaretPositionAt( float localX, float localY ) {

    // update
    if ( this->_dirty ) Repaint();
	
	// adjust coord
	localX += this->width * (this->pivotX <= 1 ? this->pivotX : ( this->pivotX / (float) this->width ) );
	localY += this->height * (this->pivotY <= 1 ? this->pivotY : ( this->pivotY / (float) this->height ) );
	
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

void RenderTextBehavior::Resize( float w, float h ) {
	this->width = fmax( 0, w );
	this->height = fmax( 0, h );
	this->_dirty = true;
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
		// set font and style
		FontResource* fres = this->fontResource;
		if ( b && i ) {
			if ( this->fontBoldItalicResource ) {
				fres = this->fontBoldItalicResource;
				TTF_SetFontStyle( fres->font, 0 );
			} else if ( this->fontBoldResource ) {
				fres = this->fontBoldResource;
				TTF_SetFontStyle( fres->font, TTF_STYLE_ITALIC );
			} else if ( this->fontItalicResource ) {
				fres = this->fontItalicResource;
				TTF_SetFontStyle( fres->font, TTF_STYLE_BOLD );
			} else {
				TTF_SetFontStyle( fres->font, (int) style );
			}
		} else if ( b ) {
			if ( this->fontBoldResource ) {
				fres = this->fontBoldResource;
				TTF_SetFontStyle( fres->font, 0 );
			} else {
				TTF_SetFontStyle( fres->font, (int) style );
			}
		} else if ( i ) {
			if ( this->fontItalicResource ) {
				fres = this->fontItalicResource;
				TTF_SetFontStyle( fres->font, 0 );
			} else {
				TTF_SetFontStyle( fres->font, (int) style );
			}
		} else {
			TTF_SetFontStyle( fres->font, 0 );
		}
		
		// ensure min advance
		gi->maxX += this->outlineWidth * 2;
		gi->maxY += this->outlineWidth * 2;
		gi->advance = fmax( gi->advance, gi->maxX - gi->minX );
		
		// draw
		TTF_SetFontOutline( fres->font, this->outlineWidth );
		TTF_SetFontHinting( fres->font, TTF_HINTING_NORMAL );
		static SDL_Color white = { 255, 255, 255, 255 };
		SDL_Surface* ss = NULL;
		if ( antialias ) {
			ss = TTF_RenderGlyph_Blended( fres->font, c, white );
		} else {
			ss = TTF_RenderGlyph_Solid( fres->font, c, white );
		}
		gi->surface = GPU_CopyImageFromSurface( ss );
		gi->surface->anchor_x = gi->surface->anchor_y = 0;
		GPU_UnsetImageVirtualResolution( gi->surface );
		GPU_SetImageFilter( gi->surface, GPU_FILTER_NEAREST );
		GPU_SetSnapMode( gi->surface, GPU_SNAP_NONE );
		GPU_SetBlendFunction( gi->surface, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
		GPU_SetBlendEquation( gi->surface, GPU_EQ_ADD, GPU_EQ_ADD);
		SDL_FreeSurface( ss );
	}
	
	// done
	return gi;
}

/// redraws surface
void RenderTextBehavior::Repaint( bool justMeasure ) {
	
	this->_dirty = ( justMeasure ? this->_dirty : false );
	
	// clear old image
	if ( this->surface && !justMeasure ) {
		GPU_FreeTarget( this->surface->target );
		GPU_FreeImage( this->surface );
		this->surface = NULL;
	}
	
	if ( this->fontResource ) {
		// redraw all lines
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
					Color* cc = script.GetInstance<Color>( this->colors->GetElement( nextChar - '0' ).value.objectValue );
					if ( cc ) currentColor = cc->rgba;
					skipTwo = true;
				} else if ( nextChar == '^' ) {
					skipOne = true;
				}
				
				// control code accepted
				RenderTextCharacter empty;
				if ( skipOne || skipTwo ) {
					empty.x = previousCharacter ?
						( previousCharacter->x +
						 previousCharacter->glyphInfo->advance + this->characterSpacing ) : 0;
					empty.value = character;
					empty.glyphInfo = GetGlyph( '\n', currentBold, currentItalic );
					empty.pos = characterPos;
					empty.isWhiteSpace = true;
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
			
			// check if need word wrap
			
			// check if current line width will exceed max line width
			if ( character != ' ' && currentLine->width + glyph->advance >= this->width && this->wrap && this->multiLine ) {
				// start new line
				lines.emplace_back();
				RenderTextLine* prevLine = &lines.at( lines.size() - 2 );
				currentLine = &lines.back();
				currentLine->firstCharacterPos = (int) characterPos;
				previousCharacter = NULL;
					
				// word wrap - take back characters and put them into new line
				if ( prevLine->characters.size() > 2 && !prevLine->characters.back().isWhiteSpace ) {
					vector<RenderTextCharacter>::iterator b = prevLine->characters.begin();
					vector<RenderTextCharacter>::iterator l = b + prevLine->characters.size() - 1;
					vector<RenderTextCharacter>::iterator i = l;
					
					// go backwards character by character
					while ( i != b ) {
						// until hitting whitespace
						if ( i->isWhiteSpace ) {
							// split width
							currentLine->width = prevLine->width - (i->x + i->width);
							prevLine->width -= currentLine->width;
							i++;
							// move characters to new line
							currentLine->firstCharacterPos = (int) i->pos;
							b = i;
							while ( i <= l ) {
								currentLine->characters.push_back( *i );
								previousCharacter = &currentLine->characters.back();
								previousCharacter->x -= prevLine->width;
								i++;
							}
							prevLine->characters.resize( prevLine->characters.size() - currentLine->characters.size() );
							i = b;
						} else i--;
					}
				}
			}
			
			// add new character
			RenderTextCharacter thisCharacter;
			thisCharacter.color = currentColor;
			thisCharacter.glyphInfo = glyph;
			thisCharacter.value = character;
			thisCharacter.width = glyph->advance + this->characterSpacing;
			thisCharacter.pos = characterPos;
			thisCharacter.isWhiteSpace = (character == ' ' || character == '\t' || character == '\r' || character == '\n');
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
		size_t lastReveal = characterPos - revealEnd;
		characterPos = 0;
		int lineHeight = ceil( TTF_FontLineSkip( this->fontResource->font ) + this->lineSpacing );
		size_t lineIndex = 0;
		size_t totalLines = (int) lines.size();
		this->scrollHeight = (int) totalLines * lineHeight;
		float x = 0, y = -scrollTop;
		GPU_Rect rect;
		int selStart = this->selectionStart;
		int selEnd = this->selectionEnd;
		if ( selStart > selEnd ) { int tmp = selStart; selStart = selEnd; selEnd = tmp; }
		
		// if wrapping, re-measure scrollWidth
		if ( wrap ) {
			this->scrollWidth = 0;
			while ( lineIndex < totalLines ) {
				this->scrollWidth = max( this->scrollWidth, (int) this->lines[ lineIndex++ ].width );
			}
		}
		
		// check size
		if ( autoResize ) {
			height = scrollHeight;
			width = scrollWidth;
		}
		if ( width <= 0 || height <= 0 ) return;
		
		// create surface
		if ( !justMeasure ) {
			this->surface = GPU_CreateImage( width, height, GPU_FORMAT_RGBA );
			if ( !this->surface ) return;			
			GPU_UnsetImageVirtualResolution( this->surface );
			GPU_SetImageFilter( this->surface, GPU_FILTER_NEAREST );
			GPU_SetSnapMode( this->surface, GPU_SNAP_NONE );
			GPU_LoadTarget( this->surface );
			GPU_ClearColor( this->surface->target, backgroundColor->rgba );
			this->surface->anchor_x = this->surface->anchor_y = 0; // reset
			this->surfaceRect.w = this->surface->base_w;
			this->surfaceRect.h = this->surface->base_h;
			GPU_ActivateShaderProgram( 0, NULL );
			GPU_SetShapeBlendFunction( GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
			GPU_SetShapeBlendEquation( GPU_EQ_ADD, GPU_EQ_ADD);
			
			// push matrices
			GPU_MatrixMode( GPU_PROJECTION );
			GPU_PushMatrix();
			GPU_LoadIdentity();
			GPU_MatrixMode( GPU_MODELVIEW );
			GPU_PushMatrix();
			GPU_LoadIdentity();
		}
		
		// for each line
		lineIndex = 0;
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
					GPU_Rect selRect;
					selRect.x = rect.x;
					selRect.y = y - 1;
					selRect.h = lineHeight + 1;
					if ( i < nc - 1 ) {
						selRect.w = currentLine->characters[ i + 1 ].x - character->x;
					} else {
						selRect.w = character->width;
					}
					
					// GPU_SetBlendMode( this->surface, GPU_BLEND_NORMAL ); // why?
					GPU_RectangleFilled2( this->surface->target, selRect, this->selectionColor->rgba );
					character->color = this->selectionTextColor->rgba;
				}
				
                // caret
				bool drawCaret = false;
				GPU_Rect caretRect;
				if ( character->pos == this->caretPosition - 1 ) {
					// after current character?
					caretRect.x = x + character->x + character->width;
					caretX = caretRect.x - width * (this->pivotX <= 1 ? this->pivotX : ( this->pivotX / (float) width ) );
					caretY = y - height * (this->pivotY <= 1 ? this->pivotY : ( this->pivotY / (float) height ) );
					caretLine = (int) lineIndex;
					drawCaret = this->showCaret && drawCurrentLine;
				} else if ( character->pos == 0 && this->caretPosition == 0 && lineIndex == 0 ) {
					caretRect.x = x + character->x;
					caretX = caretRect.x - width * (this->pivotX <= 1 ? this->pivotX : ( this->pivotX / (float) width ) );
					caretY = y - height * (this->pivotY <= 1 ? this->pivotY : ( this->pivotY / (float) height ) );
					caretLine = (int) lineIndex;
					drawCaret = this->showCaret && drawCurrentLine;
				}
				
				// do draw caret
				if ( drawCaret && !justMeasure ) {
					caretRect.y = y;
					caretRect.h = lineHeight;
					caretRect.w = max( 1.0f, this->fontSize * 0.1f );
					// GPU_SetBlendMode( this->surface, GPU_BLEND_NORMAL ); // pointless
					GPU_RectangleFilled2( this->surface->target, caretRect, character->color );
				}
				
				// draw character
				if ( drawCurrentLine &&
					character->glyphInfo && character->glyphInfo->surface &&
					!justMeasure && (character->pos >= revealStart && character->pos <= lastReveal ) ) {
					
					// set color
					GPU_SetColor( character->glyphInfo->surface, character->color );
					// GPU_SetBlendMode( this->surface, GPU_BLEND_MOD_ALPHA );
					
					// draw
					GPU_Blit( character->glyphInfo->surface, NULL, this->surface->target, rect.x, rect.y );
				}
				
				// next character
				if ( character->value ) characterPos++;
			}
			
			// new line
			y += lineHeight;
			lineIndex++;
		}
		
		// restore matrices
		if ( !justMeasure ) {
			GPU_MatrixMode( GPU_PROJECTION );
			GPU_PopMatrix();
			GPU_MatrixMode( GPU_MODELVIEW );
			GPU_PopMatrix();
		}
		
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
	if ( behavior->blendMode == BlendMode::Cut ) {
		// cut alpha
		GPU_SetBlendFunction( behavior->surface, GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetBlendEquation( behavior->surface, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	} else {
		// normal mode
		GPU_SetBlendFunction( behavior->surface, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
		GPU_SetBlendEquation( behavior->surface, GPU_EQ_ADD, GPU_EQ_ADD);
		
	}
	behavior->surface->color = color;
	
	// set shader
	behavior->SelectTexturedShader(
		behavior->surface->base_w, behavior->surface->base_h,
	    0, 0, behavior->surface->base_w, behavior->surface->base_h,
		0, 0, 0, 0,
	    0, 0,
		1, 1,
	    behavior->surface, target, (GPU_Target**) event->behaviorParam2 );
	
	// draw
	GPU_Rect dest = {
		-behavior->surfaceRect.w * (behavior->pivotX <= 1 ? behavior->pivotX : ( behavior->pivotX / (float) behavior->width ) ) - behavior->texturePad,
		-behavior->surfaceRect.h * (behavior->pivotY <= 1 ? behavior->pivotY : ( behavior->pivotY / (float) behavior->height ) ) - behavior->texturePad,
		behavior->surfaceRect.w + behavior->texturePad * 2,
		behavior->surfaceRect.h + behavior->texturePad * 2 };
	GPU_BlitRect( behavior->surface, &behavior->surfaceRect, target, &dest );
	
}

void RenderTextBehavior::ColorsSetItem(void* pcont, int index, ArgValue& newValue){
	vector<void*>* cont = (vector<void*>*) pcont;
	Color* clr = script.GetInstance<Color>( cont->at( index ) );
	if ( clr ) clr->Set( newValue );
}

/* MARK:	-				Shape from render
 -------------------------------------------------------------------- */


RigidBodyShape* RenderTextBehavior::MakeShape() {
	
	return NULL;
}
