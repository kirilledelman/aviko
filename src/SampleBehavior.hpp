#ifndef SampleBehavior_hpp
#define SampleBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

/*
	Template for creating new behaviors
*/

class SampleBehavior : public Behavior {
public:
	
	// init, destroy
	SampleBehavior( ScriptArguments* args );
	SampleBehavior();
	~SampleBehavior();

// events
	
	// placeholder events
	static void Update( SampleBehavior* behavior, void*, Event* event );
	static void LateUpdate( SampleBehavior* behavior, void*, Event* event );
	static void Added( SampleBehavior* behavior, GameObject* newParent, Event* event );
	static void Removed( SampleBehavior* behavior, GameObject* oldParent, Event* event );
	static void AddedToScene( SampleBehavior* behavior, GameObject* topObject, Event* event );
	static void RemovedFromScene( SampleBehavior* behavior, GameObject* topObject, Event* event );
	static void ActiveChanged( SampleBehavior* behavior, GameObject* object, Event* event );
	static void Render( SampleBehavior* behavior, GPU_Target* target, Event* event );
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
};

SCRIPT_CLASS_NAME( SampleBehavior, "SampleBehavior" );


#endif /* SampleBehavior_hpp */
