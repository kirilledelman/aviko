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
	script.RegisterClass<Behavior>( NULL, true );
	
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
	
	
}


/* MARK:	-				Attachment
 -------------------------------------------------------------------- */


// set a new parent for object
bool Behavior::SetGameObject( GameObject* newGameObject, int desiredPosition ){
	
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
			BehaviorVector *parentList = &this->gameObject->behaviors;
			BehaviorIterator listEnd = parentList->end();
			BehaviorIterator it = find( parentList->begin(), listEnd, this );
			
			// remove from list
			if ( it != listEnd ) parentList->erase( it );
			
			// if this was old object's singular behavior, clear it
			if ( oldGameObject->render == this ) oldGameObject->render = NULL;
			else if ( oldGameObject->body == this ) oldGameObject->body = NULL;
			else if ( oldGameObject->ui == this ) oldGameObject->ui = NULL;

			// clear
			this->gameObject = NULL;
			
			// call detached event directly on event
			BehaviorEventCallback func = this->GetCallbackForHash( HashString( EVENT_DETACHED ) );
			if ( func ) func( this, oldGameObject, &event );
			
		}
		
		// set gameObject
		this->gameObject = newGameObject;
		
		// add to new gameobject
		if ( newGameObject ) {
			
			// insert into behaviors based on desired position
			int numBehaviors = (int) newGameObject->behaviors.size();
			// convert negative pos
			if ( desiredPosition < 0 ) desiredPosition = numBehaviors + desiredPosition;
			// insert at location
			if ( desiredPosition >= numBehaviors - 1 ) {
				newGameObject->behaviors.push_back( this );
			} else {
				newGameObject->behaviors.insert( newGameObject->behaviors.begin() + max( 0, desiredPosition ), this );
			}			
			
			// call attached event directly on event
			BehaviorEventCallback func = this->GetCallbackForHash( HashString( EVENT_ATTACHED ) );
			if ( func ) func( this, gameObject, &event );
			
			// gained a parent? protect us from GC
			if ( !oldGameObject ) script.ProtectObject( &this->scriptObject, true );
			
		// no new gameobject - means we've definitely been removed from scene
		} else {
			
			// unprotect us from GC
			script.ProtectObject( &this->scriptObject, false );
			
		}
		
	}
	
	return true;
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


void Behavior::CallEventCallback( Event &event ) {

	// find function
	BehaviorEventCallback func = this->GetCallbackForHash( event.hash );
	
	// if callback for this event exists, call it
	if ( func != NULL ) (*func)( this, event.behaviorParam, &event );
	
}

void Behavior::AddEventCallback( const char* eventName, BehaviorEventCallback callback ) {
	
	eventFunctions[ HashString( eventName ) ] = callback;
	
}


