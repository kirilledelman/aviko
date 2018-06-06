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
	backgroundColor = new Color( NULL );
	backgroundColor->SetInts( 25, 50, 75, 255 );
	script.SetProperty( "backgroundColor", ArgValue( backgroundColor->scriptObject ), this->scriptObject );

	// event mask
	this->eventMask = new TypedVector( NULL );
	ArgValue dv( "String" );
	this->eventMask->InitWithType( dv );	
	
	// if have at least one argument
	if ( args && args->args.size() >= 1 ) {
		// type object - copy props
		if ( args->args[ 0 ].type == TypeObject ) {
			script.CopyProperties( args->args[ 0 ].value.objectValue, this->scriptObject );
		}
	}
	
}


// empty scene initialization
Scene::Scene() {
	
	// top level object
	this->orphan = false;
	
	// set default name
	this->name = "Scene";
	
	// create world
	gravity.Set( 0, 0 );
	this->world = new b2World( this->gravity );
	
	// create ground body
	b2BodyDef bd;
	bd.type = b2_staticBody;
	this->groundBody = this->world->CreateBody( &bd );
	
	// set debug draw
	this->SetFlags( b2Draw::e_shapeBit | b2Draw::e_pairBit | b2Draw::e_jointBit );
	this->world->SetDebugDraw( this );
	this->world->SetContactListener( this );
	this->world->SetContactFilter( this );

}

// scene clean up
Scene::~Scene() {
	
	// printf( "Scene(%s) destructor on %p\n", this->name.c_str(), this );
	
	// destroy world
	delete this->world;
	this->world = NULL;
	this->groundBody = NULL;
		
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void Scene::InitClass() {

	// init scripting class
	script.RegisterClass<Scene>( "GameObject" );
	
	// properties
	
	script.AddProperty<Scene>
	( "backgroundColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((Scene*) b)->backgroundColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		Scene* rs = (Scene*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->backgroundColor = other;
		} else {
			rs->backgroundColor->Set( val );
		}
		return rs->backgroundColor->scriptObject;
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

	script.DefineFunction<Scene>
	("rayCast",
	 static_cast<ScriptFunctionCallback>([]( void* p, ScriptArguments& sa ){
		Scene* s = (Scene*) p;
		// arguments
		float x = 0, y = 0, dx = 0, dy = 0;
		void* ignoreBody = NULL;
		bool allObjects = false;
		int maxResults = 0;
		const char *error = "usage: rayCast( Number worldX, Number worldY, Number directionX, Number directionY, [ Body ignoreBody | Boolean allObjects, [ Int maxResults ] ] )";
		if ( !sa.ReadArguments( 4, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dx, TypeFloat, &dy ) ) {
			script.ReportError( error );
			return false;
		}
		
		// ignoreBody / allObjects
		if ( sa.args.size() >= 4 ) {
			if ( !sa.ReadArgumentsFrom( 4, 1, TypeObject, &ignoreBody, TypeInt, &maxResults ) ) {
				if ( !sa.ReadArgumentsFrom( 4, 1, TypeBool, &allObjects, TypeInt, &maxResults ) ) {
					script.ReportError( error );
					return false;
				}
			}
		}
		
		// call
		ArgValueVector* res = NULL;
		if ( allObjects ) {
			res = s->RayCastAll( x, y, dx, dy, maxResults );
		} else {
			res = s->RayCast( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE, dx * WORLD_TO_BOX2D_SCALE, dy * WORLD_TO_BOX2D_SCALE, maxResults, ignoreBody );
		}
		sa.ReturnArray( *res );
		delete res;
		return true;
	}));
	
	script.DefineFunction<Scene>
	("query",
	 static_cast<ScriptFunctionCallback>([]( void* p, ScriptArguments& sa ){
		Scene* s = (Scene*) p;
		// arguments
		float x = 0, y = 0, dx = 0, dy = 0;
		void* ignoreBody = NULL;
		bool allObjects = false;
		int maxResults = 0;
		const char *error = "usage: query( Number worldX, Number worldY, Number width, Number height, [ Body ignoreBody | Boolean allObjects, [ Int maxResults ] ] )";
		if ( !sa.ReadArguments( 4, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dx, TypeFloat, &dy ) ) {
			script.ReportError( error );
			return false;
		}
		
		// ignoreBody / allObjects
		if ( sa.args.size() >= 4 ) {
			if ( !sa.ReadArgumentsFrom( 4, 1, TypeObject, &ignoreBody, TypeInt, &maxResults ) ) {
				if ( !sa.ReadArgumentsFrom( 4, 1, TypeBool, &allObjects, TypeInt, &maxResults ) ) {
					script.ReportError( error );
					return false;
				}
			}
		}
		
		// call
		ArgValueVector* res = NULL;
		if ( allObjects ) {
			res = s->QueryAll( x, y, dx, dy, maxResults );
		} else {
			res = s->Query( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE, dx * WORLD_TO_BOX2D_SCALE, dy * WORLD_TO_BOX2D_SCALE, maxResults, ignoreBody );
		}
		sa.ReturnArray( *res );
		delete res;
		return true;
	}));
	
}


/* MARK:	-				Garbage collection
 -------------------------------------------------------------------- */


void Scene::TraceProtectedObjects( vector<void **> &protectedObjects ) {
	
	// focused ui
	if ( this->focusedUI ) protectedObjects.push_back( &this->focusedUI->scriptObject );
	
	// call super
	GameObject::TraceProtectedObjects( protectedObjects );
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


/// overridden to also dispatch to overlay
void Scene::DispatchEvent( Event& event, bool callOnSelf, GameObjectCallback *forEachGameObject) {
	
	// set self
	event.scene = this;
	
	// let overlay at event first
	app.overlay->DispatchEvent( event, callOnSelf, forEachGameObject );
	
	// continue default behavior
	if ( !event.stopped ) GameObject::DispatchEvent( event, callOnSelf, forEachGameObject );
	
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
	SDL_Color &color = this->backgroundColor->rgba;
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
	if ( app.debugDraw ) {
		GPU_ActivateShaderProgram( 0, NULL );
		this->world->DrawDebugData();
	}
	
	// restore
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
	
	// draw overlay
	app.overlay->Render( event );
	
}


/* MARK:	-				Physics
 -------------------------------------------------------------------- */


/// adds results to _raycastResult, and calls callback
float32 Scene::ReportFixture( b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction ) {
	RigidBodyShape *shape = (RigidBodyShape*) fixture->GetUserData();
	if ( !shape ) return 1; // ignore
	
	// check if should be ignored
	if ( shape->scriptObject == _raycastIgnore || shape->body->scriptObject == _raycastIgnore || shape->body->gameObject->scriptObject == _raycastIgnore ) return 1;
	
	// add result
	_RayCastResult& r = _raycastResult[ fraction ];
	r.point = point * BOX2D_TO_WORLD_SCALE;
	r.normal = normal;
	r.shape = shape;
	
	// return
	_maxRaycastResults--;
	if ( _maxRaycastResults == 0 ) return fraction; // max results achieved
	return 1; // keep going
}

// returns new vector (delete after use)
ArgValueVector* Scene::RayCast( float x, float y, float dx, float dy, int maxResults, void* ignoreBody, GameObject* descendentsOf ) {
	_raycastIgnore = ignoreBody;
	_maxRaycastResults = maxResults;
	_raycastResult.clear();
	b2Vec2 p1 = { x, y };
	b2Vec2 p2 = { x + dx, y + dy };
	this->world->RayCast( this, p1, p2 );
	
	// populate and return results
	ArgValueVector* ret = new ArgValueVector();
	ret->resize( _raycastResult.size() );
	ArgValueVector::iterator vi = ret->begin();
	map<float,_RayCastResult>::iterator it = _raycastResult.begin(), end = _raycastResult.end();
	while( it != end ) {
		_RayCastResult& r = it->second;
		if ( !descendentsOf || ( descendentsOf && r.shape->body->gameObject->IsDescendantOf( descendentsOf ) ) ) {
			ArgValue& val = *vi;
			val.type = TypeObject;
			void* obj = val.value.objectValue = script.NewObject();
			script.SetProperty( "shape", ArgValue( r.shape->scriptObject ), obj );
			script.SetProperty( "body", ArgValue( r.shape->body->scriptObject ), obj );
			script.SetProperty( "gameObject", ArgValue( r.shape->body->gameObject->scriptObject ), obj );
			script.SetProperty( "x", ArgValue( r.point.x ), obj );
			script.SetProperty( "y", ArgValue( r.point.y ), obj );
			script.SetProperty( "normalX", ArgValue( r.normal.x ), obj );
			script.SetProperty( "normalY", ArgValue( r.normal.y ), obj );
		}
		it++; vi++;
	}
	return ret;
}

bool Scene::ReportFixture( b2Fixture* fixture ) {
	RigidBodyShape *shape = (RigidBodyShape*) fixture->GetUserData();
	if ( !shape ) return true; // ignore
	
	// check if should be ignored
	if ( shape->scriptObject == _raycastIgnore || shape->body->scriptObject == _raycastIgnore || shape->body->gameObject->scriptObject == _raycastIgnore ) return true;
	
	// add result
	_RayCastResult& r = _raycastResult[ (float) _maxRaycastResults ];
	r.shape = shape;
	
	// return
	_maxRaycastResults--;
	if ( _maxRaycastResults == 0 ) return false; // max results achieved
	return true; // keep going
}

ArgValueVector* Scene::Query( float x, float y, float w, float h, int maxResults, void *ignoreBody, GameObject* descendentsOf ) {
	_raycastIgnore = ignoreBody;
	_maxRaycastResults = maxResults;
	_raycastResult.clear();
	b2AABB aabb;
	aabb.lowerBound.Set( x, y );
	aabb.upperBound.Set( x + fmax( w, 1 ), y + fmax( h, 1 ) );
	this->world->QueryAABB( this, aabb );
	
	// populate and return results
	ArgValueVector* ret = new ArgValueVector();
	ret->resize( _raycastResult.size() );
	ArgValueVector::iterator vi = ret->begin();
	map<float,_RayCastResult>::iterator it = _raycastResult.begin(), end = _raycastResult.end();
	while( it != end ) {
		_RayCastResult& r = it->second;
		if ( !descendentsOf || ( descendentsOf && r.shape->body->gameObject->IsDescendantOf( descendentsOf ) ) ) {
			ArgValue& val = *vi;
			val.type = TypeObject;
			void* obj = val.value.objectValue = script.NewObject();
			script.SetProperty( "shape", ArgValue( r.shape->scriptObject ), obj );
			script.SetProperty( "body", ArgValue( r.shape->body->scriptObject ), obj );
			script.SetProperty( "gameObject", ArgValue( r.shape->body->gameObject->scriptObject ), obj );
		}
		it++; vi++;
	}
	return ret;
}

bool Scene::ShouldCollide( b2Fixture* a, b2Fixture* b ) {
	RigidBodyShape *shapeA = (RigidBodyShape*) a->GetUserData();
	RigidBodyShape *shapeB = (RigidBodyShape*) b->GetUserData();
	if ( !shapeA || !shapeA->body || !shapeA->body->gameObject ||
		!shapeB || !shapeB->body || !shapeB->body->gameObject ) return true;
	RigidBodyBehavior* bodyA = shapeA->body;
	RigidBodyBehavior* bodyB = shapeB->body;
	// get category and mask bits from shape
	long unsigned catA = shapeA->categoryBits;
	long unsigned maskA = shapeA->maskBits;
	long unsigned catB = shapeB->categoryBits;
	long unsigned maskB = shapeB->maskBits;
	// if not set, use body's
	if ( !catA ) {
		catA = bodyA->categoryBits;
		maskA = bodyA->maskBits;
	}
	if ( !catB ) {
		catB = bodyB->categoryBits;
		maskB = bodyB->maskBits;
	}
	// do bitwise check
	return (catA & maskB) && (catB & maskA);
}


/// Called when two fixtures begin to touch.
void Scene::BeginContact( b2Contact* contact ) {
	b2Fixture* a = contact->GetFixtureA();
	b2Fixture* b = contact->GetFixtureB();
	RigidBodyShape *shapeA = a ? (RigidBodyShape*) a->GetUserData() : NULL;
	RigidBodyShape *shapeB = b ? (RigidBodyShape*) b->GetUserData() : NULL;
	if ( !contact->IsTouching() || !shapeA || !shapeA->body || !shapeA->body->gameObject ||
					!shapeB || !shapeB->body || !shapeB->body->gameObject ) return;
	b2Vec2 point, normal = { 0, 0 };
	float separation = 0;
	if ( shapeA->isSensor ) {
		point = b->GetBody()->GetPosition();
	} else if ( shapeB->isSensor ) {
		point = a->GetBody()->GetPosition();
	} else {
		b2WorldManifold worldManifold;
		contact->GetWorldManifold( &worldManifold );
		point = worldManifold.points[ 0 ];
		normal = worldManifold.normal;
		separation = worldManifold.separations[ 0 ];
	}
	point *= BOX2D_TO_WORLD_SCALE;
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
		if ( !event.stopped ) {
			event.scriptParams.ResizeArguments( 0 );
			event.scriptParams.AddObjectArgument( shapeB->scriptObject );
			event.scriptParams.AddObjectArgument( shapeA->scriptObject );
			event.scriptParams.AddFloatArgument( point.x );
			event.scriptParams.AddFloatArgument( point.y );
			event.scriptParams.AddFloatArgument( normal.x );
			event.scriptParams.AddFloatArgument( normal.y );
			event.scriptParams.AddFloatArgument( separation );
			shapeB->body->CallEvent( event );
		}
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
	physicsEvents.emplace_back
	( static_cast<PhysicsEventCallback>([ shapeA, shapeB ](){
		Event event( EVENT_UNTOUCH );
		event.scriptParams.AddObjectArgument( shapeA->scriptObject );
		event.scriptParams.AddObjectArgument( shapeB->scriptObject );
		shapeA->body->CallEvent( event );
		if ( !event.stopped ) {
			event.scriptParams.ResizeArguments( 0 );
			event.scriptParams.AddObjectArgument( shapeB->scriptObject );
			event.scriptParams.AddObjectArgument( shapeA->scriptObject );
			shapeB->body->CallEvent( event );
		}
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


/* MARK:	-				Box2d debug draw
 -------------------------------------------------------------------- */


/// Draw a closed polygon provided in CCW order.
void Scene::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	
	vertexCount = FillVertsBuffer( vertices, vertexCount );
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Polyline( app.backScreen->target, vertexCount, this->verts, clr, true );
	
}

/// Draw a solid closed polygon provided in CCW order.
void Scene::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	
	vertexCount = FillVertsBuffer( vertices, vertexCount );
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_PolygonFilled( app.backScreen->target, vertexCount, this->verts, clr);
	
}

/// Draw a circle.
void Scene::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
	
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Circle( app.backScreen->target, center.x * BOX2D_TO_WORLD_SCALE, center.y * BOX2D_TO_WORLD_SCALE, radius * BOX2D_TO_WORLD_SCALE, clr);
	
}

/// Draw a solid circle.
void Scene::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
	
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_CircleFilled( app.backScreen->target, center.x * BOX2D_TO_WORLD_SCALE, center.y * BOX2D_TO_WORLD_SCALE, radius * BOX2D_TO_WORLD_SCALE, clr);
	
}

/// Draw a line segment.
void Scene::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
	
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Line( app.backScreen->target, p1.x * BOX2D_TO_WORLD_SCALE, p1.y * BOX2D_TO_WORLD_SCALE,
			 p2.x * BOX2D_TO_WORLD_SCALE, p2.y * BOX2D_TO_WORLD_SCALE, clr);
	
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void Scene::DrawTransform(const b2Transform& xf) {
}

void Scene::DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) {
}

void Scene::DrawPoint(const b2Vec2 &p, float32 size, const b2Color &color) {
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Pixel(app.backScreen->target, p.x * BOX2D_TO_WORLD_SCALE, p.y * BOX2D_TO_WORLD_SCALE, clr );
}

int Scene::FillVertsBuffer( const b2Vec2* vertices, int32 vertexCount ) {
	
	int i;
	for ( i = 0; i < vertexCount && i < MAX_DEBUG_POLY_VERTS; i++ ) {
		this->verts[ i * 2 ] = vertices[ i ].x * BOX2D_TO_WORLD_SCALE;
		this->verts[ i * 2 + 1 ] = vertices[ i ].y * BOX2D_TO_WORLD_SCALE;
	}
	
	return ( i );
}

