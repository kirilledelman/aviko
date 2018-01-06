#ifndef UIBehavior_hpp
#define UIBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

class Scene;
class RenderSpriteBehavior;

class UIBehavior : public Behavior {
public:
	
	// init, destroy
	UIBehavior( ScriptArguments* args );
	UIBehavior();
	~UIBehavior();
	
	Scene* scene = NULL;
	
	typedef enum {
		None,
		Anchors,
		Horizontal,
		Vertical,
		Grid
	} LayoutType;
	
	// UI
	
	bool mouseOver = false;
	bool mouseDown[4] = { false, false, false, false }; // which button was down
	bool focusable = true;
	
	//
	bool IsScreenPointInBounds( float x, float y, float* localX, float* localY );
	
	/// focus on this ui
	bool Focus();
	
	/// blur this ui
	void Blur( bool sendEvent=true );
	
	/// pick a new focus object
	bool Navigate( float x, float y );
	
	// focus overrides
	UIBehavior* navigationLeft = NULL;
	UIBehavior* navigationRight = NULL;
	UIBehavior* navigationUp = NULL;
	UIBehavior* navigationDown = NULL;
	
	/// overridden
	bool active( bool a );
	
	// layout
	
	/// defines how this behavior lays our its children
	LayoutType layoutType = LayoutType::Anchors;
	
	// anchors
	float anchorLeft = 0;
	float anchorRight = 0;
	float anchorTop = 0;
	float anchorBottom = 0;
	
	//
	float left = 0;
	float right = 0;
	float top = 0;
	float bottom = 0;
	
	float minWidth = 0;
	float maxWidth = 9999;
	float minHeight = 0;
	float maxHeight = 9999;
	
	// if layoutType = none (or no ui parent), these are set, otherwise, computed
	float width = 0;
	float height = 0;
	void UpdatePosition( float& x, float& y, float& w, float& h );
	
	// UI events
	static void MouseMove( UIBehavior* behavior, void*, Event* event);
	static void MouseButton( UIBehavior* behavior, void*, Event* event);
	static void MouseWheel( UIBehavior* behavior, void*, Event* event);
	static void Navigation( UIBehavior* behavior, void*, Event* event);
	static void Key( UIBehavior* behavior, void*, Event* event);
	static void KeyPress( UIBehavior* behavior, void*, Event* event);
	static void Attached( UIBehavior* behavior, GameObject* topObject, Event* event );
	static void Detached( UIBehavior* behavior, GameObject* topObject, Event* event );
	static void ActiveChanged( UIBehavior* behavior, GameObject* object, Event* event );
	static void Layout( UIBehavior* behavior, void*, Event* event );
	
	// checks and (re)sets RenderSprite behavior that clips this behavior
	void CheckClipping();
	RenderSpriteBehavior* clippedBy = NULL;
	
	// scripting
	
	/// registers class for scripting
	static void InitClass();

	
};

SCRIPT_CLASS_NAME( UIBehavior, "UI" );

#endif /* UIBehavior_hpp */
