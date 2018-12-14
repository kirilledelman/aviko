#include "RenderParticlesBehavior.hpp"
#include "Application.hpp"

/* MARK:    -                Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderParticlesBehavior::RenderParticlesBehavior( ScriptArguments* args ) : RenderParticlesBehavior() {
    
    // add scriptObject
    script.NewScriptObject<RenderParticlesBehavior>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );
    
    // add defaults
    RenderBehavior::AddDefaults();
    
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
    
    // release surface
    if ( this->surface ) {
        GPU_FreeTarget( this->surface->target );
        GPU_FreeImage( this->surface );
        this->surface = NULL;
    }
    
}


/* MARK:    -                Javascript
 -------------------------------------------------------------------- */


// init script classes
void RenderParticlesBehavior::InitClass() {
    
    // register class
    script.RegisterClass<RenderParticlesBehavior>( "RenderBehavior" );

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
        
    // if surface size doesn't match target
    if ( behavior->surface && ( behavior->surface->base_w != target->base_w || behavior->surface->base_h != target->base_h ) ) {
        GPU_FreeTarget( behavior->surface->target );
        GPU_FreeImage( behavior->surface );
        behavior->surface = NULL;
    }
    
    // if no surface
    if ( !behavior->surface ) {
        // create it
        behavior->surface = GPU_CreateImage( target->base_w, target->base_h, GPU_FORMAT_RGBA );
        if ( !behavior->surface ) return;
        GPU_UnsetImageVirtualResolution( behavior->surface );
        GPU_SetImageFilter( behavior->surface, GPU_FILTER_NEAREST );
        GPU_SetSnapMode( behavior->surface, GPU_SNAP_NONE );
        GPU_LoadTarget( behavior->surface );
        behavior->surface->anchor_x = behavior->surface->anchor_y = 0;
        GPU_ActivateShaderProgram( 0, NULL );
        GPU_SetShapeBlendFunction( GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
        GPU_SetShapeBlendEquation( GPU_EQ_ADD, GPU_EQ_ADD);
    }

    // draw particles to surface
    GPU_ClearRGBA( behavior->surface->target, 0, 0, 0, 128 );
    
    b2ParticleSystem* ps = behavior->particles->group->GetParticleSystem();
    b2ParticleGroup* group = behavior->particles->group;
    float radius = ps->GetRadius();
    int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount();
    b2Vec2 *positions = ps->GetPositionBuffer();
    b2ParticleColor* colors = ps->GetColorBuffer();
    SDL_Color clr;
    for ( ; i < last; i++ ) {
        clr = behavior->color->rgba;
        GPU_Circle( behavior->surface->target,
                   positions[ i ].x * BOX2D_TO_WORLD_SCALE,
                   positions[ i ].y * BOX2D_TO_WORLD_SCALE, radius, clr );
    }

    // prepare to draw surface to target
    
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
    
    // set shader
    behavior->SelectTexturedShader(
                                   behavior->surface->base_w, behavior->surface->base_h,
                                   0, 0, behavior->surface->base_w, behavior->surface->base_h,
                                   0, 0, 0, 0,
                                   0, 0,
                                   1, 1,
                                   behavior->surface, target, (GPU_Target**) event->behaviorParam2 );
    // reset transform
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_PushMatrix();
    GPU_LoadIdentity();
    GPU_MatrixMode( GPU_MODELVIEW );
    GPU_PushMatrix();
    GPU_LoadIdentity();

    // draw
    GPU_BlitRect( behavior->surface, &behavior->surface->target->viewport, target, &behavior->surface->target->viewport );

    GPU_PopMatrix();
    GPU_MatrixMode( GPU_PROJECTION );
    GPU_PopMatrix();

}
