#include "RenderSpriteBehavior.hpp"
#include "ImageResource.hpp"
#include "Application.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


// creating from script
RenderSpriteBehavior::RenderSpriteBehavior( ScriptArguments* args ) : RenderSpriteBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RenderSpriteBehavior>( this );
	
	// set to normal
	this->blendMode = GPU_BLEND_NORMAL;
	
	// add defaults
	RenderBehavior::AddDefaults();	
	
	// with arguments
	if ( args && args->args.size() ) {
		// first argument can be a string
		if ( args->args[ 0 ].type == TypeString ) {
			// texture
			script.SetProperty( "texture", args->args[ 0 ], this->scriptObject );
			// if there was a second param, object
			if ( args->args.size() > 1 && args->args[ 1 ].type == TypeObject && args->args[ 1 ].value.objectValue != NULL ) {
				script.CopyProperties( args->args[ 1 ].value.objectValue, this->scriptObject );
			}
		// or Image object
		} else if ( args->args[ 0 ].type == TypeObject && args->args[ 0 ].value.objectValue != NULL ) {
			Image* img = script.GetInstance<Image>( args->args[ 0 ].value.objectValue );
			if ( img ) {
				script.SetProperty( "image", args->args[ 0 ], this->scriptObject );
			} else {
				script.CopyProperties( args->args[ 0 ].value.objectValue, this->scriptObject );
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
	script.RegisterClass<RenderSpriteBehavior>( "RenderBehavior" );
	
	// properties
	
	script.AddProperty<RenderSpriteBehavior>
	( "texture",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageResource ) return ArgValue( rs->imageResource->key.c_str() );
		return ArgValue( (void*) NULL );
	 } ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		ImageResource* img = NULL;
		if ( val.type == TypeString ) {
			// "texture" - make sure it exists
			img = app.textureManager.Get( val.value.stringValue->c_str() );
			if ( img->error == ERROR_NONE ) {
				rs->width = img->frame.actualWidth;
				rs->height = img->frame.actualHeight;
				img->AdjustUseCount( 1 );
			} else {
				img = NULL;
			}
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
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageInstance ) rs->width = rs->imageInstance->width;
		return rs->width;
	}),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		rs->width = val;
		if ( rs->imageInstance ) {
			rs->imageInstance->width = val;
			rs->imageInstance->_sizeDirty = true;
		}
		return rs->width;
	 }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_LATE );

	script.AddProperty<RenderSpriteBehavior>
	( "height", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageInstance ) rs->height = rs->imageInstance->height;
		return rs->height;
	 }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		rs->height = val;
		if ( rs->imageInstance ) {
			rs->imageInstance->height = val;
			rs->imageInstance->_sizeDirty = true;
		}
		return rs->height;
	 }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_LATE );

	script.AddProperty<RenderSpriteBehavior>
	( "originalWidth", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageInstance ) return rs->imageInstance->width;
		else if ( rs->imageResource ) return (int) rs->imageResource->frame.actualWidth;
		return 0;
	}));

	script.AddProperty<RenderSpriteBehavior>
	( "originalHeight", //
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		if ( rs->imageInstance ) return rs->imageInstance->height;
		else if ( rs->imageResource ) return (int) rs->imageResource->frame.actualHeight;
		return 0;
	}));
	
	script.AddProperty<RenderSpriteBehavior>
	( "tileX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->tileX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->tileX = val ); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "tileY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->tileY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->tileY = val ); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "autoTileX",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->autoTileX; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->autoTileX = val; }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "autoTileY",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->autoTileY; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->autoTileY = val; }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "flipX",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->flipX; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->flipX = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "flipY",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderSpriteBehavior*) b)->flipY; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderSpriteBehavior*) b)->flipY = val ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "slice",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		ArgValue v;
		v.type = TypeArray;
		v.value.arrayValue = new ArgValueVector();
		v.value.arrayValue->emplace_back( ArgValue( rs->slice.x ) );
		v.value.arrayValue->emplace_back( ArgValue( rs->slice.y ) );
		v.value.arrayValue->emplace_back( ArgValue( rs->slice.w ) );
		v.value.arrayValue->emplace_back( ArgValue( rs->slice.h ) );
		return v;
	}),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		float sameVal = 0;
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( rs->slice.x ) ) {
				if ( val.value.arrayValue->size() >= 2 && val.value.arrayValue->at( 1 ).toNumber( rs->slice.y ) ) {
					if ( val.value.arrayValue->size() >= 3 && val.value.arrayValue->at( 2 ).toNumber( rs->slice.w ) ) {
						if ( val.value.arrayValue->size() >= 4 ) val.value.arrayValue->at( 3 ).toNumber( rs->slice.h );
					}
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			rs->slice.x = rs->slice.y = rs->slice.w = rs->slice.h = sameVal;
		}
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE  );
	
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

	// functions
	
	script.DefineFunction<RenderSpriteBehavior>
	( "resize", // setSize( Number width, Number height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		RenderSpriteBehavior* self = (RenderSpriteBehavior*) obj;
		float w = 0, h = 0;
		if ( sa.ReadArguments( 2, TypeFloat, &w, TypeFloat, &h ) ) {
			self->Resize( w, h );
		} else {
			script.ReportError( "usage: resize( Number width, Number height )" );
			return false;
		}
		return true;
	}));
	
}


/* MARK:	-				UI
 -------------------------------------------------------------------- */


// returns sprite bounding box
GPU_Rect RenderSpriteBehavior::GetBounds() {
	GPU_Rect rect;
	rect.w = this->width;
	rect.h = this->height;
	rect.x = -this->width * this->pivotX;
	rect.y = -this->height * this->pivotY;
	return rect;
}

void RenderSpriteBehavior::Resize( float w, float h ) {
	this->width = w;
	this->height = h;
	if ( this->imageInstance ) {
		this->imageInstance->width = this->width;
		this->imageInstance->height = this->height;
		this->imageInstance->_sizeDirty = true;
	}
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
	float effectiveWidth = behavior->width, effectiveHeight = behavior->height;
	SDL_Color color = behavior->color->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0.0 ) return;
	
	// slices
	bool sliced = ( behavior->slice.x != 0 || behavior->slice.y != 0 || behavior->slice.w != 0 || behavior->slice.h != 0 );
	bool trimmed = false;
	struct RenderSlice {
		GPU_Rect rect;
		float x, y;
		float sx, sy;
	};
	static RenderSlice slices[ 9 ];
	
	// effective tiling
	float tileX = 1, tileY = 1;
	
	// texture
	if ( behavior->imageResource ) {
		
		image = behavior->imageResource->mainResource ?
					behavior->imageResource->mainResource->image :
					behavior->imageResource->image;
		ImageFrame *frame = &behavior->imageResource->frame;
		
		// current frame
		srcRect = frame->locationOnTexture;
		rotated = frame->rotated;
		trimmed = frame->trimmed;
		
		// apply trim
		effectiveWidth = fmax( 0, effectiveWidth - frame->trimWidth );
		effectiveHeight = fmax( 0, effectiveHeight - frame->trimHeight );
		
		// draw rotated
		if ( rotated ) {
			
			sy = effectiveWidth / (frame->actualWidth - frame->trimWidth);
			sx = effectiveHeight / (frame->actualHeight - frame->trimHeight);
			if ( sliced ) {
				cy += (frame->locationOnTexture.w) * sx + frame->trimOffsetY;
			} else {
				cy += (frame->locationOnTexture.w + frame->trimOffsetY) * sx;
			}
			r = -90;
	
		// normal
		} else {
			
			sx = effectiveWidth / (frame->actualWidth - frame->trimWidth);
			sy = effectiveHeight / (frame->actualHeight - frame->trimHeight);
			if ( sliced ) {
				cy += frame->trimOffsetY;
			} else {
				cy += frame->trimOffsetY * sy;
			}
			
		}
		
		// tiling
		tileX = behavior->autoTileX ? ( behavior->tileX * sx ) : behavior->tileX;
		tileY = behavior->autoTileY ? ( behavior->tileY * sy ) : behavior->tileY;
		
		// activate shader
		behavior->SelectShader
		( true,
		 frame->rotated,
		 srcRect.x / image->base_w,
		 srcRect.y / image->base_h,
		 srcRect.w / image->base_w,
		 srcRect.h / image->base_h,
		 tileX, tileY );
		
		if ( sliced ) {
			cx += frame->trimOffsetX;
		} else {
			cx += frame->trimOffsetX * sx;
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
		
		// tiling
		tileX = behavior->autoTileX ? ( behavior->tileX * sx ) : behavior->tileX;
		tileY = behavior->autoTileY ? ( behavior->tileY * sy ) : behavior->tileY;
		
		// activate shader
		behavior->SelectShader( true, 0, 0, 1, 1, tileX, tileY );
		
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
			sliceMidX = max( 0.0f, effectiveWidth - (left + right) );
			sliceMidY = max( 0.0f, effectiveHeight - (top + bottom) );
			
			// clip
			behavior->width = max( behavior->width, left + right + sliceMidX );
			behavior->height = max( behavior->height, top + bottom + sliceMidY );
			
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
			sliceMidX = max( 0.0f, effectiveWidth - (left + right) );
			sliceMidY = max( 0.0f, effectiveHeight - (top + bottom) );
			
			// clip
			behavior->width = max( behavior->width, left + right + sliceMidX );
			behavior->height = max( behavior->height, top + bottom + sliceMidY );
			
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
			slices[ i ].y = cy + top;
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
			slices[ i ].y = cy + top + sliceMidY;
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
		
		// pivot
		cx += -behavior->width * behavior->pivotX;
		cy += -behavior->height * behavior->pivotY;
		
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
			GPU_Scale( behavior->flipX ? -1 : 1, behavior->flipY ? -1 : 1, 1 );
		}
		GPU_Translate( -behavior->width * behavior->pivotX, -behavior->height * behavior->pivotY, 0 );
		if ( rotated ) {
			GPU_Translate( cx, cy, 0 );
			GPU_Rotate( -90, 0, 0, 1 );
		}
		
		// render
		GPU_BlitTransform( image, &rs.rect, target, rs.x, rs.y, 0, rs.sx, rs.sy );
		GPU_PopMatrix();
	}
	
}










