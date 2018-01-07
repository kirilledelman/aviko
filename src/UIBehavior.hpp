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
	
	// anchors ( 0 = this edge, 1 = opposite edge, -1 = disabled (use width, height)
	float anchorLeft = 0;
	float anchorRight = 0;
	float anchorTop = 0;
	float anchorBottom = 0;
	
	// offsets from anchor
	float left = 0;
	float right = 0;
	float top = 0;
	float bottom = 0;
	
	// limits on width and height
	float minWidth = 0;
	float maxWidth = 9999;
	float minHeight = 0;
	float maxHeight = 9999;
	
	// values are computed during layout, unless object is not positioned (all anchors are -1)
	float layoutWidth = 0;
	float layoutHeight = 0;
	float layoutX = 0, layoutY = 0;
	
	// for horizontal and vertical - expands opposite axis to fill container
	bool layoutExpandCrossAxis = true;
	
	// reduces available w/h inside for layout of children
	float padTop = 0;
	float padBottom = 0;
	float padLeft = 0;
	float padRight = 0;
	
	// affects placing / sizing this UI during layout
	float marginTop = 0;
	float marginBottom = 0;
	float marginLeft = 0;
	float marginRight = 0;
	
	// float SetWidth( float w );
	// float SetHeight( float h );
	
	void GetAnchoredPosition( UIBehavior* parentUI, float& x, float& y, float& w, float& h );
	
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
