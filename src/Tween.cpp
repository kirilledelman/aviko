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
		} else {
			script.ReportError( "Tween constructor: second parameter must be String or Array of Strings." );
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
		
		// easing
		if ( args->args.size() >= 6 ) {
			int e = EaseNone;
			args->args[ 5 ].toInt( e );
			this->easingFunc = (EasingFunc) e;
		}
		
		// start, if valid
		this->running( properties.size() > 0 && duration > 0 );
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

	//
	
	script.AddProperty<Tween>
	("properties",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetProperties();
	 }),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetProperties( val );
		return val;
	 }), PROP_NOSTORE | PROP_ENUMERABLE );
	
	script.AddProperty<Tween>
	("startValues",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetStartValues();
	}),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetStartValues( val );
		return val;
	}), PROP_NOSTORE | PROP_ENUMERABLE );
	
	script.AddProperty<Tween>
	("endValues",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		return ((Tween*) p)->GetEndValues();
	}),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		((Tween*) p)->SetEndValues( val );
		return val;
	}), PROP_NOSTORE | PROP_ENUMERABLE );

	
	
}



/* MARK:	-				Update
 -------------------------------------------------------------------- */


bool Tween::ProcessTween( float deltaTime, float unscaledDeltaTime ) {
	
	// remove if became invalid
	if ( !this->_running || !this->target || !this->properties.size() ) return true;
	
	// advance time
	this->time += this->useUnscaledTime ? unscaledDeltaTime : deltaTime;
	
	// current position
	float pos = this->time / this->duration;
	
	// for each property
	size_t np = this->properties.size();
	size_t ns = startValues.size();
	size_t ne = endValues.size();
	for ( size_t i = 0; i < np; i++ ){
		string& prop = this->properties[ i ];
		float fromVal = ( i < ns ? startValues[ i ] : 0 );
		float toVal = ( i < ne ? endValues[ i ] : 0 );
		float value = fromVal;
		
		// transition using easing func
		switch ( easingFunc ) {
			default:
				value = fromVal + ( toVal - fromVal ) * pos;
				break;
			
		}
		
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

/* MARK:	-				Getters / Setters
 -------------------------------------------------------------------- */


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

ArgValue Tween::GetProperties() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = properties.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( properties[ i ].c_str() ) );
	}
	return val;
}

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

ArgValue Tween::GetStartValues() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = startValues.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( startValues[ i ] ) );
	}
	return val;
}

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

ArgValue Tween::GetEndValues() {
	ArgValue val;
	val.type = TypeArray;
	val.value.arrayValue = new ArgValueVector();
	for ( size_t i = 0, np = endValues.size(); i < np; i++ ) {
		val.value.arrayValue->push_back( ArgValue( endValues[ i ] ) );
	}
	return val;
}


/* MARK:	-				Process
 -------------------------------------------------------------------- */


void Tween::running( bool r ) {
	if ( r != _running ) {
		this->_running = r;
		if ( r ) {
			activeTweens->insert( this );
		} else {
			unordered_set<Tween*>::iterator it = activeTweens->find( this );
			if( it != activeTweens->end() ) activeTweens->erase( it );
		}
	}
}

void Tween::ProcessActiveTweens( float deltaTime, float unscaledDeltaTime ) {
	// advance all active tweens
	unordered_set<Tween*>::iterator it = activeTweens->begin();
	while( it != activeTweens->end() ) {
		if ( !(*it)->_running || (*it)->ProcessTween( deltaTime, unscaledDeltaTime ) ) {
			(*it)->_running = false;
			it = activeTweens->erase( it );
			continue;
		}
		it++;
	}
}
