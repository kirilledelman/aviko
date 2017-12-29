#ifndef BodyBehavior_hpp
#define BodyBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

/// physics behaviors should inherit from this class
class BodyBehavior : public Behavior {
public:
	
	// init, destroy
	BodyBehavior( ScriptArguments* args );
	BodyBehavior();
	~BodyBehavior();

	// collision
	
	// default collision category and mask (shapes can override)
	uint32 categoryBits = 0x1;
	uint32 maskBits = 0xFFFFFFFF;
	
	// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	// methods
	
	/// active callback
	static void ActiveChanged( BodyBehavior* behavior, GameObject* target, Event* event );
	
	/// added to scene or attached to gameObject callback
	static void Attached( BodyBehavior* behavior, GameObject* target, Event* event );
	
	/// removed from scene or detached from gameObject callback
	static void Detached( BodyBehavior* behavior, GameObject* target, Event* event );
	
	/// override Behavior active setter
	bool active( bool a );
	
	virtual void EnableBody( bool e ){};
	
	/// called after physics step on each body to sync position and rotation
	virtual void SyncObjectToBody(){};
	
	/// call to sync body to manually set object position
	virtual void SyncBodyToObject(){};
	
	/// sets body transform
	virtual void SetBodyTransform( b2Vec2, float angleInRad ){};
	
	/// gets body transform
	virtual void GetBodyTransform( b2Vec2&, float& angleInRad ){};
	
	/// set body position
	virtual void SetBodyPosition( b2Vec2 pos ){};
	
	/// set body angle
	virtual void SetBodyAngle( float angleInRad ){};
	
	bool SetGameObject( GameObject* go, int pos=-1 );
	
	/// construct and add body to scene->world
	virtual void AddBody( Scene* scene ){};
	
	/// removes body from world
	virtual void RemoveBody(){};
	
	// is set to true when body is in the world
	bool live = false;
	
	// converts flags to value array or single int
	static void BitsToValue( uint32, ArgValue& );
	
	// converts int, or array of ints to flags integer
	static uint32 ValueToBits( ArgValue& );
	
};

SCRIPT_CLASS_NAME( BodyBehavior, "BodyBehavior" );

#endif /* BodyBehavior_hpp */
