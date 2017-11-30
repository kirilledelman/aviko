#include "RenderTextBehavior.hpp"
#include "FontResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderTextBehavior::RenderTextBehavior( ScriptArguments* args ) : RenderTextBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RenderTextBehavior>( this );
	
	// create color object
	Color *color = new Color( NULL );
	script.SetProperty( "color", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create addColor object
	color = new Color( NULL );
	color->SetInts( 0, 0, 0, 0 );
	script.SetProperty( "addColor", ArgValue( color->scriptObject ), this->scriptObject );
	
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
	script.RegisterClass<RenderTextBehavior>( "Behavior" );
	
	// properties
	
	script.AddProperty<RenderTextBehavior>
	( "color",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderBehavior*) b)->color->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((RenderBehavior*) b)->color = other;
		return ((RenderBehavior*) b)->color->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "addColor",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderBehavior*) b)->addColor->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((RenderBehavior*) b)->addColor = other;
		return ((RenderBehavior*) b)->addColor->scriptObject;
	}) );
	
	script.AddProperty<RenderTextBehavior>
	( "blendMode",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderTextBehavior*) b)->blendMode; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderTextBehavior*) b)->blendMode = (GPU_BlendPresetEnum) val ); }) );
	
	script.AddProperty<RenderTextBehavior>
	( "stipple",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->stipple; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->stipple = max( 0.0f, min( 1.0f, val ))); }) );
	
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
	
}


/* MARK:	-				Font
 -------------------------------------------------------------------- */

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
		} else {
			return false;
		}
		
	}

	// clear previous
	if ( this->fontResource ) this->fontResource->AdjustUseCount( -1 );
	
	// set new
	this->fontResource = fnt;
	this->fontName = face;
	this->fontSize = size;
	this->_dirty = true;
	return true;
	
}



/* MARK:	-				Render
 -------------------------------------------------------------------- */


/// redraws surface
void RenderTextBehavior::Repaint() {
	
	this->_dirty = false;
	if ( this->fontResource ) {
		// clear old
		if ( this->surface ) {
			GPU_FreeImage( this->surface );
			this->surface = NULL;
		}
		// TODO - multiline, or multicolor, or formatted rendering can be done
		// by splitting text into pieces, measuring them, and rendering them to this->surface
		// render to new surface
		SDL_Surface* textSurface = TTF_RenderUTF8_Blended( this->fontResource->font, this->text.c_str(), this->color->rgba );
		this->surface = GPU_CopyImageFromSurface( textSurface );
		SDL_FreeSurface( textSurface );
		wstring sss;
		
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
	GPU_SetBlendMode( behavior->surface, behavior->blendMode );
	behavior->surface->color = color;
	
	// set shader
	behavior->SelectShader( true );
	
	// draw
	GPU_Blit( behavior->surface, &behavior->surfaceRect, target, 0, 0 );
	
}

