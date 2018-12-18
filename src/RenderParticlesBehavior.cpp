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
    
    // anything above this alpha val is drawn as solid
    this->alphaThresh = 0.1;
    
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
    RenderParticlesBehavior::particleTexture = app.textureManager.Get( "#particle" );
    RenderParticlesBehavior::particleTexture->LoadFromMemory( ParticleTexture, ParticleTexture_size );
    RenderParticlesBehavior::particleTexture->image->anchor_x = RenderParticlesBehavior::particleTexture->image->anchor_y = 0.5;
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
    
    // no particles
    if ( !behavior->particles || !behavior->particles->group ) return;
    
    // params
    SDL_Color color = behavior->color->rgba;
    color.a *= behavior->gameObject->combinedOpacity;
    if ( color.a == 0.0 ) return;
    
    // if surface size doesn't match target
    if ( RenderParticlesBehavior::surface &&
        ( RenderParticlesBehavior::surface->base_w != target->base_w || RenderParticlesBehavior::surface->base_h != target->base_h ) ) {
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

    // draw particles to surface
    GPU_ClearRGBA( RenderParticlesBehavior::surface->target, 0, 0, 0, 0 );
    
    // behavior->alphaThresh = 0.25;
    
    // set shader etc
    GPU_SetBlendFunction( RenderParticlesBehavior::surface, GPU_FUNC_ZERO, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_ZERO, GPU_FUNC_ONE );
    GPU_SetBlendEquation( RenderParticlesBehavior::surface, GPU_EQ_ADD, GPU_EQ_ADD);
    GPU_Image* image = NULL;
    GPU_Rect srcRect = { 0, 0, 0, 0 };
    bool rotated = false;
    float cx = 0, cy = 0, sx = 1, sy = 1;
    float effectiveWidth = 0, effectiveHeight = 0;
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
        effectiveWidth = fmax( 0, frame->actualWidth );
        effectiveHeight = fmax( 0, frame->actualHeight );
        
        // draw rotated
        if ( rotated ) {
            sy = effectiveWidth / (frame->actualWidth - frame->trimWidth);
            sx = effectiveHeight / (frame->actualHeight - frame->trimHeight);
            cx = frame->trimOffsetX * sy;
            cy = ( frame->locationOnTexture.w + frame->trimOffsetY ) * sx;
            // normal
        } else {
            sx = effectiveWidth / (frame->actualWidth - frame->trimWidth);
            sy = effectiveHeight / (frame->actualHeight - frame->trimHeight);
            cx = frame->trimOffsetX * sx;
            cy = frame->trimOffsetY * sy;
        }
        
        // shader params
        shaderU = srcRect.x;
        shaderV = srcRect.y;
        shaderW = srcRect.w;
        shaderH = srcRect.h;
        if ( rotated ) {
            cx = floor( cx - ( effectiveWidth * behavior->pivotX + behavior->texturePad * sy ) );
            cy = floor( cy - ( effectiveHeight * behavior->pivotY - behavior->texturePad * sx ) );
        } else {
            cx = floor( cx - ( effectiveWidth * behavior->pivotX + behavior->texturePad * sx ) );
            cy = floor( cy - ( effectiveHeight * behavior->pivotY + behavior->texturePad * sy ) );
        }
        
    }
    
    // tiling
    float shaderTileX, shaderTileY;
    if ( rotated ) {
        shaderTileX = ( behavior->tileY * sx );
        shaderTileY = ( behavior->tileX * sy );
        
    } else {
        shaderTileX = ( behavior->tileX * sx );
        shaderTileY = ( behavior->tileY * sy );
    }
    
    // activate shader
    behavior->SelectParticleShader(
                                   effectiveWidth, effectiveHeight,
                                   shaderU, shaderV, shaderW, shaderH,
                                   shaderTileX, shaderTileY,
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
    
    // go over each particle
    b2ParticleSystem* ps = behavior->particles->group->GetParticleSystem();
    b2ParticleGroup* group = behavior->particles->group;
    int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount();
    b2Vec2 *positions = ps->GetPositionBuffer();
    b2ParticleColor* colors = ps->GetColorBuffer();
    b2Vec2* velocities = ps->GetVelocityBuffer();
    float radius = ps->GetRadius() * BOX2D_TO_WORLD_SCALE;
    for ( ; i < last; i++ ) {
        // color
        b2ParticleColor &pclr = colors[ i ];
        RenderParticlesBehavior::particleTexture->image->color.r = pclr.r * behavior->color->r;
        RenderParticlesBehavior::particleTexture->image->color.g = pclr.g * behavior->color->g;
        RenderParticlesBehavior::particleTexture->image->color.b = pclr.b * behavior->color->b;
        RenderParticlesBehavior::particleTexture->image->color.a = pclr.a * behavior->color->a;

        // rotation
        
        // scale / lifetime
        float lifeTime = ps->GetParticleLifetime( i );
        float rotation = 0;
        sx = 1; sy = 1;
        
        GPU_BlitTransform(RenderParticlesBehavior::particleTexture->image,
                          &RenderParticlesBehavior::particleTexture->frame.locationOnTexture,
                          RenderParticlesBehavior::surface->target, positions[ i ].x * BOX2D_TO_WORLD_SCALE,
                          positions[ i ].y * BOX2D_TO_WORLD_SCALE,
                          rotation, sx, sy );
        
    }

    GPU_PopMatrix();
    
    // prepare to draw surface to target
    
    // set params
    if ( behavior->blendMode == BlendMode::Cut ) {
        // cut alpha
        GPU_SetBlendFunction( RenderParticlesBehavior::surface, GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
        GPU_SetBlendEquation( RenderParticlesBehavior::surface, GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
    } else {
        // normal mode
        GPU_SetBlendFunction( RenderParticlesBehavior::surface, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
        GPU_SetBlendEquation( RenderParticlesBehavior::surface, GPU_EQ_ADD, GPU_EQ_ADD);
    }
    
    // set shader
    behavior->SelectTexturedShader(RenderParticlesBehavior::surface->base_w, RenderParticlesBehavior::surface->base_h,
                                   0, 0, RenderParticlesBehavior::surface->base_w, RenderParticlesBehavior::surface->base_h,
                                   0, 0, 0, 0,
                                   0, 0,
                                   1, 1,
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

    // opacity
    RenderParticlesBehavior::surface->color.a = 255 * behavior->gameObject->combinedOpacity;
    
    // draw
    GPU_BlitRect( RenderParticlesBehavior::surface, &RenderParticlesBehavior::surface->target->viewport, target, &RenderParticlesBehavior::surface->target->viewport );

    GPU_PopMatrix();
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_PopMatrix();

}

size_t RenderParticlesBehavior::SelectParticleShader(float tw, float th,
                                                     float u, float v, float w, float h,
                                                     float tx, float ty,
                                                     GPU_Image *image, GPU_Target* targ ){
    
    size_t shaderIndex = SHADER_PARTICLE | ( image ? SHADER_TEXTURE : 0 );
    if ( tx != 1 || ty != 1 ) shaderIndex |= SHADER_TILE;
    
    ShaderVariant &variant = shaders[ shaderIndex ];
    if ( !variant.shader ) variant = CompileShaderWithFeatures( shaderIndex );
    
    //
    
    // activate shader
    GPU_ActivateShaderProgram( variant.shader, &variant.shaderBlock );
    
    // set params
    float params[ 4 ];
    
    // background
    if ( variant.backgroundUniform >= 0 ) {
        GPU_SetShaderImage( image, variant.backgroundUniform, 1 );
    }
    
    // addColor
    if ( variant.addColorUniform >= 0 ) {
        params[ 0 ] = this->addColor->r;
        params[ 1 ] = this->addColor->g;
        params[ 2 ] = this->addColor->b;
        params[ 3 ] = this->addColor->a;
        GPU_SetUniformfv( variant.addColorUniform, 4, 1, params );
    }
    
    // tile
    if ( variant.tileUniform >= 0 ) {
        params[ 1 ] = ty;
        params[ 0 ] = tx;
        GPU_SetUniformfv( variant.tileUniform, 2, 1, params );
    }
    
    // pad
    if ( variant.texPadUniform >= 0 ) {
        GPU_SetUniformf( variant.texPadUniform, (float) this->texturePad );
    }
    
    // texture size
    if ( variant.texSizeUniform >= 0 ) {
        params[ 0 ] = tw;
        params[ 1 ] = th;
        GPU_SetUniformfv( variant.texSizeUniform, 2, 1, params );
    }
    
    // sprite on texture
    if ( variant.texInfoUniform >= 0 ) {
        params[ 0 ] = u;
        params[ 1 ] = v;
        params[ 2 ] = w;
        params[ 3 ] = h;
        GPU_SetUniformfv( variant.texInfoUniform, 4, 1, params );
    }
    
    return shaderIndex;
    
}
