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
	bool disabled = false;
	
	//
	bool IsScreenPointInBounds( float x, float y, float* localX, float* localY );
	
	/// focus on this ui
	bool Focus();
	
	/// blur this ui
	void Blur( bool sendEvent=true );
	
	/// focus on a new focus object in direction
	bool Navigate( float x, float y );
	
	/// find focusable in direction
	UIBehavior* FindFocusable( float x, float y );
	
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
	
	// offset from layout position
	float layoutOffsetX = 0;
	float layoutOffsetY = 0;
	
	// reverse children order
	bool layoutReversed = false;
	
	// stretch this element to fill empty space in vertical and horizontal layouts, where fitChildren = false
	float flex = 0;
	
	// if true, Horizontal and Vertical layouts will wrap their children
	bool wrapEnabled = false;
	
	// if > 0, will auto-wrap row/column after this many items in Horizontal and Vertical layouts
	int wrapAfter = 0;

	// if true, row will wrap after this element in Horizontal and Vertical layouts
	bool forceWrap = false;
	
	// for horizontal and vertical - how to align child on X axis
	LayoutAlign axisAlignX = LayoutAlign::Start;
	
	// for horizontal and vertical - how to align child on Y axis
	LayoutAlign axisAlignY = LayoutAlign::Start;
	
	// overrides parent
	LayoutAlign selfAlign = LayoutAlign::Default;
	
	/// adjust own size after layout to fit children
	bool fitChildren = true;
	
	/// if set to true, layout doesn't position this object
	bool fixedPosition = false;
	
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
	
	// helper - returns true, if this UI is stretchy in X or Y dir
	bool IsStretchyX();
	bool IsStretchyY();
	
	/// requests late layout event (scene)
	void RequestLayout( ArgValue trigger );
		
	/// layout types
	void LayoutNone( vector<UIBehavior*> &childUIs );
	void LayoutHorizontal( vector<UIBehavior*> &childUIs );
	void LayoutVertical( vector<UIBehavior*> &childUIs );
	void LayoutAnchors( vector<UIBehavior*> &childUIs );
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
	
	/// blocks UI events from bubbling up
	bool blocking = false;
	
	/// currently 'over' UIBehaviors
	static unordered_set<UIBehavior*> rollovers;
	
	// debug draw
	void DebugDraw( GPU_Target* targ );
	
	// scripting
	
	/// registers class for scripting
	static void InitClass();

	
};

SCRIPT_CLASS_NAME( UIBehavior, "UI" );

#endif /* UIBehavior_hpp */
