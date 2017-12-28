#ifndef ScriptableClass_hpp
#define ScriptableClass_hpp

#include "common.h"
#include "ScriptHost.hpp"

/* MARK:	-				Scriptable class
 
 ScriptableClass is a parent class to any class wishing to be available for scripting.
 
 To make your class visible in script:
 
	 class MyClass: public ScriptableClass {...} // inherit from ScriptableClass
 
	 SCRIPT_CLASS_NAME( MyClass, "ClassName" ); // add this macro to cpp file

 Add a static method InitClass, and call it during application init phase. Inside it:
 
	 script.RegisterClass<MyClass>(); // this creates MyClass script object
	 script.AddProperty ...
	 script.SetStaticProperty ...
	 script.DefineFunction ...
 
 For inheritance, give RegisterClass a string name of previously registered parent:
 property and function lookups will be dispatched to hierarchy as expected.
 
 Your class must have a constructor of this form:
 
	 MyClass::MyClass( ScriptArguments* args ) {
		 // must call
		 script.NewScriptObject<MyClass>( this );
	 }

 It's used when constructing MyClass from script via new MyClass(...) and NewScriptObject call
 attaches a Javascript object to your instance.
 
 Scriptable class also provides functions for even handling
 to enable it in your class, define two functions:
 
	 // add event listener functions
	 script.DefineFunction<MyClass>( "on", ScriptableClass::addEventListenerHandler() ); // ( String eventName, Function handler )
	 script.DefineFunction<MyClass>( "off", ScriptableClass::removeEventListenerHandler() ); // ( String eventName, Function handler )

 Then in Javascript you can call myclass.on( 'eventname', callback ); and myclass.off( 'eventname', callback );
 to add and remove event handlers.
 
 To dispatch events, make Event structure, SetName, and Add____Argument(s) to its scriptParams. Then
 call CallEvent. It will dispatch all event listeners attached to MyClass instance.
 
 TODO - write about serialization 
 
 -------------------------------------------------------------------- */

class GameObject;
class Scene;

/// event structure
struct Event {
	
	/// event name
	const char* name;
	
	/// if true, it's dispatched in children first, then parent (GameObject uses this)
	bool bubbles = false;
	
	/// for stopping event processing loops
	bool stopped = false;
	
	/// behavior can set this to a specific gameobject to skip over it when doing hierarchy dispatch. Used with Image/autoDraw 
	GameObject* skipObject = NULL;
	
	// used during render event
	Scene* scene = NULL;
	
	/// param passed to behaviors
	void* behaviorParam = NULL;
	/// parameters passed to script event handlers for this event
	ScriptArguments scriptParams;
	
	// global event stack
	static vector<Event*> eventStack;
	
	// constructor
	Event(){ Event::eventStack.push_back( this ); };
	/// construct event with scriptObject as first script parameter
	Event( void* scriptObject ) : Event::Event() { if ( scriptObject != NULL ) { this->scriptParams.ResizeArguments( 0 ); this->scriptParams.AddObjectArgument( scriptObject ); } }
	/// construct named event with scriptObject as first script parameter, if provided
	Event( const char* name, void* scriptObject=NULL ) : Event::Event( scriptObject ) { this->name =  name; }
	// destructor
	~Event(){
		// erase self from stack
		vector<Event*>::iterator it = Event::eventStack.end(), begin = Event::eventStack.begin();
		while ( it > begin ){
			it--;
			if ( *it == this ) {
				Event::eventStack.erase( it );
				break;
			}
		}
	}
	
};

/// classes that want to be available for scripting should inherit from this class
class ScriptableClass {
public:

// general
	
	/// script object instance
	void* scriptObject = NULL;
	
	/// points to script class name after script object has been constructed
	const char* scriptClassName = NULL;
	
	/// registers base class
	static void InitClass() {
		
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
			const char* error = "usage: on( String eventName, [ Function handler [, Boolean once ] ] )";
			void* handler = NULL;
			bool once = false;
			string propName;
			
			// validate
			ScriptableClass* self = (ScriptableClass*) go;
			if ( !sa.ReadArguments( 1, TypeString, &propName, TypeFunction, &handler, TypeBool, &once ) ) {
				script.ReportError( error );
				return false;
			}
			
			// listeners
			EventListeners& list = self->eventListeners[ propName ];
			
			// function specified
			if ( handler ) {
				// append to event listeners
				list.emplace_back( handler, once );
				// return this object for chaining
				sa.ReturnObject( self->scriptObject );
			// no function specified
			} else {
				// make array of currently attached listeners
				ArgValueVector arr;
				EventListeners::iterator it = list.begin(), end = list.end();
				while( it != end ) {
					arr.emplace_back( (void*) it->funcObject );
					it++;
				}
				// return array
				sa.ReturnArray( arr );
			}
			return true;
		} ));
		
		// remove event listener
		script.DefineFunction<ScriptableClass>
		( "off",
		 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
			// validate params
			const char* error = "usage: off( String eventName [, Function handler ] )";
			void* handler = NULL;
			string propName;
			
			// validate
			if ( !sa.ReadArguments( 1, TypeString, &propName, TypeFunction, &handler ) ) {
				script.ReportError( error );
				return false;
			}
			
			// remove from event listeners
			ScriptableClass* self = (ScriptableClass*) go;
			EventListeners& list = self->eventListeners[ propName ];
			
			// is specified, find it
			if ( handler ) {
				for ( auto it = list.begin(); it != list.end(); it++ ) {
					ScriptFunctionObject& fo = *it;
					// found?
					if ( fo == handler ) {
						// remove
						list.erase( it );
						break;
					}
				}
				// not specified, clear all
			} else {
				list.clear();
			}
			
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
			self->CallEvent( event );
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
	
// events
	
	typedef vector<ScriptFunctionObject> EventListeners;
	typedef vector<ScriptFunctionObject>::iterator EventListenersIterator;
	typedef unordered_map<string, EventListeners> EventListenersMap;
	typedef unordered_map<string, EventListeners>::iterator EventListenersMapIterator;
	
	/// eventName -> vector of script function callbacks
	EventListenersMap eventListeners;
	
	/// if true, also calls eventName() by name as function
	bool dispatchEventsToPropertyFunctions = true;

	/// calls script event listeners on this ScriptableObject
	virtual void CallEvent( Event& event ) {
		// event listeners array
		EventListeners* list = &this->eventListeners[ event.name ];
		
		// call each
		EventListenersIterator it = list->begin();
		while ( it != list->end() ){
			ScriptFunctionObject *fobj = &(*it);
			fobj->thisObject = this->scriptObject;
			fobj->Invoke( event.scriptParams );
			if ( fobj->callOnce ) {
				it = list->erase( it );
			} else it++;
			
			// if event was stopped, abort
			if ( event.stopped ) return;
		}
		
		// some objects will dispatch events to functions with the same name, for ease of use
		if ( this->dispatchEventsToPropertyFunctions ) {
			// check if we have a callback with the same name as property
			ArgValue funcObject = script.GetProperty( event.name, this->scriptObject );
			if ( funcObject.type == TypeFunction ) {
				// call function
				script.CallFunction( funcObject.value.objectValue, this->scriptObject, event.scriptParams );
			}
		}
		
	}
	
	// trace ops
	
	virtual void TraceProtectedObjects( vector<void**> &protectedObjects ) {
		// push &scriptObject of each child object that needs to be protected on vector
	}

	// async, debounce
	
	struct ScheduledCall {
		ScriptFunctionObject func;
		float timeLeft = 0;
		float timeSet = 0;
		int index = 0;
		bool TimePassed( float dt ) {
			this->timeLeft -= dt;
			if ( this->timeLeft <= 0 ){
				ScriptArguments args;
				this->func.Invoke( args );
				return true;
			}
			return false;
		}
		ScheduledCall(){ index = ++ScriptableClass::asyncIndex; }
		ScheduledCall( void* obj, void* fun, float delay ) : func( fun, true ) {
			func.thisObject = obj;
			timeSet = timeLeft = delay;
			index = ++ScriptableClass::asyncIndex;
		}
	};
	typedef list<ScheduledCall> ScheduledCallList;
	typedef unordered_map<void*, ScheduledCallList> AsyncMap;
	typedef unordered_map<void*, unordered_map<string, ScheduledCall>> DebouncerMap;
	
	/// auto-incremented async index, so we can remove it by index
	static int asyncIndex;
	
	// scriptedObject -> list of ScheduledCalls
	static AsyncMap* scheduledAsyncs;
	
	// scriptedObject -> map "debounce string" -> ScheduledCall
	static DebouncerMap* scheduledDebouncers;
	
	/// adds scheduled async
	static int AddAsync( void* obj, void* func, float delay ) {
		list<ScheduledCall>& asyncs = (*scheduledAsyncs)[ obj ];
		asyncs.emplace_back( obj, func, delay );
		return asyncs.back().index;
	}
	
	/// removes async by index
	static bool CancelAsync( void *obj, int index ){
		AsyncMap::iterator it = scheduledAsyncs->find( obj );
		if ( it == scheduledAsyncs->end() ) return false;
		// index = -1 kills all asyncs for this object
		if ( index == -1 ) {
			scheduledAsyncs->erase( it );
			return true;
		// erase scheduled call with index
		} else {
			ScheduledCallList &list = it->second;
			ScheduledCallList::iterator lit = list.begin(), lend = list.end();
			while ( lit != lend ) {
				if ( lit->index == index ) {
					lit = list.erase( lit );
					return true;
				}
				lit++;
			}
		}
		return false;
	}
	
	/// add/replaces debounce
	static void AddDebouncer( void *obj, string name, void* func, float delay ){
		unordered_map<string, ScheduledCall> &debouncers = (*scheduledDebouncers)[ obj ];
		unordered_map<string, ScheduledCall>::iterator it = debouncers.find( name );
		// already exists
		if ( it != debouncers.end() ){
			ScheduledCall &sched = it->second;
			sched.func.SetFunc( func );
			// delay specified? update to new delay
			if ( delay > 0 ) {
				sched.timeSet = sched.timeLeft = delay;
			// reset delay
			} else {
				sched.timeLeft = sched.timeSet;
			}
		// new
		} else {
			ScheduledCall &sched = debouncers[ name ];
			sched.func.SetFunc( func );
			sched.func.thisObject = obj;
			sched.timeLeft = sched.timeSet = delay;
		}
		
	}
	
	/// cancels debouncer
	static bool CancelDebouncer( void *obj, string name ){
		DebouncerMap::iterator it = scheduledDebouncers->find( obj );
		if ( it == scheduledDebouncers->end() ) return false;
		// name = "" kills all debouncers for this object
		if ( name.length() == 0 ) {
			scheduledDebouncers->erase( it );
			return true;
		// find debouncer with name
		} else {
			unordered_map<string, ScheduledCall> &debouncers = it->second;
			unordered_map<string, ScheduledCall>::iterator dit = debouncers.find( name );
			if ( dit != debouncers.end() ) {
				debouncers.erase( dit );
				return true;
			}
		}
		return false;
	}
	
	static void ProcessScheduledCalls( float dt ) {
		AsyncMap::iterator it = scheduledAsyncs->begin();
		while ( it != scheduledAsyncs->end() ) {
			ScheduledCallList &list = it->second;
			ScheduledCallList::iterator lit = list.begin();
			if ( lit == list.end() ) {
				it = scheduledAsyncs->erase( it );
				continue;
			}
			while ( lit != list.end() ) {
				if ( lit->TimePassed( dt ) ) {
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
				if ( sched.TimePassed( dt ) ) {
					dbit = debouncers.erase( dbit );
				} else {
					dbit++;
				}
			}
			dit++;
		}
	}
	
// init
	
	// constructor
	ScriptableClass(){}
	
	// abstract
	ScriptableClass( ScriptArguments* ) { script.ReportError( "ScriptableObject can't be created using 'new'" ); }	

	// destructor
	~ScriptableClass() {
		// script
		if ( this->scriptObject && script.js ) {
			// remove from all scheduled calls
			ScriptableClass::CancelAsync( this->scriptObject, -1 );
			ScriptableClass::CancelDebouncer( this->scriptObject, "" );
			// clean up
			script.ProtectObject( &this->scriptObject, false );
			JS_SetPrivate( (JSObject*) this->scriptObject, NULL );
			this->scriptObject = NULL;
		}
	}
};

SCRIPT_CLASS_NAME( ScriptableClass, "ScriptableObject" );

#endif /* ScriptableClass_hpp */
