#ifndef RenderParticlesBehavior_hpp
#define RenderParticlesBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"
#include "ParticleGroupBehavior.hpp"
#include "ImageResource.hpp"

class RenderParticlesBehavior: public RenderBehavior {
public:
    
    // init, destroy
    RenderParticlesBehavior( ScriptArguments* args );
    RenderParticlesBehavior();
    ~RenderParticlesBehavior();
    
    /// render
    static GPU_Image* surface;
    static ImageResource* particleTexture;
    
    /// texture to fill with
    ImageResource* imageResource = NULL;
    float tileX = 1;
    float tileY = 1;
    
    // params
    float fadeTime = 0.25;
    float velocityStretch = 50;
    float velocityStretchFactor = 0.25;
    float extents = -20;
    
    // particles
    ParticleGroupBehavior* particles = NULL;
    
    /// render callback
    static void Render( RenderParticlesBehavior* behavior, GPU_Target* target, Event* event );
    
    /// shader select
    virtual size_t SelectParticleShader(float u, float v, float w, float h,
                                        float tx, float ty, bool rotated,
                                        GPU_Image *image, GPU_Target* targ );
    // scripting
    
    /// registers class for scripting
    static void InitClass();    
    void TraceProtectedObjects( vector<void**> &protectedObjects );

};

SCRIPT_CLASS_NAME( RenderParticlesBehavior, "RenderParticles" );

#endif /* RenderParticlesBehavior_hpp */
