#include "RenderSpriteBehavior.hpp"
#include "ImageResource.hpp"
#include "Application.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


// creating from script
RenderSpriteBehavior::RenderSpriteBehavior( ScriptArguments* args ) : RenderSpriteBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RenderSpriteBehavior>( this );
	
	// create color object
	Color *color = new Color( NULL );
	script.SetProperty( "color", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create addColor object
	color = new Color( NULL );
	color->SetInts( 0, 0, 0, 0 );
	script.SetProperty( "addColor", ArgValue( color->scriptObject ), this->scriptObject );

	// with arguments
	if ( args && args->args.size() ) {
		// first argument can be a string
		if ( args->args[ 0 ].type == TypeString ) {
			// texture
			script.SetProperty( "texture", args->args[ 0 ], this->scriptObject );
			
		// or Image object
		} else if ( args->args[ 0 ].type == TypeObject && args->args[ 0 ].value.objectValue != NULL ) {
			Image* img = script.GetInstance<Image>( args->args[ 0 ].value.objectValue );
			if ( img ) {
				script.SetProperty( "image", args->args[ 0 ], this->scriptObject );
			} else {
				script.ReportError( "RenderSprite constructor accepts String texture, or Image instance" );
			}
		}
	}
}

// init
RenderSpriteBehavior::RenderSpriteBehavior() {

	// register event functions
	AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &RenderSpriteBehavior::Render );
	
	// can render
	this->isRenderBehavior = true;
	
}

// init with image name
RenderSpriteBehavior::RenderSpriteBehavior( const char* imageResourceName ) : RenderSpriteBehavior() {
	
	// set image
	script.SetProperty( "texture", ArgValue( imageResourceName ), this->scriptObject );

}

/// destructor
RenderSpriteBehavior::~RenderSpriteBehavior() {
	
	// release
	if ( this->imageResource ) this->imageResource->AdjustUseCount( -1 );
	
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void RenderSpriteBehavior::InitClass() {
	
	// register class
	script.RegisterClass<RenderSpriteBehavior>( "Behavior" );
	
	// properties
	
	script.AddProperty<RenderSpriteBehavior>
	( "color",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderSpriteBehavior*) b)->color->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((RenderSpriteBehavior*) b)->color = other;
		return ((RenderSpriteBehavior*) b)->color->scriptObject;
	}) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "addColor",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderSpriteBehavior*) b)->addColor->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((RenderSpriteBehavior*) b)->addColor = other;
		return ((RenderSpriteBehavior*) b)->addColor->scriptObject;
	}) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "blendMode",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderSpriteBehavior*) b)->blendMode; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderSpriteBehavior*) b)->blendMode = (GPU_BlendPresetEnum) val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "stipple",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->stipple; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->stipple = max( 0.0f, min( 1.0f, val ))); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "stippleAlpha",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->stippleAlpha; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->stippleAlpha = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "centered",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->centered; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->centered = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "texture",
	 static_cast<ScriptStringCallback>([](void *b, string val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageResource ) return rs->imageResource->key;
		return string("");
	 } ),
	 static_cast<ScriptStringCallback>([](void *b, string val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		ImageResource* img = NULL;
		
		// "texture" - make sure it exists
		img = app.textureManager.Get( val.c_str() );
		if ( img->error == ERROR_NONE ) {
			rs->width = img->frame.actualWidth;
			rs->height = img->frame.actualHeight;
			img->AdjustUseCount( 1 );
		} else {
			img = NULL;
		}
	
		// clear previous
		if ( rs->imageResource ) rs->imageResource->AdjustUseCount( -1 );
		
		// set new
		rs->imageResource = img;
		return val;
	}));
	
	script.AddProperty<RenderSpriteBehavior>
	( "image",
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageInstance ) return rs->imageInstance->scriptObject;
		return (void*) NULL;
	 } ),
	 static_cast<ScriptObjectCallback>([](void *b, void* val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		// object
		if ( val != NULL ) {
			Image *img = script.GetInstance<Image>( val );
			if ( !img ) {
				script.ReportError( ".image property can be only set to Image instance, or null" );
				return val;
			}
			// set
			rs->imageInstance = img;
			if ( img->image ) {
				rs->width = img->image->base_w;
				rs->height = img->image->base_h;
			}
			// assigned image's autoDraw is direct child of this gameObject
			if ( img->autoDraw && img->autoDraw->parent == rs->gameObject ){
				// update clipping of all descendent UIObjects
				vector<UIBehavior*> uis;
				rs->gameObject->GetBehaviors( true, uis );
				for ( size_t i = 0, nb = uis.size(); i < nb; i++ ){
					uis[ i ]->CheckClipping();
				}
			}
		// NULL passed
		} else {
			rs->imageInstance = NULL;
		}
		return val;
	}));
	
	script.AddProperty<RenderSpriteBehavior>
	( "width", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->width; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->width = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "height", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->height; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->height = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "flipX",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->flipX; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->flipX = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "flipY",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->flipY; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->flipY = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "sliceTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.x; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.x = val ); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "sliceRight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.y; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.y = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "sliceBottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.w; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.w = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "sliceLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.h; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.h = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "tileX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->tileX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->tileX = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "tileY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->tileY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->tileY = val ); }) );
}


/* MARK:	-				UI
 -------------------------------------------------------------------- */


// returns sprite bounding box
GPU_Rect RenderSpriteBehavior::GetBounds() {
	GPU_Rect rect;
	rect.w = this->width;
	rect.h = this->height;
	rect.x = this->centered ? -this->width * 0.5f : 0;
	rect.y = this->centered ? -this->height * 0.5f : 0;
	return rect;
}


/* MARK:	-				Render
 -------------------------------------------------------------------- */


/// render callback
void RenderSpriteBehavior::Render( RenderSpriteBehavior* behavior, GPU_Target* target, Event* event ) {
	
	// setup
	GPU_Image* image = NULL;
	GPU_Rect srcRect = { 0, 0, 0, 0 };
	bool rotated = false;
	float cx = 0, cy = 0, sx = 1, sy = 1, r = 0;
	SDL_Color color = behavior->color->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0.0 ) return;
	
	// slices
	bool sliced = ( behavior->slice.x != 0 || behavior->slice.y != 0 || behavior->slice.w != 0 || behavior->slice.h != 0 );
	struct RenderSlice {
		GPU_Rect rect;
		float x, y;
		float sx, sy;
	};
	static RenderSlice slices[ 9 ];
	
	// texture
	if ( behavior->imageResource ) {
		
		image = behavior->imageResource->mainResource ?
					behavior->imageResource->mainResource->image :
					behavior->imageResource->image;
		ImageFrame *frame = &behavior->imageResource->frame;
		
		// current frame
		srcRect = frame->locationOnTexture;
		rotated = frame->rotated;
		
		// activate shader
		behavior->SelectShader
		( true,
		  frame->rotated,
		  srcRect.x / image->base_w,
		  srcRect.y / image->base_h,
	      srcRect.w / image->base_w,
		  srcRect.h / image->base_h );
		
		cx += frame->trimOffsetX * sx;
		
		// draw rotated
		if ( rotated ) {
			
			sy = behavior->width / frame->actualWidth;
			sx = behavior->height / frame->actualHeight;
			cy += (frame->trimOffsetY + frame->locationOnTexture.w) * sx;
			r = -90;
	
		// normal
		} else {
			
			sx = behavior->width / frame->actualWidth;
			sy = behavior->height / frame->actualHeight;
			cy += frame->trimOffsetY * sy;
			
		}
				
	// Image instance
	} else if ( behavior->imageInstance ) {
		
		// get image
		image = behavior->imageInstance->GetImage();
		if ( !image ) return;
		
		// if autodraw and is direct child of this behavior's gameObject
		if ( behavior->imageInstance->autoDraw && behavior->imageInstance->autoDraw->parent == behavior->gameObject ){
			// tell render to not draw it (since we just drew it to texture)
			event->skipObject = behavior->imageInstance->autoDraw;
			event->skipObject->DirtyTransform();
		}
		
		// size
		srcRect.w = image->base_w;
		srcRect.h = image->base_h;
		
		// set scale
		sy = behavior->height / srcRect.h;
		sx = behavior->width / srcRect.w;
		
		// activate shader
		behavior->SelectShader( true );
		
	}
	
	// bail if no image
	if ( !image ) return;
	
	// blend mode and color
	if ( behavior->blendMode <= GPU_BLEND_NORMAL_FACTOR_ALPHA ) {
		// normal mode
		GPU_SetBlendMode( image, (GPU_BlendPresetEnum) behavior->blendMode );
	// special mode
	} else if ( behavior->blendMode == GPU_BLEND_CUT_ALPHA ) {
		// cut alpha
		GPU_SetBlendFunction( image,  GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetBlendEquation( image, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	}
	image->color = color;

	// slices
	if ( sliced ) {
		float top, right, bottom, left;
		float midX = 0, midY = 0, sliceMidX = 0, sliceMidY = 0;
		
		if ( rotated ) {
			// compute margins
			top = ( (behavior->slice.x < 1) ? (srcRect.w * behavior->slice.x) : behavior->slice.x );
			left = ( (behavior->slice.h < 1) ? (srcRect.h * behavior->slice.h) : behavior->slice.h );
			bottom = ( (behavior->slice.w < 1) ? (srcRect.w * behavior->slice.w) : behavior->slice.w );
			right = ( (behavior->slice.y < 1) ? (srcRect.h * behavior->slice.y) : behavior->slice.y );
			midX = ( srcRect.w - (top + bottom) );
			midY = ( srcRect.h - (left + right) );
			sliceMidX = max( 0.0f, behavior->width - (left + right) );
			sliceMidY = max( 0.0f, behavior->height - (top + bottom) );
			
			// clip
			behavior->width = max( behavior->width, left + right + sliceMidX );
			behavior->height = max( behavior->height, top + right + sliceMidY );
			
			// bottom left
			int i = 0;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.w = bottom;
			slices[ i ].rect.h = left;
			slices[ i ].x = 0;
			slices[ i ].y = 0;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// left
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = left;
			slices[ i ].x = bottom;
			slices[ i ].y = 0;
			slices[ i ].sx = sliceMidY / midX;
			slices[ i ].sy = 1;
			
			// upper left
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom + midX;
			slices[ i ].rect.w = top;
			slices[ i ].rect.h = left;
			slices[ i ].x = bottom + sliceMidY;
			slices[ i ].y = 0;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// bottom
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.y += left;
			slices[ i ].rect.w = bottom;
			slices[ i ].rect.h = midY;
			slices[ i ].x = 0;
			slices[ i ].y = left;
			slices[ i ].sx = 1;
			slices[ i ].sy = sliceMidX / midY;
			
			// mid
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom;
			slices[ i ].rect.y += left;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = midY;
			slices[ i ].x = bottom;
			slices[ i ].y = left;
			slices[ i ].sx = sliceMidY / midX;
			slices[ i ].sy = sliceMidX / midY;
			
			// top
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom + midX;
			slices[ i ].rect.y += left;
			slices[ i ].rect.w = top;
			slices[ i ].rect.h = midY;
			slices[ i ].x = sliceMidY + bottom;
			slices[ i ].y = left;
			slices[ i ].sx = 1;
			slices[ i ].sy = sliceMidX / midY;
			
			// upper right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.y += left + midY;
			slices[ i ].rect.w = bottom;
			slices[ i ].rect.h = right;
			slices[ i ].x = 0;
			slices[ i ].y = left + sliceMidX;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom;
			slices[ i ].rect.y += left + midY;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = right;
			slices[ i ].x = bottom;
			slices[ i ].y = left + sliceMidX;
			slices[ i ].sx = sliceMidY / midX;
			slices[ i ].sy = 1;
			
			// bottom right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += bottom + midX;
			slices[ i ].rect.y += left + midY;
			slices[ i ].rect.w = top;
			slices[ i ].rect.h = right;
			slices[ i ].x = bottom + sliceMidY;
			slices[ i ].y = left + sliceMidX;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
		} else {
			// compute margins
			top = ( (behavior->slice.x < 1) ? (srcRect.h * behavior->slice.x) : behavior->slice.x );
			left = ( (behavior->slice.h < 1) ? (srcRect.w * behavior->slice.h) : behavior->slice.h );
			bottom = ( (behavior->slice.w < 1) ? (srcRect.h * behavior->slice.w) : behavior->slice.w );
			right = ( (behavior->slice.y < 1) ? (srcRect.w * behavior->slice.y) : behavior->slice.y );
			midX = ( srcRect.w - (left + right) );
			midY = ( srcRect.h - (top + bottom) );
			sliceMidX = max( 0.0f, behavior->width - (left + right) );
			sliceMidY = max( 0.0f, behavior->height - (top + bottom) );
			
			// clip
			behavior->width = max( behavior->width, left + right + sliceMidX );
			behavior->height = max( behavior->height, top + right + sliceMidY );
			
			// upper left
			int i = 0;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.w = left;
			slices[ i ].rect.h = top;
			slices[ i ].x = cx;
			slices[ i ].y = cy;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// top
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += left;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = top;
			slices[ i ].x = cx + left;
			slices[ i ].y = cy;
			slices[ i ].sx = sliceMidX / midX;
			slices[ i ].sy = 1;
			
			// upper right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += slices[ i ].rect.w - right;
			slices[ i ].rect.w = right;
			slices[ i ].rect.h = top;
			slices[ i ].x = cx + sliceMidX + left;
			slices[ i ].y = cy;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// left
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.y += top;
			slices[ i ].rect.w = left;
			slices[ i ].rect.h = midY;
			slices[ i ].x = cx;
			slices[ i ].y = top;
			slices[ i ].sx = 1;
			slices[ i ].sy = sliceMidY / midY;
			
			// mid
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += left;
			slices[ i ].rect.y += top;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = midY;
			slices[ i ].x = cx + left;
			slices[ i ].y = cy + top;
			slices[ i ].sx = sliceMidX / midX;
			slices[ i ].sy = sliceMidY / midY;
			
			// right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += slices[ i ].rect.w - right;
			slices[ i ].rect.y += top;
			slices[ i ].rect.w = right;
			slices[ i ].rect.h = midY;
			slices[ i ].x = cx + sliceMidX + left;
			slices[ i ].y = cy + top;
			slices[ i ].sx = 1;
			slices[ i ].sy = sliceMidY / midY;
			
			// bottom left
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.y += top + midY;
			slices[ i ].rect.w = left;
			slices[ i ].rect.h = bottom;
			slices[ i ].x = cx;
			slices[ i ].y = top + sliceMidY;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
			
			// bottom
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += left;
			slices[ i ].rect.y += top + midY;
			slices[ i ].rect.w = midX;
			slices[ i ].rect.h = bottom;
			slices[ i ].x = cx + left;
			slices[ i ].y = cy + top + sliceMidY;
			slices[ i ].sx = sliceMidX / midX;
			slices[ i ].sy = 1;
			
			// bottom right
			i++;
			slices[ i ].rect = srcRect;
			slices[ i ].rect.x += left + midX;
			slices[ i ].rect.y += top + midY;
			slices[ i ].rect.w = right;
			slices[ i ].rect.h = bottom;
			slices[ i ].x = cx + left + sliceMidX;
			slices[ i ].y = cy + top + sliceMidY;
			slices[ i ].sx = 1;
			slices[ i ].sy = 1;
		}
		
	// no slices
	} else {
		
		// set flip
		if ( behavior->flipX ) {
			if ( rotated ) sy *= -1; else sx *= -1;
			cx += behavior->width;

		}
		if ( behavior->flipY ) {
			if ( rotated ) {
				sx *= -1;
				cy -= behavior->height;
			} else {
				sy *= -1;
				cy += behavior->height;
			}
			
		}
		
		// center
		if ( behavior->centered ) {
			cx += -behavior->width * 0.5;
			cy += -behavior->height * 0.5;
		}
		
		// single slice
		GPU_BlitTransform( image, &srcRect, target, cx, cy, r, sx, sy );
		return;
	}
	
	// render each slice
	int nslices = 9;
	while( --nslices >= 0 ) {
		RenderSlice &rs = slices[ nslices ];
		
		// skip if not visible
		if ( rs.rect.w <= 0 || rs.rect.h <= 0 ) continue;
		
		// transform
		GPU_PushMatrix();
		if ( behavior->flipX || behavior->flipY ) {
			if ( !behavior->centered ) GPU_Translate( behavior->flipX ? behavior->width : 0, behavior->flipY ? behavior->height : 0, 0 );
			GPU_Scale( behavior->flipX ? -1 : 1, behavior->flipY ? -1 : 1, 1 );
		}
		if ( behavior->centered )  GPU_Translate( -behavior->width * 0.5, -behavior->height * 0.5, 0 );
		if ( rotated ) {
			GPU_Translate( cx, cy, 0 );
			GPU_Rotate( -90, 0, 0, 1 );
		}
		
		// render
		GPU_BlitTransform( image, &rs.rect, target, rs.x, rs.y, 0, rs.sx, rs.sy );
		GPU_PopMatrix();
	}
	
}
