#ifndef ParticleSystem_hpp
#define ParticleSystem_hpp

#include "common.h"
#include "ScriptableClass.hpp"

class Scene;
class ParticleGroupBehavior;

class ParticleSystem : public ScriptableClass {
public:
    
    b2ParticleSystem* particleSystem = NULL;
    
    unordered_set<ParticleGroupBehavior*> groups;
    ArgValueVector* GetParticleGroupsVector();
    // ArgValueVector* SetParticleGroupsVector( ArgValueVector* in );

    // scripting
    
    static void InitClass();
    
    
    // physics
    
    Scene* scene = NULL;
    void SetScene( Scene* s, int pos = -1);
    
    bool active = true;
    
    b2ParticleSystemDef psDef;
    
    void AddToWorld();
    void RemoveFromWorld();
    
    // garbage collector
    void TraceProtectedObjects( vector<void**> &protectedObjects );
    
    // init/destroy
    ParticleSystem();
    ParticleSystem( ScriptArguments* args );
    ~ParticleSystem();
    
};

SCRIPT_CLASS_NAME( ParticleSystem, "ParticleSystem" );

#endif /* ParticleSystem_hpp */
