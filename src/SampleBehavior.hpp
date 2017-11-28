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
	static void Update( SampleBehavior* behavior, void* );
	static void LateUpdate( SampleBehavior* behavior, void* );
	static void Added( SampleBehavior* behavior, GameObject* newParent );
	static void Removed( SampleBehavior* behavior, GameObject* oldParent );
	static void AddedToScene( SampleBehavior* behavior, GameObject* topObject );
	static void RemovedFromScene( SampleBehavior* behavior, GameObject* topObject );
	static void ActiveChanged( SampleBehavior* behavior, GameObject* object );
	static void Render( SampleBehavior* behavior, GPU_Target* target );
	
// scripting
	
	/// registers class for scripting
	static void InitClass();


	
};

#endif /* SampleBehavior_hpp */
