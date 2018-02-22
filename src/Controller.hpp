#ifndef Controller_hpp
#define Controller_hpp

#include "common.h"
#include "ScriptableClass.hpp"

/* MARK:	-				Controller
 
 Represents a plugged in controller (joystick or gamepad) with its action
 mappings loaded/saved to config/JoystickName.json
 
 Also, there's always a default one, representing keyboard
 -------------------------------------------------------------------- */

/// class reporting input events
class Controller : public ScriptableClass {
protected:
	
	enum ActionType {
		BUTTON,
		DIR_NEGATIVE,
		DIR_POSITIVE
	};
	
	typedef struct {
		string action;
		ActionType type = BUTTON; // button, - or + direction
		int index = 0; // button, axis, or hat index
	} Binding;
	
	typedef unordered_map<string, int> ActionStates;
	typedef unordered_map<int, vector<Binding>> BindMap;
	typedef unordered_map<int, vector<Binding>>::iterator BindMapIterator;
	
public:
	
	SDL_Joystick* joystick = NULL;
	SDL_JoystickID id = 0;
	SDL_JoystickGUID guid;
	string guidString;
	string name;
	int numButtons = 0;
	int numAxis = 0;
	int numHats = 0;
	float deadZone = 0.2f;
	
	BindMap bindings;
	ActionStates states;
	
	static void InitClass();
	
	/// event forwarded by Input
	void HandleEvent( SDL_Event& e );
	
	/// generates events for key event
	void DispatchActions( int key, SDL_Event& e, vector<Binding>& actions );
	
	/// tries to load config
	bool LoadConfig();
	
	/// saves config
	bool SaveConfig();
	
	/// deletes config file
	bool DeleteConfig();
	
	Controller( SDL_Joystick* joy );
	Controller( ScriptArguments* );
	~Controller();
	
};

SCRIPT_CLASS_NAME( Controller, "Controller" );

#endif /* Controller_hpp */
