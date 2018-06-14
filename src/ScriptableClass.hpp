#ifndef ScriptableClass_hpp
#define ScriptableClass_hpp

#include "common.h"
#include "ScriptHost.hpp"


/* MARK:	-				Scriptable class
 
 
 -------------------------------------------------------------------- */

class GameObject;
class Scene;

/// event structure
struct Event {
	
	/// event name
	const char* name = "";
	
	/// if true, it's dispatched in children first, then parent (GameObject uses this)
	bool bubbles = false;
	
	/// only dispatched on behaviors
	bool behaviorsOnly = false;
	
	/// a subset of events originating from Input class, subject to UI->blockable
	bool isBlockableUIEvent = false;
	
	/// UI behavior considering this event in bounds will set this to true
	bool isUIEventInBounds = false;
	
	/// will be set to true when entering blocking UI event subtree
	bool willBlockUIEvent = false;
	
	/// is set by disabled UIBehavior
	bool skipChildren = false;
	
	/// behavior can set this to a specific gameobject to skip over it when doing hierarchy dispatch. Used with Image/autoDraw 
	GameObject* skipObject = NULL;
	GameObject* skipObject2 = NULL;
	
	// used during render and UI events
	Scene* scene = NULL;
	
	// true to stop processing
	bool stopped = false;
	
	/// param passed to behaviors
	void* behaviorParam = NULL;
	void* behaviorParam2 = NULL;
	
	/// parameters passed to script event handlers for this event
	ScriptArguments scriptParams;
	
	static vector<Event*> eventStack;
	
	// constructor
	Event(){ eventStack.push_back( this ); };
	/// construct event with scriptObject as first script parameter
	Event( void* scriptObject ) : Event::Event() { if ( scriptObject != NULL ) { this->scriptParams.ResizeArguments( 0 ); this->scriptParams.AddObjectArgument( scriptObject ); } }
	/// construct named event with scriptObject as first script parameter, if provided
	Event( const char* name, void* scriptObject=NULL ) : Event::Event( scriptObject ) { this->name =  name; }
	// destructor
	~Event(){
		if ( eventStack.back() == this ) {
			eventStack.pop_back();
		} else {
			vector<Event*>::iterator it = eventStack.begin();
			printf( "eventStack order broken, Event: %s\n", this->name );
			while ( it != eventStack.end() ) {
				if ( *it == this ) {
					printf( "~~%s~~\n", this->name );
					it = eventStack.erase( it );
					if ( it == eventStack.end() ) break;
				} else {
					printf( "%s\n", (*it)->name ? (*it)->name : "???" );
				}
				it++;
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
	static void InitClass();
	
// events
	
	typedef vector<ScriptFunctionObject> EventListeners;
	typedef unordered_map<string, EventListeners> EventListenersMap;
	
	/// eventName -> vector of script function callbacks
	EventListenersMap eventListeners;
	
	/// if true, also calls eventName() by name as function
	bool dispatchEventsToPropertyFunctions = true;
	
	/// calls script event listeners on this ScriptableObject
	virtual void CallEvent( Event& event ) {
		
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
			if ( fobj->callOnce ) {
				it = list->erase( it );
			} else it++;
			if ( event.stopped ) return;
		}
		
	}
	
	// true if .eventName, or "on" event listeners registered
	bool HasListenersForEvent( const char* eventName ) {
		EventListenersMap::iterator hit = this->eventListeners.find( string( eventName ) );
		bool hasHandler = ( hit != this->eventListeners.end() && hit->second.size() > 0 );
		if ( !hasHandler && dispatchEventsToPropertyFunctions ) {
			ArgValue hval = script.GetProperty( eventName, this->scriptObject );
			if ( hval.type == TypeFunction ) {
				return true;
			}
		}
		return hasHandler;
	}
	
	// trace ops
	
	virtual void TraceProtectedObjects( vector<void**> &protectedObjects ) {
		// push &scriptObject of each child object that needs to be protected on vector
		
		// add event listeners function objects
		EventListenersMap::iterator it = eventListeners.begin(), end = eventListeners.end();
		while ( it != end ) {
			EventListeners& listeners = it->second;
			EventListeners::iterator lit = listeners.begin(), lend = listeners.end();
			while ( lit != lend ) {
				ScriptFunctionObject &sf = *lit;
				protectedObjects.push_back( &sf.funcObject );
				if ( sf.thisObject ) protectedObjects.push_back( &sf.thisObject );
				lit++;
			}
			it++;
		}
		
	}

	// async, debounce
	
	struct ScheduledCall {
		ScriptFunctionObject func;
		float timeLeft = 0;
		float timeSet = 0;
		int index = 0;
		bool unscaled = false;
		bool TimePassed( float dt, float ts ) {
			if ( !unscaled ) dt *= ts;
			this->timeLeft -= dt;
			if ( this->timeLeft <= 0 ){
				ScriptArguments args;
				this->func.Invoke( args );
				// timeLeft is still 0 after Invoke (for serial debounce)
				return ( this->timeLeft <= 0 );
			}
			return false;
		}
		ScheduledCall(){ index = ++ScriptableClass::asyncIndex; }
		ScheduledCall( void* obj, void* fun, float delay, bool unscaledTime ) : func( fun, true ) {
			func.thisObject = obj;
			timeSet = timeLeft = delay;
			unscaled = unscaledTime;
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
	static int AddAsync( void* obj, void* func, float delay, bool unscaled ) {
		list<ScheduledCall>& asyncs = (*scheduledAsyncs)[ obj ];
		asyncs.emplace_front( obj, func, delay, unscaled );
		return asyncs.front().index;
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
					// if not about to be destroyed, erase
					if ( lit->timeLeft > 0 ) lit = list.erase( lit );
					return true;
				}
				lit++;
			}
		}
		return false;
	}
	
	/// add/replaces debounce
	static void AddDebouncer( void *obj, string name, void* func, float delay, bool unscaled ){
		unordered_map<string, ScheduledCall> &debouncers = (*scheduledDebouncers)[ obj ];
		unordered_map<string, ScheduledCall>::iterator it = debouncers.find( name );
		// already exists
		if ( it != debouncers.end() ){
			ScheduledCall &sched = it->second;
			sched.func.funcObject = func;
			// delay specified? update to new delay
			if ( delay > 0 ) {
				sched.timeSet = sched.timeLeft = delay;
			// reset delay
			} else {
				sched.timeLeft = sched.timeSet;
			}
			sched.unscaled = unscaled;
		// new
		} else {
			ScheduledCall &sched = debouncers[ name ];
			sched.func.funcObject = func;
			sched.func.thisObject = obj;
			sched.timeLeft = sched.timeSet = delay;
			sched.unscaled = unscaled;
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
				// if not about to clear, erase
				if ( dit->second.timeLeft > 0 ) debouncers.erase( dit );
				return true;
			}
		}
		return false;
	}
	
	static void ProcessScheduledCalls( float dt );
	
// safe casting

	template <class CLASS>
	/// compares class names to return desired class instance, or NULL
	static CLASS* ClassInstance( ScriptableClass* inst ) {
		if ( !inst ) return NULL;
		if ( strcmp( inst->scriptClassName, ScriptClassDesc<CLASS>::name().c_str() ) == 0 ) return static_cast<CLASS*>( inst );
		return NULL;
	}
	
// init
	
	// constructor
	ScriptableClass(){ }
	
	// abstract
	ScriptableClass( ScriptArguments* ) { script.ReportError( "ScriptableObject can't be created using 'new'" ); }	

	// destructor
	~ScriptableClass();
};

SCRIPT_CLASS_NAME( ScriptableClass, "ScriptableObject" );

#endif /* ScriptableClass_hpp */
