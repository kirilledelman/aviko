#include "RenderParticlesBehavior.hpp"
#include "Application.hpp"

GPU_Image* RenderParticlesBehavior::surface = NULL;
ImageResource* RenderParticlesBehavior::particleTexture = NULL;

/* MARK:    -                Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderParticlesBehavior::RenderParticlesBehavior( ScriptArguments* args ) : RenderParticlesBehavior() {
    
    // add scriptObject
    script.NewScriptObject<RenderParticlesBehavior>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );
    
    // add defaults
    RenderBehavior::AddDefaults();
    
    // create effect color object
    Color* color = new Color( NULL );
    color->SetInts( 0, 0, 0, 255 );
    script.SetProperty( "outlineColor", ArgValue( color->scriptObject ), this->scriptObject );

    // anything above this alpha val is drawn as solid
    this->alphaThresh = 0.15;
    
    // with arguments
    if ( args ) {
        void* initObj = NULL;
        if ( args->ReadArguments( 1, TypeObject, &initObj ) ) {
            script.CopyProperties( initObj, this->scriptObject );
            return;
        }
    }
}

// init
RenderParticlesBehavior::RenderParticlesBehavior() {
    
    // register event functions
    AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &RenderParticlesBehavior::Render );
    
    // can render
    this->isRenderBehavior = true;
    
}

/// destructor
RenderParticlesBehavior::~RenderParticlesBehavior() {
    
    if ( this->imageResource ) this->imageResource->AdjustUseCount( -1 );
    
}


/* MARK:    -                Javascript
 -------------------------------------------------------------------- */


// init script classes
void RenderParticlesBehavior::InitClass() {
    
    // register class
    script.RegisterClass<RenderParticlesBehavior>( "RenderBehavior" );

    // make base texture
    RenderParticlesBehavior::particleTexture = new ImageResource();
    RenderParticlesBehavior::particleTexture->LoadFromMemory( ParticleTexture, ParticleTexture_size );
    RenderParticlesBehavior::particleTexture->image->anchor_x = RenderParticlesBehavior::particleTexture->image->anchor_y = 0.5;
    GPU_SetBlendMode( RenderParticlesBehavior::particleTexture->image, GPU_BLEND_NORMAL );
    RenderParticlesBehavior::particleTexture->AdjustUseCount( 1 );

    script.AddProperty<RenderParticlesBehavior>
    ( "texture",
     static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
        RenderParticlesBehavior* rs = (RenderParticlesBehavior*) b;
        if ( rs->imageResource ) return ArgValue( rs->imageResource->key.c_str() );
        return ArgValue( "" );
    } ),
     static_cast<ScriptValueCallback>([](void *b, ArgValue val) {
        RenderParticlesBehavior* rs = (RenderParticlesBehavior*) b;
        ImageResource* img = NULL;
        if ( val.type == TypeString && val.value.stringValue->length()    ) {
            // check if changed
            if ( img && img->key.compare( val.value.stringValue->c_str() ) == 0 ) return val;
            // "texture" - make sure it exists
            img = app.textureManager.Get( val.value.stringValue->c_str() );
            if ( img->error == ERROR_NONE ) {
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
    
    script.AddProperty<RenderParticlesBehavior>
    ( "velocityStretch",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->velocityStretch; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->velocityStretch = fmax( 0, val ) );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "velocityStretchFactor",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->velocityStretchFactor; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->velocityStretchFactor = fmax( 0, val ) );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "fadeTime",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->fadeTime; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->fadeTime = fmax( 0, val ) );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "extents",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->extents; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->extents = val );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "alphaThreshold",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->alphaThresh; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->alphaThresh = fmax( 0.0, fmin( 1.0, val ) ) );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "tileX",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->tileX; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->tileX = val );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "tileY",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->tileY; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->tileY = val );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "offsetX",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->offsetX; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->offsetX = val );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "offsetY",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderParticlesBehavior*) b)->offsetY; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderParticlesBehavior* ps = (RenderParticlesBehavior*) b;
        return ( ps->offsetY = val );
    } ) );
    
    script.AddProperty<RenderParticlesBehavior>
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
    
    script.AddProperty<RenderParticlesBehavior>
    ( "outlineOffsetX",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetX; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderBehavior *rs = (RenderBehavior*) b;
        if ( rs->outlineOffsetX != val ) {
            rs->outlineOffsetX = val;
        }
        return val;
    }) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "outlineOffsetY",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineOffsetY; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderBehavior *rs = (RenderBehavior*) b;
        if ( rs->outlineOffsetY != val ) {
            rs->outlineOffsetY = val;
        }
        return val;
    }) );
    
    script.AddProperty<RenderParticlesBehavior>
    ( "outlineRadius",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->outlineRadius; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        RenderBehavior *rs = (RenderBehavior*) b;
        if ( rs->outlineRadius != val ) {
            rs->outlineRadius = fmax( -16, fmin( val, 16 ) );
        }
        return val;
    }) );
    
}

void RenderParticlesBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
    
    // colors
    // protectedObjects.push_back( &this->textColor->scriptObject );
    
    // call super
    RenderBehavior::TraceProtectedObjects( protectedObjects );
    
}

/* MARK:    -                Render
 -------------------------------------------------------------------- */


/// render callback
void RenderParticlesBehavior::Render( RenderParticlesBehavior* behavior, GPU_Target* target, Event* event ) {
        
    // find particles
    if ( ( !behavior->particles || ( behavior->particles && behavior->particles->gameObject != behavior->gameObject ) ) && behavior->gameObject->body ) {
        behavior->particles = script.GetInstance<ParticleGroupBehavior>( behavior->gameObject->body->scriptObject );
    }
    
    // no particles? bail
    if ( !behavior->particles || !behavior->particles->group ) return;
    
    // get/check color
    SDL_Color color = behavior->color->rgba;
    color.a *= behavior->gameObject->combinedOpacity;
    if ( color.a == 0.0 ) return;
    
    // if rendering surface size doesn't match target
    if ( RenderParticlesBehavior::surface &&
        ( RenderParticlesBehavior::surface->base_w != target->base_w || RenderParticlesBehavior::surface->base_h != target->base_h ) ) {
        // free and clear it
        GPU_FreeTarget( RenderParticlesBehavior::surface->target );
        GPU_FreeImage( RenderParticlesBehavior::surface );
        RenderParticlesBehavior::surface = NULL;
    }
    
    // if no surface
    if ( !RenderParticlesBehavior::surface ) {
        // create it
        RenderParticlesBehavior::surface = GPU_CreateImage( target->base_w, target->base_h, GPU_FORMAT_RGBA );
        if ( !RenderParticlesBehavior::surface ) return;
        GPU_UnsetImageVirtualResolution( RenderParticlesBehavior::surface );
        GPU_SetImageFilter( RenderParticlesBehavior::surface, GPU_FILTER_NEAREST );
        GPU_SetSnapMode( RenderParticlesBehavior::surface, GPU_SNAP_NONE );
        GPU_LoadTarget( RenderParticlesBehavior::surface );
        RenderParticlesBehavior::surface->anchor_x = RenderParticlesBehavior::surface->anchor_y = 0;
    }

    // prepare texture
    GPU_Image* image = NULL;
    GPU_Rect srcRect = { 0, 0, 0, 0 };
    bool rotated = false;
    float shaderU = 0, shaderV = 0, shaderW = 0, shaderH = 0; // bounds of texture slice
    float shaderTileX = 1, shaderTileY = 1;

    // texture
    if ( behavior->imageResource ) {
        
        image = behavior->imageResource->mainResource ?
        behavior->imageResource->mainResource->image :
        behavior->imageResource->image;
        ImageFrame *frame = &behavior->imageResource->frame;
        
        // current frame
        srcRect = frame->locationOnTexture;
        rotated = frame->rotated;
        
        // shader params
        shaderU = srcRect.x;
        shaderV = srcRect.y;
        shaderW = srcRect.w;
        shaderH = srcRect.h;
        
        // tiling
        if ( rotated ) {
            shaderTileX = ( behavior->tileY );
            shaderTileY = ( behavior->tileX );
            
        } else {
            shaderTileX = ( behavior->tileX );
            shaderTileY = ( behavior->tileY );
        }
        
    }

    // clear surface
    GPU_ClearRGBA( RenderParticlesBehavior::surface->target, 0, 0, 0, 0 );
    
    // activate shader
    behavior->SelectParticleShader(shaderU, shaderV, shaderW, shaderH,
                                   shaderTileX, shaderTileY, rotated,
                                   image, RenderParticlesBehavior::surface->target );
    
    // push view matrix
    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_PushMatrix();
    GPU_LoadIdentity();
    
    // if rendering to image / clipped
    if ( event->clippedBy ) {
        // apply inverse of container's world transform
        GPU_MatrixCopy( GPU_GetCurrentMatrix(), event->clippedBy->gameObject->InverseWorld() );
    }
    
    // set up to go over each particle
    b2ParticleSystem* ps = behavior->particles->group->GetParticleSystem();
    b2ParticleGroup* group = behavior->particles->group;
    int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount();
    b2Vec2 *positions = ps->GetPositionBuffer();
    b2ParticleColor* colors = ps->GetColorBuffer();
    b2Vec2* velocities = ps->GetVelocityBuffer();
    float radius = ps->GetRadius() * BOX2D_TO_WORLD_SCALE;
    float baseScale = ( 2 * radius + behavior->extents + ( 1 - behavior->alphaThresh ) * RenderParticlesBehavior::particleTexture->image->base_w )
                       / (float) RenderParticlesBehavior::particleTexture->image->base_w;
    float rotation = 0, lifeTime = 0, vel = 0;
    float velStretch = 0, velStretchSquared = WORLD_TO_BOX2D_SCALE * behavior->velocityStretch * behavior->velocityStretch;
    bool doFade = ( behavior->fadeTime > 0 );
    float sx = 1, sy = 1;
    
    // for each particle
    for ( ; i < last; i++ ) {
        
        // color
        b2ParticleColor &pclr = colors[ i ];
        RenderParticlesBehavior::particleTexture->image->color.r = pclr.r * behavior->color->r;
        RenderParticlesBehavior::particleTexture->image->color.g = pclr.g * behavior->color->g;
        RenderParticlesBehavior::particleTexture->image->color.b = pclr.b * behavior->color->b;
        RenderParticlesBehavior::particleTexture->image->color.a = pclr.a * behavior->color->a;
        
        // rotation
        rotation = atan2( velocities[ i ].y, velocities[ i ].x );
        
        // velocity
        vel = velocities[ i ].LengthSquared();
        if ( vel > 0 ) {
            velStretch = vel / velStretchSquared;
            sx = baseScale * ( 1 + behavior->velocityStretchFactor * fmin( 1.0, velStretch ) );
            sy = baseScale * ( 1 - 0.6 * behavior->velocityStretchFactor * fmin( 1.0, velStretch ) );
        } else {
            sx = sy = baseScale;
        }

        // lifetime
        lifeTime = ps->GetParticleLifetime( i );
        if ( doFade && lifeTime > 0 && lifeTime <= behavior->fadeTime ) {
            float fade = lifeTime / behavior->fadeTime;
            sx *= fade; sy *= fade;
        }
        
        // apply transform
        GPU_PushMatrix();
        GPU_Translate( positions[ i ].x * BOX2D_TO_WORLD_SCALE, positions[ i ].y * BOX2D_TO_WORLD_SCALE, 0 );
        GPU_Rotate( RAD_TO_DEG * rotation, 0, 0, 1 );
        GPU_Scale( sx, sy, 1 );

        // draw
        GPU_Blit(RenderParticlesBehavior::particleTexture->image,
                 &RenderParticlesBehavior::particleTexture->frame.locationOnTexture,
                 RenderParticlesBehavior::surface->target, 0, 0 );
        
        // pop transform
        GPU_PopMatrix();
        
    }

    // pop modelview
    GPU_PopMatrix();
    
    // prepare to draw surface to target
    
    // set params
    if ( behavior->blendMode == BlendMode::Cut ) {
        // cut alpha
        GPU_SetBlendFunction( RenderParticlesBehavior::surface, GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
        GPU_SetBlendEquation( RenderParticlesBehavior::surface, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
    } else {
        // normal mode
        GPU_SetBlendMode( RenderParticlesBehavior::surface, GPU_BLEND_NORMAL );
    }
    
    // set shader
    behavior->SelectTexturedShader(RenderParticlesBehavior::surface->base_w, RenderParticlesBehavior::surface->base_h,
                                   0, 0, RenderParticlesBehavior::surface->base_w, RenderParticlesBehavior::surface->base_h,
                                   0, 0, 0, 0,
                                   0, 0,
                                   1, 1,
                                   0, 0,
                                   RenderParticlesBehavior::surface, target, (GPU_Target**) event->behaviorParam2 );
    // reset transform
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_PushMatrix();
    float *pp = GPU_GetProjection();
    GPU_MatrixIdentity( pp );
    GPU_MatrixOrtho( pp, 0, target->w, 0, target->h, -1024, 1024 );
    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_PushMatrix();
    GPU_LoadIdentity();

    // set color
    RenderParticlesBehavior::surface->color = behavior->color->rgba;
    RenderParticlesBehavior::surface->color.a *= behavior->gameObject->combinedOpacity;
    
    // draw
    GPU_BlitRect( RenderParticlesBehavior::surface, &RenderParticlesBehavior::surface->target->viewport, target, &RenderParticlesBehavior::surface->target->viewport );

    // pop modelview and projection
    GPU_PopMatrix();
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_PopMatrix();
}

size_t RenderParticlesBehavior::SelectParticleShader(float u, float v, float w, float h,
                                                     float tx, float ty, bool rotated,
                                                     GPU_Image *image, GPU_Target* targ ){
    
    size_t shaderIndex = SHADER_PARTICLE | ( image ? SHADER_TEXTURE : 0 );
    
    ShaderVariant &variant = shaders[ shaderIndex ];
    if ( !variant.shader ) variant = CompileShaderWithFeatures( shaderIndex );
    
    // activate shader
    GPU_ActivateShaderProgram( variant.shader, &variant.shaderBlock );
    
    // set params
    float params[ 4 ];
    
    // background
    if ( variant.backgroundUniform >= 0 ) {
        GPU_SetShaderImage( image, variant.backgroundUniform, 1 );
        
        // rotated flag
        if ( variant.backgroundSizeUniform >= 0 ) {
            params[ 0 ] = rotated ? 1 : 0;
            params[ 1 ] = 1;
            GPU_SetUniformfv( variant.backgroundSizeUniform, 2, 1, params );
        }
        
        // tile
        if ( variant.tileUniform >= 0 ) {
            params[ 1 ] = ty;
            params[ 0 ] = tx;
            GPU_SetUniformfv( variant.tileUniform, 2, 1, params );
        }
        
        // texture size
        if ( variant.texSizeUniform >= 0 ) {
            params[ 0 ] = image->base_w;
            params[ 1 ] = image->base_h;
            GPU_SetUniformfv( variant.texSizeUniform, 2, 1, params );
        }
        
        // scroll
        if ( variant.scrollOffsetUniform >= 0 ) {
            params[ 0 ] = this->offsetX;
            params[ 1 ] = this->offsetY;
            GPU_SetUniformfv( variant.scrollOffsetUniform, 2, 1, params );
        }
        
        // sprite on texture
        if ( variant.texInfoUniform >= 0 ) {
            params[ 0 ] = u;
            params[ 1 ] = v;
            params[ 2 ] = w;
            params[ 3 ] = h;
            GPU_SetUniformfv( variant.texInfoUniform, 4, 1, params );
        }
        
    }
    
    // addColor
    if ( variant.addColorUniform >= 0 ) {
        params[ 0 ] = this->addColor->r;
        params[ 1 ] = this->addColor->g;
        params[ 2 ] = this->addColor->b;
        params[ 3 ] = this->addColor->a;
        GPU_SetUniformfv( variant.addColorUniform, 4, 1, params );
    }
    
    return shaderIndex;
    
}
