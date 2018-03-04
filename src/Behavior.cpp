#include "Behavior.hpp"
#include "Application.hpp"
#include "GameObject.hpp"

#include "RenderShapeBehavior.hpp"
#include "RenderSpriteBehavior.hpp"
#include "SampleBehavior.hpp"
#include "RigidBodyBehavior.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


// base behavior class
Behavior::Behavior( ScriptArguments* args ) { script.ReportError( "Behavior can't be created using 'new'" ); }

Behavior::Behavior() {}

Behavior::~Behavior() {}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void Behavior::InitClass() {
	
	// register class
	script.RegisterClass<Behavior>( "ScriptableObject", true );
	
	// properties
	script.AddProperty<Behavior>
	( "gameObject",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		 Behavior* self = (Behavior*) go;
		 return self->gameObject ? self->gameObject->scriptObject : NULL;
	 }),
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		 Behavior* self = (Behavior*) go;
		 GameObject* other = script.GetInstance<GameObject>( p );
		 self->SetGameObject( other );
		 return self->gameObject ? self->gameObject->scriptObject : NULL;
	 }), PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<Behavior>
	( "active",
	 static_cast<ScriptBoolCallback>([](void* b, bool a ) { return ((Behavior*) b)->active(); }),
	 static_cast<ScriptBoolCallback>([](void* b, bool a ) { return ((Behavior*) b)->active( a ); }));
	
	// functions
	
	script.DefineFunction<Behavior>
	( "toString",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ) {
		static char buf[512];
		Behavior* self = (Behavior*) o;
		
		if ( !self ) {
			sprintf( buf, "[Behavior prototype]" );
		} else if ( self->gameObject && self->gameObject->name.size() ){
			sprintf( buf, "[%s (%s) %p]", self->scriptClassName, self->gameObject->name.c_str(), self );
		} else {
			sprintf( buf, "[%s %p]", self->scriptClassName, self );
		}
		
		sa.ReturnString( buf );
		return true;
	}));
	
}

/// garbage collection callback
void Behavior::TraceProtectedObjects( vector<void **> &protectedObjects ) {

	// game object
	if ( this->gameObject ) protectedObjects.push_back( &this->gameObject->scriptObject );
		
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
		
}



/* MARK:	-				Attachment
 -------------------------------------------------------------------- */


// set a new parent for object
bool Behavior::SetGameObject( GameObject* newGameObject ){
	
	// if parent is different
	if ( newGameObject != this->gameObject ) {
		
		/// prepare event
		Event event( this->scriptObject );
		
		// attaching to a new game object
		if ( newGameObject ) {
			// if this is a rendering behavior
			if ( this->isRenderBehavior ) {
				// if object already has rendering behavior
				if ( newGameObject->render != NULL ) {
					script.ReportError( "%s already has a rendering behavior attached.", newGameObject->name.c_str() );
					return false;
				} else newGameObject->render = (RenderBehavior*) this;
			} else if ( this->isBodyBehavior ) {
				// if object already has body behavior
				if ( newGameObject->body != NULL ) {
					script.ReportError( "%s already has a physics body attached.", newGameObject->name.c_str() );
					return false;
				} else newGameObject->body = (RigidBodyBehavior*) this;
			} else if ( this->isUIBehavior ) {
				// if object already has ui behavior
				if ( newGameObject->ui != NULL ) {
					script.ReportError( "%s already has a UI behavior attached.", newGameObject->name.c_str() );
					return false;
				} else newGameObject->ui = (UIBehavior*) this;
			}
		}
		
		// if had parent
		GameObject *oldGameObject = this->gameObject;
		if ( oldGameObject ) {
			
			// find this object in parent's list of children
			BehaviorList *parentList = &this->gameObject->behaviors;
			BehaviorList::iterator listEnd = parentList->end();
			BehaviorList::iterator it = find( parentList->begin(), listEnd, this );
			
			// remove from list
			if ( it != listEnd ) parentList->erase( it );
			
			// if this was old object's singular behavior, clear it
			if ( oldGameObject->render == this ) oldGameObject->render = NULL;
			else if ( oldGameObject->body == this ) oldGameObject->body = NULL;
			else if ( oldGameObject->ui == this ) oldGameObject->ui = NULL;

			// clear
			this->gameObject = NULL;
			
			// call detached event directly on event
			BehaviorEventCallback func = this->GetCallbackForEvent( EVENT_DETACHED );
			if ( func ) func( this, oldGameObject, &event );
			
		}
		
		// set gameObject
		this->gameObject = newGameObject;
		
		// add to new gameobject
		if ( newGameObject ) {
			
			newGameObject->behaviors.push_back( this );
			
			// call attached event directly on event
			BehaviorEventCallback func = this->GetCallbackForEvent( EVENT_ATTACHED );
			if ( func ) func( this, gameObject, &event );
			
		}
		
	}
	
	return true;
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


void Behavior::CallEventCallback( Event &event ) {

	// find function
	BehaviorEventCallback func = this->GetCallbackForEvent( event.name );
	
	// if callback for this event exists, call it
	if ( func != NULL ) (*func)( this, event.behaviorParam, &event );
	
}

void Behavior::AddEventCallback( const char* eventName, BehaviorEventCallback callback ) {
	
	eventFunctions[ eventName ] = callback;
	
}


