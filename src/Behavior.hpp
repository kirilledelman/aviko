#ifndef Behavior_hpp
#define Behavior_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "Color.hpp"

/* MARK:	-				Behaviors
 
 Any number of behaviors can be attached to GameObject, and are
 meant to provide modular / reusable functionality to them.
 Behavior class is the base class for all other behaviors.
 Each frame, GameObject hierarchy dispatches Events to Behaviors.
 These events let behaviors update themselves, or gameobject they
 are attached to, or render gameobjects to screen.
 -------------------------------------------------------------------- */


struct Event;
class GameObject;

/// Behavior callback method type via a static class function
typedef void (*BehaviorEventCallback) ( void* behavior, void* param, Event* event );

/// maps hash(char*) -> BehaviorEventCallback
typedef unordered_map<string, BehaviorEventCallback> BehaviorEventMap;
typedef unordered_map<string, BehaviorEventCallback>::iterator BehaviorEventIterator;

class Behavior : public ScriptableClass {
public:
	
	// init, destroy
	Behavior( ScriptArguments* args );
	Behavior();
	~Behavior();
	
// behavior type
	
	bool isRenderBehavior = false;
	bool isBodyBehavior = false;
	bool isUIBehavior = false;
	
// parent object
	
	/// the GameObject this behavior is attached to
	GameObject *gameObject = NULL;

	/// attach/detach from a gameObject
	virtual bool SetGameObject( GameObject* go, int pos=-1 );
	
// scripting
		
	/// registers classes for scripting
	static void InitClass();
	
// events
	
	/// function pointers for event dispatcher
	BehaviorEventMap eventFunctions;
	
	/// register each callback using this in Behavior constructor
	void AddEventCallback( const char* eventName, BehaviorEventCallback callback );
	
	/// dispatches this behavior's callback
	void CallEventCallback( Event& event );
	
	/// returns callback, or NULL
	inline BehaviorEventCallback GetCallbackForEvent( const char* eventName ) { BehaviorEventIterator it = eventFunctions.find( eventName ); return it == eventFunctions.end() ? NULL : it->second; }
	
// active
	
	/// inactive behaviors don't get BehaviorEvents (or render)
	bool _active = true;
	
	/// inactive behaviors don't get BehaviorEvents (or render)
	bool active(){ return this->_active; } // getter
	
	/// inactive behaviors don't get BehaviorEvents (or render)
	virtual bool active( bool a ) { return this->_active = a; }; // setter
		
};

// Behavior -> script class "Behavior"
SCRIPT_CLASS_NAME( Behavior, "Behavior" );

#endif /* Behavior_hpp */
