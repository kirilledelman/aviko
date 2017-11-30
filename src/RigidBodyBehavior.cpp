#include "RigidBodyBehavior.hpp"
#include "GameObject.hpp"
#include "Scene.hpp"
#include "ScriptHost.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


// creating from script
RigidBodyBehavior::RigidBodyBehavior( ScriptArguments* args ) : RigidBodyBehavior() {
	
	// add scriptObject
	script.NewScriptObject<RigidBodyBehavior>( this );
	
}

// init
RigidBodyBehavior::RigidBodyBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_ATTACHED, (BehaviorEventCallback) &RigidBodyBehavior::Attached );
	AddEventCallback( EVENT_DETACHED, (BehaviorEventCallback) &RigidBodyBehavior::Detached );
	AddEventCallback( EVENT_ADDED_TO_SCENE, (BehaviorEventCallback) &RigidBodyBehavior::Attached );
	AddEventCallback( EVENT_REMOVED_FROM_SCENE, (BehaviorEventCallback) &RigidBodyBehavior::Detached );
	AddEventCallback( EVENT_ACTIVE_CHANGED, (BehaviorEventCallback) &RigidBodyBehavior::ActiveChanged );
	
	// can render
	this->isBodyBehavior = true;
	
}

// destroy
RigidBodyBehavior::~RigidBodyBehavior() {
	// clean up - remove body
	this->RemoveBody();
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void RigidBodyBehavior::InitClass() {
	
	// register class
	script.RegisterClass<RigidBodyBehavior>( "Behavior" );
	
}


/* MARK:	-				Serialization
 -------------------------------------------------------------------- */


/* MARK:	-				Sync body <-> object
 -------------------------------------------------------------------- */


/// IRigidBody method - copies transform to game object
void RigidBodyBehavior::SyncObjectToBody() {
	
	// copy from body
	b2Vec2 pos = this->body->GetPosition();
	float angle = (float) this->body->GetAngle() * RAD_TO_DEG;
	
	// construct world transform matrix for object
	GPU_MatrixIdentity( this->gameObject->_worldTransform );
	GPU_MatrixTranslate( this->gameObject->_worldTransform, pos.x, pos.y, 0 );
	GPU_MatrixRotate( this->gameObject->_worldTransform, angle, 0, 0, 1 );
	GPU_MatrixScale( this->gameObject->_worldTransform, this->gameObject->_scale.x, this->gameObject->_scale.y, 1 );
	this->gameObject->_worldTransformDirty = false;
	this->gameObject->_localCoordsAreDirty = this->gameObject->_inverseWorldDirty = this->gameObject->_transformDirty = true;
		
}

/// converts game object's local transform to body
void RigidBodyBehavior::SyncBodyToObject() {

	// parent's world transform times local transform = this object world transform
	GPU_MatrixMultiply( this->gameObject->_worldTransform, this->gameObject->parent->WorldTransform(), this->gameObject->Transform() );
	this->gameObject->_worldTransformDirty = false;
	
	// extract pos, rot, scale
	b2Vec2 pos, scale;
	float angle;
	this->gameObject->DecomposeTransform( this->gameObject->_worldTransform, pos, angle, scale );
	
	// update body
	this->body->SetTransform( pos, angle * DEG_TO_RAD );
	
}

/// just sets body transform
void RigidBodyBehavior::SetBodyTransform( b2Vec2 pos, float angleInRad ) {
	this->body->SetTransform( pos, angleInRad );
}

/// set body position
void RigidBodyBehavior::SetBodyPosition( b2Vec2 pos ) {
	this->body->SetTransform( pos, this->body->GetAngle() );
}

/// set body angle
void RigidBodyBehavior::SetBodyAngle( float angleInRad ) {
	this->body->SetTransform( this->body->GetPosition(), angleInRad );
}

// gets body transform
void RigidBodyBehavior::GetBodyTransform( b2Vec2& pos, float& angle ) {
	pos = this->body->GetPosition();
	angle = this->body->GetAngle();
}



/* MARK:	-				Attached / removed / active
 -------------------------------------------------------------------- */


/// attach/detach from a gameObject
bool RigidBodyBehavior::SetGameObject( GameObject* go, int pos ) {
	
	if ( go && go != this->gameObject ) {
		// check if gameObject already has a body, fail
		if ( go->body ) {
			script.ReportError( "%s already has a physics body behavior attached.", go->name.c_str() );
			return false;
		} else if ( dynamic_cast<Scene*>(go) ) {
			script.ReportError( "Scene can not have a physics behavior." );
			return false;
		}
	}
	
	// base
	return Behavior::SetGameObject( go, pos );
}

// attached to scene callback
void RigidBodyBehavior::Attached( RigidBodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// add body to world
	behavior->AddBody( target->GetScene() );
	
}

/// detached from scene callback
void RigidBodyBehavior::Detached( RigidBodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// remove from world
	behavior->RemoveBody();
	
}

/// overrides behavior active setter
bool RigidBodyBehavior::active( bool a ) {
	
	this->_active = a;
	if ( this->body ) this->body->SetActive( _active && this->gameObject->active() );
	return a;
}

/// active changed callback
void RigidBodyBehavior::ActiveChanged( RigidBodyBehavior* behavior, GameObject* target, Event* event ) {
	
	// set its active status to combination of active + on scene
	if ( behavior->body ) behavior->body->SetActive( behavior->Behavior::active() && behavior->gameObject->active() );
	
}

void RigidBodyBehavior::AddBody( Scene *scene ) {
	
	// ignore
	if ( !scene || this->body ) return;

	// TODO - shapes etc.
	
	b2BodyDef bodyDef;
	bodyDef.angle = gameObject->_angle * DEG_TO_RAD;
	bodyDef.position = gameObject->GetWorldPosition();
	bodyDef.userData = this;
	bodyDef.linearDamping = 0.25;
	bodyDef.angularDamping = 0.25;
	bodyDef.allowSleep = true;
	bodyDef.active = false;
	bodyDef.type = b2_dynamicBody;
	
	// create body
	this->body = scene->world->CreateBody( &bodyDef );
	
	// add default fixture
	b2PolygonShape box;
	box.SetAsBox( 5, 5);
	b2Fixture* f = this->body->CreateFixture( &box, 10 );
	f->SetRestitution( 0.25 );
	f->SetFriction( 1 );
	
	// make active, if gameObject and behavior are active
	this->body->SetActive( this->gameObject->active() && this->_active );
	
	// set reference
	this->gameObject->body = this;

}

void RigidBodyBehavior::RemoveBody() {
	
	// remove body
	if ( this->body ) this->body->GetWorld()->DestroyBody( this->body );
	this->body = NULL;
	
	// clear body ref
	if( this->gameObject && this->gameObject->body == this ) this->gameObject->body = NULL;
}

