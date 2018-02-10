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
	
	typedef enum class LayoutType {
		None,
		Anchors,
		Horizontal,
		Vertical,
		Grid
	} LayoutType;
	
	typedef enum class LayoutAlign {
		Default,
		Start,
		Center,
		End,
		Stretch
	} LayoutAlign;
	
	// UI
	
	bool mouseOver = false;
	bool mouseDown[4] = { false, false, false, false }; // which button was down
	bool focusable = false;
	
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
	string navigationGroup;
	bool autoNavigate = true;
	
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
	float maxWidth = 0;
	float minHeight = 0;
	float maxHeight = 0;
	
	// layout size (including padding, but not margins)
	float layoutWidth = 0;
	float layoutHeight = 0;
	
	// stretch this element to fill empty space in vertical and horizontal layouts, where fitChildren = false
	float flex = 0;
	
	// for horizontal and vertical - how to align child on secondary axis
	LayoutAlign crossAxisAlign = LayoutAlign::Stretch;
	
	// overrides parent
	LayoutAlign selfAlign = LayoutAlign::Default;
	
	// adjust own size after layout to fit children
	bool fitChildren = true;
	
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
	
	// extra spacing between items for Horizontal, Vertical, Grid layouts
	float spacingX = 0;
	float spacingY = 0;
	
	/// requests late layout event (scene)
	void RequestLayout( ArgValue trigger );
	
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
