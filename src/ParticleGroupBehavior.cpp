#include "ParticleGroupBehavior.hpp"

#include "GameObject.hpp"
#include "Scene.hpp"
#include "ScriptHost.hpp"


/* MARK:    -                Init / destroy
 -------------------------------------------------------------------- */


// creating from script
ParticleGroupBehavior::ParticleGroupBehavior( ScriptArguments* args ) : ParticleGroupBehavior() {
    
    // add scriptObject
    script.NewScriptObject<ParticleGroupBehavior>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );
    
    // defaults
    this->groupDef.flags = b2_particleGroupCanBeEmpty | b2_solidParticleGroup;
    this->groupDef.userData = this;
    
    // obj argument - init object
    void *initObj = NULL;
    if ( args && args->ReadArguments( 1, TypeObject, &initObj ) ) {
        script.CopyProperties( initObj, this->scriptObject );
    }
    
}

// init
ParticleGroupBehavior::ParticleGroupBehavior() : BodyBehavior::BodyBehavior() {}

// destroy
ParticleGroupBehavior::~ParticleGroupBehavior() {
    
    // remove self from particle system groups
    if ( this->particleSystem ) this->SetSystem( NULL );
        
}


/* MARK:    -                Javascript
 -------------------------------------------------------------------- */


// init script classes
void ParticleGroupBehavior::InitClass() {
    
    // register class
    script.RegisterClass<ParticleGroupBehavior>( "BodyBehavior" );
    
    // constants
    
    // properties
    
    script.AddProperty<ParticleGroupBehavior>
    ( "particleSystem",
     static_cast<ScriptObjectCallback>([](void* go, void* p) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) go;
        return self->particleSystem ? self->particleSystem->scriptObject : NULL;
    }),
     static_cast<ScriptObjectCallback>([](void* go, void* p) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) go;
        ParticleSystem* other = script.GetInstance<ParticleSystem>( p );
        self->SetSystem( other );
        return self->particleSystem ? self->particleSystem->scriptObject : NULL;
    }), PROP_ENUMERABLE | PROP_NOSTORE );
    
    script.AddProperty<ParticleGroupBehavior>
    ( "shape",
     static_cast<ScriptObjectCallback>([](void* go, void* p) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) go;
        return self->shape ? self->shape->scriptObject : NULL;
    }),
     static_cast<ScriptObjectCallback>([](void* go, void* p) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) go;
        RigidBodyShape* other = script.GetInstance<RigidBodyShape>( p );
        self->shape = other;
        // replacing shape repopulates
        if ( other && self->particleSystem && self->particleSystem->scene ) {
            self->RemoveBody();
            self->AddBody( self->particleSystem->scene );
        }
        return self->shape ? self->shape->scriptObject : NULL;
    }), PROP_ENUMERABLE | PROP_NOSTORE );
    
}

void ParticleGroupBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
    
    // store particle system
    if ( this->particleSystem ) protectedObjects.push_back( &this->particleSystem->scriptObject );
    
    // shape
    if ( this->shape ) protectedObjects.push_back( &this->shape->scriptObject );
    
    // call super
    Behavior::TraceProtectedObjects( protectedObjects );
    
}


/* MARK:    -                Velocity and impulse
 -------------------------------------------------------------------- */


/* MARK:    -                Sync body <-> object
 -------------------------------------------------------------------- */


/// copies body transform to game object
void ParticleGroupBehavior::SyncObjectToBody() {
    
    if ( !this->group || !this->live ) return;
    printf( "void ParticleGroupBehavior::SyncObjectToBody" );
    
    // copy from body
    /*b2Vec2 pos = this->body->GetPosition();
    float angle = (float) this->body->GetAngle() * RAD_TO_DEG;
    
    if ( isnan( pos.x) || isnan( pos.y ) ) {
        printf( "RigidBodyBehavior::SyncObjectToBody pos NAN\n" );
        return;
    }
    
    pos *= BOX2D_TO_WORLD_SCALE;
    
    // construct world transform matrix for object
    GPU_MatrixIdentity( this->gameObject->_worldTransform );
    GPU_MatrixTranslate( this->gameObject->_worldTransform, pos.x, pos.y, this->gameObject->_z );
    if ( angle != 0 ) GPU_MatrixRotate( this->gameObject->_worldTransform, angle, 0, 0, 1 );
    if ( this->gameObject->_scale.x != 1 || this->gameObject->_scale.y != 1 ) GPU_MatrixScale( this->gameObject->_worldTransform, this->gameObject->_scale.x, this->gameObject->_scale.y, 1 );
    if ( this->gameObject->_skew.x != 1 || this->gameObject->_skew.y != 1 ) MatrixSkew( this->gameObject->_worldTransform, this->gameObject->_skew.x, this->gameObject->_skew.y );
    this->gameObject->_worldTransformDirty = false;
    this->gameObject->_localCoordsAreDirty = this->gameObject->_inverseWorldDirty = this->gameObject->_transformDirty = true;
    */
}

/// converts game object's local transform to body
void ParticleGroupBehavior::SyncBodyToObject() {
    
    // unlink body
    /*this->gameObject->body = NULL;
    
    // parent's world transform times local transform = this object world transform
    if ( this->gameObject->parent ) GPU_MatrixMultiply( this->gameObject->_worldTransform, this->gameObject->parent->WorldTransform(), this->gameObject->Transform() );
    this->gameObject->_worldTransformDirty = false;
    
    // relink
    this->gameObject->body = this;
    
    // extract pos, rot, scale
    b2Vec2 pos, scale;
    float angle;
    this->gameObject->DecomposeTransform( this->gameObject->_worldTransform, pos, angle, scale );
    
    // update body
    pos *= WORLD_TO_BOX2D_SCALE;
    if ( this->body ) {
        this->body->SetTransform( pos, angle * DEG_TO_RAD );
        if ( !this->body->IsAwake() ) this->body->SetAwake( true );
    }*/
    
}

/* MARK:    -                Shapes
 -------------------------------------------------------------------- */

/* MARK:    -                Attached / removed / active
 -------------------------------------------------------------------- */

/// attach to particle system
void ParticleGroupBehavior::SetSystem( ParticleSystem* newSystem ) {
    
    // changed
    if ( this->particleSystem != newSystem ) {
        
        // remove particles
        this->RemoveBody();
        
        // add to groups
        if ( this->particleSystem ) this->particleSystem->groups.erase( this );
        
        // new sys
        this->particleSystem = newSystem;
        
        // add to new
        if ( newSystem ) {
            
            // add self
            newSystem->groups.insert( this );
            
            // new system is on scene, and this object isn't an orphan
            if ( newSystem->scene && !this->gameObject->orphan ) {
                // create group
                this->AddBody( newSystem->scene );
            }
        }
    }
    
}

/// overrides behavior active setter
void ParticleGroupBehavior::EnableBody( bool e ) {
    
    if ( e && !this->group && this->particleSystem ) this->AddBody( this->particleSystem->scene );
    else if ( !e && this->group ) this->RemoveBody();
    
    this->live = ( this->group != NULL );
    
}

/// called when gameObject is added to scene
void ParticleGroupBehavior::AddBody( Scene *scene ) {
    
    if ( this->group || !this->_active || !this->gameObject || !this->gameObject->activeRecursive() ) {
        return;
    }

    if ( !scene || !scene->particleSystems.size() || ( this->particleSystem && this->particleSystem->scene != scene ) ) {
        return;
    }

    // if particleSystem is null, set to last one in scene, it will call AddBody again
    if ( !this->particleSystem ) {
        this->SetSystem( scene->particleSystems.back() );
        return;
    }
    
    // particle system isn't live?
    if ( !this->particleSystem->particleSystem ) {
        this->particleSystem->AddToWorld(); // will call AddBody again
        return;
    }
    
    // if there are no points, or shape, ignore
    if ( !this->shape && !this->points.size() ) return;

    // get origin
    b2Vec2 testPos = this->gameObject->GetWorldPosition();
    
    // create group
    this->groupDef.position = testPos * WORLD_TO_BOX2D_SCALE;
    this->groupDef.groupFlags = b2_rigidParticleGroup;
    this->group = this->particleSystem->particleSystem->CreateParticleGroup( this->groupDef );
    
    /// populate with shape
    if ( this->shape ) {
        
        
    } else if ( this->points.size() ) {
        
    }
    
    // TODO - populate group with store particle info
    
    printf( "AddBody successful %p\n ", this );
    b2ParticleDef pd;
    pd.group = this->group;
    for ( int i = 0; i < 10; i++ ) {
        pd.position.x = cos( 2 * M_PI * i / 10.0 ) * 20 * WORLD_TO_BOX2D_SCALE;
        pd.position.y = sin( 2 * M_PI * i / 10.0 ) * 20 * WORLD_TO_BOX2D_SCALE;
        this->particleSystem->particleSystem->CreateParticle( pd );
    }
    
    // make active, if gameObject and behavior are active
    this->EnableBody( this->gameObject->active() && this->_active );
    
}

void ParticleGroupBehavior::RemoveBody() {
    
    this->live = false;
    if ( this->group ) {
        
        // store particles info
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        int32 i = this->group->GetBufferIndex();
        int32 last = i + this->group->GetParticleCount();
        this->points.clear();
        b2ParticleColor* colors = ps->GetColorBuffer();
        b2Vec2* positions = ps->GetPositionBuffer();
        b2Vec2* velocities = ps->GetVelocityBuffer();
        for (; i < last; i++) {
            this->points.emplace_back();
            b2ParticleDef &pf = this->points.back();
            pf.flags = ps->GetParticleFlags( i );
            pf.color = colors[ i ];
            pf.lifetime = ps->GetParticleLifetime( i );
            pf.position = positions[ i ];
            pf.velocity = velocities[ i ];
            
            // delete next step
            ps->SetParticleFlags( i, b2_zombieParticle );
        }
        
        // group will be deleted next step
        this->group->SetUserData( NULL );
        this->group->SetGroupFlags( ~b2_particleGroupCanBeEmpty ); // makes empty group be deleted next iteration
        this->group->DestroyParticles( false );
        printf( "Group destroyed %p (%d)\n", this, this->group->GetParticleCount() );
        this->group = NULL;
        
    }
    
}

