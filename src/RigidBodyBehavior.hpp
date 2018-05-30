#ifndef RigidBodyBehavior_hpp
#define RigidBodyBehavior_hpp

#include "BodyBehavior.hpp"
#include "RigidBodyShape.hpp"
#include "RigidBodyJoint.hpp"

class Scene;

/// Box2D body behavior
class RigidBodyBehavior : public BodyBehavior {
public:
	
	// init, destroy
	RigidBodyBehavior( ScriptArguments* args );
	RigidBodyBehavior();
	~RigidBodyBehavior();	
	
	/// box2d body attached to this GameObject
	b2Body* body = NULL;
	
	/// body type
	b2BodyType bodyType = b2BodyType::b2_dynamicBody;
	
	/// mass and center of mass
	b2MassData massData;
	
	bool fixedRotation = false;
	
	bool canSleep = true;
	
	bool bullet = false;
	
	float angularDamping = 0.1;
	
	float linearDamping = 0.1;
	
	b2Vec2 velocity = { 0, 0 };
	float angularVelocity = 0;
	
	b2Vec2 GetVelocity();
	void SetVelocity( b2Vec2 );
	float GetAngularVelocity();
	void SetAngularVelocity( float );
	void Impulse( b2Vec2 impulse, b2Vec2 point );
	void AngularImpulse( b2Vec2 impulse, b2Vec2 point );
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
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
	
// joints
	
	/// joints, where body = this body
	vector<RigidBodyJoint*> joints;
	
	/// joints, where otherBody = this body
	vector<RigidBodyJoint*> otherJoints;
	
	/// replace all joints with one or NULL
	void ReplaceJoints( RigidBodyJoint* rbs );
	
	// getter/setter for joints array
	ArgValueVector* GetJointsVector( bool other );
	ArgValueVector* SetJointsVector( ArgValueVector* in, bool other );
	
// methods
	
	/// turns body on and off ( active switch )
	void EnableBody( bool e );
	
	/// called after physics step on each body to sync position and rotation
	void SyncObjectToBody();
	
	/// call to sync body to manually set object position
	void SyncBodyToObject();
	
	/// sets body transform
	void SetBodyTransform( b2Vec2, float angleInRad );
	
	/// gets body transform
	void GetBodyTransform( b2Vec2&, float& angleInRad );
	
	/// set body position
	void SetBodyPosition( b2Vec2 pos );
	
	/// set body angle
	void SetBodyAngle( float angleInRad );
	
	/// construct and add body to scene->world
	void AddBody( Scene* scene );
	
	/// removes body from world
	void RemoveBody();
	
};

SCRIPT_CLASS_NAME( RigidBodyBehavior, "Body" );

#endif /* RigidBodyBehavior_hpp */
