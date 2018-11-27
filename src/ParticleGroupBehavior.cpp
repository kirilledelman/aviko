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
    this->groupDef.groupFlags = b2_particleGroupCanBeEmpty | b2_solidParticleGroup;
    this->groupDef.flags = b2_fixtureContactFilterParticle | b2_particleContactFilterParticle;
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
    
    void* constants = script.NewObject();
    script.AddGlobalNamedObject( "ParticleFlags", constants );
    script.SetProperty( "Zombie", ArgValue( b2_zombieParticle ), constants );
    script.SetProperty( "Wall", ArgValue( b2_wallParticle ), constants );
    script.SetProperty( "Spring", ArgValue( b2_springParticle ), constants );
    script.SetProperty( "Elastic", ArgValue( b2_elasticParticle ), constants );
    script.SetProperty( "Viscous", ArgValue( b2_viscousParticle ), constants );
    script.SetProperty( "Powder", ArgValue( b2_powderParticle ), constants );
    script.SetProperty( "Tensile", ArgValue( b2_tensileParticle ), constants );
    script.SetProperty( "ColorMixing", ArgValue( b2_colorMixingParticle ), constants );
    script.SetProperty( "DestructionEvent", ArgValue( b2_destructionListenerParticle ), constants );
    script.SetProperty( "Barrier", ArgValue( b2_barrierParticle ), constants );
    script.SetProperty( "StaticPressure", ArgValue( b2_staticPressureParticle ), constants );
    script.SetProperty( "Reactive", ArgValue( b2_reactiveParticle ), constants );
    script.SetProperty( "Repulsive", ArgValue( b2_repulsiveParticle ), constants );
    script.SetProperty( "CollisionEvent", ArgValue( b2_fixtureContactListenerParticle | b2_particleContactListenerParticle ), constants );
    script.FreezeObject( constants );
    
    /* enum b2ParticleFlag
     {
     /// Water particle.
     b2_waterParticle = 0,
     /// Removed after next simulation step.
     b2_zombieParticle = 1 << 1,
     /// Zero velocity.
     b2_wallParticle = 1 << 2,
     /// With restitution from stretching.
     b2_springParticle = 1 << 3,
     /// With restitution from deformation.
     b2_elasticParticle = 1 << 4,
     /// With viscosity.
     b2_viscousParticle = 1 << 5,
     /// Without isotropic pressure.
     b2_powderParticle = 1 << 6,
     /// With surface tension.
     b2_tensileParticle = 1 << 7,
     /// Mix color between contacting particles.
     b2_colorMixingParticle = 1 << 8,
     /// Call b2DestructionListener on destruction.
     b2_destructionListenerParticle = 1 << 9,
     /// Prevents other particles from leaking.
     b2_barrierParticle = 1 << 10,
     /// Less compressibility.
     b2_staticPressureParticle = 1 << 11,
     /// Makes pairs or triads with other particles.
     b2_reactiveParticle = 1 << 12,
     /// With high repulsive force.
     b2_repulsiveParticle = 1 << 13,
     /// Call b2ContactListener when this particle is about to interact with
     /// a rigid body or stops interacting with a rigid body.
     /// This results in an expensive operation compared to using
     /// b2_fixtureContactFilterParticle to detect collisions between
     /// particles.
     b2_fixtureContactListenerParticle = 1 << 14,
     /// Call b2ContactListener when this particle is about to interact with
     /// another particle or stops interacting with another particle.
     /// This results in an expensive operation compared to using
     /// b2_particleContactFilterParticle to detect collisions between
     /// particles.
     b2_particleContactListenerParticle = 1 << 15,
     /// Call b2ContactFilter when this particle interacts with rigid bodies.
     b2_fixtureContactFilterParticle = 1 << 16,
     /// Call b2ContactFilter when this particle interacts with other
     /// particles.
     b2_particleContactFilterParticle = 1 << 17,
     };
     */
    
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
    
    script.AddProperty<ParticleGroupBehavior>
    ( "stride",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) { return ((ParticleGroupBehavior*)p)->groupDef.stride * BOX2D_TO_WORLD_SCALE; }),
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.stride = val * WORLD_TO_BOX2D_SCALE;
        if ( self->particleSystem && self->particleSystem->scene ) {
            self->RemoveBody();
            self->AddBody( self->particleSystem->scene );
        }
        return val;
    }));
    
   // TODO - particle color - update all particles on the fly
    
    script.AddProperty<ParticleGroupBehavior>
    ( "solid",
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        return ((ParticleGroupBehavior*)p)->groupDef.groupFlags & b2_solidParticleGroup; }),
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( val ) {
            self->groupDef.groupFlags |= b2_solidParticleGroup;
        } else {
            self->groupDef.groupFlags &= ~b2_solidParticleGroup;
        }
        if ( self->group ) self->group->SetGroupFlags( self->groupDef.groupFlags );
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "rigid",
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        return ((ParticleGroupBehavior*)p)->groupDef.groupFlags & b2_rigidParticleGroup; }),
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( val ) {
            self->groupDef.groupFlags |= b2_rigidParticleGroup;
        } else {
            self->groupDef.groupFlags &= b2_rigidParticleGroup;
        }
        if ( self->group ) self->group->SetGroupFlags( self->groupDef.groupFlags );
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "cohesion",
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        return ((ParticleGroupBehavior*)p)->groupDef.strength; }),
     static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        float pv = self->groupDef.strength;
        self->groupDef.strength = val;
        if ( pv != val && self->particleSystem && self->particleSystem->scene ) {
            self->RemoveBody();
            self->AddBody( self->particleSystem->scene );
        }
        return val;
    }));
    
    // particle flags
    script.AddProperty<ParticleGroupBehavior>
    ( "flags",
     static_cast<ScriptIntCallback>([]( void* p, int val ) {
        int32 flags = ((ParticleGroupBehavior*)p)->groupDef.flags;
        return ( flags & ~b2_fixtureContactFilterParticle ) & ~b2_particleContactFilterParticle;
    }),
     static_cast<ScriptIntCallback>([]( void* p, int val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.flags = ( val | b2_fixtureContactFilterParticle | b2_particleContactFilterParticle );
        
        // re-apply to all particles
        if ( self->group ) {
            // TODO
        }
        return val;
    }));
    
    // TODO - velocity / ang velocity
    
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
    this->gameObject->body = NULL;
    
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
    if ( this->group ) {
        // TODO offset / rotate group?
    }
    
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
            if ( newSystem->scene && this->gameObject && !this->gameObject->orphan ) {
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
    
    // sanity check
    if ( this->group || !this->_active || !this->gameObject || this->gameObject->orphan || !this->gameObject->activeRecursive() ||
        !scene || !scene->particleSystems.size() || ( this->particleSystem && this->particleSystem->scene != scene ) ) {
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
    b2Vec2 wpos, wscale;
    float wangle;
    this->gameObject->DecomposeTransform( this->gameObject->WorldTransform(), wpos, wangle, wscale );
    
    // create group
    b2ParticleSystem* ps = this->particleSystem->particleSystem;
    
    /// populate with stored points
    if ( this->points.size() ) {
        
        // reset
        this->groupDef.position.SetZero();
        this->groupDef.angle = 0 ;
        
        // make points
        vector<b2Vec2> pts;
        int32 count = (int32) this->points.size();
        pts.resize( count );
        for ( int32 i = 0; i < count; i++ ) {
            b2ParticleDef& pd = this->points[ i ];
            pts[ i ] = pd.position;
        }
        this->groupDef.positionData = pts.data();
        this->groupDef.particleCount = count;
        
        // create
        this->group = ps->CreateParticleGroup( this->groupDef );
        
        // update other params
        int32 offset = this->group->GetBufferIndex();
        b2Vec2* velocityBuf = this->particleSystem->particleSystem->GetVelocityBuffer();
        b2ParticleColor* colorBuf = this->particleSystem->particleSystem->GetColorBuffer();
        float32* weightBuf = this->particleSystem->particleSystem->GetWeightBuffer();
        for ( int32 i = offset, last = offset + count; i < last; i++ ) {
            b2ParticleDef& pd = this->points[ i - offset ];
            ps->SetParticleFlags( i, pd.flags );
            ps->SetParticleLifetime( i, pd.lifetime );
            velocityBuf[ i ].Set( pd.velocity.x, pd.velocity.y );
            colorBuf[ i ].Set( pd.color.r, pd.color.g, pd.color.b, pd.color.a );
            weightBuf[ i ] = (float32) ( ( (long) pd.userData ) * 0.001f );
        }
        
        this->points.clear();
        this->groupDef.particleCount = 0;
        this->groupDef.positionData = NULL;
        
    } else if ( this->shape ) {

        this->groupDef.position = wpos * WORLD_TO_BOX2D_SCALE;
        this->groupDef.angle = wangle * DEG_TO_RAD;
        
        this->shape->MakeShapesList();
        this->groupDef.shapes = this->shape->shapes.data();
        this->groupDef.shapeCount = (int32) this->shape->shapes.size();
        this->group = ps->CreateParticleGroup( this->groupDef );
        this->groupDef.shapeCount = 0;
        this->groupDef.shapes = NULL;
        
    }
    
    // make live
    this->live = true;
    
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
        float32* weights = ps->GetWeightBuffer();
        for (; i < last; i++) {
            this->points.emplace_back();
            b2ParticleDef &pf = this->points.back();
            pf.flags = ps->GetParticleFlags( i );
            pf.color = colors[ i ];
            pf.lifetime = ps->GetParticleLifetime( i );
            pf.position = positions[ i ];
            pf.velocity = velocities[ i ];
            pf.userData = (void*) ((long) weights[ i ] * 1000);
            
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

