#ifndef ParticleGroupBehavior_hpp
#define ParticleGroupBehavior_hpp

#include "BodyBehavior.hpp"
#include "RigidBodyShape.hpp"

class Scene;
class ParticleSystem;

/// Box2D body behavior
class ParticleGroupBehavior : public BodyBehavior {
public:
    
    // init, destroy
    ParticleGroupBehavior( ScriptArguments* args );
    ParticleGroupBehavior();
    ~ParticleGroupBehavior();
    
    /// attached to a specific particle system
    ParticleSystem* particleSystem = NULL;
    
    /// box2d particle group associated with this GameObject
    b2ParticleGroup* group = NULL;
    
    /// group props
    b2ParticleGroupDef groupDef;
    
    /// stored shape
    RigidBodyShape* shape = NULL;
    
    /// stored points
    struct ParticleInfo {
        b2ParticleDef def;
        float32 weight;
        ParticleInfo(){ weight = 0; }
    };
    vector<ParticleInfo> points;
    
    /// color
    Color* color = NULL;
    ColorCallback colorUpdated;
    
    // functions update live particles or stored points
    void UpdateColor();
    void UpdateFlags();
    void UpdateVelocity( bool updateX, bool updateY );
    void UpdateAngularVelocity();
    void UpdateLifetime();
    
    // scripting
    
    /// registers class for scripting
    static void InitClass();
    
    void TraceProtectedObjects( vector<void**> &protectedObjects );
    
    // serializing
    ArgValueVector* GetParticleVector();
    ArgValueVector* SetParticleVector( ArgValueVector* in );
    
    // methods
    
    /// turns on and off ( active switch )
    void EnableBody( bool e );
    
    bool syncObjectPosition = true;
    
    /// called after physics step on each body to sync position and rotation
    void SyncObjectToBody();
    
    /// call to sync body to manually set object position
    void SyncBodyToObject();
    
    /// attach to particle system
    void SetSystem( ParticleSystem* s );
    
    
    /// sets body transform
    void SetBodyTransform( b2Vec2, float angleInRad );
    
    /// gets body transform
    void GetBodyTransform( b2Vec2&, float& angleInRad );
    
    /// set body position
    void SetBodyPosition( b2Vec2 pos );
    
    /// set body angle
    void SetBodyAngle( float angleInRad );
    
    /// construct and add group to scene->world
    void AddBody( Scene* scene );
    
    /// removes body from world
    void RemoveBody();
    
};

SCRIPT_CLASS_NAME( ParticleGroupBehavior, "Particles" );

#endif /* ParticleGroupBehavior_hpp */
