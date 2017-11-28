#ifndef UIBehavior_hpp
#define UIBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

class Scene;

class UIBehavior : public Behavior {
public:
	
	// init, destroy
	UIBehavior( ScriptArguments* args );
	UIBehavior();
	~UIBehavior();
	
	Scene* scene = NULL;
	
	// UI
	
	bool mouseOver = false;
	bool mouseDown[4] = { false, false, false, false }; // which button was down
	bool focusable = true;
	bool IsScreenPointInBounds( float x, float y, float* localX, float* localY );
	
	void Focus();
	void Blur();
	
	// UI events
	static void MouseMove( UIBehavior* behavior, Event* event);
	static void MouseButton( UIBehavior* behavior, Event* event);
	static void MouseWheel( UIBehavior* behavior, Event* event);
	static void Navigation( UIBehavior* behavior, Event* event);
	static void Key( UIBehavior* behavior, Event* event);
	static void KeyPress( UIBehavior* behavior, Event* event);
	static void Attached( UIBehavior* behavior, GameObject* );
	static void Detached( UIBehavior* behavior, GameObject* );
	static void AddedToScene( UIBehavior* behavior, GameObject* topObject );
	static void RemovedFromScene( UIBehavior* behavior, GameObject* topObject );
	static void ActiveChanged( UIBehavior* behavior, GameObject* object );
	
	// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	
	
};

#endif /* UIBehavior_hpp */
