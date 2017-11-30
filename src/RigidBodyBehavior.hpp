#ifndef RigidBodyBehavior_hpp
#define RigidBodyBehavior_hpp

#include "Behavior.hpp"

class Scene;

/// Box2D body behavior
class RigidBodyBehavior : public Behavior {
public:
	
	// init, destroy
	RigidBodyBehavior( ScriptArguments* args );
	RigidBodyBehavior();
	~RigidBodyBehavior();	
	
	/// box2d body attached to this GameObject
	b2Body* body = NULL;
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
// methods
	
	/// active callback
	static void ActiveChanged( RigidBodyBehavior* behavior, GameObject* target, Event* event );
	
	/// added to scene or attached to gameObject callback
	static void Attached( RigidBodyBehavior* behavior, GameObject* target, Event* event );
	
	/// removed from scene or detached from gameObject callback
	static void Detached( RigidBodyBehavior* behavior, GameObject* target, Event* event );
	
	/// override Behavior active setter
	bool active( bool a );
	
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
	
	// overrides Behavior's version
	bool SetGameObject( GameObject* go, int pos=-1 );
	
	/// construct and add body to scene->world
	void AddBody( Scene* scene );
	
	/// removes body from world
	void RemoveBody();
	
};

SCRIPT_CLASS_NAME( RigidBodyBehavior, "RigidBody" );

#endif /* RigidBodyBehavior_hpp */
