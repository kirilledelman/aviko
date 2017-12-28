#include "BodyBehavior.hpp"
#include "Scene.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// no 'new BodyBehavior()'
BodyBehavior::BodyBehavior( ScriptArguments* ) { script.ReportError( "BodyBehavior can't be created using 'new'." ); }

/// default constructor
BodyBehavior::BodyBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_ATTACHED, (BehaviorEventCallback) &BodyBehavior::Attached );
	AddEventCallback( EVENT_DETACHED, (BehaviorEventCallback) &BodyBehavior::Detached );
	AddEventCallback( EVENT_ADDED_TO_SCENE, (BehaviorEventCallback) &BodyBehavior::Attached );
	AddEventCallback( EVENT_REMOVED_FROM_SCENE, (BehaviorEventCallback) &BodyBehavior::Detached );
	AddEventCallback( EVENT_ACTIVE_CHANGED, (BehaviorEventCallback) &BodyBehavior::ActiveChanged );
	
	// is body
	this->isBodyBehavior = true;
	
};

//
BodyBehavior::~BodyBehavior() { };


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void BodyBehavior::InitClass() {
	
	// nothing for now
	
}


/* MARK:	-				Attached / removed / active
 -------------------------------------------------------------------- */


/// attach/detach from a gameObject
bool BodyBehavior::SetGameObject( GameObject* go, int pos ) {
	
	if ( go && go != this->gameObject && dynamic_cast<Scene*>(go) ) {
		script.ReportError( "Scene can not have a physics body." );
		return false;
	}
	
	// removing from object
	if ( this->gameObject && this->gameObject != go ) {
		// stamp object local coords
		this->gameObject->DirtyTransform();
		this->gameObject->Transform();
	}
	
	if ( go ) { go->DirtyTransform(); go->Transform(); }
	
	// base
	bool r = Behavior::SetGameObject( go, pos );
	if ( r && go ) go->DirtyTransform();
	return r;
}

// attached to scene callback
void BodyBehavior::Attached( BodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// add body to world
	behavior->AddBody( target->GetScene() );
	
}

/// detached from scene callback
void BodyBehavior::Detached( BodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// remove from world
	behavior->RemoveBody();
	
}

/// overrides behavior active setter
bool BodyBehavior::active( bool a ) {
	
	this->_active = a;
	if ( this->gameObject ) this->EnableBody( _active && this->gameObject->active() );
	return a;
	
}

/// active changed callback
void BodyBehavior::ActiveChanged( BodyBehavior* behavior, GameObject* target, Event* event ) {
	
	// set its active status to combination of active + on scene
	behavior->EnableBody( behavior->_active && behavior->gameObject->active() );
	
}
