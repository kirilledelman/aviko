#include "SampleBehavior.hpp"
#include "GameObject.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
SampleBehavior::SampleBehavior( ScriptArguments* args ) : SampleBehavior() {
	
	// add scriptObject
	script.NewScriptObject<SampleBehavior>( this );
	
}

// init
SampleBehavior::SampleBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_UPDATE, (BehaviorEventCallback) &SampleBehavior::Update );
	AddEventCallback( EVENT_LATEUPDATE, (BehaviorEventCallback) &SampleBehavior::LateUpdate );
	AddEventCallback( EVENT_ADDED, (BehaviorEventCallback) &SampleBehavior::Added );
	AddEventCallback( EVENT_REMOVED, (BehaviorEventCallback) &SampleBehavior::Removed );
	AddEventCallback( EVENT_ADDEDTOSCENE, (BehaviorEventCallback) &SampleBehavior::AddedToScene );
	AddEventCallback( EVENT_REMOVEDFROMSCENE, (BehaviorEventCallback) &SampleBehavior::RemovedFromScene );
	AddEventCallback( EVENT_ACTIVECHANGED, (BehaviorEventCallback) &SampleBehavior::ActiveChanged );
	// AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &SampleBehavior::Render );
	
}

// destructor
SampleBehavior::~SampleBehavior() {}


/* MARK:	-				Serialization
 -------------------------------------------------------------------- */



/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void SampleBehavior::InitClass() {
	
	// register class
	script.RegisterClass<SampleBehavior>( "Behavior" );
	
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


// update
void SampleBehavior::Update( SampleBehavior* behavior, void* param, Event* e ) {}

void SampleBehavior::LateUpdate( SampleBehavior* behavior, void* go, Event* e ){}

void SampleBehavior::Added( SampleBehavior* behavior, GameObject* newParent, Event* e ){}

void SampleBehavior::Removed( SampleBehavior* behavior, GameObject* oldParent, Event* e ){}

void SampleBehavior::AddedToScene( SampleBehavior* behavior, GameObject* topObject, Event* e ){}

void SampleBehavior::RemovedFromScene( SampleBehavior* behavior, GameObject* topObject, Event* e ){}

void SampleBehavior::ActiveChanged( SampleBehavior* behavior, GameObject* topObject, Event* e ){}

void SampleBehavior::Render( SampleBehavior* behavior, GPU_Target* target, Event* e ){}



