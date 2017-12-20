#include "Tween.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// init
Tween::Tween( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<Tween>( this );
	
	// with arguments
	if ( args && args->args.size() ) {
		
		// required object
		if ( args->args[ 0 ].type != TypeObject || !args->args[ 0 ].value.objectValue ){
			script.ReportError( "Tween constructor: first parameter must be an Object." );
			return;
		} else {
			this->target = args->args[ 0 ].value.objectValue;
		}
		
		// param 1 is either a string or array
		if ( args->args.size() >= 2 && ( args->args[ 1 ].type == TypeArray || args->args[ 1 ].type == TypeString ) ) {
			this->SetProperties( args->args[ 1 ] );
		// callback function
		} else if ( args->args.size() >= 2 && args->args[ 1 ].type == TypeFunction ) {
			script.SetProperty( "callback", args->args[ 1 ], this->scriptObject );
			this->startValues.push_back( 0 );
			this->endValues.push_back( 1 );
		} else {
			script.ReportError( "Tween constructor: second parameter must be String, or Array of Strings, or callback Function." );
			return;
		}
		
		// param 2 - start value(s)
		if ( args->args.size() >= 3 ) {
			this->SetStartValues( args->args[ 2 ] );
		}
		
		// param 3 - end value(s)
		if ( args->args.size() >= 4 ) {
			this->SetEndValues( args->args[ 3 ] );
		}
		
		// duration
		if ( args->args.size() >= 5 ) {
			args->args[ 4 ].toNumber( this->duration );
		}
		
		// ease type
		if ( args->args.size() >= 6 ) {
			int e = EaseNone;
			args->args[ 5 ].toInt( e );
			this->easeType = (EasingType) e;
		}
		
		// ease func
		if ( args->args.size() >= 7 ) {
			int e = EaseNone;
			args->args[ 6 ].toInt( e );
			this->easeFunc = (EasingFunc) e;
		}
		
		// start, if valid
		this->active( true );
	}
	
}

/// destructor
Tween::~Tween() {
	
	// make sure to remove self from activeTweens
	unordered_set<Tween*>::iterator it = activeTweens->find( this );
	if ( it != activeTweens->end() ) activeTweens->erase( it );
	
}


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void Tween::InitClass() {
	
	// create class
	script.RegisterClass<Tween>( "Tween" );

	// constants
	script.SetGlobalConstant( "EASE_NONE", EaseNone );
	script.SetGlobalConstant( "EASE_IN", EaseIn );
	script.SetGlobalConstant( "EASE_OUT", EaseOut );
	script.SetGlobalConstant( "EASE_INOUT", EaseInOut );

	// http://easings.net/
	script.SetGlobalConstant( "EASE_LINEAR", EaseLinear );
	script.SetGlobalConstant( "EASE_SINE", EaseSine );
	script.SetGlobalConstant( "EASE_QUAD", EaseQuad );
	script.SetGlobalConstant( "EASE_CUBIC", EaseCubic );
	script.SetGlobalConstant( "EASE_QUART", EaseQuart );
	script.SetGlobalConstant( "EASE_QUINT", EaseQuint );
	script.SetGlobalConstant( "EASE_CIRC", EaseCirc );
	script.SetGlobalConstant( "EASE_EXPO", EaseExpo );
	script.SetGlobalConstant( "EASE_BACK", EaseBack );
	script.SetGlobalConstant( "EASE_ELASTIC", EaseElastic );
	script.SetGlobalConstant( "EASE_BOUNCE", EaseBounce );
	
	// properties
	
	// easing type
	script.AddProperty<Tween>
	("easeType",
	 static_cast<ScriptIntCallback>([]( void* p, int val ){ return (int)((Tween*)p)->easeType; }),
	 static_cast<ScriptIntCallback>([]( void* p, int val ){ ((Tween*)p)->easeType = (EasingType) val; return val; }));
	
	// easing function
	script.AddProperty<Tween>
	("easeFunc",
	 static_cast<ScriptIntCallback>([]( void* p, int val ){ return (int)((Tween*)p)->easeFunc; }),
	 static_cast<ScriptIntCallback>([]( void* p, int val ){ ((Tween*)p)->easeFunc = (EasingFunc) val; return val; }));
	
	// active (use to pause)
	script.AddProperty<Tween>
	("active",
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ){ return ((Tween*)p)->active(); }),
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ){ ((Tween*)p)->active( val ); return val; } ));
	
	// time
	script.AddProperty<Tween>
	("time",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*)p)->time; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*) p)->time = max( 0.0f, val ); }));
	
	// duration
	script.AddProperty<Tween>
	("duration",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*)p)->duration; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*) p)->duration = max( 0.0f, val ); }));

	script.AddProperty<Tween>
	("pos",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*)p)->time / ((Tween*)p)->duration; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ){ return ((Tween*) p)->time = max( 0.0f, min( 1.0f, val * ((Tween*)p)->duration ) ); }));

	// use unscaled time
	script.AddProperty<Tween>
	("unscaledTime",
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ){ return ((Tween*)p)->useUnscaledTime; }),
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ){ return ((Tween*)p)->useUnscaledTime = val; } ));
	
	// callback function
	script.AddProperty<Tween>
	("callback",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){ return ArgValue( ((Tween*)p)->callback.funcObject ); }),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		Tween* t = (Tween*) p;
		void* f = NULL;
		if ( val.type == TypeFunction ) {
			f = val.value.objectValue;
			t->callback.SetFunc( f );
			t->callback.thisObject = t->scriptObject;
		}
		return ArgValue( f );
	}), PROP_ENUMERABLE );
	
	//
	script.AddProperty<Tween>
	("properties",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetProperties();
	 }),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetProperties( val );
		return val;
	 }), PROP_NOSTORE | PROP_ENUMERABLE | PROP_SERIALIZED);
	
	script.AddProperty<Tween>
	("startValues",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetStartValues();
	}),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetStartValues( val );
		return val;
	}), PROP_NOSTORE | PROP_ENUMERABLE | PROP_SERIALIZED );
	
	script.AddProperty<Tween>
	("endValues",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetEndValues();
	}),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetEndValues( val );
		return val;
	}), PROP_NOSTORE | PROP_ENUMERABLE | PROP_SERIALIZED );

	// functions
	
	// stop
	script.DefineFunction<Tween>
	("stop",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		((Tween*)p)->active( false );
		return true;
	}));
	
	// start
	script.DefineFunction<Tween>
	("start",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		Tween* t = (Tween*)p;
		t->active( true );
		args.ReturnBool( t->active() );
		return true;
	}));
	
	// reverse
	script.DefineFunction<Tween>
	("reverse",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		((Tween*)p)->Reverse();
		return true;
	}));
	
	// cut off beginning
	script.DefineFunction<Tween>
	("cut",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		((Tween*)p)->Cut();
		return true;
	}));
	
	// static func
	
	// returns an array of all active tweens (optionally filtered by object)
	script.DefineClassFunction( "Tween", "getActive", true,
    static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		void *obj = NULL;
		args.ReadArguments( 1, TypeObject, &obj );
		ArgValueVector vec;
		unordered_set<Tween*>::iterator it = activeTweens->begin(), end = activeTweens->end();
		while( it != end ) {
			if ( !obj || (*it)->target == obj ) {
				vec.emplace_back( (*it)->scriptObject );
			}
			it++;
		}
		args.ReturnArray( vec );
		return true;
	}));
	
	script.DefineClassFunction( "Tween", "stopActive", true,
		static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& args){
		void *obj = NULL;
		args.ReadArguments( 1, TypeObject, &obj );
		unordered_set<Tween*>::iterator it = activeTweens->begin();
		while( it != activeTweens->end() ) {
			if ( !obj || (*it)->target == obj ) {
				Tween* t = (*it);
				it = activeTweens->erase( it );
				t->active( false );
			} else it++;
		}
		return true;
	}));
	
}

bool Tween::_canRun() {
	return this->target != NULL && duration > 0 &&
		( ( properties.size() && endValues.size() ) || ( callback.funcObject != NULL ) );
}


/* MARK:	-				Reverse and Split
 -------------------------------------------------------------------- */


void Tween::Reverse() {

	// reverse start and end values
	vector<float> tmp = endValues;
	endValues = startValues;
	startValues = tmp;
	
	// reverse time
	if ( time > 0 ) time = max( 0.0f, duration - time );
	
}

void Tween::Cut() {

	// truncate start values to current value
	float pos = min( this->time / this->duration, 1.0f );
	size_t np = this->properties.size();
	size_t ns = startValues.size();
	size_t ne = endValues.size();
	if ( ns < np ) startValues.resize( np );
	for ( size_t i = 0; i < np; i++ ){
		float fromVal = startValues[ i ];
		float toVal = ( i < ne ? endValues[ i ] : 0 );
		startValues[ i ] = fromVal + ( toVal - fromVal ) * Tween::Ease( easeType, easeFunc, pos );
	}
	
	// clip time
	duration -= time;
	time = 0;
	
}

/* MARK:	-				Easing
 -------------------------------------------------------------------- */


float _bounceEaseOut(float p) {
	if(p < 4/11.0) {
		return (121 * p * p)/16.0;
	} else if(p < 8/11.0) {
		return (363/40.0 * p * p) - (99/10.0 * p) + 17/5.0;
	} else if(p < 9/10.0) {
		return (4356/361.0 * p * p) - (35442/1805.0 * p) + 16061/1805.0;
	} else {
		return (54/5.0 * p * p) - (513/25.0 * p) + 268/25.0;
	}
}

float Tween::Ease( Tween::EasingType type, Tween::EasingFunc func, float p ) {
	
	// no easing = linear
	if ( type == EaseNone || func == EaseLinear ) return p;
	
	// EASE IN
	if ( type == EaseIn ) {
		switch ( func ) {
			case EaseSine: return sin((p - 1) * M_PI_2) + 1;
			case EaseQuad: return p * p;
			case EaseCubic: return p * p * p;
			case EaseQuart: return p * p * p * p;
			case EaseQuint: return p * p * p * p * p;
			case EaseCirc: return 1 - sqrt(1 - (p * p));
			case EaseExpo: return (p == 0.0) ? p : pow(2, 10 * (p - 1));
			case EaseBack: return p * p * p - p * sin(p * M_PI);
			case EaseElastic: return sin(13 * M_PI_2 * p) * pow(2, 10 * (p - 1));
			default: return 1 - _bounceEaseOut(1 - p);
		}
	// EASE OUT
	} else if ( type == EaseOut ) {
		float f = 0;
		switch ( func ) {
			case EaseSine: return sin(p * M_PI_2);
			case EaseQuad: return -(p * (p - 2));
			case EaseCubic: f = (p - 1); return f * f * f + 1;
			case EaseQuart: f = (p - 1); return f * f * f * (1 - p) + 1;
			case EaseQuint: f = (p - 1); return f * f * f * f * f + 1;
			case EaseCirc: return sqrt((2 - p) * p);
			case EaseExpo: return (p == 1.0) ? p : 1 - pow(2, -10 * p);
			case EaseBack: f = (1 - p); return 1 - (f * f * f - f * sin(f * M_PI));
			case EaseElastic: return sin(-13 * M_PI_2 * (p + 1)) * pow(2, -10 * p) + 1;
			default:
				if(p < 4/11.0) {
					return (121 * p * p)/16.0;
				} else if(p < 8/11.0) {
					return (363/40.0 * p * p) - (99/10.0 * p) + 17/5.0;
				} else if(p < 9/10.0) {
					return (4356/361.0 * p * p) - (35442/1805.0 * p) + 16061/1805.0;
				}
				return (54/5.0 * p * p) - (513/25.0 * p) + 268/25.0;
		}
		
	// EASE IN OUT
	} else {
		float f = 0;
		switch ( func ) {
			case EaseSine: return 0.5 * (1 - cos(p * M_PI));
			case EaseQuad:
				if(p < 0.5) {
					return 2 * p * p;
				}
				return (-2 * p * p) + (4 * p) - 1;
			case EaseCubic:
				if(p < 0.5) {
					return 4 * p * p * p;
				}
				f = ((2 * p) - 2);
				return 0.5 * f * f * f + 1;
			case EaseQuart:
				if(p < 0.5) {
					return 8 * p * p * p * p;
				}
				f = (p - 1);
				return -8 * f * f * f * f + 1;
			case EaseQuint:
				if(p < 0.5) {
					return 16 * p * p * p * p * p;
				}
				f = ((2 * p) - 2);
				return  0.5 * f * f * f * f * f + 1;
			case EaseCirc:
				if(p < 0.5) {
					return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
				}
				return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
			case EaseExpo:
				if(p == 0.0 || p == 1.0) return p;
				if(p < 0.5) return 0.5 * pow(2, (20 * p) - 10);
				return -0.5 * pow(2, (-20 * p) + 10) + 1;
			case EaseBack:
				if(p < 0.5) {
					f = 2 * p;
					return 0.5 * (f * f * f - f * sin(f * M_PI));
				} else {
					f = (1 - (2*p - 1));
					return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
				}
			case EaseElastic:
				if(p < 0.5) {
					return 0.5 * sin(13 * M_PI_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
				} else {
					return 0.5 * (sin(-13 * M_PI_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
				}
			default:
				if(p < 0.5) {
					return 0.5 * ( 1 - _bounceEaseOut(1 - p * 2) );
				} else {
					return 0.5 * _bounceEaseOut(p * 2 - 1) + 0.5;
				}
		}
	}
}

/* MARK:	-				Getters / Setters
 -------------------------------------------------------------------- */


/// setter for property names
void Tween::SetProperties( ArgValue val ) {
	if ( val.type == TypeString ) {
		// just one
		properties.resize( 1 );
		properties[ 0 ] = *val.value.stringValue;
		startValues.resize( 1 );
		endValues.resize( 1 );
	} else if ( val.type == TypeArray ) {
		properties.resize( 0 );
		for ( size_t i = 0, np = val.value.arrayValue->size(); i < np; i++ ) {
			ArgValue& v = val.value.arrayValue->at( i );
			if ( v.type == TypeString ) {
				properties.push_back( *v.value.stringValue );
			}
		}
		startValues.resize( properties.size() );
		endValues.resize( properties.size() );
	} else {
		properties.resize( 0 );
		startValues.resize( 0 );
		endValues.resize( 0 );
	}
}

/// getter for property names
ArgValue Tween::GetProperties() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = properties.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( properties[ i ].c_str() ) );
	}
	return val;
}

/// setter for start values
void Tween::SetStartValues( ArgValue val ) {
	float f = 0;
	if ( val.toNumber( f ) ) {
		// just one
		startValues.resize( 1 );
		startValues[ 0 ] = f;
	} else if ( val.type == TypeArray ) {
		startValues.resize( 0 );
		for ( size_t i = 0, np = val.value.arrayValue->size(); i < np; i++ ) {
			ArgValue& v = val.value.arrayValue->at( i );
			if ( v.toNumber( f ) ) {
				startValues.push_back( f );
			}
		}
	} else {
		startValues.resize( properties.size() );
		// try to populate with current values
		if ( this->target ) {
			for ( size_t i = 0, np = properties.size(); i < np; i++ ) {
				ArgValue val = script.GetProperty( properties[ i ].c_str(), this->target );
				if ( val.toNumber( f ) ) startValues[ i ] = f;
			}
		}
	}
}

/// getter for start values
ArgValue Tween::GetStartValues() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = startValues.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( startValues[ i ] ) );
	}
	return val;
}

/// setter for end values
void Tween::SetEndValues( ArgValue val ) {
	float f = 0;
	if ( val.toNumber( f ) ) {
		// just one
		endValues.resize( 1 );
		endValues[ 0 ] = f;
	} else if ( val.type == TypeArray ) {
		endValues.resize( 0 );
		for ( size_t i = 0, np = val.value.arrayValue->size(); i < np; i++ ) {
			ArgValue& v = val.value.arrayValue->at( i );
			if ( v.toNumber( f ) ) {
				endValues.push_back( f );
			}
		}
	} else {
		endValues.resize( startValues.size() );
		// try to populate with start values
		for ( size_t i = 0, np = startValues.size(); i < np; i++ ) {
			endValues[ i ] = startValues[ i ];
		}
	}
}

/// getter for end values
ArgValue Tween::GetEndValues() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = endValues.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( endValues[ i ] ) );
	}
	return val;
}

/// setter for .active property. Adds/removes this tween to global running tweens array, and protects/unprotects from garbage collection.
void Tween::active( bool r ) {
	if ( r != _active ) {
		if ( r && !this->_canRun() ) return;
		this->_active = r;
		if ( r ) {
			// restart if ended
			if ( time >= duration ) time = 0;
			activeTweens->insert( this );
			script.ProtectObject( &this->scriptObject, true );
		} else {
			unordered_set<Tween*>::iterator it = activeTweens->find( this );
			if( it != activeTweens->end() ) activeTweens->erase( it );
			script.ProtectObject( &this->scriptObject, false );
		}
	}
}


/* MARK:	-				Process
 -------------------------------------------------------------------- */


bool Tween::ProcessTween( float deltaTime, float unscaledDeltaTime ) {
	
	// remove if became invalid
	if ( !this->_active || !_canRun() ) return true;
	
	// advance time
	this->time += this->useUnscaledTime ? unscaledDeltaTime : deltaTime;
	
	// current position
	float pos = min( this->time / this->duration, 1.0f );
	
	// use callback, if specified
	if ( this->callback.funcObject ) {
		ScriptArguments callbackArguments;
		callbackArguments.AddObjectArgument( this->scriptObject );
		this->callback.Invoke( callbackArguments );
	}
	
	// for each property
	size_t np = this->properties.size();
	size_t ns = startValues.size();
	size_t ne = endValues.size();
	for ( size_t i = 0; i < np; i++ ){
		string& prop = this->properties[ i ];
		float fromVal = ( i < ns ? startValues[ i ] : 0 );
		float toVal = ( i < ne ? endValues[ i ] : 0 );
		float value = fromVal + ( toVal - fromVal ) * Tween::Ease( easeType, easeFunc, pos );
		
		// apply final value
		script.SetProperty( prop.c_str(), ArgValue( value ), this->target );
	}
	
	// dispatch end event
	if ( pos >= 1 ) {
		
		Event event( EVENT_FINISHED );
		event.scriptParams.AddObjectArgument( this->scriptObject );
		this->CallEvent( event );
		
		// remove
		return true;
	}
	
	// keep going
	return false;
	
}

/// calls ProcessTween on all running tweens (called by game loop)
void Tween::ProcessActiveTweens( float deltaTime, float unscaledDeltaTime ) {
	// advance all active tweens
	unordered_set<Tween*>::iterator it = activeTweens->begin();
	while( it != activeTweens->end() ) {
		if ( !(*it)->_active || (*it)->ProcessTween( deltaTime, unscaledDeltaTime ) ) {
			(*it)->_active = false;
			it = activeTweens->erase( it );
			continue;
		}
		it++;
	}
}
