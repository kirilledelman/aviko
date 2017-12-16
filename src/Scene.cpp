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
	this->world = new b2World( b2Vec2( 0, 125 ) );
	
	// set debug draw
	this->_sceneDebugDraw.SetFlags( b2Draw::e_shapeBit );
	this->world->SetDebugDraw( &this->_sceneDebugDraw );

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
	
	script.AddProperty<GameObject>
	( "cameraX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camX; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camX = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "cameraY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camY; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camY = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "cameraPivotX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camPivotX; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camPivotX = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "cameraPivotY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camPivotY; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camPivotY = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));

	script.AddProperty<GameObject>
	( "cameraAngle",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((Scene*) o)->camAngle; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		Scene* s = (Scene*) o;
		s->camAngle = val;
		s->_camTransformDirty = s->_inverseCamTransformDirty = true;
		return val;
	}));

	script.AddProperty<GameObject>
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
	
	// functions
	//...
	
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
		//GPU_MatrixIdentity( inverseCameraMatrix );
		//float unzoom = 1.0f / camZoom;
		//GPU_MatrixTranslate( inverseCameraMatrix, camX, camY, 0 );
		//GPU_MatrixRotate( inverseCameraMatrix, camAngle, 0, 0, 1 );
		//GPU_MatrixScale( inverseCameraMatrix, camZoom, camZoom, 1 );
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
	if ( this->debugDraw ) this->world->DrawDebugData();
	
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	
}


/* MARK:	-				Physics
 -------------------------------------------------------------------- */


/// Box2D simulation
void Scene::SimulatePhysics() {
	
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
	
}

