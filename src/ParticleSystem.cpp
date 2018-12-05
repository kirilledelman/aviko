#include "ParticleSystem.hpp"
#include "ParticleGroupBehavior.hpp"
#include "Scene.hpp"


/* MARK:    -                Init / destroy
 -------------------------------------------------------------------- */


ParticleSystem::ParticleSystem( ScriptArguments* args ) {
    
    // add scriptObject
    script.NewScriptObject<ParticleSystem>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );
   
    // defaults
    psDef.radius = 4 * WORLD_TO_BOX2D_SCALE;
    psDef.density = 5;
    
    // read params
    void *initObj = NULL;
    
    // if arguments are given
    if ( args && args->ReadArguments( 1, TypeObject, &initObj ) ) {
            
        // use as init
        script.CopyProperties( initObj, this->scriptObject );

    }
    
}

ParticleSystem::~ParticleSystem() {
    
    this->RemoveFromWorld();
    
    // remove all groups
    unordered_set<ParticleGroupBehavior*>::iterator it;
    while ( ( it = this->groups.begin() ) != this->groups.end() ) {
        (*it)->SetSystem( NULL );
    }
    
}


/* MARK:    -                Scripting
 -------------------------------------------------------------------- */


void ParticleSystem::InitClass() {
    
    // register class
    script.RegisterClass<ParticleSystem>( NULL );
    
    // props
    
    script.AddProperty<ParticleSystem>
    ( "scene",
     static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((ParticleSystem*) b)->scene ? ((ParticleSystem*) b)->scene->scriptObject : NULL; }),
     static_cast<ScriptObjectCallback>([](void *b, void* val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        Scene* s = script.GetInstance<Scene>( val );
        ps->SetScene( s, -1 );
        return ((ParticleSystem*) b)->scene ? ((ParticleSystem*) b)->scene->scriptObject : NULL;
    } ), PROP_ENUMERABLE | PROP_NOSTORE );
    
    script.AddProperty<ParticleSystem>
    ( "active",
     static_cast<ScriptBoolCallback>([](void *p, bool val ){ return ((ParticleSystem*) p)->active; }),
     static_cast<ScriptBoolCallback>([](void *p, bool val ){
        ParticleSystem* ps = (ParticleSystem*) p;
        ps->active = val;
        if( ps->particleSystem ) ps->particleSystem->SetPaused( !val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "groups",
     static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((ParticleSystem*) go)->GetParticleGroupsVector(); }),
     // static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((ParticleSystem*) go)->SetParticleGroupsVector( in ); }),
     PROP_ENUMERABLE | PROP_NOSTORE
     );
    
    script.AddProperty<ParticleSystem>
    ( "colorMixingStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.colorMixingStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.colorMixingStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "damping",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.dampingStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.dampingStrength = val;
        if ( ps->particleSystem ) ps->particleSystem->SetDamping( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "density",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.density; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.density = val;
        if ( ps->particleSystem ) ps->particleSystem->SetDensity( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "destroyByAge",
     static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((ParticleSystem*) b)->psDef.destroyByAge; }),
     static_cast<ScriptBoolCallback>([](void *b, bool val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.destroyByAge = val;
        if ( ps->particleSystem ) ps->particleSystem->SetDestructionByAge( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "ejectionStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.ejectionStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.ejectionStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "elasticStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.elasticStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.elasticStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "gravityScale",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.gravityScale; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.gravityScale = val;
        if ( ps->particleSystem ) ps->particleSystem->SetGravityScale( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "lifetimeGranularity",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.lifetimeGranularity; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.lifetimeGranularity = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "maxCount",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((ParticleSystem*) b)->psDef.maxCount; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.maxCount = val;
        if ( ps->particleSystem ) ps->particleSystem->SetMaxParticleCount( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "powderStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.powderStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.powderStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "pressureStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.pressureStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.pressureStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "radius",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.radius * BOX2D_TO_WORLD_SCALE; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.radius = val * WORLD_TO_BOX2D_SCALE;
        if ( ps->particleSystem ) ps->particleSystem->SetRadius( ps->psDef.radius );
        return val;
    } ) );

    script.AddProperty<ParticleSystem>
    ( "repulsiveStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.repulsiveStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.repulsiveStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );

    script.AddProperty<ParticleSystem>
    ( "springStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.springStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.springStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );

    script.AddProperty<ParticleSystem>
    ( "staticPressureIterations",
     static_cast<ScriptIntCallback>([](void *b, int val ){ return ((ParticleSystem*) b)->psDef.staticPressureIterations; }),
     static_cast<ScriptIntCallback>([](void *b, int val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.staticPressureIterations = val;
        if ( ps->particleSystem ) ps->particleSystem->SetStaticPressureIterations( val );
        return val;
    } ) );

    script.AddProperty<ParticleSystem>
    ( "staticPressureRelaxation",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.staticPressureRelaxation; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.staticPressureRelaxation = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );

    script.AddProperty<ParticleSystem>
    ( "staticPressureStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.staticPressureStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.staticPressureStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "strictContactCheck",
     static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((ParticleSystem*) b)->psDef.strictContactCheck; }),
     static_cast<ScriptBoolCallback>([](void *b, bool val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.strictContactCheck = val;
        if ( ps->particleSystem ) ps->particleSystem->SetStrictContactCheck( val );
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "surfaceTensionNormalStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.surfaceTensionNormalStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.surfaceTensionNormalStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "surfaceTensionPressureStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.surfaceTensionPressureStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.surfaceTensionPressureStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
    script.AddProperty<ParticleSystem>
    ( "surfaceTensionPressureStrength",
     static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((ParticleSystem*) b)->psDef.viscousStrength; }),
     static_cast<ScriptFloatCallback>([](void *b, float val ){
        ParticleSystem* ps = (ParticleSystem*) b;
        ps->psDef.viscousStrength = val;
        if ( ps->particleSystem ) {
            ps->RemoveFromWorld();
            ps->AddToWorld();
        }
        return val;
    } ) );
    
}

void ParticleSystem::TraceProtectedObjects( vector<void**> &protectedObjects ) {

    // store scene
    if ( this->scene ) protectedObjects.push_back( &this->scene->scriptObject );
    
    // groups
    /* unordered_set<ParticleGroupBehavior*>::iterator it = this->groups.begin(), end = this->groups.end();
    while ( it != end ) {
        protectedObjects.push_back( &((*it)->scriptObject) );
        it++;
    }*/
    
    // call super
    ScriptableClass::TraceProtectedObjects( protectedObjects );
    
}

/* MARK:    -                Groups
 -------------------------------------------------------------------- */

/// returns ArgValueVector with each Particles group
ArgValueVector* ParticleSystem::GetParticleGroupsVector() {
    ArgValueVector* vec = new ArgValueVector();
    unordered_set<ParticleGroupBehavior*>::iterator it = this->groups.begin(), end = this->groups.end();
    while ( it != end ){
        ArgValue val;
        val.type = TypeObject;
        val.value.objectValue = (*it)->scriptObject;
        vec->push_back( val );
        it++;
    }
    return vec;
}

/// overwrites particle systems, skips nulls
/* ArgValueVector* ParticleSystem::SetParticleGroupsVector( ArgValueVector* in ) {
    
    // make copy of current, erase current
    unordered_set<ParticleGroupBehavior*> oldGroups = this->groups;
    this->groups.clear();
    
    // remove bodies of all current
    unordered_set<ParticleGroupBehavior*>::iterator it = oldGroups.begin(), end = oldGroups.end();
    while ( it != end ) { (*it)->RemoveBody(); it++; }
    
    // go over passed array
    int nc = (int) in->size();
    size_t i = 0;
    
    // each element
    for (; i < nc; i++ ){
        ArgValue &val = (*in)[ i ];
        ParticleGroupBehavior* g = NULL;
        if ( val.type == TypeObject && val.value.objectValue != NULL ) {
            g = script.GetInstance<ParticleGroupBehavior>( val.value.objectValue );
        }
        
        // if valid
        if ( g ) {
            // remove from stored, if present
            oldGroups.erase( g );
            
            // set system
            g->particleSystem = NULL;
            g->SetSystem( this );
        }
    }
    
    // remove remaining
    it = oldGroups.begin(), end = oldGroups.end();
    while ( it != end ) { (*it)->SetSystem( NULL ); it++; }
    
    return in;
} */


/* MARK:    -                Physics
 -------------------------------------------------------------------- */

void ParticleSystem::AddToWorld() {
    
    // if there are no groups, ignore for now
    if ( this->groups.begin() == this->groups.end() ) {
        // printf("deferred addtoworld for %p (%s)\n", this, this->scene->name.c_str() );
        return;
    }
    
    // create ps
    this->particleSystem = this->scene->world->CreateParticleSystem( &this->psDef );
    
    // add bodies for all groups
    unordered_set<ParticleGroupBehavior*>::iterator it = this->groups.begin(), end = this->groups.end();
    while ( it != end ) { (*it)->AddBody( this->scene ); it++; }
    
}

void ParticleSystem::RemoveFromWorld() {
    
    if ( this->particleSystem && this->scene ) {
        
        printf( "ParticleSystem::RemoveFromWorld\n" );
        
        // remove all groups
        unordered_set<ParticleGroupBehavior*>::iterator it = this->groups.begin();
        while ( it != this->groups.end() ) {
            ParticleGroupBehavior* g = *it;
            g->RemoveBody();
            printf( "ParticleSystem::RemoveFromWorld->removebody %p!\n", g );
            it++;
        }

        // destroy ps
        this->scene->world->DestroyParticleSystem( this->particleSystem );
        this->particleSystem = NULL;        
    }
}

void ParticleSystem::SyncObjectsToGroups() {
 
    unordered_set<ParticleGroupBehavior*>::iterator it = this->groups.begin(), end = this->groups.end();
    while( it != end ) {
        ParticleGroupBehavior* pg = *it;
        if ( pg->live && pg->syncObjectPosition ) pg->SyncObjectToBody();
        it++;
    }
    
}

/* MARK:    -                Properties
 -------------------------------------------------------------------- */

void ParticleSystem::SetScene( Scene *newScene, int desiredPosition ) {

    // if scene is different
    if ( newScene != this->scene ) {
        
        // if had scene
        Scene* oldScene = this->scene;
        if ( oldScene ) {
            
            // remove
            this->RemoveFromWorld();
            
            // find this object in parent's list
            vector<ParticleSystem*> *parentList = &oldScene->particleSystems;
            vector<ParticleSystem*>::iterator listEnd = parentList->end();
            vector<ParticleSystem*>::iterator it = find( parentList->begin(), listEnd, this );
            
            // remove from list
            int removedAt = -1;
            if ( it != listEnd ) {
                removedAt = (int) (it - parentList->begin());
                parentList->erase( it );
            }

            // printf( "PARTICLESYSTEM %p removed from scene %p\n", this, oldScene );
            
            // clear
            this->scene = NULL;
            
        }
        
        // set parent
        this->scene = newScene;
        
        // add to new scene
        if ( newScene ) {
            
            // insert based on desired position
            int num = (int) newScene->particleSystems.size();
            // convert negative pos
            if ( desiredPosition < 0 ) desiredPosition = num + 1 + desiredPosition;
            // insert at location
            if ( desiredPosition <= num && num > 0 ) {
                desiredPosition = max( 0, desiredPosition );
                newScene->particleSystems.insert( newScene->particleSystems.begin() + desiredPosition, this );
            } else {
                newScene->particleSystems.push_back( this );
                desiredPosition = (int) newScene->particleSystems.size();
            }
            
            // printf( "PARTICLESYSTEM %p added to scene %p at pos %d\n", this, newScene, desiredPosition );
            this->AddToWorld();
            
        // no new scene - means we've definitely been removed from scene
        } else {
            
            // printf( "PARTICLESYSTEM %p removed from all scenes\n", this );
        }
    }
    
}
