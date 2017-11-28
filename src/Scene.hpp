#ifndef Scene_hpp
#define Scene_hpp

#include "common.h"
#include "GameObject.hpp"
#include "Color.hpp"
#include "UIBehavior.hpp"
#include "SceneDebugDraw.hpp"

class Scene : public GameObject {
protected:
	
	/// instance of Box2D debug draw class
	SceneDebugDraw _sceneDebugDraw;
	
public:

	// init, destroy
	Scene( ScriptArguments* args );
	Scene( const char* filename ); // load from file
	Scene();
	~Scene();
	
// Javascript
	
	/// registers classes for scripting
	static void InitClass();	

// UI elements
	
	vector<UIBehavior*> uiElements;
	
	UIBehavior* focusedUI = NULL;
	
	// notifications by UIBehaviors
	void UIAdded( UIBehavior* ui );
	void UIRemoved( UIBehavior* ui );
	
// physics
	
	/// Box2D world
	b2World* world = NULL;
	
	/// called at the top of the frame
	void SimulatePhysics();
	
// rendering
	
	/// set to true to draw Box2D debug overlay
	bool debugDraw = true;
	
	// clear color
	Color *clearColor = NULL;
	
	/// called to render all objects
	void Render( Event& event );
	
// hierarchy
	
	/// returns scene
	Scene* GetScene() { return this; }
	
};

#endif /* Scene_hpp */
