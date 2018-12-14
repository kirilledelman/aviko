#ifndef RenderParticlesBehavior_hpp
#define RenderParticlesBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"
#include "ParticleGroupBehavior.hpp"

class RenderParticlesBehavior: public RenderBehavior {
public:
    
    // init, destroy
    RenderParticlesBehavior( ScriptArguments* args );
    RenderParticlesBehavior();
    ~RenderParticlesBehavior();
    
    /// render surface
    GPU_Image* surface = NULL;
    
    // particles
    ParticleGroupBehavior* particles = NULL;
    
    /// render callback
    static void Render( RenderParticlesBehavior* behavior, GPU_Target* target, Event* event );
    
    // scripting
    
    /// registers class for scripting
    static void InitClass();    
    void TraceProtectedObjects( vector<void**> &protectedObjects );

};

SCRIPT_CLASS_NAME( RenderParticlesBehavior, "RenderParticles" );

#endif /* RenderParticlesBehavior_hpp */
