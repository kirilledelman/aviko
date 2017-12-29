#ifndef Scene_hpp
#define Scene_hpp

#include "common.h"
#include "GameObject.hpp"
#include "Color.hpp"
#include "UIBehavior.hpp"
#include "SceneDebugDraw.hpp"

class Scene : public GameObject, b2ContactListener, b2ContactFilter, b2RayCastCallback, b2QueryCallback {
protected:
	
	/// instance of Box2D debug draw class
	SceneDebugDraw _sceneDebugDraw;
	
public:

	// init, destroy
	Scene( ScriptArguments* args );
	Scene( const char* filename ); // load from file
	Scene();
	~Scene();
	
// Javascript
	
	/// registers classes for scripting
	static void InitClass();	

// UI elements
	
	// current focus
	UIBehavior* focusedUI = NULL;
	
// physics
	
	/// Box2D world
	b2World* world = NULL;
	
	b2Vec2 gravity;
	
	typedef function<void()> PhysicsEventCallback;
	
	vector<PhysicsEventCallback> physicsEvents;
	
	/// called at the top of the frame
	void SimulatePhysics();
	
	/// Called when two fixtures begin to touch.
	void BeginContact(b2Contact* contact);
	
	/// Called when two fixtures cease to touch.
	void EndContact(b2Contact* contact);
	
	// void BeginContact(b2ParticleSystem* particleSystem, b2ParticleBodyContact* particleBodyContact);
	
	// void EndContact(b2Fixture* fixture, b2ParticleSystem* particleSystem, int32 index);
	
	// void BeginContact(b2ParticleSystem* particleSystem, b2ParticleContact* particleContact);
	
	// void EndContact(b2ParticleSystem* particleSystem, int32 indexA, int32 indexB);
	
	// void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	
	// void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
	
	/// Return true if contact calculations should be performed between these two shapes.
	bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB);
	
	/// Return true if contact calculations should be performed between a fixture and particle.  This is only called if the b2_fixtureContactListenerParticle flag is set on the particle.
	// bool ShouldCollide(b2Fixture* fixture, b2ParticleSystem* particleSystem, int32 particleIndex);
	
	/// Return true if contact calculations should be performed between two particles.  This is only called if the b2_particleContactListenerParticle flag is set on the particle.
	// bool ShouldCollide(b2ParticleSystem* particleSystem, int32 particleIndexA, int32 particleIndexB);
	
	/// AABB query callback
	bool ReportFixture(b2Fixture* fixture);
	
	// RaycastCallback
	float32 ReportFixture( b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction );
	
	/// raycasts from x, y in dx, dy direction, returns up to maxResults Objects with info
	ArgValueVector* RayCast( float x, float y, float dx, float dy, int maxResults, void* ignoreBody );
	
	/// returns bodies in volume, similar to rayCast 
	ArgValueVector* Query( float x, float y, float w, float h, int maxResults, void* ignoreBody );
	
	// holds results of raycast throughout query
	struct _RayCastResult {
		b2Vec2 point = { 0, 0 };
		b2Vec2 normal = { 0, 0 };
		RigidBodyShape* shape = NULL;
	};
	map<float,_RayCastResult> _raycastResult;
	void* _raycastIgnore = NULL;
	int _maxRaycastResults = 0;
	
// rendering
	
	/// set to true to draw Box2D debug overlay
	bool debugDraw = true;
	
	// clear color
	Color *clearColor = NULL;
	
	// camera
	float inverseCameraMatrix[ 16 ];
	float cameraMatrix[ 16 ];
	float camX = 0, camY = 0, camAngle = 0, camZoom = 1, camPivotX = 0, camPivotY = 0;
	bool _camTransformDirty = true;
	bool _inverseCamTransformDirty = true;
	float* CameraTransform();
	float* InverseCameraTransform();
	
	/// called to render all objects
	void Render( Event& event );
	
// hierarchy
	
	/// returns scene
	Scene* GetScene() { return this; }
	
};

SCRIPT_CLASS_NAME( Scene, "Scene" );

#endif /* Scene_hpp */
