#include "RenderSpriteBehavior.hpp"
#include "ImageResource.hpp"
#include "Application.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


// creating from script
RenderSpriteBehavior::RenderSpriteBehavior( ScriptArguments* args ) : RenderSpriteBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RenderSpriteBehavior>( this );
	
	// add defaults
	RenderBehavior::AddDefaults();	
	
	// create effect color object
	Color* color = new Color( NULL );
	color->SetInts( 0, 0, 0, 255 );
	script.SetProperty( "outlineColor", ArgValue( color->scriptObject ), this->scriptObject );
	
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
		return ArgValue( "" );
	 } ),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
		RenderSpriteBehavior* rs = (RenderSpriteBehavior*) b;
		ImageResource* img = NULL;
		if ( val.type == TypeString && val.value.stringValue->length()	) {
			// check if changed
			if ( img && img->key.compare( val.value.stringValue->c_str() ) == 0 ) return val;
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
			if ( img->autoDraw && rs->gameObject && img->autoDraw->parent == rs->gameObject ){
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
	( "pivotX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotX = val ); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "pivotY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotY = val ); }) );
	
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
		v.value.arrayValue->emplace_back( ArgValue( fmax(0, rs->slice.x ) ) );
		v.value.arrayValue->emplace_back( ArgValue( fmax(0, rs->slice.y ) ) );
		v.value.arrayValue->emplace_back( ArgValue( fmax(0, rs->slice.w ) ) );
		v.value.arrayValue->emplace_back( ArgValue( fmax(0, rs->slice.h ) ) );
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
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.x = fmax(0, val) ); }) );
	
	script.AddProperty<RenderSpriteBehavior>
	( "sliceRight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.y; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.y = fmax(0, val) ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "sliceBottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.w; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.w = fmax(0, val) ); }) );

	script.AddProperty<RenderSpriteBehavior>
	( "sliceLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderSpriteBehavior*) b)->slice.h; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderSpriteBehavior*) b)->slice.h = fmax(0, val) ); }) );

	script.AddProperty<RenderSpriteBehavior>
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
	
	script.AddProperty<RenderSpriteBehavior>
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
	
	script.AddProperty<RenderSpriteBehavior>
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
	
	script.AddProperty<RenderSpriteBehavior>
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
	bool sliced = ( behavior->slice.x > 0 || behavior->slice.y > 0 || behavior->slice.w > 0 || behavior->slice.h > 0 );
	float cx = 0, cy = 0, sx = 1, sy = 1;
	float effectiveWidth = behavior->width,
		  effectiveHeight = behavior->height;
	SDL_Color color = behavior->color->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0.0 ) return;
	
	// shader params
	float shaderU = 0, shaderV = 0, shaderW = 0, shaderH = 0; // bounds of texture slice
	
	// texture
	if ( behavior->imageResource ) {
		
		image = behavior->imageResource->mainResource ?
					behavior->imageResource->mainResource->image :
					behavior->imageResource->image;
		ImageFrame *frame = &behavior->imageResource->frame;
		
		// current frame
		srcRect = frame->locationOnTexture;
		rotated = frame->rotated;
		
		// apply trim
		effectiveWidth = fmax( 0, effectiveWidth - frame->trimWidth );
		effectiveHeight = fmax( 0, effectiveHeight - frame->trimHeight );
		
		// draw rotated
		if ( rotated ) {
			sy = effectiveWidth / (frame->actualWidth - frame->trimWidth);
			sx = effectiveHeight / (frame->actualHeight - frame->trimHeight);
			if ( sliced ) {
				cx = frame->trimOffsetX;
				cy = frame->locationOnTexture.w * sx + frame->trimOffsetY;
			} else {
				cx = frame->trimOffsetX * sy;
				cy = ( frame->locationOnTexture.w + frame->trimOffsetY ) * sx;
			}
		// normal
		} else {
			sx = effectiveWidth / (frame->actualWidth - frame->trimWidth);
			sy = effectiveHeight / (frame->actualHeight - frame->trimHeight);
			if ( sliced ) {
				cx = frame->trimOffsetX;
				cy = frame->trimOffsetY;
			} else {
				cx = frame->trimOffsetX * sx;
				cy = frame->trimOffsetY * sy;
			}
		}
		
		// shader params
		shaderU = srcRect.x;
		shaderV = srcRect.y;
		shaderW = srcRect.w;
		shaderH = srcRect.h;
		
	// Image instance
	} else if ( behavior->imageInstance ) {
		
		// get image
		image = behavior->imageInstance->GetImage();
		if ( !image ) return;
		
		// if autodraw and this is a direct child of this behavior's gameObject
		if ( behavior->imageInstance->autoDraw && behavior->imageInstance->autoDraw->parent == behavior->gameObject ){
			// tell render loop to not draw it (since we just drew it to texture)
			event->skipObject = behavior->imageInstance->autoDraw;
			event->skipObject->DirtyTransform();
		} else if ( behavior->imageInstance->autoMask && behavior->imageInstance->autoMask->parent == behavior->gameObject ){
			// tell render loop to not draw it (since we just drew it to texture)
			event->skipObject2 = behavior->imageInstance->autoMask;
			event->skipObject2->DirtyTransform();
		}
		
		// size
		shaderW = srcRect.w = image->base_w;
		shaderH = srcRect.h = image->base_h;
		
		// set scale
		sy = behavior->height / srcRect.h;
		sx = behavior->width / srcRect.w;
		
	}
	
	// bail if nothing to draw
	if ( !image ) return;
	
	// blend mode and color
	if ( behavior->blendMode == BlendMode::Cut ) {
		// cut alpha
		GPU_SetBlendFunction( image, GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetBlendEquation( image, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	} else {
		// normal mode
		GPU_SetBlendFunction( image, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
		GPU_SetBlendEquation( image, GPU_EQ_ADD, GPU_EQ_ADD);
	}
	image->color = color;

	// slices
	float top = 0, right = 0, bottom = 0, left = 0;
	if ( sliced ) {
		// compute slices in pixels
		if ( rotated ) {
			right = ( (behavior->slice.x < 1) ? (srcRect.w * behavior->slice.x) : behavior->slice.x );
			bottom = ( (behavior->slice.y < 1) ? (srcRect.h * behavior->slice.y) : behavior->slice.y );
			left = ( (behavior->slice.w < 1) ? (srcRect.w * behavior->slice.w) : behavior->slice.w );
			top = ( (behavior->slice.h < 1) ? (srcRect.h * behavior->slice.h) : behavior->slice.h );
			behavior->width = max( behavior->width, top + bottom );
			behavior->height = max( behavior->height, left + right );
		} else {
			top = ( (behavior->slice.x < 1) ? (srcRect.h * behavior->slice.x) : behavior->slice.x );
			left = ( (behavior->slice.h < 1) ? (srcRect.w * behavior->slice.h) : behavior->slice.h );
			bottom = ( (behavior->slice.w < 1) ? (srcRect.h * behavior->slice.w) : behavior->slice.w );
			right = ( (behavior->slice.y < 1) ? (srcRect.w * behavior->slice.y) : behavior->slice.y );
			behavior->width = max( behavior->width, left + right );
			behavior->height = max( behavior->height, top + bottom );
		}
	}
		
	// flip
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
	if ( rotated ) {
		cx = floor( cx - ( behavior->width * behavior->pivotX + behavior->texturePad * sy ) );
		cy = floor( cy - ( behavior->height * behavior->pivotY - behavior->texturePad * sx ) );
	} else {
		cx = floor( cx - ( behavior->width * behavior->pivotX + behavior->texturePad * sx ) );
		cy = floor( cy - ( behavior->height * behavior->pivotY + behavior->texturePad * sy ) );
	}
	
	// tiling, slicing
	float shaderTileX, shaderTileY;
	float sliceScaleX, sliceScaleY;
	if ( rotated ) {
		shaderTileX = behavior->autoTileY ? ( behavior->tileY * sx ) : behavior->tileY;
		shaderTileY = behavior->autoTileX ? ( behavior->tileX * sy ) : behavior->tileX;
		sliceScaleY = behavior->width / srcRect.h;
		sliceScaleX = behavior->height / srcRect.w;

	} else {
		shaderTileX = behavior->autoTileX ? ( behavior->tileX * sx ) : behavior->tileX;
		shaderTileY = behavior->autoTileY ? ( behavior->tileY * sy ) : behavior->tileY;
		sliceScaleX = behavior->width / srcRect.w;
		sliceScaleY = behavior->height / srcRect.h;
	}
	
	// activate shader
	behavior->SelectTexturedShader(
		image->base_w, image->base_h,
	    shaderU, shaderV, shaderW, shaderH,
		top, right, bottom, left,
		sliceScaleX, sliceScaleY,
	    shaderTileX, shaderTileY,
	    image, target, (GPU_Target**) event->behaviorParam2 );
	
	GPU_Rect dest = {
		cx, cy,
		( srcRect.w + behavior->texturePad * 2 ) * sx,
		( srcRect.h + behavior->texturePad * 2 ) * sy
	};
	
	GPU_BlitRectX( image, &srcRect, target, &dest, rotated ? -90 : 0, 0, 0, GPU_FLIP_NONE );
	
}


/* MARK:	-				Shape from render
 -------------------------------------------------------------------- */


RigidBodyShape* RenderSpriteBehavior::MakeShape() {
	
	return NULL;
}








