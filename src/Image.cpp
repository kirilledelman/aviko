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
		if ( args->ReadArguments( 2, TypeInt, &w, TypeInt, &h ) ) {
			this->width = max( 0, w );
			this->height = max( 0, h );
			this->image = this->_MakeImage();
		}
		// todo GameObject (later, when getBounds is available)
		// todo "http://url"
	}
	
}

Image::Image() {}

Image::~Image() {
	
	// clean up
	if ( this->image ) {
		GPU_FreeImage( this->image );
		this->image = NULL;
	}
	
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */

void Image::InitClass() {
	
	// create class
	script.RegisterClass<Image>( "ScriptableObject" );
	
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
			if ( go ) {
				// update clipping of all descendent UIObjects
				vector<UIBehavior*> uis;
				go->GetBehaviors( true, uis );
				for ( size_t i = 0, nb = uis.size(); i < nb; i++ ){
					uis[ i ]->CheckClipping();
				}
			}
		}
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
		 img->_sizeDirty = true;
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
		img->_sizeDirty = true;
		return val;
	}));
	
	script.AddProperty<Image>
	( "x",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->x;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->x = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "y",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->y;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->y = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "angle",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->angle;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->angle = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "scaleX",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->scaleX;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->scaleX = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "scaleY",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->scaleY;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->scaleY = val;
		return val;
	}));
	
	script.AddProperty<Image>
	( "scale",
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		return img->scaleX;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val) {
		Image* img = (Image*) b;
		img->scaleY = img->scaleX = val;
		return val;
	}));
	
	script.DefineFunction<Image>
	( "setSize", // setSize( Int width, Int height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		if ( sa.ReadArguments( 2, TypeInt, &img->width, TypeInt, &img->height ) ) {
			img->_sizeDirty = true;
		} else {
			script.ReportError( "usage: setSize( Int width, Int height )" );
			return false;
		}
		return true;
	}));
	
	script.DefineFunction<Image>
	( "clear", // clear( [ Color clearColor | Int r, Int g, Int b [, Int a ] ] )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		if ( img->image ) {
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
			GPU_ClearColor( img->image->target, clr );
		}
		
		return true;
	}));
	
	script.DefineFunction<Image>
	( "draw", // draw( GameObject gameObject, [ x, y [, angle[, scale[, scaleY ] ] ] ] )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		Image* img = (Image*) obj;
		const char *error = "usage: draw( GameObject gameObject, [ x, y [, angle[, scale[, scaleY ] ] ] ] ) ";
		void *go = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &go, TypeFloat, &img->x, TypeFloat, &img->y, TypeFloat, &img->angle, TypeFloat, &img->scaleX, TypeFloat, &img->scaleY ) ) {
			script.ReportError( error );
			return false;
		}
		// scale is given, but not scaleY, copy from scaleX
		if ( sa.args.size() == 5 ) img->scaleY = img->scaleX;
		
		// ensure object is GameObject
		GameObject* gameObject = script.GetInstance<GameObject>( go );
		if ( !go ) {
			script.ReportError( error );
			return false;
		}
		
		// draw object
		img->Draw( gameObject );
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


void Image::Save( const char *filename ) {

	// make surace
	
}

/* MARK:	-				Image
 -------------------------------------------------------------------- */


/// helper to make image and set flags
GPU_Image* Image::_MakeImage() {
	GPU_Image* img = GPU_CreateImage( this->width, this->height, GPU_FORMAT_RGBA );
	if ( !img ) {
		script.ReportError( "Failed to create Image with dimensions %d, %d", this->width, this->height );
	} else {
		GPU_UnsetImageVirtualResolution( img );
		GPU_SetImageFilter( img, GPU_FILTER_NEAREST );
		GPU_SetSnapMode( img, GPU_SNAP_NONE );
		GPU_LoadTarget( img );
		GPU_AddDepthBuffer( img->target );
		GPU_SetDepthTest( img->target, true );
		GPU_SetDepthWrite( img->target, true );
		GPU_AddDepthBuffer( img->target );
		img->target->camera.z_near = -65535;
		img->target->camera.z_far = 65535;
		img->anchor_x = img->anchor_y = 0;
		this->_sizeDirty = false;
	}
	return img;
}

/// returns updated image
GPU_Image* Image::GetImage() {
	// check if needs redraw
	if ( this->autoDraw && this->lastRedrawFrame < app.frames ) {
		this->lastRedrawFrame = app.frames;
		this->Draw( this->autoDraw );
	}
	
	// return image
	return this->image;
}

/// draws gameobject
void Image::Draw( GameObject* go ) {
	
	// make sure image exists
	if ( !this->image ) {
		
		// create
		this->image = _MakeImage();
		if ( !image ) return;
		
	// otherwise, if resize is needed
	} else if ( this->_sizeDirty ) {
		
		// create image of new size
		this->_sizeDirty = false;
		GPU_Image *img = _MakeImage();
		if ( !img ) {
			this->width = this->image->base_w;
			this->height = this->image->base_h;
			return;
		}
		
		// copy old image to new
		GPU_Rect srcRect = { 0, 0, (float) this->image->base_w, (float) this->image->base_h };
		GPU_Blit( this->image, &srcRect, img->target, 0, 0 );
		
		// replace
		GPU_FreeImage( this->image );
		this->image = img;
	}
	
	// if autodraw, clear
	if ( this->autoDraw ) {
		SDL_Color clr = { 0, 0, 0, 20 };
		GPU_ClearColor( this->image->target, clr );
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
	
	// GPU_FlushBlitBuffer(); // without this, child transform affects parent
	
	// invoke render
	Event renderEvent;
	renderEvent.name = EVENT_RENDER;
	renderEvent.behaviorParam = this->image->target;
	GPU_MatrixMode( GPU_MODELVIEW );
	go->Render( renderEvent );
	
	// pop matrices
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PopMatrix();
}


