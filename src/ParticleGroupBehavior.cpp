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
    this->groupDef.groupFlags = b2_particleGroupCanBeEmpty | b2_solidParticleGroup | b2_rigidParticleGroup;
    this->groupDef.flags = b2_fixtureContactFilterParticle | b2_particleContactFilterParticle | b2_destructionListenerParticle | b2_particleContactListenerParticle | b2_colorMixingParticle;
    this->groupDef.userData = NULL;
    
    // create color object
    colorUpdated = static_cast<ColorCallback>([this](Color* c){ this->UpdateColor(); });
    Color *clr = new Color( NULL );
    clr->callback = colorUpdated;
    script.SetProperty( "color", ArgValue( clr->scriptObject ), this->scriptObject );
    
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
    // script.SetProperty( "DestructionEvent", ArgValue( b2_destructionListenerParticle ), constants );
    script.SetProperty( "Barrier", ArgValue( b2_barrierParticle ), constants );
    script.SetProperty( "StaticPressure", ArgValue( b2_staticPressureParticle ), constants );
    script.SetProperty( "Reactive", ArgValue( b2_reactiveParticle ), constants );
    script.SetProperty( "Repulsive", ArgValue( b2_repulsiveParticle ), constants );
    // script.SetProperty( "CollisionEvent", ArgValue( b2_fixtureContactListenerParticle | b2_particleContactListenerParticle ), constants );
    script.FreezeObject( constants );
    
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
        if ( other && self->group ) {
            self->RemoveBody();
            self->AddBody( self->particleSystem->scene );
        }
        return self->shape ? self->shape->scriptObject : NULL;
    }), PROP_ENUMERABLE | PROP_NOSTORE );
    
    script.AddProperty<ParticleGroupBehavior>
    ( "color",
     static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((ParticleGroupBehavior*) b)->color->scriptObject); }),
     static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
        ParticleGroupBehavior* pg = (ParticleGroupBehavior*) b;
        if ( val.type == TypeObject ) {
            // replace if it's a color
            Color* other = script.GetInstance<Color>( val.value.objectValue );
            if ( other ) pg->color = other;
        } else {
            pg->color->Set( val );
        }
        SDL_Color sclr = pg->color->rgba;
        pg->groupDef.color.Set( sclr.r, sclr.g, sclr.b, sclr.a );

        // update colors
        pg->UpdateColor();
        return pg->color->scriptObject;
    }) );
    
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
            self->groupDef.groupFlags &= ~b2_rigidParticleGroup;
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
        return (((( flags & ~b2_fixtureContactFilterParticle )
                & ~b2_particleContactFilterParticle )
                & ~b2_destructionListenerParticle )
                & ~b2_particleContactListenerParticle );
    }),
     static_cast<ScriptIntCallback>([]( void* p, int val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.flags = ( val | b2_fixtureContactFilterParticle | b2_particleContactFilterParticle | b2_destructionListenerParticle | b2_particleContactListenerParticle );
        self->UpdateFlags();
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "velocityX",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return self->group->GetLinearVelocity().x * BOX2D_TO_WORLD_SCALE;
        return self->groupDef.linearVelocity.x;
    }),
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.linearVelocity.x = val * WORLD_TO_BOX2D_SCALE;
        self->UpdateVelocity( true, false );
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "velocityY",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return self->group->GetLinearVelocity().y * BOX2D_TO_WORLD_SCALE;
        return self->groupDef.linearVelocity.y;
    }),
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.linearVelocity.y = val * WORLD_TO_BOX2D_SCALE;
        self->UpdateVelocity( false, true );
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "lifetime",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        return self->groupDef.lifetime;
    }),
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.lifetime = val;
        self->UpdateLifetime();
        return val;
    }));
    
    script.AddProperty<ParticleGroupBehavior>
    ( "particles",
     static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((ParticleGroupBehavior*) go)->GetParticleVector(); }),
     static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((ParticleGroupBehavior*) go)->SetParticleVector( in ); }),
     PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE | PROP_LATE
     );

    script.AddProperty<ParticleGroupBehavior>
    ( "angularVelocity",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return self->group->GetAngularVelocity();
        return self->groupDef.angularVelocity * RAD_TO_DEG;
    }),
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        self->groupDef.angularVelocity = DEG_TO_RAD * val;
        self->UpdateAngularVelocity();
        return val;
    }));
    
    // mass
    script.AddProperty<ParticleGroupBehavior>
    ( "mass",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return self->group->GetMass();
        return 0.0f;
    }));
    
    // inertia
    script.AddProperty<ParticleGroupBehavior>
    ( "inertia",
     static_cast<ScriptFloatCallback>([]( void* p, float val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return self->group->GetInertia();
        return 0.0f;
    }));
    
    // mass
    script.AddProperty<ParticleGroupBehavior>
    ( "numParticles",
     static_cast<ScriptIntCallback>([]( void* p, int val ) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*)p;
        if ( self->group ) return (int) self->group->GetParticleCount();
        else return (int) self->points.size();
    }));
    
    // methods
    
    script.DefineFunction<ParticleGroupBehavior>
    ("impulse",
     static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) p;
        const char *error = "usage: impulse( Number linearImpulseX, Number linearImpulseY )";
        b2Vec2 imp = { 0, 0 };
        if ( !sa.ReadArguments( 2, TypeFloat, &imp.x, TypeFloat, &imp.y ) ) {
            script.ReportError( error );
            return false;
        }
        // apply
        if ( self->group ) self->group->ApplyLinearImpulse( imp * WORLD_TO_BOX2D_SCALE );
        return true;
    } ));
    
    script.DefineFunction<ParticleGroupBehavior>
    ("force",
     static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) p;
        const char *error = "usage: force( Number forceX, Number forceY )";
        b2Vec2 ctr = { 0, 0 }, imp = { 0, 0 };
        if ( !sa.ReadArguments( 2, TypeFloat, &imp.x, TypeFloat, &imp.y ) ) {
            script.ReportError( error );
            return false;
        }
        // apply
        if ( self->group ) self->group->ApplyForce( imp * WORLD_TO_BOX2D_SCALE );
        return true;
    } ));
    
    script.DefineFunction<ParticleGroupBehavior>
    ("addParticles",
     static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
        ParticleGroupBehavior* self = (ParticleGroupBehavior*) p;
        const char *error = "usage: addParticles( Array particles, [ Number offsetX, Number offsetY, [ Number angle ] ] )";
        ArgValueVector* vec = NULL;
        float ox = 0, oy = 0, oa = 0;
        if ( !sa.ReadArguments( 1, TypeArray, &vec, TypeFloat, &ox, TypeFloat, &oy, TypeFloat, &oa ) ) {
            script.ReportError( error );
            return false;
        }
        self->SetParticleVector( vec, true, ox * WORLD_TO_BOX2D_SCALE, oy * WORLD_TO_BOX2D_SCALE, oa * DEG_TO_RAD );
        return true;
    } ));
    
//    getParticle( i, [ Bool asArray ] )
//    addParticle( Obj | Array | x, y, ....)
//    updateParticle( i, Obj | Array )
//    destroyParticle( i )
//      clear
//    join( otherGroup ) - otherGroup loses all particles, but remains
    
}

void ParticleGroupBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
    
    // store particle system
    if ( this->particleSystem ) protectedObjects.push_back( &this->particleSystem->scriptObject );
    
    // shape
    if ( this->shape ) protectedObjects.push_back( &this->shape->scriptObject );
    
    // if live
    if ( this->group ) {
        void** userData = this->group->GetParticleSystem()->GetUserDataBuffer();
        for ( int32 i = this->group->GetBufferIndex(), np = i + this->group->GetParticleCount(); i < np; i++ ){
            if ( userData[ i ] != NULL ) {
                protectedObjects.push_back( &userData[ i ] );
            }
        }
    // add points otherwise
    } else {
        // points userdata
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            b2ParticleDef& pd = this->points[ i ].def;
            if ( pd.userData != NULL ) {
                protectedObjects.push_back( &pd.userData );
            }
        }
    }
    
    // call super
    Behavior::TraceProtectedObjects( protectedObjects );
    
}


/* MARK:    -                Sync body <-> object
 -------------------------------------------------------------------- */

/// use this body transform if this is a rigid group
bool ParticleGroupBehavior::UseBodyTransform() {
    return (this->group && this->live && (this->groupDef.groupFlags & b2_rigidParticleGroup) );
}

/// copies body transform to game object
/// only valid for rigid groups
void ParticleGroupBehavior::SyncObjectToBody() {
    
    if ( !(this->group && this->live && (this->groupDef.groupFlags & b2_rigidParticleGroup ) ) ) return;
    
    b2Vec2 pos = this->group->GetCenter() * BOX2D_TO_WORLD_SCALE;
    float angle = this->group->GetAngle() * RAD_TO_DEG;
    
    // construct world transform matrix for object
    GPU_MatrixIdentity( this->gameObject->_worldTransform );
    GPU_MatrixTranslate( this->gameObject->_worldTransform, pos.x, pos.y, this->gameObject->_z );
    if ( angle != 0 ) GPU_MatrixRotate( this->gameObject->_worldTransform, angle, 0, 0, 1 );
    if ( this->gameObject->_scale.x != 1 || this->gameObject->_scale.y != 1 ) GPU_MatrixScale( this->gameObject->_worldTransform, this->gameObject->_scale.x, this->gameObject->_scale.y, 1 );
    if ( this->gameObject->_skew.x != 1 || this->gameObject->_skew.y != 1 ) MatrixSkew( this->gameObject->_worldTransform, this->gameObject->_skew.x, this->gameObject->_skew.y );
    this->gameObject->_worldTransformDirty = false;
    this->gameObject->_localCoordsAreDirty = this->gameObject->_inverseWorldDirty = this->gameObject->_transformDirty = true;

}

/// converts game object's local transform to body
/// only valid for rigid groups
void ParticleGroupBehavior::SyncBodyToObject() {
    
    // unlink body
    this->gameObject->body = NULL;
    
    // parent's world transform times local transform = this object world transform
    if ( this->gameObject->parent ) GPU_MatrixMultiply( this->gameObject->_worldTransform, this->gameObject->parent->WorldTransform(), this->gameObject->Transform() );
    this->gameObject->_worldTransformDirty = false;
    
    // relink
    this->gameObject->body = this;
    
    // update body
    if ( this->group ) {
        // extract pos, rot, scale
        b2Vec2 pos, scale;
        float angle;
        this->gameObject->DecomposeTransform( this->gameObject->_worldTransform, pos, angle, scale );
        this->SetBodyTransform( pos, angle * DEG_TO_RAD );
    }
}

/// just sets body transform
void ParticleGroupBehavior::SetBodyTransform( b2Vec2 newPos, float angleInRad ) {
    if ( this->group ) {
        newPos *= WORLD_TO_BOX2D_SCALE;
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        b2Vec2 center = { 0, 0 };
        float angle = 0;
        center = this->group->GetCenter();
        angle = this->group->GetAngle();
        int32 i = this->group->GetBufferIndex();
        int32 last = i + this->group->GetParticleCount();
        b2Vec2* positions = ps->GetPositionBuffer();
        b2Vec2* velocities = ps->GetVelocityBuffer();
        b2Vec2 pos;
        float co = cos( -angle ), so = sin( -angle );
        float ca = cos( angleInRad ), sa = sin( angleInRad );
        for (; i < last; i++) {
            pos = positions[ i ] - center;
            pos.Set( pos.x * co - pos.y * so, pos.x * so + pos.y * co );
            pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
            positions[ i ] = pos + newPos;
            pos = velocities[ i ];
            pos.Set( pos.x * co - pos.y * so, pos.x * so + pos.y * co );
            pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
            velocities[ i ] = pos;
        }
        this->group->m_transform.Set( newPos, angleInRad );
        this->group->m_center = newPos;
    }
}

/// set body position
void ParticleGroupBehavior::SetBodyPosition( b2Vec2 newPos ) {
    newPos *= WORLD_TO_BOX2D_SCALE;
    b2ParticleSystem* ps = this->group->GetParticleSystem();
    b2Vec2 center = this->group->GetCenter();
    int32 i = this->group->GetBufferIndex();
    int32 last = i + this->group->GetParticleCount();
    b2Vec2* positions = ps->GetPositionBuffer();
    for (; i < last; i++) {
        positions[ i ] = positions[ i ] - center + newPos;
    }
    this->group->m_transform.Set( newPos, this->group->GetAngle() );
    this->group->m_center = newPos;
}

/// set body angle
void ParticleGroupBehavior::SetBodyAngle( float angleInRad ) {
    b2ParticleSystem* ps = this->group->GetParticleSystem();
    b2Vec2 center = this->group->GetCenter();
    float angle = this->group->GetAngle();
    int32 i = this->group->GetBufferIndex();
    int32 last = i + this->group->GetParticleCount();
    b2Vec2* positions = ps->GetPositionBuffer();
    b2Vec2* velocities = ps->GetVelocityBuffer();
    b2Vec2 pos;
    float co = cos( -angle ), so = sin( -angle );
    float ca = cos( angleInRad ), sa = sin( angleInRad );
    for (; i < last; i++) {
        pos = positions[ i ] - center;
        pos.Set( pos.x * co - pos.y * so, pos.x * so + pos.y * co );
        pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
        positions[ i ] = pos + center;
        pos = velocities[ i ];
        pos.Set( pos.x * co - pos.y * so, pos.x * so + pos.y * co );
        pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
        velocities[ i ] = pos;
    }
    this->group->m_transform.Set( center, angleInRad );
}

// gets body transform
void ParticleGroupBehavior::GetBodyTransform( b2Vec2& pos, float& angle ) {
    pos = this->group->GetCenter() * BOX2D_TO_WORLD_SCALE;
    angle = this->group->GetAngle();
}


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
    this->groupDef.position = wpos * WORLD_TO_BOX2D_SCALE;
    this->groupDef.angle = wangle * DEG_TO_RAD;

    // create group
    b2ParticleSystem* ps = this->particleSystem->particleSystem;

    /// populate with stored points
    if ( this->points.size() ) {
        
        // make points
        vector<b2Vec2> pts;
        int32 count = (int32) this->points.size();
        pts.resize( count );
        for ( int32 i = 0; i < count; i++ ) {
            ParticleInfo& pi = this->points[ i ];
            pts[ i ] = pi.def.position;
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
        b2Vec2 vel;
        float ca = cos( wangle ), sa = sin( wangle );
        for ( int32 i = offset, last = offset + count; i < last; i++ ) {
            ParticleInfo& pi = this->points[ i - offset ];
            b2ParticleDef &pd = pi.def;
            ps->SetParticleFlags( i, pd.flags );
            ps->SetParticleLifetime( i, pd.lifetime );
            vel.Set( pd.velocity.x * ca - pd.velocity.y * sa, pd.velocity.x * sa + pd.velocity.y * ca );
            velocityBuf[ i ] += vel;
            colorBuf[ i ].Set( pd.color.r, pd.color.g, pd.color.b, pd.color.a );
            weightBuf[ i ] = pi.weight;
        }
        
        this->points.clear();
        this->groupDef.particleCount = 0;
        this->groupDef.positionData = NULL;
        
    } else if ( this->shape ) {

        this->shape->MakeShapesList();
        this->groupDef.shapes = this->shape->shapes.data();
        this->groupDef.shapeCount = (int32) this->shape->shapes.size();
        this->group = ps->CreateParticleGroup( this->groupDef );
        this->groupDef.shapeCount = 0;
        this->groupDef.shapes = NULL;
        
    }
    
    // self
    this->group->SetUserData( this );
    
    // make live
    this->live = true;
    
}

void ParticleGroupBehavior::RemoveBody() {
    
    this->gameObject->_worldTransformDirty = this->gameObject->_localCoordsAreDirty = true;
    this->gameObject->Transform();
    
    this->live = false;
    if ( this->group ) {
        
        b2Vec2 center = { 0, 0 };
        float angle = 0, ca = 0, sa = 0;
        
        // if have gameObject
        if ( this->gameObject ) {
            // get world pos / angle
            b2Vec2 wscale;
            this->gameObject->DecomposeTransform( this->gameObject->WorldTransform(), center, angle, wscale );
            center *= WORLD_TO_BOX2D_SCALE;
            angle *= DEG_TO_RAD;
        } else {
            // use body center
            center = this->group->GetCenter();
            angle = this->group->GetAngle();
        }
        
        // clear stored
        this->points.clear();
        
        // get particles info
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        int32 i = this->group->GetBufferIndex();
        int32 last = i + this->group->GetParticleCount();
        b2ParticleColor* colors = ps->GetColorBuffer();
        b2Vec2* positions = ps->GetPositionBuffer();
        b2Vec2* velocities = ps->GetVelocityBuffer();
        void** userData = ps->GetUserDataBuffer();
        float32 *weights = ps->GetWeightBuffer();
        ca = cos( -angle ), sa = sin( -angle );

        // for each particle
        for (; i < last; i++) {
            
            // new stored point
            this->points.emplace_back();
            ParticleInfo &pi = this->points.back();
            
            // store data
            pi.def.flags = ps->GetParticleFlags( i );
            pi.def.color = colors[ i ];
            pi.def.lifetime = ps->GetParticleLifetime( i );
            pi.def.userData = userData[ i ];
            pi.weight = weights[ i ];
            
            // if have gameObject
            if ( this->gameObject ) {
                // convert to local, relative to object
                this->gameObject->ConvertPoint(
                   positions[ i ].x * BOX2D_TO_WORLD_SCALE, positions[ i ].y * BOX2D_TO_WORLD_SCALE,
                   pi.def.position.x, pi.def.position.y, false );
                pi.def.position *= WORLD_TO_BOX2D_SCALE;
            } else {
                // store relative to center
                pi.def.position = positions[ i ] - center;
                pi.def.position.Set( pi.def.position.x * ca - pi.def.position.y * sa, pi.def.position.x * sa + pi.def.position.y * ca );
            }
            
            // store velocity transformed relative to object
            pi.def.velocity = velocities[ i ];
            pi.def.velocity.Set( pi.def.velocity.x * ca - pi.def.velocity.y * sa, pi.def.velocity.x * sa + pi.def.velocity.y * ca );
            
            // delete next step
            ps->SetParticleFlags( i, b2_zombieParticle );
        }
        
        // clear definition velocity
        this->groupDef.angularVelocity = 0;
        this->groupDef.linearVelocity.SetZero();
        
        // group will be deleted next step
        this->group->SetUserData( NULL );
        this->group->SetGroupFlags( ~b2_particleGroupCanBeEmpty ); // makes empty group be deleted next iteration
        this->group->DestroyParticles( false );
        this->group = NULL;
        
    }
    
}

void ParticleGroupBehavior::ParticleDestroyed( int32 index ) {
    Event event( EVENT_DESTROYED );
    event.scriptParams.AddIntArgument( index );
    event.scriptParams.AddObjectArgument( this->scriptObject );
    this->CallEvent( event );
}


/* MARK:    -                Update
 -------------------------------------------------------------------- */


/// apply color to all
void ParticleGroupBehavior::UpdateColor() {
    SDL_Color& sclr = this->color->rgba;
    if ( this->group ) {
        b2ParticleColor* colors = this->group->GetParticleSystem()->GetColorBuffer();
        for ( int32 i = this->group->GetBufferIndex(), np = i + this->group->GetParticleCount(); i < np; i++ ){
            colors[ i ].Set( sclr.r, sclr.g, sclr.b, sclr.a );
        }
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            pi.def.color.Set( sclr.r, sclr.g, sclr.b, sclr.a );
        }
    }
}

/// update particle flags
void ParticleGroupBehavior::UpdateFlags() {
    if ( group ) {
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        for (int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount(); i < last; i++) {
            ps->SetParticleFlags( i, this->groupDef.flags );
        }
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            pi.def.flags = this->groupDef.flags;
        }
    }
}

/// update lifetime
void ParticleGroupBehavior::UpdateLifetime() {
    if ( group ) {
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        for (int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount(); i < last; i++) {
            ps->SetParticleLifetime( i, this->groupDef.lifetime );
        }
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            pi.def.lifetime = this->groupDef.lifetime;
        }
    }
}

/// update velocity
void ParticleGroupBehavior::UpdateVelocity( bool updateX, bool updateY ) {
    if ( group ) {
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        b2Vec2 *velocities = ps->GetVelocityBuffer();
        for (int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount(); i < last; i++) {
            if ( updateX ) velocities[ i ].x = this->groupDef.linearVelocity.x;
            if ( updateY ) velocities[ i ].y = this->groupDef.linearVelocity.y;
        }
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            if ( updateX ) pi.def.velocity.x = this->groupDef.linearVelocity.x;
            if ( updateY ) pi.def.velocity.y = this->groupDef.linearVelocity.y;
        }
    }
}

void ParticleGroupBehavior::UpdateAngularVelocity() {
    b2Vec2 pos;
    if ( group ) {
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        b2Vec2 linearVel = group->GetLinearVelocity();
        b2Vec2 *velocities = ps->GetVelocityBuffer();
        b2Vec2 *positions = ps->GetPositionBuffer();
        b2Vec2 center = group->GetCenter();
        for (int32 i = group->GetBufferIndex(), last = i + group->GetParticleCount(); i < last; i++) {
            pos = positions[ i ] - center;
            float ang = atan2( pos.y, pos.x ) + M_PI_2;
            float r = pos.Length();
            velocities[ i ].Set( linearVel.x + cos( ang ) * r * this->groupDef.angularVelocity,
                                linearVel.y + sin( ang ) * r * this->groupDef.angularVelocity );
        }
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            pos = pi.def.position;
            float ang = atan2( pos.y, pos.x ) + M_PI_2;
            float r = pos.Length();
            pi.def.velocity.Set( cos( ang ) * r * this->groupDef.angularVelocity,
                                 sin( ang ) * r * this->groupDef.angularVelocity );
        }
    }
}


/// returns ArgValueVector with each particle's info
ArgValueVector* ParticleGroupBehavior::GetParticleVector() {
    ArgValueVector* vec = new ArgValueVector();
    int32 flags;

    if ( this->group ) {
        
        b2Vec2 center = { 0, 0 };
        float32 angle = 0;
        
        if ( this->UseBodyTransform() ) {
            center = this->group->GetCenter();
            angle = this->group->GetAngle();
        } else if ( this->gameObject ) {
            center = this->gameObject->_position * WORLD_TO_BOX2D_SCALE;
            angle = this->gameObject->_angle;
        }

        // store particles info
        b2ParticleSystem* ps = this->group->GetParticleSystem();
        int32 i = this->group->GetBufferIndex();
        int32 last = i + this->group->GetParticleCount();
        b2ParticleColor* colors = ps->GetColorBuffer();
        b2Vec2* positions = ps->GetPositionBuffer();
        b2Vec2* velocities = ps->GetVelocityBuffer();
        void** userData = ps->GetUserDataBuffer();
        float ca = cos( -angle ), sa = sin( -angle );
        b2Vec2 pos;
        for (; i < last; i++) {
            vec->emplace_back();
            ArgValue& val = vec->back();
            val.type = TypeArray;
            ArgValueVector *pt = val.value.arrayValue = new ArgValueVector();
            
            // unrotate positions
            pos = positions[ i ] - center;
            pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
            pt->emplace_back( (float) pos.x * BOX2D_TO_WORLD_SCALE );
            pt->emplace_back( (float) pos.y * BOX2D_TO_WORLD_SCALE );

            // unrotate velocities
            pos = velocities[ i ];
            pos.Set( pos.x * ca - pos.y * sa, pos.x * sa + pos.y * ca );
            pt->emplace_back( (float) pos.x * BOX2D_TO_WORLD_SCALE );
            pt->emplace_back( (float) pos.y * BOX2D_TO_WORLD_SCALE );

            flags = (((( ps->GetParticleFlags( i ) & ~b2_fixtureContactFilterParticle )
                       & ~b2_particleContactFilterParticle )
                      & ~b2_destructionListenerParticle )
                     & ~b2_particleContactListenerParticle );
            pt->emplace_back( (int) flags );
            
            pt->emplace_back( (float) ps->GetParticleLifetime( i ) );
            
            pt->emplace_back( (int) colors[ i ].r );
            pt->emplace_back( (int) colors[ i ].g );
            pt->emplace_back( (int) colors[ i ].b );
            pt->emplace_back( (int) colors[ i ].a );
            
            pt->emplace_back( (void*) userData[ i ] );
        }
    
    } else {
        for ( size_t i = 0, np = this->points.size(); i < np; i++ ){
            ParticleInfo& pi = this->points[ i ];
            vec->emplace_back();
            ArgValue& val = vec->back();
            val.type = TypeArray;
            ArgValueVector *pt = val.value.arrayValue = new ArgValueVector();
            
            pt->emplace_back( (float) pi.def.position.x * BOX2D_TO_WORLD_SCALE );
            pt->emplace_back( (float) pi.def.position.y * BOX2D_TO_WORLD_SCALE );
            pt->emplace_back( (float) pi.def.velocity.x * BOX2D_TO_WORLD_SCALE );
            pt->emplace_back( (float) pi.def.velocity.y * BOX2D_TO_WORLD_SCALE );
            
            flags = (((( pi.def.flags & ~b2_fixtureContactFilterParticle )
                       & ~b2_particleContactFilterParticle )
                      & ~b2_destructionListenerParticle )
                     & ~b2_particleContactListenerParticle );
            pt->emplace_back( (int) flags );
            pt->emplace_back( (float) pi.def.lifetime );
            
            pt->emplace_back( (int) pi.def.color.r );
            pt->emplace_back( (int) pi.def.color.g );
            pt->emplace_back( (int) pi.def.color.b );
            pt->emplace_back( (int) pi.def.color.a );

            pt->emplace_back( (void*) pi.def.userData );
        }
    }
    return vec;
}

/// overwrites particles
ArgValueVector* ParticleGroupBehavior::SetParticleVector( ArgValueVector* in, bool append, float offsetX, float offsetY, float offsetAngle ) {

    // transform point to world
    b2Vec2 center = { 0, 0 };
    float angle = 0;
    
    // if have gameObject
    if ( this->gameObject ) {
        // get world pos / angle
        b2Vec2 wscale;
        this->gameObject->DecomposeTransform( this->gameObject->WorldTransform(), center, angle, wscale );
        center *= WORLD_TO_BOX2D_SCALE;
        angle *= DEG_TO_RAD;
    } else {
        // use body center
        center = this->group->GetCenter();
        angle = this->group->GetAngle();
    }
    
    if ( !append ) {
        // remove current
        this->RemoveBody();
        this->points.clear();
    }
    
    // add new points
    int nc = (int) in->size();
    size_t i = 0;
    float so = sin( offsetAngle ), co = cos( offsetAngle );
    for (; i < nc; i++ ){
        
        // add a point w defaults
        ParticleInfo p;
        p.def.position.SetZero();
        p.def.velocity.SetZero();
        p.def.color.r = this->color->rgba.r;
        p.def.color.g = this->color->rgba.g;
        p.def.color.b = this->color->rgba.b;
        p.def.color.a = this->color->rgba.a;
        p.def.lifetime = this->groupDef.lifetime;
        p.def.flags = this->groupDef.flags | b2_fixtureContactFilterParticle | b2_particleContactFilterParticle | b2_destructionListenerParticle | b2_particleContactListenerParticle;
        p.weight = 0;
        p.def.userData = NULL;
        
        // each element
        ArgValue &val = (*in)[ i ];
        if ( val.type == TypeArray ) {
            ArgValueVector &pd = *val.value.arrayValue;
            size_t len = pd.size();
            if ( len >= 2 ){
                pd[ 0 ].toNumber( p.def.position.x );
                pd[ 1 ].toNumber( p.def.position.y );
                p.def.position *= WORLD_TO_BOX2D_SCALE;
            }
            if ( len >= 4 ){
                pd[ 2 ].toNumber( p.def.velocity.x );
                pd[ 3 ].toNumber( p.def.velocity.y );
                p.def.velocity *= WORLD_TO_BOX2D_SCALE;
            }
            if ( len >= 5 ) {
                uint32 flags = p.def.flags | b2_fixtureContactFilterParticle | b2_particleContactFilterParticle | b2_destructionListenerParticle | b2_particleContactListenerParticle;
                pd[ 4 ].toInt32( flags );
            }
            if ( len >= 6 ) pd[ 5 ].toNumber( p.def.lifetime );
            if ( len >= 10 ){
                pd[ 6 ].toInt8( p.def.color.r );
                pd[ 7 ].toInt8( p.def.color.g );
                pd[ 8 ].toInt8( p.def.color.b );
                pd[ 9 ].toInt8( p.def.color.a );
            }
            if ( len >= 11 && pd[ 10 ].type == TypeObject ) p.def.userData = pd[ 10 ].value.objectValue;
        }
        
        // apply offset
        p.def.position.Set( offsetX + p.def.position.x * co - p.def.position.y * so,
                           offsetY + p.def.position.x * so + p.def.position.y * co );
        p.def.velocity.Set( p.def.velocity.x * co - p.def.velocity.y * so,
                           p.def.velocity.x * so + p.def.velocity.y * co );
        
        // add particle
        this->AddParticle( p, center, angle );

    }
    
    // add body
    this->AddBody( this->particleSystem->scene );
    return in;
}

/// particleinfo is always in local coords
int32 ParticleGroupBehavior::AddParticle( ParticleInfo& p, b2Vec2 &worldPos, float worldAngle ) {
    
    // if have live group
    if ( this->group ) {
        
        // apply body transform
        float sa = sin( worldAngle ), ca = cos( worldAngle );
        p.def.position.Set( worldPos.x + p.def.position.x * ca - p.def.position.y * sa,
                           worldPos.y + p.def.position.x * sa + p.def.position.y * ca );
        p.def.velocity.Set( p.def.velocity.x * ca - p.def.velocity.y * sa,
                           p.def.velocity.x * sa + p.def.velocity.y * ca );    

        // add
        p.def.group = this->group;
        return this->group->GetParticleSystem()->CreateParticle( p.def );
        
    // not live
    } else {
        
        this->points.push_back( p );
        return (int32) this->points.size();
    }
}
