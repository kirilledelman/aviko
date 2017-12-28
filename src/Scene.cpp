#include "Scene.hpp"
#include "Application.hpp"

#include "RenderShapeBehavior.hpp"
#include "RenderSpriteBehavior.hpp"
#include "RigidBodyBehavior.hpp"
#include "SampleBehavior.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

Scene::Scene( ScriptArguments* args ) : Scene() {
	
	// add scriptObject
	script.NewScriptObject<Scene>( this );
	
	// create color object
	clearColor = new Color( NULL );
	clearColor->SetInts( 25, 50, 75, 255 );
	script.SetProperty( "clearColor", ArgValue( clearColor->scriptObject ), this->scriptObject );
	
	// if have at least one argument
	if ( args && args->args.size() >= 1 ) {
		// 
		if ( args->args[ 0 ].type == TypeString ) {
		}
	}
	
}


// empty scene initialization
Scene::Scene() {
	
	// top level object
	this->orphan = false;
	
	// set default name
	static char buf[256];
	sprintf( buf, "Scene-%p", this );
	this->name = buf;
	
	// create world
	gravity.Set( 0, 0 );
	this->world = new b2World( this->gravity );
	
	// set debug draw
	this->_sceneDebugDraw.SetFlags( b2Draw::e_shapeBit | b2Draw::e_pairBit | b2Draw::e_jointBit );
	this->world->SetDebugDraw( &this->_sceneDebugDraw );
	this->world->SetContactListener( this );

}

// scene clean up
Scene::~Scene() {
	
	printf( "Scene(%s) destructor on %p\n", this->name.c_str(), this );
	
	// remove all children before destroying world in Scene
	
	// unparent all children
	for( int i = (int) this->children.size() - 1; i >= 0; i-- ) {
		GameObject* obj = (GameObject*) this->children[ i ];
		obj->SetParent( NULL );
	}
	
	// remove all behaviors
	for ( int i = (int) this->behaviors.size() - 1; i >= 0; i-- ) {
		Behavior* beh = (Behavior*) this->behaviors[ i ];
		beh->SetGameObject( NULL );
	}
	
	// destroy world
	delete this->world;
	this->world = NULL;
		
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void Scene::InitClass() {

	// init scripting class
	script.RegisterClass<Scene>( "GameObject" );
	
	// properties
	
	script.AddProperty<Scene>
	( "debugDraw",
	 static_cast<ScriptBoolCallback>([]( void* s, bool val ){ return ((Scene*)s)->debugDraw; }),
	 static_cast<ScriptBoolCallback>([]( void* s, bool val ){ return ((Scene*)s)->debugDraw = val; }));
	
	script.AddProperty<Scene>
	( "clearColor",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((Scene*) b)->clearColor->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((Scene*) b)->clearColor = other;
		return ((Scene*) b)->clearColor->scriptObject;
	}) );
	
	script.AddProperty<Scene>
	( "cameraX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camX; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camX = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<Scene>
	( "cameraY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camY; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camY = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<Scene>
	( "cameraPivotX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camPivotX; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camPivotX = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<Scene>
	( "cameraPivotY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camPivotY; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camPivotY = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));

	script.AddProperty<Scene>
	( "cameraAngle",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camAngle; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camAngle = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));

	script.AddProperty<Scene>
	( "cameraZoom",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camZoom; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camZoom = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<Scene>
	( "currentFocus",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		Scene* scene = (Scene*) b;
		if ( scene->focusedUI ) return scene->focusedUI->scriptObject;
		return (void*) NULL;
	 }));
	
	script.AddProperty<Scene>
	( "gravityX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->gravity.x; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->gravity.x = val;
		s->world->SetGravity( s->gravity );
		return val;
	}));
	
	script.AddProperty<Scene>
	( "gravityY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->gravity.y; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->gravity.y = val;
		s->world->SetGravity( s->gravity );
		return val;
	}));
	
	// functions

	
}


/* MARK:	-				Render
 -------------------------------------------------------------------- */

/// returns updated camera transform
float* Scene::CameraTransform() {
	if ( _camTransformDirty ){
		GPU_MatrixIdentity( cameraMatrix );
		GPU_MatrixTranslate( cameraMatrix, camX + camPivotX, camY + camPivotY, 0 );
		GPU_MatrixRotate( cameraMatrix, camAngle, 0, 0, 1 );
		GPU_MatrixTranslate( cameraMatrix, -camPivotX, -camPivotY, 0 );
		GPU_MatrixScale( cameraMatrix, camZoom, camZoom, 1 );
		_camTransformDirty = false;
		_inverseCamTransformDirty = true;
	}
	
	return cameraMatrix;
	
}

// returns inverse of current cam transform
float* Scene::InverseCameraTransform() {
	if ( _camTransformDirty || _inverseCamTransformDirty ) {
		MatrixInverse( this->CameraTransform(), inverseCameraMatrix );
		_inverseCamTransformDirty = false;
	}
	return inverseCameraMatrix;
}

/// render all objects
void Scene::Render( Event& event ) {
	
	GPU_Target* rt = (GPU_Target*) event.behaviorParam;
	
	// clear screen
	SDL_Color &color = this->clearColor->rgba;
	GPU_ClearRGBA( rt, color.r, color.g, color.b, color.a );
	
	// pass self
	event.scene = this;
	
	// set up camera transform
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PushMatrix();
	GPU_MatrixCopy( GPU_GetCurrentMatrix(), this->CameraTransform() );
	
	// base
	GameObject::Render( event );
	
	// debug draw world
	if ( this->debugDraw ) {
		GPU_ActivateShaderProgram( 0, NULL );
		this->world->DrawDebugData();
	}
	
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	
}


/* MARK:	-				Physics
 -------------------------------------------------------------------- */


/// Called when two fixtures begin to touch.
void Scene::BeginContact( b2Contact* contact ) {
	b2Fixture* a = contact->GetFixtureA();
	b2Fixture* b = contact->GetFixtureB();
	RigidBodyShape *shapeA = a ? (RigidBodyShape*) a->GetUserData() : NULL;
	RigidBodyShape *shapeB = b ? (RigidBodyShape*) b->GetUserData() : NULL;
	if ( !contact->IsTouching() || !shapeA || !shapeA->body || !shapeA->body->gameObject ||
					!shapeB || !shapeB->body || !shapeB->body->gameObject ) return;
	b2WorldManifold worldManifold;
	contact->GetWorldManifold( &worldManifold );
	b2Vec2 point = worldManifold.points[ 0 ] * BOX2D_TO_WORLD_SCALE;
	b2Vec2 normal = worldManifold.normal;
	float separation = worldManifold.separations[ 0 ];
	physicsEvents.emplace_back
	( static_cast<PhysicsEventCallback>([ shapeA, shapeB, point, normal, separation ](){
		Event event( EVENT_TOUCH );
		event.scriptParams.AddObjectArgument( shapeA->scriptObject );
		event.scriptParams.AddObjectArgument( shapeB->scriptObject );
		event.scriptParams.AddFloatArgument( point.x );
		event.scriptParams.AddFloatArgument( point.y );
		event.scriptParams.AddFloatArgument( normal.x );
		event.scriptParams.AddFloatArgument( normal.y );
		event.scriptParams.AddFloatArgument( separation );
		shapeA->body->CallEvent( event );
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( shapeB->scriptObject );
		event.scriptParams.AddObjectArgument( shapeA->scriptObject );
		event.scriptParams.AddFloatArgument( point.x );
		event.scriptParams.AddFloatArgument( point.y );
		event.scriptParams.AddFloatArgument( normal.x );
		event.scriptParams.AddFloatArgument( normal.y );
		event.scriptParams.AddFloatArgument( separation );
		shapeB->body->CallEvent( event );
	}) );
	
}

/// Called when two fixtures cease to touch.
void Scene::EndContact( b2Contact* contact ) {
	b2Fixture* a = contact->GetFixtureA();
	b2Fixture* b = contact->GetFixtureB();
	RigidBodyShape *shapeA = a ? (RigidBodyShape*) a->GetUserData() : NULL;
	RigidBodyShape *shapeB = b ? (RigidBodyShape*) b->GetUserData() : NULL;
	if ( !shapeA || !shapeA->body || !shapeA->body->gameObject ||
		!shapeB || !shapeB->body || !shapeB->body->gameObject ) return;
	b2WorldManifold worldManifold;
	contact->GetWorldManifold( &worldManifold );
	b2Vec2 point = worldManifold.points[ 0 ] * BOX2D_TO_WORLD_SCALE;
	physicsEvents.emplace_back
	( static_cast<PhysicsEventCallback>([ shapeA, shapeB, point ](){
		Event event( EVENT_UNTOUCH );
		event.scriptParams.AddObjectArgument( shapeA->scriptObject );
		event.scriptParams.AddObjectArgument( shapeB->scriptObject );
		event.scriptParams.AddFloatArgument( point.x );
		event.scriptParams.AddFloatArgument( point.y );
		shapeA->body->CallEvent( event );
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( shapeB->scriptObject );
		event.scriptParams.AddObjectArgument( shapeA->scriptObject );
		event.scriptParams.AddFloatArgument( point.x );
		event.scriptParams.AddFloatArgument( point.y );
		shapeB->body->CallEvent( event );
	}) );
}

/// Box2D simulation
void Scene::SimulatePhysics() {
	
	// clear physics events
	physicsEvents.clear();
	
	// step world
	this->world->Step( app.deltaTime, BOX2D_VELOCITY_ITERATIONS, BOX2D_POSITION_ITERATIONS );
	
	// sync positions of bodies with GameObjects
	b2Body* body = this->world->GetBodyList();
	while( body != NULL ) {
		
		// call sync on RigidBodyBehavior
		RigidBodyBehavior* rbb = (RigidBodyBehavior*) body->GetUserData();
		if ( rbb != NULL ) rbb->SyncObjectToBody();
		
		// keep going
		body = body->GetNext();
		
	};
	
	// dispatch physics events
	for ( size_t i = 0, ne = physicsEvents.size(); i < ne; i++ ){
		physicsEvents[ i ]();
	}
	
}

