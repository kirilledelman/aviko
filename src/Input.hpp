#ifndef Input_hpp
#define Input_hpp

#include "common.h"
#include "ScriptableClass.hpp"

class Controller;

/* MARK:	-				Input
 
 There's global instance of Input available in script as app.input
 Provides access to keyboard, mouse, joystick events and real-time values
 -------------------------------------------------------------------- */


/// class reporting input events
class Input : public ScriptableClass {
public:

	// capture mouse or not
	bool captureMouse = false;
	
	// show mouse cursor or not
	bool showCursor = true;
	
	// send repeated keydowns while holding down a key
	bool repeatKeyEnabled = true;
	
	// Joystick info 
	typedef unordered_map<SDL_JoystickID, Controller*> JoystickMap;
	JoystickMap joysticks;
	
// scripting
	
	void InitClass();
	
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
	void AddKeyboardController();
	
// update
	
	void HandleEvent( SDL_Event& event );
	
	/// true if button is down. -1 = any button
	bool IsJoystickButtonDown( Controller* joy, int btnIndex=-1 );
	float GetJoystickAxis( Controller* joy, int axis=-1 );
	float GetJoystickHat( Controller* joy, int hat=-1, int axis=-1 );
	
// ui
	
	/// passes event on current scene's UIBehaviors
	void UIEvent( Event& event );
	
	// controllers check these axis names and forward events to UIBehaviors if matched
	string navigationXAxis = "horizontal";
	string navigationYAxis = "vertical";
	string navigationAccept = "accept";
	string navigationCancel = "cancel";
	
	// init, destroy
	Input();
	Input(ScriptArguments*);
	~Input();
};

SCRIPT_CLASS_NAME( Input, "Input" );

#endif /* Input_hpp */
