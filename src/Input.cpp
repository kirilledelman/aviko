#include "Input.hpp"
#include "Application.hpp"
#include "Controller.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


Input::Input() {
	
	SDL_StartTextInput();
	
}

Input::Input( ScriptArguments* ) { script.ReportError( "Input can't be created using 'new'. Only one instance is available as global 'app.input' property." ); }

Input::~Input() {
	SDL_StopTextInput();
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
	script.RegisterClass<Input>( "ScriptableObject", true );
	
	// key constants
	script.SetGlobalConstant( "KEY_A", SDL_SCANCODE_A );
	script.SetGlobalConstant( "KEY_B", SDL_SCANCODE_B );
	script.SetGlobalConstant( "KEY_C", SDL_SCANCODE_C );
	script.SetGlobalConstant( "KEY_D", SDL_SCANCODE_D );
	script.SetGlobalConstant( "KEY_E", SDL_SCANCODE_E );
	script.SetGlobalConstant( "KEY_F", SDL_SCANCODE_F );
	script.SetGlobalConstant( "KEY_G", SDL_SCANCODE_G );
	script.SetGlobalConstant( "KEY_H", SDL_SCANCODE_H );
	script.SetGlobalConstant( "KEY_I", SDL_SCANCODE_I );
	script.SetGlobalConstant( "KEY_J", SDL_SCANCODE_J );
	script.SetGlobalConstant( "KEY_K", SDL_SCANCODE_K );
	script.SetGlobalConstant( "KEY_L", SDL_SCANCODE_L );
	script.SetGlobalConstant( "KEY_M", SDL_SCANCODE_M );
	script.SetGlobalConstant( "KEY_N", SDL_SCANCODE_N );
	script.SetGlobalConstant( "KEY_O", SDL_SCANCODE_O );
	script.SetGlobalConstant( "KEY_P", SDL_SCANCODE_P );
	script.SetGlobalConstant( "KEY_Q", SDL_SCANCODE_Q );
	script.SetGlobalConstant( "KEY_R", SDL_SCANCODE_R );
	script.SetGlobalConstant( "KEY_S", SDL_SCANCODE_S );
	script.SetGlobalConstant( "KEY_T", SDL_SCANCODE_T );
	script.SetGlobalConstant( "KEY_U", SDL_SCANCODE_U );
	script.SetGlobalConstant( "KEY_V", SDL_SCANCODE_V );
	script.SetGlobalConstant( "KEY_W", SDL_SCANCODE_W );
	script.SetGlobalConstant( "KEY_X", SDL_SCANCODE_X );
	script.SetGlobalConstant( "KEY_Y", SDL_SCANCODE_Y );
	script.SetGlobalConstant( "KEY_Z", SDL_SCANCODE_Z );
	script.SetGlobalConstant( "KEY_1", SDL_SCANCODE_1 );
	script.SetGlobalConstant( "KEY_2", SDL_SCANCODE_2 );
	script.SetGlobalConstant( "KEY_3", SDL_SCANCODE_3 );
	script.SetGlobalConstant( "KEY_4", SDL_SCANCODE_4 );
	script.SetGlobalConstant( "KEY_5", SDL_SCANCODE_5 );
	script.SetGlobalConstant( "KEY_6", SDL_SCANCODE_6 );
	script.SetGlobalConstant( "KEY_7", SDL_SCANCODE_7 );
	script.SetGlobalConstant( "KEY_8", SDL_SCANCODE_8 );
	script.SetGlobalConstant( "KEY_9", SDL_SCANCODE_9 );
	script.SetGlobalConstant( "KEY_0", SDL_SCANCODE_0 );
	script.SetGlobalConstant( "KEY_RETURN", SDL_SCANCODE_RETURN );
	script.SetGlobalConstant( "KEY_ENTER", SDL_SCANCODE_RETURN );
	script.SetGlobalConstant( "KEY_ESCAPE", SDL_SCANCODE_ESCAPE );
	script.SetGlobalConstant( "KEY_BACKSPACE", SDL_SCANCODE_BACKSPACE );
	script.SetGlobalConstant( "KEY_TAB", SDL_SCANCODE_TAB );
	script.SetGlobalConstant( "KEY_SPACE", SDL_SCANCODE_SPACE );
	script.SetGlobalConstant( "KEY_MINUS", SDL_SCANCODE_MINUS );
	script.SetGlobalConstant( "KEY_EQUALS", SDL_SCANCODE_EQUALS );
	script.SetGlobalConstant( "KEY_LEFTBRACKET", SDL_SCANCODE_LEFTBRACKET );
	script.SetGlobalConstant( "KEY_RIGHTBRACKET", SDL_SCANCODE_RIGHTBRACKET );
	script.SetGlobalConstant( "KEY_BACKSLASH", SDL_SCANCODE_BACKSLASH );
	script.SetGlobalConstant( "KEY_NONUSHASH", SDL_SCANCODE_NONUSHASH );
	script.SetGlobalConstant( "KEY_SEMICOLON", SDL_SCANCODE_SEMICOLON );
	script.SetGlobalConstant( "KEY_APOSTROPHE", SDL_SCANCODE_APOSTROPHE );
	script.SetGlobalConstant( "KEY_GRAVE", SDL_SCANCODE_GRAVE );
	script.SetGlobalConstant( "KEY_COMMA", SDL_SCANCODE_COMMA );
	script.SetGlobalConstant( "KEY_PERIOD", SDL_SCANCODE_PERIOD );
	script.SetGlobalConstant( "KEY_SLASH", SDL_SCANCODE_SLASH );
	script.SetGlobalConstant( "KEY_CAPSLOCK", SDL_SCANCODE_CAPSLOCK );
	script.SetGlobalConstant( "KEY_F1", SDL_SCANCODE_F1 );
	script.SetGlobalConstant( "KEY_F2", SDL_SCANCODE_F2 );
	script.SetGlobalConstant( "KEY_F3", SDL_SCANCODE_F3 );
	script.SetGlobalConstant( "KEY_F4", SDL_SCANCODE_F4 );
	script.SetGlobalConstant( "KEY_F5", SDL_SCANCODE_F5 );
	script.SetGlobalConstant( "KEY_F6", SDL_SCANCODE_F6 );
	script.SetGlobalConstant( "KEY_F7", SDL_SCANCODE_F7 );
	script.SetGlobalConstant( "KEY_F8", SDL_SCANCODE_F8 );
	script.SetGlobalConstant( "KEY_F9", SDL_SCANCODE_F9 );
	script.SetGlobalConstant( "KEY_F10", SDL_SCANCODE_F10 );
	script.SetGlobalConstant( "KEY_F11", SDL_SCANCODE_F11 );
	script.SetGlobalConstant( "KEY_F12", SDL_SCANCODE_F12 );
	script.SetGlobalConstant( "KEY_PRINTSCREEN", SDL_SCANCODE_PRINTSCREEN );
	script.SetGlobalConstant( "KEY_SCROLLLOCK", SDL_SCANCODE_SCROLLLOCK );
	script.SetGlobalConstant( "KEY_PAUSE", SDL_SCANCODE_PAUSE );
	script.SetGlobalConstant( "KEY_INSERT", SDL_SCANCODE_INSERT );
	script.SetGlobalConstant( "KEY_HOME", SDL_SCANCODE_HOME );
	script.SetGlobalConstant( "KEY_PAGEUP", SDL_SCANCODE_PAGEUP );
	script.SetGlobalConstant( "KEY_DELETE", SDL_SCANCODE_DELETE );
	script.SetGlobalConstant( "KEY_END", SDL_SCANCODE_END );
	script.SetGlobalConstant( "KEY_PAGEDOWN", SDL_SCANCODE_PAGEDOWN );
	script.SetGlobalConstant( "KEY_RIGHT", SDL_SCANCODE_RIGHT );
	script.SetGlobalConstant( "KEY_LEFT", SDL_SCANCODE_LEFT );
	script.SetGlobalConstant( "KEY_DOWN", SDL_SCANCODE_DOWN );
	script.SetGlobalConstant( "KEY_UP", SDL_SCANCODE_UP );
	script.SetGlobalConstant( "KEY_NUMLOCKCLEAR", SDL_SCANCODE_NUMLOCKCLEAR );
	script.SetGlobalConstant( "KEY_KP_DIVIDE", SDL_SCANCODE_KP_DIVIDE );
	script.SetGlobalConstant( "KEY_KP_MULTIPLY", SDL_SCANCODE_KP_MULTIPLY );
	script.SetGlobalConstant( "KEY_KP_MINUS", SDL_SCANCODE_KP_MINUS );
	script.SetGlobalConstant( "KEY_KP_PLUS", SDL_SCANCODE_KP_PLUS );
	script.SetGlobalConstant( "KEY_KP_ENTER", SDL_SCANCODE_KP_ENTER );
	script.SetGlobalConstant( "KEY_KP_1", SDL_SCANCODE_KP_1 );
	script.SetGlobalConstant( "KEY_KP_2", SDL_SCANCODE_KP_2 );
	script.SetGlobalConstant( "KEY_KP_3", SDL_SCANCODE_KP_3 );
	script.SetGlobalConstant( "KEY_KP_4", SDL_SCANCODE_KP_4 );
	script.SetGlobalConstant( "KEY_KP_5", SDL_SCANCODE_KP_5 );
	script.SetGlobalConstant( "KEY_KP_6", SDL_SCANCODE_KP_6 );
	script.SetGlobalConstant( "KEY_KP_7", SDL_SCANCODE_KP_7 );
	script.SetGlobalConstant( "KEY_KP_8", SDL_SCANCODE_KP_8 );
	script.SetGlobalConstant( "KEY_KP_9", SDL_SCANCODE_KP_9 );
	script.SetGlobalConstant( "KEY_KP_0", SDL_SCANCODE_KP_0 );
	script.SetGlobalConstant( "KEY_KP_PERIOD", SDL_SCANCODE_KP_PERIOD );
	script.SetGlobalConstant( "KEY_APPLICATION", SDL_SCANCODE_APPLICATION );
	script.SetGlobalConstant( "KEY_KP_EQUALS", SDL_SCANCODE_KP_EQUALS );
	script.SetGlobalConstant( "KEY_F13", SDL_SCANCODE_F13 );
	script.SetGlobalConstant( "KEY_F14", SDL_SCANCODE_F14 );
	script.SetGlobalConstant( "KEY_F15", SDL_SCANCODE_F15 );
	script.SetGlobalConstant( "KEY_F16", SDL_SCANCODE_F16 );
	script.SetGlobalConstant( "KEY_F17", SDL_SCANCODE_F17 );
	script.SetGlobalConstant( "KEY_F18", SDL_SCANCODE_F18 );
	script.SetGlobalConstant( "KEY_F19", SDL_SCANCODE_F19 );
	script.SetGlobalConstant( "KEY_F20", SDL_SCANCODE_F20 );
	script.SetGlobalConstant( "KEY_F21", SDL_SCANCODE_F21 );
	script.SetGlobalConstant( "KEY_F22", SDL_SCANCODE_F22 );
	script.SetGlobalConstant( "KEY_F23", SDL_SCANCODE_F23 );
	script.SetGlobalConstant( "KEY_F24", SDL_SCANCODE_F24 );
	script.SetGlobalConstant( "KEY_KP_COMMA", SDL_SCANCODE_KP_COMMA );
	script.SetGlobalConstant( "KEY_RETURN2", SDL_SCANCODE_RETURN2 );
	script.SetGlobalConstant( "KEY_LCTRL", SDL_SCANCODE_LCTRL );
	script.SetGlobalConstant( "KEY_LSHIFT", SDL_SCANCODE_LSHIFT );
	script.SetGlobalConstant( "KEY_LALT", SDL_SCANCODE_LALT );
	script.SetGlobalConstant( "KEY_LGUI", SDL_SCANCODE_LGUI );
	script.SetGlobalConstant( "KEY_RCTRL", SDL_SCANCODE_RCTRL );
	script.SetGlobalConstant( "KEY_RSHIFT", SDL_SCANCODE_RSHIFT );
	script.SetGlobalConstant( "KEY_RALT", SDL_SCANCODE_RALT );
	script.SetGlobalConstant( "KEY_RGUI", SDL_SCANCODE_RGUI );
	script.SetGlobalConstant( "KEY_RETURN", SDL_SCANCODE_RETURN );
	script.SetGlobalConstant( "KEY_SPACE", SDL_SCANCODE_SPACE );
	
	script.SetGlobalConstant( "MOUSE_BUTTON", KEY_MOUSE_BUTTON );
	script.SetGlobalConstant( "MOUSE_WHEEL", KEY_MOUSE_WHEEL );
	script.SetGlobalConstant( "MOUSE_WHEEL_X", KEY_MOUSE_WHEEL_X );
	script.SetGlobalConstant( "JOY_BUTTON", KEY_JOY_BUTTON );
	script.SetGlobalConstant( "JOY_AXIS", KEY_JOY_AXIS );
	script.SetGlobalConstant( "JOY_HAT_X", KEY_JOY_HAT_X );
	script.SetGlobalConstant( "JOY_HAT_Y", KEY_JOY_HAT_Y );
	
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
	 static_cast<ScriptBoolCallback>([](void* inp, bool val){ SDL_ShowCursor( val ); return ((Input*) inp)->repeatKeyEnabled = val; }));
	
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
	script.AddGlobalNamedObject( "input", this->scriptObject );

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
}


/* MARK:	-				Input event
 -------------------------------------------------------------------- */


void Input::HandleEvent( SDL_Event& e ) {
	
	Uint32 etype = e.type;
	Event event;
	event.bubbles = true;
	if ( etype == SDL_KEYDOWN && ( repeatKeyEnabled || e.key.repeat == 0 ) ) {
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
		event.name = EVENT_KEYPRESS;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddStringArgument( e.text.text );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_MOUSEBUTTONDOWN ) {
		event.name = EVENT_MOUSEDOWN;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.button.button );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_MOUSEBUTTONUP ) {
		event.name = EVENT_MOUSEUP;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.button.button );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_MOUSEMOTION ) {
		event.name = EVENT_MOUSEMOVE;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddFloatArgument( ( e.motion.x - app.backScreenDstRect.x ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( ( e.motion.y - app.backScreenDstRect.y ) * app.backscreenScale );
		event.scriptParams.AddFloatArgument( e.motion.xrel * app.backscreenScale );
		event.scriptParams.AddFloatArgument( e.motion.yrel * app.backscreenScale );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_MOUSEWHEEL ) {
		event.name = EVENT_MOUSEWHEEL;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddFloatArgument( e.wheel.y );
		event.scriptParams.AddFloatArgument( e.wheel.x );
		CallEvent( event );
		UIEvent( event );
	} else if ( etype == SDL_JOYDEVICEADDED ) {
		// add to joysticks
		SDL_Joystick* jck = SDL_JoystickOpen( e.jdevice.which );
		SDL_JoystickID jid = SDL_JoystickInstanceID( jck );
		Controller* joy = new Controller( jck );
		this->joysticks[ jid ] = joy;
		// fire event
		event.name = EVENT_CONTROLLERADDED;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( joy->scriptObject );
		CallEvent( event );
	} else if ( etype == SDL_JOYDEVICEREMOVED ) {
		JoystickMap::iterator it = this->joysticks.find( e.jdevice.which );
		Controller* joy = it->second;
		this->joysticks.erase( it );
		event.name = EVENT_CONTROLLERREMOVED;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( joy->scriptObject );
		CallEvent( event );
	} else if ( etype == SDL_JOYBUTTONDOWN ) {
		event.name = EVENT_JOYDOWN;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.jbutton.button );
		event.scriptParams.AddIntArgument( e.jbutton.which );
		CallEvent( event );
	} else if ( etype == SDL_JOYBUTTONUP ) {
		event.name = EVENT_JOYUP;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddIntArgument( e.jbutton.button );
		event.scriptParams.AddIntArgument( e.jbutton.which );
		CallEvent( event );
	} else if ( etype == SDL_JOYAXISMOTION ) {
		event.name = EVENT_JOYAXIS;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		Sint16 v = e.jaxis.value;
		float val = v ? ( v < 0 ? ( (float) v / 32768.0f ) : ( (float) v / 32767.0f ) ) : 0;
		event.scriptParams.AddFloatArgument( val );
		event.scriptParams.AddIntArgument( e.jaxis.axis );
		event.scriptParams.AddIntArgument( e.jaxis.which );
		CallEvent( event );
	} else if ( etype == SDL_JOYHATMOTION ) {
		event.name = EVENT_JOYHAT;
		event.behaviorParam = &e;
		event.scriptParams.ResizeArguments( 0 );
		Uint8 v = e.jhat.value;
		float x = ( v & SDL_HAT_LEFT ) ? -1 : ( ( v & SDL_HAT_RIGHT ) ? 1 : 0 );
		float y = ( v & SDL_HAT_UP ) ? -1 : ( ( v & SDL_HAT_DOWN ) ? 1 : 0 );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		event.scriptParams.AddIntArgument( e.jhat.hat );
		event.scriptParams.AddIntArgument( e.jhat.which );
		CallEvent( event );
	}
	
	// pass to controllers
	for ( JoystickMap::iterator it = this->joysticks.begin(), end = this->joysticks.end(); it != end; it++ ) {
		(it->second)->HandleEvent( e );
	}
	
}

/* MARK:	-				UI dispatchers
 -------------------------------------------------------------------- */


void Input::UIEvent( Event &event ) {
	if ( !app.sceneStack.size() ) return;
	Scene* scene = app.sceneStack.back();
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






