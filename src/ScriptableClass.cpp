#include "ScriptableClass.hpp"
#include "Application.hpp"


/// static event stack
vector<Event*> Event::eventStack;

// destructor
ScriptableClass::~ScriptableClass() {
	
	// script
	if ( this->scriptObject != NULL && script.js != NULL ) {
		
		// remove from all scheduled calls
		ScriptableClass::CancelAsync( this->scriptObject, -1 );
		ScriptableClass::CancelDebouncer( this->scriptObject, "" );
		
		// clean up late events
		app.RemoveLateEvents( this );
		
		// clean up, if protected
		JS_SetPrivate( (JSObject*) this->scriptObject, NULL );
		this->scriptObject = NULL;
		
	}
	
}

/// registers base class
void ScriptableClass::InitClass() {
	
	// register class
	script.RegisterClass<ScriptableClass>( NULL, true );
	
	// functions
	
	// schedule a call
	script.DefineFunction<ScriptableClass>
	( "async",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: async( Function handler [, Number delay ] )";
		void* handler = NULL;
		float delay = 0;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 1, TypeFunction, &handler, TypeFloat, &delay ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		sa.ReturnInt( ScriptableClass::AddAsync( self->scriptObject, handler, delay ) );
		return true;
	}));
	
	// cancel a call
	script.DefineFunction<ScriptableClass>
	( "cancelAsync",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelAsync( Int asyncId )";
		int index = 0;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
			script.ReportError( error );
			return false;
		}
		// cancel scheduled call
		sa.ReturnBool( ScriptableClass::CancelAsync( self->scriptObject, index ) );
		return true;
	}));
	
	// schedule a call
	script.DefineFunction<ScriptableClass>
	( "debounce",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: debounce( String debounceId, Function handler [, Number delay ] )";
		void* handler = NULL;
		string name;
		float delay = 0;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 2, TypeString, &name, TypeFunction, &handler, TypeFloat, &delay ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		ScriptableClass::AddDebouncer( self->scriptObject, name, handler, delay );
		return true;
	}));
	
	// cancel a call
	script.DefineFunction<ScriptableClass>
	( "cancelDebouncer",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelDebouncer( String debounceId )";
		string name;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 1, TypeString, &name ) ) {
			script.ReportError( error );
			return false;
		}
		// cancel scheduled call
		sa.ReturnBool( ScriptableClass::CancelDebouncer( self->scriptObject, name ) );
		return true;
	}));
	
	// add event listener
	script.DefineFunction<ScriptableClass>
	( "on",
		static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: on( String eventName | Array eventNames, [ Function handler [, Boolean once ] ] )";
		void* handler = NULL;
		bool once = false;
		string propName;
		ArgValueVector props;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) go;
		if ( !sa.ReadArguments( 1, TypeString, &propName, TypeFunction, &handler, TypeBool, &once ) ) {
			if ( !sa.ReadArguments( 1, TypeArray, &props, TypeFunction, &handler, TypeBool, &once ) ){
				script.ReportError( error );
				return false;
			}
		}
		
		// loop over array of event names
		size_t nth = 0;
		ArgValueVector retArr;
		do {
			// next property
			if ( props.size() ) {
				ArgValue &v = props[ nth ];
				if ( v.type == TypeString ){
					propName = *v.value.stringValue;
				} else {
					script.ReportError( error );
					return false;
				}
			}
			
			// listeners
			EventListeners& list = self->eventListeners[ propName ];
			
			// function specified
			if ( handler ) {
				// append to event listeners
				list.emplace_back( handler, once );
			// no function specified
			} else {
				// make array of currently attached listeners
				EventListeners::iterator it = list.begin(), end = list.end();
				while( it != end ) {
					retArr.emplace_back( (void*) it->funcObject );
					it++;
				}
			}
		} while (++nth < props.size() );
		
		// return self of array of handlers
		if ( handler ) sa.ReturnObject( self->scriptObject );
		else sa.ReturnArray( retArr );
		return true;
	} ));
	
	// remove event listener
	script.DefineFunction<ScriptableClass>
	( "off",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: off( String eventName | Array eventNames [, Function handler ] )";
		void* handler = NULL;
		string propName;
		ArgValueVector props;

		// validate
		ScriptableClass* self = (ScriptableClass*) go;
		if ( !sa.ReadArguments( 1, TypeString, &propName, TypeFunction, &handler ) ) {
			if ( !sa.ReadArguments( 1, TypeArray, &props, TypeFunction, &handler ) ) {
				script.ReportError( error );
				return false;
			}
		}
		
		// loop over array of event names
		size_t nth = 0;
		do {
			// next property
			if ( props.size() ) {
				ArgValue &v = props[ nth ];
				if ( v.type == TypeString ){
					propName = *v.value.stringValue;
				} else {
					script.ReportError( error );
					return false;
				}
			}
			
			// remove from event listeners
			EventListeners& list = self->eventListeners[ propName ];
			EventListeners::iterator it = list.begin();
			while ( it != list.end() ) {
				ScriptFunctionObject& fo = *it;
				// is specified, find it
				if ( handler ) {
					// found?
					if ( fo == handler ) {
						// if executing, force it to be deleted after executing
						if ( fo.executing ) {
							fo.callOnce = true;
						} else {
							// remove
							list.erase( it );
						}
						break;
					}
					it++;
				} else {
					if ( fo.executing ) {
						fo.callOnce = true;
						it++;
					} else {
						// remove
						it = list.erase( it );
					}
				}
			}
		} while (++nth < props.size() );
		
		// for chaining
		sa.ReturnObject( self->scriptObject );
		return true;
	}));
	
	script.DefineFunction<ScriptableClass>
	( "fire",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		const char* error = "usage: fire( String eventName [, arguments... ] )";
		string eventName;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeString, &eventName ) || !eventName.length() ) {
			script.ReportError( error );
			return false;
		}
		
		// make event with additional arguments
		Event event( eventName.c_str() );
		for ( size_t i = 1, np = sa.args.size(); i < np; i++ ) {
			event.scriptParams.AddArgument( sa.args[ i ] );
		}
		// dispatch
		ScriptableClass* self = (ScriptableClass*) go;
		self->ScriptableClass::CallEvent( event );
		return true;
	}));
	
	script.DefineFunction<ScriptableClass>
	( "fireLate",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		const char* error = "usage: fireLate( String eventName [, arguments... ] )";
		string eventName;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeString, &eventName ) || !eventName.length() ) {
			script.ReportError( error );
			return false;
		}
		
		// add to late events
		ScriptableClass* self = (ScriptableClass*) go;
		ArgValueVector *lateEvent = app.AddLateEvent( self, eventName.c_str() );
		if ( lateEvent ) {
			for ( size_t i = 1, np = sa.args.size(); i < np; i++ ) {
				lateEvent->push_back( sa.args[ i ] );
			}
		}
		return true;
	}));
	
	script.DefineFunction<ScriptableClass>
	( "toString",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ) {
		static char buf[512];
		ScriptableClass* self = (ScriptableClass*) o;
		
		if ( !self ) {
			sprintf( buf, "[ScriptableClass prototype]" );
		} else sprintf( buf, "[%s %p]", self->scriptClassName, self );
		
		sa.ReturnString( buf );
		return true;
	}));
	
}
