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
	
	/// param passed to behaviors
	void* behaviorParam = NULL;
	/// parameters passed to script event handlers for this event
	ScriptArguments scriptParams;
	
	// constructor
	Event(){};
	/// construct event with scriptObject as first script parameter
	Event( void* scriptObject ) { if ( scriptObject != NULL ) { this->scriptParams.ResizeArguments( 0 ); this->scriptParams.AddObjectArgument( scriptObject ); } }
	/// construct named event with scriptObject as first script parameter, if provided
	Event( const char* name, void* scriptObject=NULL ) : Event::Event( scriptObject ) { this->name =  name; }
	
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
	
	/// hash(eventName) -> vector of script function callbacks
	EventListenersMap eventListeners;
	
	/// 
	bool dispatchEventsToPropertyFunctions = false;

	/// calls script event listeners on this ScriptableObject
	virtual void CallEvent( Event& event ) {
		// event listeners array
		EventListeners* list = &this->eventListeners[ event.name ];
		
		// call each
		EventListenersIterator it = list->begin();
		while ( it != list->end() ){
			ScriptFunctionObject *fobj = &(*it);
			fobj->Invoke( event.scriptParams, this->scriptObject );
			if ( fobj->callOnce ) {
				it = list->erase( it );
			} else it++;
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

// init
	
	// constructor
	ScriptableClass(){}
	
	// abstract
	ScriptableClass( ScriptArguments* ) { script.ReportError( "ScriptableObject can't be created using 'new'" ); }	

	// destructor
	~ScriptableClass() {		
		// clean up
		if ( this->scriptObject && script.js ) {
			script.ProtectObject( &this->scriptObject, false );
			JS_SetPrivate( (JSObject*) this->scriptObject, NULL );
			this->scriptObject = NULL;
		}
	}
};

SCRIPT_CLASS_NAME( ScriptableClass, "ScriptableObject" );

#endif /* ScriptableClass_hpp */
