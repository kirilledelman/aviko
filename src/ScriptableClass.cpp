#include "ScriptableClass.hpp"
#include "Application.hpp"


/// static event stack
vector<Event*> Event::eventStack;

// destructor
ScriptableClass::~ScriptableClass() {

    // debug
    debugObjectsDestroyed++;
    
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
		const char* error = "usage: async( Function handler [, Number delay, [ Boolean useUnscaledTime ] ] )";
		void* handler = NULL;
		float delay = 0;
		bool unscaled = false;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 1, TypeFunction, &handler, TypeFloat, &delay, TypeBool, &unscaled ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		sa.ReturnInt( ScriptableClass::AddAsync( self->scriptObject, handler, delay, unscaled ) );
		return true;
	}));
	
	// cancel a call
	script.DefineFunction<ScriptableClass>
	( "cancelAsync",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelAsync( Int asyncId )";
		int index = -1;
		ScriptableClass* self = (ScriptableClass*) o;
		
		// one param? cancel single one
		if ( sa.args.size() >= 1 ) {
			if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
				script.ReportError( error );
				return false;
			}
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
		const char* error = "usage: debounce( String debounceId, Function handler [, Number delay, [ Boolean useUnscaledTime ] ] )";
		void* handler = NULL;
		string name;
		float delay = 0;
		bool unscaled = false;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) o;
		if ( !sa.ReadArguments( 2, TypeString, &name, TypeFunction, &handler, TypeFloat, &delay, TypeBool, &unscaled ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		ScriptableClass::AddDebouncer( self->scriptObject, name, handler, delay, unscaled );
		return true;
	}));
	
	// cancel a call
	script.DefineFunction<ScriptableClass>
	( "cancelDebouncer",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelDebouncer( String debounceId )";
		string name;
		ScriptableClass* self = (ScriptableClass*) o;
		
		// one param? cancel single one
		if ( sa.args.size() >= 1 ) {
			if ( !sa.ReadArguments( 1, TypeString, &name ) ) {
				script.ReportError( error );
				return false;
			}
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
		const char* error = "usage: on( String eventName | Array eventNames, [ Function handler [, Boolean once [, Integer priority ] ] ] )";
		void* handler = NULL;
		bool once = false;
		int priority = 0;
		string propName;
		ArgValueVector props;
		
		// validate
		ScriptableClass* self = (ScriptableClass*) go;
		if ( !sa.ReadArguments( 1, TypeString, &propName, TypeFunction, &handler, TypeBool, &once, TypeInt, &priority ) ) {
			if ( !sa.ReadArguments( 1, TypeArray, &props, TypeFunction, &handler, TypeBool, &once, TypeInt, &priority ) ){
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
				// check if already exists
				EventListeners::iterator it, end = list.end();
				it = find_if( list.begin(), end, [&handler](const ScriptFunctionObject &arg) { return arg.funcObject == handler; } );
				if ( it == end ) {
					// insert at priority
					it = list.begin();
					bool inserted = false;
					while ( it != end ) {
						if ( it->priority > priority ) {
							list.emplace( it, handler, once );
							inserted = true;
							break;
						}
						it++;
					}
					// append to event listeners
					if ( !inserted ) list.emplace_back( handler, once );
				}
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
		sa.ReturnBool( !event.stopped );
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

void ScriptableClass::ProcessScheduledCalls( float dt ) {
	AsyncMap::iterator it = scheduledAsyncs->begin();
	while ( it != scheduledAsyncs->end() ) {
		ScheduledCallList &list = it->second;
		ScheduledCallList::iterator lit = list.begin();
		if ( lit == list.end() ) {
			it = scheduledAsyncs->erase( it );
			continue;
		}
		while ( lit != list.end() ) {
			if ( lit->TimePassed( dt, app.timeScale ) ) {
				lit = list.erase( lit );
			} else {
				lit++;
			}
		}
		it++;
	}
	DebouncerMap::iterator dit = scheduledDebouncers->begin();
	while( dit != scheduledDebouncers->end() ) {
		unordered_map<string, ScheduledCall> &debouncers = dit->second;
		unordered_map<string, ScheduledCall>::iterator dbit = debouncers.begin();
		if ( dbit == debouncers.end() ) {
			dit = scheduledDebouncers->erase( dit );
			continue;
		}
		while ( dbit != debouncers.end() ) {
			ScheduledCall& sched = dbit->second;
			//printf( "Debouncer %s\n", dbit->first.c_str()  );
			if ( sched.TimePassed( dt, app.timeScale ) ) {
				dbit = debouncers.erase( dbit );
			} else {
				dbit++;
			}
		}
		dit++;
	}
}

/// calls script event listeners on this ScriptableObject
void ScriptableClass::CallEvent( Event& event ) {
	
	// ignore if finalizing
	if ( script.IsAboutToBeFinalized( &this->scriptObject ) || event.stopped ) return;
	
	// event listeners array
	EventListeners* list = &this->eventListeners[ event.name ];
	
	// some objects will dispatch events to functions with the same name, for ease of use
	if ( this->dispatchEventsToPropertyFunctions ) {
		// check if we have a callback with the same name as property
		ArgValue funcObject = script.GetProperty( event.name, this->scriptObject );
		if ( funcObject.type == TypeFunction ) {
			// call function
			script.CallFunction( funcObject.value.objectValue, this->scriptObject, event.scriptParams );
            debugEventsDispatched[ string(event.name) ]++;
		}
	}
	
	// if event was stopped, exit
	if ( event.stopped ) return;
	
	// call on all listeners - note that listeners may be removed while dispatching
	EventListeners::iterator it = list->begin();
	while ( it != list->end() ){
		ScriptFunctionObject *fobj = &(*it);
		fobj->thisObject = this->scriptObject;
		fobj->Invoke( event.scriptParams );
        debugEventsDispatched[ string(event.name) ]++;
		if ( fobj->callOnce ) {
			it = list->erase( it );
		} else it++;
		if ( event.stopped ) return;
	}
	
}

