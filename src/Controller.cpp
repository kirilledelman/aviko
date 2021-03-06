#include "Controller.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

Controller::Controller( SDL_Joystick* joy ){

	static char buf[33];
	joystick = joy;
	if ( joystick ) {
		id = SDL_JoystickInstanceID( joystick );
		guid = SDL_JoystickGetGUID( joystick );
		SDL_JoystickGetGUIDString( guid, buf, 32 );
		guidString = buf;
		name = SDL_JoystickName( joystick );
		numButtons = SDL_JoystickNumButtons( joystick );
		numHats = SDL_JoystickNumHats( joystick );
		numAxis = SDL_JoystickNumAxes( joystick );
	} else {
		name = "Keyboard";
		memset( guid.data, 0, sizeof( guid.data ) );
	}

	// add scriptObject
	script.NewScriptObject<Controller>( this );

	// attempt to load json
	this->LoadConfig();
	
}

Controller::Controller( ScriptArguments* sa ){
	
	// add scriptObject, because this class isn't singleton / needs to be serializable
	script.NewScriptObject<Controller>( this );
	script.ReportError( "Controller can't be created using 'new'. app.input creates controllers automatically when they're detected." );

};

Controller::~Controller(){}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void Controller::InitClass() {
	
	// register class
	script.RegisterClass<Controller>( NULL, false );

	// props
	
	script.AddProperty<Controller>
	("id",
	 static_cast<ScriptIntCallback>([](void* p, int ){ return (int) ((Controller*)p)->id; }),
	PROP_ENUMERABLE);
	
	script.AddProperty<Controller>
	("name",
	 static_cast<ScriptStringCallback>([](void* p, string ){ return ((Controller*)p)->name; }),
	 PROP_ENUMERABLE);

	script.AddProperty<Controller>
	("guid",
	 static_cast<ScriptStringCallback>([](void* p, string ){ return ((Controller*)p)->guidString; }),
	 PROP_ENUMERABLE);
	
	script.AddProperty<Controller>
	("numButtons",
	 static_cast<ScriptIntCallback>([](void* p, int ){ return ((Controller*)p)->numButtons; }),
	 PROP_ENUMERABLE);

	script.AddProperty<Controller>
	("numAxis",
	 static_cast<ScriptIntCallback>([](void* p, int ){ return ((Controller*)p)->numAxis; }),
	 PROP_ENUMERABLE);
	
	script.AddProperty<Controller>
	("numHats",
	 static_cast<ScriptIntCallback>([](void* p, int ){ return ((Controller*)p)->numHats; }),
	 PROP_ENUMERABLE);
	
	// set/clear bindings
	script.AddProperty<Controller>
	( "bindings",
	 static_cast<ScriptObjectCallback>([](void *p, void* in ) {
		Controller* self = (Controller*) p;
		
		// if bindings are empty, return null
		if ( !self->bindings.size() ) return (void*) NULL;
		
		// construct an object
		ArgValue obj( script.NewObject() );
		
		// for each binding key
		BindMapIterator it = self->bindings.begin(), end = self->bindings.end();
		while( it != end ) {
			// make array
			ArgValue b;
			b.type = TypeArray;
			b.value.arrayValue = new ArgValueVector();
			vector<Binding> *bindVec = &it->second;
			size_t nc = bindVec->size();
			b.value.arrayValue->resize( nc );
			// fill array with bindings for that key
			for ( size_t i = 0; i < nc; i++ ) {
				const Binding &bnd = (*bindVec)[ i ];
				ArgValue &val = (*b.value.arrayValue)[ i ];
				val.type = TypeArray;
				val.value.arrayValue = new ArgValueVector();
				// action, type, index
				val.value.arrayValue->emplace_back( bnd.action.c_str() );
				val.value.arrayValue->emplace_back( (int) bnd.type );
				val.value.arrayValue->emplace_back( (int) bnd.index );
			}
			// set obj[ _key ] = array
			static char key[16];
			sprintf( key, "_%d", it->first );
			script.SetProperty( key, b, obj.value.objectValue );
			it++;
		}

		return obj.value.objectValue;
	}),
	 static_cast<ScriptObjectCallback>([](void *p, void* in ){
		Controller* self = (Controller*) p;
		
		// reset bindings
		self->bindings.clear();
		
		// if empty, return
		if ( !in ) return (void*) NULL;
		
		// for each "_key" in passed object
		ArgValueVector keys;
		script.GetProperties( in, &keys, false, false, false );
		ArgValueVector::iterator it = keys.begin(), end = keys.end();
		while( it != end ) {
			// expect array
			const char *skey = it->value.stringValue->c_str();
			if ( skey[ 0 ] != '_' || skey[ 1 ] < '0' || skey[ 1 ] > '9' ) { it++; continue; }
			ArgValue vec = script.GetProperty( skey, in );
			if ( vec.type != TypeArray ) continue;
			
			// convert string "_key" -> int
			int key = -1;
			key = SDL_atoi( skey + 1 );
			
			// each binding in array
			for ( size_t i = 0, nb = vec.value.arrayValue->size(); i < nb; i++ ) {
				ArgValue &bnd = (*vec.value.arrayValue)[ i ];
				if ( bnd.type != TypeArray || bnd.value.arrayValue->size() != 3 ) continue;
				
				// unpack into Binding struct
				ArgValueVector &els = *bnd.value.arrayValue;
				Binding b;
				b.action = *els[ 0 ].value.stringValue;
				b.type = (ActionType) els[ 1 ].value.intValue;
				b.index = els[ 2 ].value.intValue;
				
				// add to bindings
				self->bindings[ key ].push_back( b );
				self->states[ b.action ] = 0;
			}
			it++;
		}
		
		return in;
	}),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );
	
	// axis deadzone
	
	script.AddProperty<Controller>
	("deadZone",
	 static_cast<ScriptFloatCallback>([](void* p, float ){ return ((Controller*)p)->deadZone; }),
	 static_cast<ScriptFloatCallback>([](void* p, float val ){ return ((Controller*)p)->deadZone = min( 1.0f, max( 0.0f, val )); }));
	
	// load / save
	
	script.DefineFunction<Controller>
	("save",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		((Controller*) p)->SaveConfig();
		return true;
	 }));
	
	script.DefineFunction<Controller>
	("load",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		sa.ReturnBool( ((Controller*) p)->LoadConfig() );
		return true;
	}));
	
	// bindings
	
	script.DefineFunction<Controller>
	("reset",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		Controller* self = (Controller*) p;
		// optionally delete config
		bool deleteConfig = false;
		sa.ReadArguments( 1, TypeBool, &deleteConfig );
		if ( deleteConfig ) self->DeleteConfig();
		// reset bindings
		self->bindings.clear();
		return true;
	}));
	
	script.DefineFunction<Controller>
	("get",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		string action;
		// read args
		if ( !sa.ReadArguments( 1, TypeString, &action ) || !action.length() ) {
			script.ReportError( "usage: get( String action )" );
			return false;
		}
		//
		Controller* self = (Controller*) p;
		ActionStates::iterator as = self->states.find( action );
		if ( as != self->states.end() ) {
			sa.ReturnInt( as->second );
		}
		return true;
	}));
	
	// e.g:
	// bind( 'fire', KEY_SPACE )
	// bind( 'fire', MOUSE_BUTTON, 1 )
	// bind( 'fire', JOY_BUTTON, 1 )
	script.DefineFunction<Controller>
	("bind",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		
		string action;
		int key = -1, index = -999;
		
		// read args
		if ( !sa.ReadArguments( 2, TypeString, &action, TypeInt, &key, TypeInt, &index ) || !action.length() ) {
			script.ReportError( "usage: bind( String action, Int key[, Int index ] )" );
			return false;
		}
		
		// non-keyboard
		if ( ( key == KEY_MOUSE_BUTTON || key == KEY_JOY_BUTTON ) && index < 0 ) {
			script.ReportError( "bind: MOUSE_BUTTON, and JOY_BUTTON require a third parameter for button index" );
			return false;
		}
		
		// axis
		if ( key == KEY_JOY_AXIS || key == KEY_JOY_HAT_X || key == KEY_JOY_HAT_Y ) {
			script.ReportError( "bind: can't bind a momentary action to JOY_AXIS, or JOY_HAT_" );
			return false;
		}
		
		// wheel
		if ( key == KEY_MOUSE_WHEEL || key == KEY_MOUSE_WHEEL_X ) {
			script.ReportError( "bind: can't bind a momentary action to MOUSE_WHEEL" );
			return false;
		}
		
		// bind
		Controller* self = (Controller*) p;
		Binding b;
		b.action = action;
		b.type = BUTTON;
		b.index = index;
		self->bindings[ key ].push_back( b );
		self->states[ action ] = 0;
		
		return true;
	}));
	
	// e.g:
	// bindAxis( '-horizontal', KEY_LEFT )
	// bindAxis( '+horizontal', MOUSE_BUTTON, 1 )
	// bindAxis( '+horizontal', JOY_BUTTON, 1 )
	// bindAxis( 'horizontal', JOY_AXIS, 1 ) // normal
	// bindAxis( '-horizontal', JOY_AXIS, 1 ) // reversed
	// bindAxis( 'horizontal', JOY_HAT_X, 0 ) // normal
	// bindAxis( '-horizontal', JOY_HAT_X, 0 ) // reversed
	
	script.DefineFunction<Controller>
	("bindAxis",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa){
		
		string action;
		int key = -1, index = -999, axisDir = -999;
		
		// read args
		if ( !sa.ReadArguments( 2, TypeString, &action, TypeInt, &key, TypeInt, &index, TypeInt, &axisDir ) || action.length() < 2 ) {
			script.ReportError( "usage: bindAxis( String action, Int key[, Int index ] )" );
			return false;
		}
		
		ActionType type = DIR_POSITIVE;
		
		// non-axis, requires + or - in action name
		char firstChar = action.c_str()[ 0 ];
		if ( ( key != KEY_JOY_AXIS && key != KEY_JOY_HAT_X && key != KEY_JOY_HAT_Y && key != KEY_MOUSE_WHEEL && key != KEY_MOUSE_WHEEL_X ) && firstChar != '+' && firstChar != '-' ) {
			script.ReportError( "bindAxis: when binding axis to a key or a button, action must start with '+' or '-'" );
			return false;
		}
		
		// action starts with + or -
		if ( firstChar == '+' ){
			// strip, direction remains positive
			action = action.substr( 1 );
		} else if ( firstChar == '-' ){
			// reverse dir
			type = DIR_NEGATIVE;
			// strip
			action = action.substr( 1 );
		}
		
		// non-keyboard
		if ( key == KEY_MOUSE_BUTTON || key == KEY_JOY_AXIS || key == KEY_JOY_HAT_X || key == KEY_JOY_HAT_Y || key == KEY_JOY_BUTTON ) {
			if ( index < 0 ) {
				script.ReportError( "bindAxis: MOUSE_BUTTON, JOY_BUTTON, and JOY_AXIS require a third parameter for button, or axis index" );
				return false;
			}
		}
		
		// bind
		Controller* self = (Controller*) p;
		Binding b;
		b.action = action;
		b.type = type;
		b.index = index;
		self->bindings[ key ].push_back( b );
		self->states[ action ] = 0;
		
		return true;
	}));

	script.DefineFunction<Controller>
	( "toString",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		static char buf[512];
		Controller* self = (Controller*) o;
		
		if ( !self ) {
			sprintf( buf, "[Controller prototype]" );
		} else if ( self->name.size() ) {
			sprintf( buf, "[Controller \"%s\"]", self->name.c_str() );
		} else sprintf( buf, "[Controller %p]", self );
		
		sa.ReturnString( buf );
		return true;
	}));
	
}


/* MARK:	-				Load / save / reset
 -------------------------------------------------------------------- */

bool Controller::LoadConfig() {
	
	// read file
	string path = app.configDirectory + this->name;
	const char* file = ReadFile( path.c_str(), "json", NULL, NULL, NULL );
	if ( !file ) return false;
	
	// make obj
	ScriptArguments parseArgs;
	parseArgs.AddStringArgument( file );
	free( (void*) file );
	ArgValue obj( script.CallClassFunction( "JSON", "parse", parseArgs ) );
	if ( obj.type != TypeObject || !obj.value.objectValue ) return false;
	
	// init self
	script.CopyProperties( obj.value.objectValue, this->scriptObject );
	
	return true;
}

bool Controller::SaveConfig() {
	
	// convert to initObject
	ArgValue ret( this->scriptObject );
	
	// call global stringify function
	ScriptArguments parseArgs;
	parseArgs.AddObjectArgument( ret.value.objectValue );
	ArgValue str = script.CallGlobalFunction( "stringify", parseArgs );
	if ( str.type != TypeString ) return false;
	
	// save to file
	string path = app.configDirectory + this->name;
	bool overwrite = true;
	return SaveFile( str.value.stringValue->c_str(), str.value.stringValue->length(), path.c_str(), "json", &overwrite );
}

bool Controller::DeleteConfig() {

	string path = app.configDirectory + this->name + ".json";
	return DeleteFile( path.c_str() );
	
}

/* MARK:	-				Controller actions
 -------------------------------------------------------------------- */


void Controller::DispatchActions( int key, SDL_Event& e, vector<Binding>& actions ) {

	Event event;
	Uint32 etype = e.type;
	
	// for each binding
	for ( size_t i = 0, na = actions.size(); i < na; i++ ){
		
		// set event name to action
		Binding& b = actions[ i ];
		int state = this->states[ b.action ];
		int onState = ( b.type == BUTTON ? 1 : ( b.type == DIR_NEGATIVE ? -1 : 1 ) );
		int newState = 0;
		event.name = b.action.c_str();
		event.scriptParams.ResizeArguments( 0 );
		
		// joystick axis
		if ( etype == SDL_JOYAXISMOTION && e.jaxis.axis == b.index ) {
			
			newState = e.jaxis.value;
			float val = newState ? ( newState < 0 ? ( (float) newState / 32768.0f ) : ( (float) newState / 32767.0f ) ) : 0;
			if ( b.type == DIR_NEGATIVE ) val = -val;
			
			// adjust using dead zone
			if ( val < 0 ) {
				val = ( val < -this->deadZone ) ? ( ( val + this->deadZone ) / ( 1 - this->deadZone ) ) : 0;
			} else {
				val = ( val > this->deadZone ) ? ( ( val - this->deadZone ) / ( 1 - this->deadZone ) ) : 0;
			}
			
			// back to int
			newState = ( val * ( val >= 0 ? 32767 : 32768 ) );
			
			// ignore if value hasn't changed from previous state
			if ( this->states[ b.action ] == newState )  continue;

			// add argument
			event.scriptParams.AddStringArgument( b.action.c_str() );
			event.scriptParams.AddFloatArgument( val );
			
			// remember new state
			this->states[ b.action ] = newState;
			
		// hat
		} else if ( etype == SDL_JOYHATMOTION && e.jhat.hat == b.index ) {
			
			Uint8 v = e.jhat.value;
			newState = ( key == KEY_JOY_HAT_X ?
						 (( v & SDL_HAT_LEFT ) ? -1 : ( ( v & SDL_HAT_RIGHT ) ? 1 : 0 )) :
						 (( v & SDL_HAT_UP ) ? -1 : ( ( v & SDL_HAT_DOWN ) ? 1 : 0 )) );
			
			if ( b.type == DIR_NEGATIVE ) newState = -newState;
			
			// ignore if value hasn't changed from previous state
			if ( this->states[ b.action ] == newState )  continue;
			
			// add argument
			event.scriptParams.AddStringArgument( b.action.c_str() );
			event.scriptParams.AddIntArgument( newState );
			
			// remember new state
			this->states[ b.action ] = newState;

		// mouse wheel
		} else if ( etype == SDL_MOUSEWHEEL ) {
			
			newState = ( key == KEY_MOUSE_WHEEL ? e.wheel.y : e.wheel.x);
			if ( b.type == DIR_NEGATIVE ) newState = -newState;
			
			// add argument
			event.scriptParams.AddStringArgument( b.action.c_str() );
			event.scriptParams.AddIntArgument( newState );
			
			// dispatch now with value
			this->CallEvent( event );
			
			// switch to 0
			event.scriptParams.ResizeArguments( 1 );
			event.scriptParams.AddIntArgument( 0 );
			
		// button
		} else {
			
			// key
			if ( etype == SDL_KEYDOWN || etype == SDL_KEYUP ) {

				// new value
				newState = ( etype == SDL_KEYDOWN ? onState : 0 );
				
				// if it's a release, skip if state already changed
				if ( etype == SDL_KEYUP && state != onState ) continue;
				
			// mouse button
			} else if ( ( etype == SDL_MOUSEBUTTONDOWN || etype == SDL_MOUSEBUTTONUP ) && b.index == e.button.button ) {
				
				// new value
				newState = ( etype == SDL_MOUSEBUTTONDOWN ? onState : 0 );
				
				// if it's a release, skip if state already changed
				if ( etype == SDL_MOUSEBUTTONUP && state != onState ) continue;
				
		
			// joystick button
			} else if ( ( etype == SDL_JOYBUTTONDOWN || etype == SDL_JOYBUTTONUP ) && e.jbutton.which == id && b.index == e.jbutton.button ) {
			
				// new value
				newState = ( etype == SDL_JOYBUTTONDOWN ? onState : 0 );
				
				// if it's a release, skip if state already changed
				if ( etype == SDL_JOYBUTTONUP && state != onState ) continue;
				
			} else continue;
			
			// add param ( button down/up )
			event.scriptParams.AddStringArgument( b.action.c_str() );
			event.scriptParams.AddIntArgument( newState );
			
			// remember state
			this->states[ b.action ] = newState;

		}
		
		// dispatch
		this->CallEvent( event );
		
		// check if binding name matches any of UI navigation events
		if ( newState != 0 && ( b.action.compare( app.input.navigationXAxis ) == 0 ||
			b.action.compare( app.input.navigationYAxis ) == 0 ||
			b.action.compare( app.input.navigationAccept ) == 0 ||
			b.action.compare( app.input.navigationCancel ) == 0 ) ) {
			
			// add self as param
			event.scriptParams.AddObjectArgument( this->scriptObject );
			
			// dispatch as navigation event
			event.name = EVENT_NAVIGATION;
			app.input.UIEvent( event );
			
		}
		
	}
	
}

void Controller::HandleEvent( SDL_Event& e ) {
	
	// only bother if we have event listeners
	Uint32 etype = e.type;
	int key = -1;
	BindMapIterator it;
	if ( ( etype == SDL_KEYDOWN && e.key.repeat == 0 ) || etype == SDL_KEYUP ) {
		
		key = e.key.keysym.scancode;
		
	} else if ( etype == SDL_MOUSEBUTTONDOWN || etype == SDL_MOUSEBUTTONUP ) {
		
		key = KEY_MOUSE_BUTTON;
		
	} else if ( etype == SDL_JOYBUTTONDOWN || etype == SDL_JOYBUTTONUP ) {
		
		key = KEY_JOY_BUTTON;
		
	} else if ( etype == SDL_JOYAXISMOTION ) {
		
		key = KEY_JOY_AXIS;
		
	} else if ( etype == SDL_JOYHATMOTION ) {
		
		// do horizontal
		key = KEY_JOY_HAT_X;
		
		// dispatch
		it = this->bindings.find( key );
		if ( it != this->bindings.end() ) this->DispatchActions( key, e, it->second );
		
		// switch to vertical
		key = KEY_JOY_HAT_Y;

	} else if ( etype == SDL_MOUSEWHEEL ) {
		
		// do standard vertical
		key = KEY_MOUSE_WHEEL;
		
		// dispatch
		it = this->bindings.find( key );
		if ( it != this->bindings.end() ) this->DispatchActions( key, e, it->second );
		
		// switch to horizontal
		key = KEY_MOUSE_WHEEL_X;
		
	} else return;
	
	// dispatch
	it = this->bindings.find( key );
	if ( it != this->bindings.end() ) this->DispatchActions( key, e, it->second );
	
}


