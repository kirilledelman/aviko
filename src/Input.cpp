#include "Input.hpp"
#include "Application.hpp"
#include "Controller.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


Input::Input() {
	// SDL_StartTextInput();
}

Input::Input( ScriptArguments* ) { script.ReportError( "Input can't be created using 'new'. Only one instance is available as global 'app.input' property." ); }

Input::~Input() {
	// SDL_StopTextInput();
}

void Input::AddKeyboardController () {
	// create blank one
	Controller* kbd = new Controller( (SDL_Joystick*) NULL );
	this->joysticks[ -9999 ] = kbd;
	// fire event
	Event event( EVENT_CONTROLLERADDED );
	event.scriptParams.ResizeArguments( 0 );
	event.scriptParams.AddObjectArgument( kbd->scriptObject );
	CallEvent( event );
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void Input::InitClass() {
	
	// register class
	script.RegisterClass<Input>( NULL, true );
	
	// key constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "Key", constants );		
	script.SetProperty( "A", ArgValue( SDL_SCANCODE_A ), constants );
	script.SetProperty( "B", ArgValue( SDL_SCANCODE_B ), constants );
	script.SetProperty( "C", ArgValue( SDL_SCANCODE_C ), constants );
	script.SetProperty( "D", ArgValue( SDL_SCANCODE_D ), constants );
	script.SetProperty( "E", ArgValue( SDL_SCANCODE_E ), constants );
	script.SetProperty( "F", ArgValue( SDL_SCANCODE_F ), constants );
	script.SetProperty( "G", ArgValue( SDL_SCANCODE_G ), constants );
	script.SetProperty( "H", ArgValue( SDL_SCANCODE_H ), constants );
	script.SetProperty( "I", ArgValue( SDL_SCANCODE_I ), constants );
	script.SetProperty( "J", ArgValue( SDL_SCANCODE_J ), constants );
	script.SetProperty( "K", ArgValue( SDL_SCANCODE_K ), constants );
	script.SetProperty( "L", ArgValue( SDL_SCANCODE_L ), constants );
	script.SetProperty( "M", ArgValue( SDL_SCANCODE_M ), constants );
	script.SetProperty( "N", ArgValue( SDL_SCANCODE_N ), constants );
	script.SetProperty( "O", ArgValue( SDL_SCANCODE_O ), constants );
	script.SetProperty( "P", ArgValue( SDL_SCANCODE_P ), constants );
	script.SetProperty( "Q", ArgValue( SDL_SCANCODE_Q ), constants );
	script.SetProperty( "R", ArgValue( SDL_SCANCODE_R ), constants );
	script.SetProperty( "S", ArgValue( SDL_SCANCODE_S ), constants );
	script.SetProperty( "T", ArgValue( SDL_SCANCODE_T ), constants );
	script.SetProperty( "U", ArgValue( SDL_SCANCODE_U ), constants );
	script.SetProperty( "V", ArgValue( SDL_SCANCODE_V ), constants );
	script.SetProperty( "W", ArgValue( SDL_SCANCODE_W ), constants );
	script.SetProperty( "X", ArgValue( SDL_SCANCODE_X ), constants );
	script.SetProperty( "Y", ArgValue( SDL_SCANCODE_Y ), constants );
	script.SetProperty( "Z", ArgValue( SDL_SCANCODE_Z ), constants );
	script.SetProperty( "One", ArgValue( SDL_SCANCODE_1 ), constants );
	script.SetProperty( "Two", ArgValue( SDL_SCANCODE_2 ), constants );
	script.SetProperty( "Three", ArgValue( SDL_SCANCODE_3 ), constants );
	script.SetProperty( "Four", ArgValue( SDL_SCANCODE_4 ), constants );
	script.SetProperty( "Five", ArgValue( SDL_SCANCODE_5 ), constants );
	script.SetProperty( "Six", ArgValue( SDL_SCANCODE_6 ), constants );
	script.SetProperty( "Seven", ArgValue( SDL_SCANCODE_7 ), constants );
	script.SetProperty( "Eight", ArgValue( SDL_SCANCODE_8 ), constants );
	script.SetProperty( "Nine", ArgValue( SDL_SCANCODE_9 ), constants );
	script.SetProperty( "Zero", ArgValue( SDL_SCANCODE_0 ), constants );
	script.SetProperty( "Return", ArgValue( SDL_SCANCODE_RETURN ), constants );
	script.SetProperty( "Enter", ArgValue( SDL_SCANCODE_RETURN ), constants );
	script.SetProperty( "Escape", ArgValue( SDL_SCANCODE_ESCAPE ), constants );
	script.SetProperty( "Backspace", ArgValue( SDL_SCANCODE_BACKSPACE ), constants );
	script.SetProperty( "Tab", ArgValue( SDL_SCANCODE_TAB ), constants );
	script.SetProperty( "Space", ArgValue( SDL_SCANCODE_SPACE ), constants );
	script.SetProperty( "Minus", ArgValue( SDL_SCANCODE_MINUS ), constants );
	script.SetProperty( "Equals", ArgValue( SDL_SCANCODE_EQUALS ), constants );
	script.SetProperty( "LeftBracket", ArgValue( SDL_SCANCODE_LEFTBRACKET ), constants );
	script.SetProperty( "RightBracket", ArgValue( SDL_SCANCODE_RIGHTBRACKET ), constants );
	script.SetProperty( "Backslash", ArgValue( SDL_SCANCODE_BACKSLASH ), constants );
	script.SetProperty( "Semicolon", ArgValue( SDL_SCANCODE_SEMICOLON ), constants );
	script.SetProperty( "Apostrophe", ArgValue( SDL_SCANCODE_APOSTROPHE ), constants );
	script.SetProperty( "Grave", ArgValue( SDL_SCANCODE_GRAVE ), constants );
	script.SetProperty( "Comma", ArgValue( SDL_SCANCODE_COMMA ), constants );
	script.SetProperty( "Period", ArgValue( SDL_SCANCODE_PERIOD ), constants );
	script.SetProperty( "Slash", ArgValue( SDL_SCANCODE_SLASH ), constants );
	script.SetProperty( "CapsLock", ArgValue( SDL_SCANCODE_CAPSLOCK ), constants );
	script.SetProperty( "F1", ArgValue( SDL_SCANCODE_F1 ), constants );
	script.SetProperty( "F2", ArgValue( SDL_SCANCODE_F2 ), constants );
	script.SetProperty( "F3", ArgValue( SDL_SCANCODE_F3 ), constants );
	script.SetProperty( "F4", ArgValue( SDL_SCANCODE_F4 ), constants );
	script.SetProperty( "F5", ArgValue( SDL_SCANCODE_F5 ), constants );
	script.SetProperty( "F6", ArgValue( SDL_SCANCODE_F6 ), constants );
	script.SetProperty( "F7", ArgValue( SDL_SCANCODE_F7 ), constants );
	script.SetProperty( "F8", ArgValue( SDL_SCANCODE_F8 ), constants );
	script.SetProperty( "F9", ArgValue( SDL_SCANCODE_F9 ), constants );
	script.SetProperty( "F10", ArgValue( SDL_SCANCODE_F10 ), constants );
	script.SetProperty( "F11", ArgValue( SDL_SCANCODE_F11 ), constants );
	script.SetProperty( "F12", ArgValue( SDL_SCANCODE_F12 ), constants );
	script.SetProperty( "PrintScreen", ArgValue( SDL_SCANCODE_PRINTSCREEN ), constants );
	script.SetProperty( "ScrollLock", ArgValue( SDL_SCANCODE_SCROLLLOCK ), constants );
	script.SetProperty( "Pause", ArgValue( SDL_SCANCODE_PAUSE ), constants );
	script.SetProperty( "Insert", ArgValue( SDL_SCANCODE_INSERT ), constants );
	script.SetProperty( "Home", ArgValue( SDL_SCANCODE_HOME ), constants );
	script.SetProperty( "PageUp", ArgValue( SDL_SCANCODE_PAGEUP ), constants );
	script.SetProperty( "Delete", ArgValue( SDL_SCANCODE_DELETE ), constants );
	script.SetProperty( "End", ArgValue( SDL_SCANCODE_END ), constants );
	script.SetProperty( "PageDown", ArgValue( SDL_SCANCODE_PAGEDOWN ), constants );
	script.SetProperty( "Right", ArgValue( SDL_SCANCODE_RIGHT ), constants );
	script.SetProperty( "Left", ArgValue( SDL_SCANCODE_LEFT ), constants );
	script.SetProperty( "Down", ArgValue( SDL_SCANCODE_DOWN ), constants );
	script.SetProperty( "Up", ArgValue( SDL_SCANCODE_UP ), constants );
	script.SetProperty( "NumLockClear", ArgValue( SDL_SCANCODE_NUMLOCKCLEAR ), constants );
	script.SetProperty( "KeypadDivide", ArgValue( SDL_SCANCODE_KP_DIVIDE ), constants );
	script.SetProperty( "KeypadMultiply", ArgValue( SDL_SCANCODE_KP_MULTIPLY ), constants );
	script.SetProperty( "KeypadMinus", ArgValue( SDL_SCANCODE_KP_MINUS ), constants );
	script.SetProperty( "KeypadPlus", ArgValue( SDL_SCANCODE_KP_PLUS ), constants );
	script.SetProperty( "KeypadEnter", ArgValue( SDL_SCANCODE_KP_ENTER ), constants );
	script.SetProperty( "Keypad1", ArgValue( SDL_SCANCODE_KP_1 ), constants );
	script.SetProperty( "Keypad2", ArgValue( SDL_SCANCODE_KP_2 ), constants );
	script.SetProperty( "Keypad3", ArgValue( SDL_SCANCODE_KP_3 ), constants );
	script.SetProperty( "Keypad4", ArgValue( SDL_SCANCODE_KP_4 ), constants );
	script.SetProperty( "Keypad5", ArgValue( SDL_SCANCODE_KP_5 ), constants );
	script.SetProperty( "Keypad6", ArgValue( SDL_SCANCODE_KP_6 ), constants );
	script.SetProperty( "Keypad7", ArgValue( SDL_SCANCODE_KP_7 ), constants );
	script.SetProperty( "Keypad8", ArgValue( SDL_SCANCODE_KP_8 ), constants );
	script.SetProperty( "Keypad9", ArgValue( SDL_SCANCODE_KP_9 ), constants );
	script.SetProperty( "Keypad0", ArgValue( SDL_SCANCODE_KP_0 ), constants );
	script.SetProperty( "KeypadPeriod", ArgValue( SDL_SCANCODE_KP_PERIOD ), constants );
	script.SetProperty( "Application", ArgValue( SDL_SCANCODE_APPLICATION ), constants );
	script.SetProperty( "KeypadEquals", ArgValue( SDL_SCANCODE_KP_EQUALS ), constants );
	script.SetProperty( "F13", ArgValue( SDL_SCANCODE_F13 ), constants );
	script.SetProperty( "F14", ArgValue( SDL_SCANCODE_F14 ), constants );
	script.SetProperty( "F15", ArgValue( SDL_SCANCODE_F15 ), constants );
	script.SetProperty( "F16", ArgValue( SDL_SCANCODE_F16 ), constants );
	script.SetProperty( "F17", ArgValue( SDL_SCANCODE_F17 ), constants );
	script.SetProperty( "F18", ArgValue( SDL_SCANCODE_F18 ), constants );
	script.SetProperty( "F19", ArgValue( SDL_SCANCODE_F19 ), constants );
	script.SetProperty( "F20", ArgValue( SDL_SCANCODE_F20 ), constants );
	script.SetProperty( "F21", ArgValue( SDL_SCANCODE_F21 ), constants );
	script.SetProperty( "F22", ArgValue( SDL_SCANCODE_F22 ), constants );
	script.SetProperty( "F23", ArgValue( SDL_SCANCODE_F23 ), constants );
	script.SetProperty( "F24", ArgValue( SDL_SCANCODE_F24 ), constants );
	script.SetProperty( "KeypadComma", ArgValue( SDL_SCANCODE_KP_COMMA ), constants );
	script.SetProperty( "Return2", ArgValue( SDL_SCANCODE_RETURN2 ), constants );
	script.SetProperty( "Control", ArgValue( SDL_SCANCODE_LCTRL ), constants );
	script.SetProperty( "LeftControl", ArgValue( SDL_SCANCODE_LCTRL ), constants );
	script.SetProperty( "Shift", ArgValue( SDL_SCANCODE_LSHIFT ), constants );
	script.SetProperty( "LeftShift", ArgValue( SDL_SCANCODE_LSHIFT ), constants );
	script.SetProperty( "Alt", ArgValue( SDL_SCANCODE_LALT ), constants );
	script.SetProperty( "LeftAlt", ArgValue( SDL_SCANCODE_LALT ), constants );
	script.SetProperty( "Command", ArgValue( SDL_SCANCODE_LGUI ), constants );
	script.SetProperty( "LeftCommand", ArgValue( SDL_SCANCODE_LGUI ), constants );
	script.SetProperty( "Windows", ArgValue( SDL_SCANCODE_LGUI ), constants );
	script.SetProperty( "LeftWindows", ArgValue( SDL_SCANCODE_LGUI ), constants );
	script.SetProperty( "RightControl", ArgValue( SDL_SCANCODE_RCTRL ), constants );
	script.SetProperty( "RightShift", ArgValue( SDL_SCANCODE_RSHIFT ), constants );
	script.SetProperty( "RightAlt", ArgValue( SDL_SCANCODE_RALT ), constants );
	script.SetProperty( "RightCommand", ArgValue( SDL_SCANCODE_RGUI ), constants );
	script.SetProperty( "RightWindows", ArgValue( SDL_SCANCODE_RGUI ), constants );
	
	script.SetProperty( "MouseButton", ArgValue( KEY_MOUSE_BUTTON ), constants );
	script.SetProperty( "MouseWheel", ArgValue( KEY_MOUSE_WHEEL ), constants );
	script.SetProperty( "MouseWheelX", ArgValue( KEY_MOUSE_WHEEL_X ), constants );
	script.SetProperty( "MouseWheelY", ArgValue( KEY_MOUSE_WHEEL ), constants );
	script.SetProperty( "JoyButton", ArgValue( KEY_JOY_BUTTON ), constants );
	script.SetProperty( "JoyAxis", ArgValue( KEY_JOY_AXIS ), constants );
	script.SetProperty( "JoyHatX", ArgValue( KEY_JOY_HAT_X ), constants );
	script.SetProperty( "JoyHatY", ArgValue( KEY_JOY_HAT_Y ), constants );
	script.FreezeObject( constants );
	
	//SDL_NUM_SCANCODES
	
	// properties
	
	script.AddProperty<Input>
	( "captureMouse",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return ((Input*) inp)->captureMouse; }),
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ SDL_CaptureMouse( (SDL_bool) val ); return ((Input*) inp)->captureMouse = val; }));
	
	script.AddProperty<Input>
	( "showCursor",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return ((Input*) inp)->showCursor; }),
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ SDL_ShowCursor( val ); return ((Input*) inp)->showCursor = val; }));
	
	script.AddProperty<Input>
	( "repeatKeyEnabled",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return ((Input*) inp)->repeatKeyEnabled; }),
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return ((Input*) inp)->repeatKeyEnabled = val; }));
	
	script.AddProperty<Input>
	( "mouseX",
	 static_cast<ScriptFloatCallback>([](void* inp, float val){ int x, y; SDL_GetMouseState( &x, &y ); return ( x - app.backScreenDstRect.x ) * app.backscreenScale; }));

	script.AddProperty<Input>
	( "mouseY",
	 static_cast<ScriptFloatCallback>([](void* inp, float val){ int x, y; SDL_GetMouseState( &x, &y ); return ( y - app.backScreenDstRect.y ) * app.backscreenScale; }));

	script.AddProperty<Input>
	( "mouseLeft",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_LEFT); }));

	script.AddProperty<Input>
	( "mouseMiddle",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_MIDDLE); }));

	script.AddProperty<Input>
	( "mouseRight",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_RIGHT); }));

	script.AddProperty<Input>
	( "mouseWheelScale",
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ return ((Input*) inp)->mouseWheelScale; }),
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){
		((Input*) inp)->mouseWheelScale = val;
		return val;
	}));
	
	script.AddProperty<Input>
	( "numJoysticks",
	 static_cast<ScriptIntCallback>([](void* inp, int val){ return SDL_NumJoysticks(); }));
	
	script.AddProperty<Input>
	( "controllers",
	 static_cast<ScriptArrayCallback>([](void* inp, ArgValueVector* val ){
		ArgValueVector* ret = new ArgValueVector();
		size_t nj = app.input.joysticks.size();
		ret->resize( nj );
		JoystickMap::iterator it = app.input.joysticks.begin();
		for( size_t i = 0; i < nj; i++ ) {
			ArgValue &v = ret->at( i );
			v.type = TypeObject;
			Controller* contr = it->second;
			v.value.objectValue = contr->scriptObject;
			it++;
		}
		return ret;
	}));
	
	script.AddProperty<Input>
	( "navigationHorizontalAxis",
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return ((Input*) inp)->navigationXAxis; }),
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return (((Input*) inp)->navigationXAxis = val ); }));
	
	script.AddProperty<Input>
	( "navigationVerticalAxis",
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return ((Input*) inp)->navigationYAxis; }),
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return (((Input*) inp)->navigationYAxis = val ); }));
	
	script.AddProperty<Input>
	( "navigationAccept",
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return ((Input*) inp)->navigationAccept; }),
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return (((Input*) inp)->navigationAccept = val ); }));
	
	script.AddProperty<Input>
	( "navigationCancel",
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return ((Input*) inp)->navigationCancel; }),
	 static_cast<ScriptStringCallback>([](void* inp, string val){ return (((Input*) inp)->navigationCancel = val ); }));
	
	// functions
	
	script.DefineFunction<Input>
	("keyName",
	 static_cast<ScriptFunctionCallback>([](void* inp, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: keyName( int key )";
		bool goodCall = true;
		int index = 0;
		
		// first param must be an int
		if ( sa.args.size() == 1 && sa.args[ 0 ].type == TypeInt ) {
			index = sa.args[ 0 ].value.intValue;
		} else goodCall = false;
		
		// if not a valid call report error
		if ( !goodCall ) {
			script.ReportError( error );
			return false;
		}
		
		int numKeys;
		SDL_GetKeyboardState( &numKeys );
		if ( index < numKeys ) {
			sa.ReturnString( string( SDL_GetKeyName( SDL_SCANCODE_TO_KEYCODE( index ) ) ) );
		} else {
			sa.ReturnUndefined();
		}
		return true;
	}));
	
	script.DefineFunction<Input>
	("get",
	 static_cast<ScriptFunctionCallback>([](void* inp, ScriptArguments& sa ){
		
		// validate params
		const char* error = "usage: get( Int key[, Int index[, Int joystickID ]] )";
		int index = 0, btnIndex = -1, joyId = -1;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeInt, &index, TypeInt, &btnIndex, TypeInt, &joyId ) ) {
			script.ReportError( error );
			return false;
		}
		
		// keyboard
		if ( index < SDL_NUM_SCANCODES ) {
			int numKeys;
			const Uint8* state = SDL_GetKeyboardState( &numKeys );
			sa.ReturnBool( state[ index ] );
			
		// mouse
		} else if ( index == KEY_MOUSE_BUTTON ){
			Uint32 button = 0;
			// any mouse button
			if ( btnIndex < 0 ) {
				button = SDL_BUTTON( SDL_BUTTON_RIGHT ) | SDL_BUTTON( SDL_BUTTON_LEFT ) | SDL_BUTTON( SDL_BUTTON_MIDDLE );
			} else {
				button = SDL_BUTTON( btnIndex - 1 );
			}
			sa.ReturnBool( SDL_GetMouseState( NULL, NULL ) & button );
		
		// joystick button
		} else if ( index == KEY_JOY_BUTTON ) {
			bool btnPressed = false;
			JoystickMap::iterator it;
			
			// any joystick
			if ( joyId == -1 ) {
				it = app.input.joysticks.begin();
				while( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					if ( ( btnPressed = app.input.IsJoystickButtonDown( joy, btnIndex ) ) ) break;
					it++;
				}
			// specific joystick
			} else {
				it = app.input.joysticks.find( (SDL_JoystickID) joyId );
				if( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					btnPressed = app.input.IsJoystickButtonDown( joy, btnIndex );
				}
			}
			sa.ReturnBool( btnPressed );
		
		// hat direction
		} else if ( index == KEY_JOY_HAT_X || index == KEY_JOY_HAT_Y ) {
			float axisValue = 0;
			JoystickMap::iterator it;
			
			// any joystick
			if ( joyId == -1 ) {
				it = app.input.joysticks.begin();
				while( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					if ( ( axisValue = app.input.GetJoystickHat( joy, index - KEY_JOY_HAT_X, btnIndex ) ) != 0 ) break;
					it++;
				}
			// specific joystick
			} else {
				it = app.input.joysticks.find( (SDL_JoystickID) joyId );
				if( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					axisValue = app.input.GetJoystickHat( joy, index - KEY_JOY_HAT_X, btnIndex );
				}
			}
			sa.ReturnFloat( axisValue );
			
		// axis
		} else if ( index == KEY_JOY_AXIS ) {
			float axisValue = 0;
			JoystickMap::iterator it;
			
			// any joystick
			if ( joyId == -1 ) {
				it = app.input.joysticks.begin();
				while( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					if ( ( axisValue = app.input.GetJoystickAxis( joy, btnIndex ) ) != 0 ) break;
					it++;
				}
				// specific joystick
			} else {
				it = app.input.joysticks.find( (SDL_JoystickID) joyId );
				if( it != app.input.joysticks.end() ){
					Controller* joy = it->second;
					axisValue = app.input.GetJoystickAxis( joy, btnIndex );
				}
			}
			sa.ReturnFloat( axisValue );
		} else {
			// unknown
			sa.ReturnInt( 0 );
		}
		return true;
	}));
		
	// spawn object
	script.NewScriptObject<Input>( this );
	script.AddGlobalNamedObject( "Input", this->scriptObject );

	// set defaults
	SDL_ShowCursor( this->showCursor );
	SDL_CaptureMouse( (SDL_bool) this->captureMouse );
}

void Input::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	// add controllers
	JoystickMap::iterator it = this->joysticks.begin(), end = this->joysticks.end();
	while( it != end ) {
		protectedObjects.push_back( &it->second->scriptObject );
		it++;
	}
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );	
}


/* MARK:	-				Input event
 -------------------------------------------------------------------- */


void Input::HandleEvent( SDL_Event& e ) {
	
	Uint32 etype = e.type;
	Event event;
	event.bubbles = true;
	Controller* joy = NULL;
	if ( etype == SDL_KEYDOWN && ( repeatKeyEnabled || e.key.repeat == 0 ) ) {
		joy = joysticks[ -9999 ];
		event.name = EVENT_KEYDOWN;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.key.keysym.scancode );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_SHIFT );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_ALT );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_CTRL );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_GUI );
		event.scriptParams.AddBoolArgument( e.key.repeat > 0 );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_KEYUP ) {
		joy = joysticks[ -9999 ];
		event.name = EVENT_KEYUP;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.key.keysym.scancode );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_SHIFT );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_ALT );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_CTRL );
		event.scriptParams.AddBoolArgument( e.key.keysym.mod & KMOD_GUI );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_TEXTINPUT ) {
		joy = joysticks[ -9999 ];
		event.name = EVENT_KEYPRESS;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddStringArgument( e.text.text );
		CallEvent( event );
		UIEvent( event );
	} /*else if ( etype == SDL_TEXTEDITING ) {
		joy = joysticks[ -9999 ];
		printf( "SDL_TEXTEDITING %s\n", e.edit.text );
		event.name = EVENT_KEYPRESS;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddStringArgument( e.text.text );
		CallEvent( event );
		UIEvent( event );
	} */ else if ( etype == SDL_MOUSEBUTTONDOWN ) {
		event.name = EVENT_MOUSEDOWN;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.button.button );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		CallEvent( event );
		UIEvent( event, true );
	} else if ( etype == SDL_MOUSEBUTTONUP ) {
		event.name = EVENT_MOUSEUP;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.button.button );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		CallEvent( event );
		UIEvent( event, true );
	} else if ( etype == SDL_MOUSEMOTION ) {
		event.name = EVENT_MOUSEMOVE;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( e.motion.xrel * app.backscreenScale );
		event.scriptParams.AddFloatArgument( e.motion.yrel * app.backscreenScale );
		CallEvent( event );
		UIEvent( event, true );
	} else if ( etype == SDL_MOUSEWHEEL ) {
		event.name = EVENT_MOUSEWHEEL;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddFloatArgument( e.wheel.y * mouseWheelScale );
		event.scriptParams.AddFloatArgument( e.wheel.x * mouseWheelScale );
		CallEvent( event );
		UIEvent( event, true );
	} else if ( etype == SDL_JOYDEVICEADDED ) {
		// add to joysticks
		SDL_Joystick* jck = SDL_JoystickOpen( e.jdevice.which );
		SDL_JoystickID jid = SDL_JoystickInstanceID( jck );
		joy = new Controller( jck );
		this->joysticks[ jid ] = joy;
		// fire event
		event.name = EVENT_CONTROLLERADDED;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( joy->scriptObject );
		CallEvent( event );
	} else if ( etype == SDL_JOYDEVICEREMOVED ) {
		JoystickMap::iterator it = this->joysticks.find( e.jdevice.which );
		if ( it == this->joysticks.end() ) return;
		joy = it->second;
		this->joysticks.erase( it );
		event.name = EVENT_CONTROLLERREMOVED;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( joy->scriptObject );
		CallEvent( event );
	} else if ( etype == SDL_JOYBUTTONDOWN ) {
		JoystickMap::iterator it = joysticks.find( e.jbutton.which );
		if ( it == joysticks.end() ) return;
		joy = it->second;
		event.name = EVENT_JOYDOWN;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.jbutton.button );
		event.scriptParams.AddObjectArgument( joy ? joy->scriptObject : NULL );
		CallEvent( event );
	} else if ( etype == SDL_JOYBUTTONUP ) {
		JoystickMap::iterator it = joysticks.find( e.jbutton.which );
		if ( it == joysticks.end() ) return;
		joy = it->second;
		event.name = EVENT_JOYUP;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.jbutton.button );
		event.scriptParams.AddObjectArgument( joy ? joy->scriptObject : NULL );
		CallEvent( event );
	} else if ( etype == SDL_JOYAXISMOTION ) {
		JoystickMap::iterator it = joysticks.find( e.jaxis.which );
		if ( it == joysticks.end() ) return;
		joy = it->second;
		event.name = EVENT_JOYAXIS;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		Sint16 v = e.jaxis.value;
		float val = v ? ( v < 0 ? ( (float) v / 32768.0f ) : ( (float) v / 32767.0f ) ) : 0;
		event.scriptParams.AddFloatArgument( val );
		event.scriptParams.AddIntArgument( e.jaxis.axis );
		event.scriptParams.AddObjectArgument( joy ? joy->scriptObject : NULL );
		CallEvent( event );
	} else if ( etype == SDL_JOYHATMOTION ) {
		JoystickMap::iterator it = joysticks.find( e.jhat.which );
		if ( it == joysticks.end() ) return;
		joy = it->second;
		event.name = EVENT_JOYHAT;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		Uint8 v = e.jhat.value;
		float x = ( v & SDL_HAT_LEFT ) ? -1 : ( ( v & SDL_HAT_RIGHT ) ? 1 : 0 );
		float y = ( v & SDL_HAT_UP ) ? -1 : ( ( v & SDL_HAT_DOWN ) ? 1 : 0 );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		event.scriptParams.AddIntArgument( e.jhat.hat );
		event.scriptParams.AddObjectArgument( joy ? joy->scriptObject : NULL );
		CallEvent( event );
	}
	
	// if controller was found, pass the event
	if ( joy && !event.stopped ) {
		joy->HandleEvent( e );
	}
	
}

/* MARK:	-				UI dispatchers
 -------------------------------------------------------------------- */


void Input::UIEvent( Event &event, bool blockable ) {
	if ( !app.sceneStack.size() || event.stopped ) return;
	Scene* scene = app.sceneStack.back();
	event.bubbles = true;
	event.isBlockableUIEvent = blockable;
	event.behaviorsOnly = true;
	event.behaviorParam = &event;
	scene->DispatchEvent( event );
}


/* MARK:	-				Get joystick state helpers
 -------------------------------------------------------------------- */


bool Input::IsJoystickButtonDown( Controller *joy, int btnIndex ) {
	// specific button
	if ( btnIndex >= 0 ) {
		return (bool) SDL_JoystickGetButton( joy->joystick, btnIndex );
	// any button
	} else {
		for ( int i = 0; i < joy->numButtons; i++ ){
			if ( SDL_JoystickGetButton( joy->joystick, i ) ) return true;
		}
	}
	return false;
}

float Input::GetJoystickAxis( Controller* joy, int axis ) {

	Sint16 val = 0;
	
	// specific axis
	if ( axis >= 0 ) {
		val = SDL_JoystickGetAxis( joy->joystick, axis );
	// any axis
	} else {
		for ( int i = 0; i < joy->numAxis; i++ ){
			val = SDL_JoystickGetAxis( joy->joystick, i );
			if ( val != 0 ) break;
		}
	}
	
	// return dir
	return val ? ( val < 0 ? ( (float) val / 32768.0f ) : ( (float) val / 32767.0f ) ) : 0;
	
}

float Input::GetJoystickHat( Controller* joy, int axis, int hat ) {
	
	Uint8 val = 0;
	
	// specific hat
	if ( hat >= 0 ) {
		val = SDL_JoystickGetHat( joy->joystick, hat );
	// any hat
	} else {
		for ( int i = 0; i < joy->numHats; i++ ){
			val = SDL_JoystickGetHat( joy->joystick, i );
			if ( val != 0 ) break;
		}
	}
	
	// return dir
	return ( val & ( axis ? ( SDL_HAT_UP | SDL_HAT_DOWN ) : ( SDL_HAT_LEFT | SDL_HAT_RIGHT ) ) );
	
}






