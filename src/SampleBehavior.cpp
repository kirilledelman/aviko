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
	AddEventCallback( EVENT_LATE_UPDATE, (BehaviorEventCallback) &SampleBehavior::LateUpdate );
	AddEventCallback( EVENT_ADDED, (BehaviorEventCallback) &SampleBehavior::Added );
	AddEventCallback( EVENT_REMOVED, (BehaviorEventCallback) &SampleBehavior::Removed );
	AddEventCallback( EVENT_ADDED_TO_SCENE, (BehaviorEventCallback) &SampleBehavior::AddedToScene );
	AddEventCallback( EVENT_REMOVED_FROM_SCENE, (BehaviorEventCallback) &SampleBehavior::RemovedFromScene );
	AddEventCallback( EVENT_ACTIVE_CHANGED, (BehaviorEventCallback) &SampleBehavior::ActiveChanged );
	// AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &SampleBehavior::Render );
	
}

// destructor
SampleBehavior::~SampleBehavior() {}


/* MARK:	-				Serialization
 -------------------------------------------------------------------- */



/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// Behavior -> script class "Behavior"
SCRIPT_CLASS_NAME( SampleBehavior, "SampleBehavior" );

// init script classes
void SampleBehavior::InitClass() {
	
	// register class
	script.RegisterClass<SampleBehavior>( "Behavior" );
	
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


// update
void SampleBehavior::Update( SampleBehavior* behavior, void* param ) {}

void SampleBehavior::LateUpdate( SampleBehavior* behavior, void* go ){}

void SampleBehavior::Added( SampleBehavior* behavior, GameObject* newParent ){}

void SampleBehavior::Removed( SampleBehavior* behavior, GameObject* oldParent ){}

void SampleBehavior::AddedToScene( SampleBehavior* behavior, GameObject* topObject ){}

void SampleBehavior::RemovedFromScene( SampleBehavior* behavior, GameObject* topObject ){}

void SampleBehavior::ActiveChanged( SampleBehavior* behavior, GameObject* topObject ){}

void SampleBehavior::Render( SampleBehavior* behavior, GPU_Target* target ){}



