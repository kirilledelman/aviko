#include "Image.hpp"
#include "Application.hpp"
#include "Color.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

Image::Image( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<Image>( this );
	
	// with arguments
	if ( args ) {
		
		int w = 0, h = 0;
		void* obj = NULL;
		string url;
		// width, height, draw object
		if ( args->ReadArguments( 2, TypeInt, &w, TypeInt, &h, TypeObject, &obj ) ) {
			this->width = max( 0, w );
			this->height = max( 0, h );
			this->MakeImage();
			if ( obj ) {
				GameObject* go = script.GetInstance<GameObject>( obj );
				if ( go ) this->Draw( go );
				//autoDraw = go;
			}
		// string
		} else if ( args->ReadArguments( 1, TypeString, &url ) ){
			// try from data
			if ( !this->FromDataURL( url ) ) {
				// otherwise try from texture
				this->FromTexture( url );
			}
		}
		
	}
	
}

Image::Image() {}

Image::~Image() {
	
	// clean up
	if ( this->image ) {
		GPU_FreeTarget( this->image->target );
		GPU_FreeImage( this->image );
		this->image = NULL;
	}
	if ( this->mask ) {
		GPU_FreeTarget( this->mask->target );
		GPU_FreeImage( this->mask );
		this->mask = NULL;
	}
	if ( this->blendTarget ) {
		GPU_FreeTarget( this->blendTarget );
		GPU_FreeImage( this->blendTarget->image );
		this->blendTarget = NULL;
	}
	
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */

void Image::InitClass() {
	
	// create class
	script.RegisterClass<Image>( "Image" );
	
	// props
	
	script.AddProperty<Image>
	( "autoDraw",
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		Image* img = (Image*) b;
		return img->autoDraw ? img->autoDraw->scriptObject : NULL;
	}),
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		Image* img = (Image*) b;
		GameObject* go = script.GetInstance<GameObject>( val );
		// object changed?
		if ( img->autoDraw != go ) {
			img->autoDraw = go;
			img->lastRedrawFrame = 0;
		}
		return val;
	}));
	
	script.AddProperty<Image>
	( "autoMask",
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		Image* img = (Image*) b;
		return img->autoMask ? img->autoMask->scriptObject : NULL;
	}),
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		Image* img = (Image*) b;
		GameObject* go = script.GetInstance<GameObject>( val );
		img->autoMask = go;
		return val;
	}));
	
	script.AddProperty<Image>
	( "autoMaskInverted",
	 static_cast<ScriptBoolCallback>([](void *b, bool val) {
		Image* img = (Image*) b;
		return img->autoMaskInverted;
	}),
	 static_cast<ScriptBoolCallback>([](void *b, bool val) {
		Image* img = (Image*) b;
		img->autoMaskInverted = val;
		return val;
	}));
	
	script.AddProperty<Image>
	 ( "width",
	 static_cast<ScriptIntCallback>([](void *b, int val) {
		 Image* img = (Image*) b;
		 return img->width;
	 }),
	  static_cast<ScriptIntCallback>([](void *b, int val) {
		 Image* img = (Image*) b;
		 img->width = val;
		 return val;
	 }));
	
	script.AddProperty<Image>
	( "height",
	 static_cast<ScriptIntCallback>([](void *b, int val) {
		Image* img = (Image*) b;
		return img->height;
	}),
	 static_cast<ScriptIntCallback>([](void *b, int val) {
		Image* img = (Image*) b;
		img->height = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "base64",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		Image* img = (Image*) b;
		ArgValue ret;
		img->ToDataURL( ret );
		return ret;
	}),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		Image* img = (Image*) b;
		ArgValue ret( false );
		if ( val.type == TypeString ) {
			ret.value.boolValue = img->FromDataURL( *val.value.stringValue );
		}
		return ret;
	}), PROP_NOSTORE );
	
	// functions
	
	script.DefineFunction<Image>
	( "resize", // setSize( Int width, Int height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		if ( !sa.ReadArguments( 2, TypeInt, &img->width, TypeInt, &img->height ) ) {
			script.ReportError( "usage: resize( Int width, Int height )" );
			return false;
		}
		return true;
	}));
	
	script.DefineFunction<Image>
	( "clear", // clear( [ Color clearColor | Int r, Int g, Int b [, Int a ] ] )
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		Image* img = (Image*) o;
		void* obj = NULL;
		Color* clearColor = NULL;
		int r = 0, g = 0, b = 0, a = 255;
		// passed Color object?
		if ( sa.ReadArguments( 1, TypeObject, &obj ) ) {
			clearColor = script.GetInstance<Color>( obj );
			if ( !clearColor && obj ) {
				script.ReportError( "usage: clear( [ Color clearColor | Int r, Int g, Int b [, Int a ] ] )" );
				return false;
			} else if ( clearColor ) {
				r = clearColor->rgba.r;
				g = clearColor->rgba.g;
				b = clearColor->rgba.b;
				a = clearColor->rgba.a;
			}
		// maybe passed r, g, b, a
		} else {
			sa.ReadArguments( 3, TypeInt, &r, TypeInt, &g, TypeInt, &b, TypeInt, &a );
		}
		// make SDL_Color
		SDL_Color clr;
		clr.r = min( 255, max( 0, r ) );
		clr.g = min( 255, max( 0, g ) );
		clr.b = min( 255, max( 0, b ) );
		clr.a = min( 255, max( 0, a ) );
		// clear
		if ( img->image ) GPU_ClearColor( img->image->target, clr );
		if ( img->mask ) GPU_Clear( img->mask->target );
		
		return true;
	}));
	
	script.DefineFunction<Image>
	( "draw", // draw( GameObject gameObject, [ x, y [, angle[, scale[, scaleY ] ] ] ] )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		const char *error = "usage: draw( GameObject gameObject, [ Number x, Number y [, Number angle[, Number scale[, Number scaleY ] ] ] ] ) ";
		void *go = NULL;
		float x = 0, y = 0, angle = 0, scaleX = 1, scaleY = 1;
		if ( !sa.ReadArguments( 1, TypeObject, &go, TypeFloat, &x, TypeFloat, &y, TypeFloat, &angle, TypeFloat, &scaleX, TypeFloat, &scaleY ) ) {
			script.ReportError( error );
			return false;
		}
		// scale is given, but not scaleY, copy from scaleX
		if ( sa.args.size() == 5 ) scaleY = scaleX;
		
		// ensure object is GameObject
		GameObject* gameObject = script.GetInstance<GameObject>( go );
		if ( !go ) {
			script.ReportError( error );
			return false;
		}
		
		// draw object
		img->Draw( gameObject, false, x, y, angle, scaleX, scaleY );
		return true;
	}));
	
	script.DefineFunction<Image>
	( "drawMask", //
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		const char *error = "usage: drawMask( GameObject gameObject, [ Boolean inverted, [ Number x, Number y [, Number angle[, Number scale[, Number scaleY ] ] ] ] ] ) ";
		void *go = NULL;
		float x = 0, y = 0, angle = 0, scaleX = 1, scaleY = 1;
		bool inverted = false;
		if ( !sa.ReadArguments( 1, TypeObject, &go, TypeBool, &inverted, TypeFloat, &x, TypeFloat, &y, TypeFloat, &angle, TypeFloat, &scaleX, TypeFloat, &scaleY ) ) {
			script.ReportError( error );
			return false;
		}
		// scale is given, but not scaleY, copy from scaleX
		if ( sa.args.size() == 6 ) scaleY = scaleX;
		
		// ensure object is GameObject
		GameObject* gameObject = script.GetInstance<GameObject>( go );
		if ( !go ) {
			script.ReportError( error );
			return false;
		}
		
		// draw object
		img->Draw( gameObject, true, x, y, angle, scaleX, scaleY );
		img->ApplyMask( inverted );
		return true;
	}));
	
	
	script.DefineFunction<Image>
	( "save", //
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		const char *error = "usage: save( String filename ) ";
		string filename;
		// check if image has data
		if ( !img->image ) {
			script.ReportError( "save: Image has no data" );
			return true;
		}
		// read filename
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( error );
			return false;
		}
		// save
		img->Save( filename.c_str() );
		return true;
	}));
}

/// garbage collection callback
void Image::TraceProtectedObjects( vector<void **> &protectedObjects ) {
	
	if ( autoDraw ) protectedObjects.push_back( &autoDraw->scriptObject );
	if ( autoMask ) protectedObjects.push_back( &autoMask->scriptObject );
	
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
}


/* MARK:	-				Saving / Loading
 -------------------------------------------------------------------- */

size_t _ImageRWOpsWrite(SDL_RWops *context, const void *ptr, size_t size, size_t num) {
	string* buf = (string*) context->hidden.unknown.data1;
	if ( !buf ) {
		buf = new string();
		context->hidden.unknown.data1 = buf;
	}
	size_t prevPos = buf->length();
	buf->resize( prevPos + num );
	SDL_memcpy( (void*)( buf->data() + prevPos ), ptr, num );
	return num;
}

int _ImageRWOpsClose( SDL_RWops *context ) {
	if ( context->hidden.unknown.data1 ) {
		delete ((string*) context->hidden.unknown.data1);
	}
	context->hidden.unknown.data1 = NULL;
	SDL_FreeRW( context );
	return 0;
}

bool Image::ToDataURL( ArgValue &val ) {
	
	// nothing to convert
	if ( !this->image ) return false;

	SDL_RWops* rwops = SDL_AllocRW();
	rwops->hidden.unknown.data1 = NULL;
	rwops->write = _ImageRWOpsWrite;
	rwops->close = _ImageRWOpsClose;
	bool res = GPU_SaveImage_RW( this->image, rwops, false, GPU_FileFormatEnum::GPU_FILE_PNG );
	if ( !res || !rwops->hidden.unknown.data1 ) {
		SDL_RWclose( rwops );
		return false;
	}
	
	// encode
	val.type = TypeString;
	val.value.stringValue = new string( "#" );
	string *src = ( ( string* ) rwops->hidden.unknown.data1 );
	*val.value.stringValue += base64_encode( (unsigned char const*) src->data(), (int) src->size() );
	
	// done
	SDL_RWclose( rwops );
	return true;
}

/// inits image from base64 encoded png
bool Image::FromDataURL( string &s ) {
	
	if ( s.length() < 1 || s.substr( 0, 1 ).compare( "#" ) != 0 ) return false;
	
	// decode
	string decoded = base64_decode( s.substr( 1 ) );
	SDL_RWops* rwops = SDL_RWFromMem( (void*) decoded.data(), (int) decoded.size() );
	GPU_Image* img = GPU_LoadImage_RW( rwops, true );
	if ( !img ) return false;
	
	// set
	if ( this->image ) {
		GPU_FreeTarget( this->image->target );
		GPU_FreeImage( this->image );
	}
	this->image = img;
	GPU_UnsetImageVirtualResolution( img );
	GPU_SetImageFilter( img, GPU_FILTER_NEAREST );
	GPU_SetSnapMode( img, GPU_SNAP_NONE );
	this->width = this->image->base_w;
	this->height = this->image->base_h;
	return true;
	
}

bool Image::FromTexture( string &s ) {
	// load texture
	ImageResource* res = app.textureManager.Get( s.c_str() );
	if ( res && res->error == ResourceError::ERROR_NONE ) {
		
		// push view matrices
		GPU_MatrixMode( GPU_PROJECTION );
		GPU_PushMatrix();
		GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
		GPU_MatrixMode( GPU_MODELVIEW );
		GPU_PushMatrix();
		GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
		
		// make image
		GPU_Image* src = res->mainResource ? res->mainResource->image : res->image;
		GPU_Image* img = GPU_CreateImage( res->frame.actualWidth, res->frame.actualHeight, app.backScreen->format );
		if ( !img ) return false;
		img->anchor_x = img->anchor_y = 0;
		GPU_UnsetImageVirtualResolution( img );
		GPU_SetImageFilter( img, GPU_FILTER_NEAREST );
		GPU_SetSnapMode( img, GPU_SNAP_NONE );
		GPU_LoadTarget( img );
		
		// draw to it
		if ( res->frame.rotated ) {
			GPU_BlitRotate( src, &res->frame.locationOnTexture, img->target, res->frame.trimOffsetX, res->frame.trimOffsetY + res->frame.locationOnTexture.w, -90 );
		} else {
			GPU_Blit( src, &res->frame.locationOnTexture, img->target, res->frame.trimOffsetX, res->frame.trimOffsetY );
		}
		
		// clear previous
		if ( this->image ) {
			GPU_FreeTarget( this->image->target );
			GPU_FreeImage( this->image );
		}
		// assign
		this->image = img;
		this->width = img->base_w;
		this->height = img->base_h;
		
		// pop
		GPU_MatrixMode( GPU_PROJECTION );
		GPU_PopMatrix();
		GPU_MatrixMode( GPU_MODELVIEW );
		GPU_PopMatrix();
		return true;
	} else return false;
}

void Image::Save( const char *filename ) {

	// make surface
	//SaveFile(<#const char *data#>, <#size_t numBytes#>, <#const char *filepath#>, <#const char *ext#>)
	
}

/* MARK:	-				Image
 -------------------------------------------------------------------- */


/// helper to make image and set flags
GPU_Image* Image::MakeImage( bool makeMask ) {
	if ( this->width <= 0 || this->height <= 0 ) return NULL;
	GPU_Image* curTarget = makeMask ? this->mask : this->image;
	// new image
	GPU_Image* img = GPU_CreateImage( this->width, this->height, GPU_FORMAT_RGBA );
	if ( !img ) {
		script.ReportError( "Failed to create Image with dimensions %d, %d", this->width, this->height );
		return NULL;
	}
	GPU_UnsetImageVirtualResolution( img );
	GPU_SetImageFilter( img, GPU_FILTER_NEAREST );
	GPU_SetSnapMode( img, GPU_SNAP_NONE );
	GPU_LoadTarget( img );
	if ( curTarget ) {
		// copy old image to new
		GPU_Clear( img->target );
		GPU_Rect srcRect = { 0, 0, (float) curTarget->base_w, (float) curTarget->base_h };
		GPU_Blit( curTarget, &srcRect, img->target, 0, 0 );
		GPU_FreeTarget( curTarget->target );
		GPU_FreeImage( curTarget );
	}
	if ( !makeMask ) {
		GPU_AddDepthBuffer( img->target );
		GPU_SetDepthTest( img->target, true );
		GPU_SetDepthWrite( img->target, true );
		img->target->camera.z_near = -1024;
		img->target->camera.z_far = 1024;
	}
	img->anchor_x = img->anchor_y = 0;
	
	// done
	if ( makeMask ) {
		this->mask = img;
	} else {
		this->image = img;
		// reset blend target
		if ( this->blendTarget ) {
			GPU_FreeTarget( this->blendTarget );
			GPU_FreeImage( this->blendTarget->image );
			this->blendTarget = NULL;
		}
	}
	return img;
}

/// returns updated image
GPU_Image* Image::GetImage() {
	// check if needs redraw
	if ( this->autoDraw && this->lastRedrawFrame < app.frames ) {
		this->lastRedrawFrame = app.frames;
		this->Draw( this->autoDraw, false );
		// if have automask
		if ( this->autoMask ) {
			this->Draw( this->autoMask, true );
			this->ApplyMask( this->autoMaskInverted );
		}
	}
	
	// return image
	return this->image;
}

/// draws gameobject
void Image::Draw( GameObject* go, bool toMask, float x, float y, float angle, float scaleX, float scaleY ) {
	
	GPU_Image* curTarget = toMask ? this->mask : this->image;
	
	// make sure image exists and of right size
	if ( !curTarget || curTarget->w != this->width || curTarget->h != this->height ) {
		
		// create
		if ( ( curTarget = MakeImage( toMask ) ) == NULL ) return;
		
	}
	
	// if autodraw, or mask clear
	if ( toMask || ( !toMask && this->autoDraw ) ) {
		GPU_Clear( curTarget->target );
	}
	
	// transform
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PushMatrix();
	
	// set view matrix
	float mat[ 16 ];
	GPU_MatrixIdentity( mat );
	GPU_MatrixTranslate( mat, x, y, 0 );
	GPU_MatrixRotate( mat, angle, 0, 0, 1 );
	GPU_MatrixScale( mat, scaleX, scaleY, 1 );
	GPU_MatrixCopy( GPU_GetCurrentMatrix(), mat );
	
	// set transform matrix
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PushMatrix();
	GPU_MatrixIdentity( mat );
	GPU_MatrixCopy( GPU_GetCurrentMatrix(), mat );
	
	// invoke render
	Event renderEvent;
	renderEvent.name = EVENT_RENDER;
	renderEvent.behaviorParam = curTarget->target;
	renderEvent.behaviorParam2 = toMask ? NULL : &this->blendTarget;
	GPU_MatrixMode( GPU_MODELVIEW );
	go->Render( renderEvent );
	
	// pop matrices
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PopMatrix();
}

/// cuts out image using mask
void Image::ApplyMask( bool inverted ) {
	
	// sanity
	if ( !this->mask || !this->image ) return;
	
	// push
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PushMatrix();
	GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PushMatrix();
	GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
	
	if ( inverted ) {
		GPU_SetBlendEquation( this->mask, GPU_EQ_ADD, GPU_EQ_ADD );
		GPU_SetBlendFunction( this->mask, GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_FUNC_ZERO, GPU_FUNC_SRC_ALPHA );
	} else {
		GPU_SetBlendEquation( this->mask, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT );
		GPU_SetBlendFunction( this->mask, GPU_FUNC_ZERO, GPU_FUNC_ONE, GPU_FUNC_ONE, GPU_FUNC_ONE );
	}
	
	// blit
	GPU_Blit( this->mask, &this->mask->target->clip_rect, this->image->target, 0, 0 );
	
	// pop matrices
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PopMatrix();
}


