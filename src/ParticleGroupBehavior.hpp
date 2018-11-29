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
    vector<b2ParticleDef> points;
    
    /// color
    Color* color = NULL;
    ColorCallback colorUpdated;
    void UpdateColor();
    
    /*
    float angularDamping = 0.1;
    
    float linearDamping = 0.1;
    
    float gravityScale = 1;
    
    b2Vec2 velocity = { 0, 0 };
    float angularVelocity = 0;
    
    b2Vec2 GetVelocity();
    void SetVelocity( b2Vec2 );
    float GetAngularVelocity();
    void SetAngularVelocity( float );
    void Impulse( b2Vec2& impulse, b2Vec2& point );
    void AngularImpulse( float impulse );
    void Force( b2Vec2& force, b2Vec2& point );
    void AngularForce( float force );
     
     */
    
    // scripting
    
    /// registers class for scripting
    static void InitClass();
    
    void TraceProtectedObjects( vector<void**> &protectedObjects );
    
    /*
    // shapes
    
    /// shapes
    vector<RigidBodyShape*> shapes;
    
    /// replace all shapes with one or NULL
    void ReplaceShapes( RigidBodyShape* rbs );
    
    // getter/setter for shapes array
    ArgValueVector* GetShapesVector();
    ArgValueVector* SetShapesVector( ArgValueVector* in );
    
    // make a shape from render component
    bool MakeShapeFromRender();
    */
    
    // methods
    
    /// turns on and off ( active switch )
    void EnableBody( bool e );
    
    /// called after physics step on each body to sync position and rotation
    void SyncObjectToBody();
    
    /// call to sync body to manually set object position
    void SyncBodyToObject();
    
    /// attach to particle system
    void SetSystem( ParticleSystem* s );
    
    /*
    /// sets body transform
    void SetBodyTransform( b2Vec2, float angleInRad );
    
    /// gets body transform
    void GetBodyTransform( b2Vec2&, float& angleInRad );
    
    /// set body position
    void SetBodyPosition( b2Vec2 pos );
    
    /// set body angle
    void SetBodyAngle( float angleInRad );
    */
    
    /// construct and add group to scene->world
    void AddBody( Scene* scene );
    
    /// removes body from world
    void RemoveBody();
    
};

SCRIPT_CLASS_NAME( ParticleGroupBehavior, "Particles" );

#endif /* ParticleGroupBehavior_hpp */
