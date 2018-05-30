#ifndef Scene_hpp
#define Scene_hpp

#include "common.h"
#include "GameObject.hpp"
#include "Color.hpp"
#include "UIBehavior.hpp"

#define MAX_DEBUG_POLY_VERTS 512

class Scene : public GameObject, b2Draw, b2ContactListener, b2ContactFilter, b2RayCastCallback, b2QueryCallback {
public:

	// init, destroy
	Scene( ScriptArguments* args );
	Scene();
	~Scene();
	
// Javascript
	
	/// registers classes for scripting
	static void InitClass();
	
	/// garbage collection callback
	void TraceProtectedObjects( vector<void **> &protectedObjects );

// UI elements
	
	// current focus
	UIBehavior* focusedUI = NULL;
	
// physics
	
	/// Box2D world
	b2World* world = NULL;
	
	b2Vec2 gravity;
	
	b2Body *groundBody = NULL;
	
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
	
	/// raycasts from x, y in dx, dy direction, returns up to maxResults Objects with bodies with info
	ArgValueVector* RayCast( float x, float y, float dx, float dy, int maxResults, void* ignoreBody, GameObject* descendentsOf=NULL );
	
	/// returns bodies in area, similar to rayCast
	ArgValueVector* Query( float x, float y, float w, float h, int maxResults, void* ignoreBody, GameObject* descendentsOf=NULL );
		
	// holds results of raycast throughout query
	struct _RayCastResult {
		b2Vec2 point = { 0, 0 };
		b2Vec2 normal = { 0, 0 };
		RigidBodyShape* shape = NULL;
	};
	map<float,_RayCastResult> _raycastResult;
	void* _raycastIgnore = NULL;
	int _maxRaycastResults = 0;
	
// box2d debug draw
	
	/// buffer to hold body verts for drawing
	float verts[ MAX_DEBUG_POLY_VERTS * 2 ];
	
	/// helper to copy b2Vec2 vertices to verts buffer for drawing
	int FillVertsBuffer( const b2Vec2* vertices, int32 vertexCount );
	
	/// Draw a closed polygon provided in CCW order.
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	
	/// Draw a solid closed polygon provided in CCW order.
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	
	/// Draw a circle.
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	
	/// Draw a solid circle.
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	
	/// Draw a particle array
	void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count);
	
	/// Draw a line segment.
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	
	/// Draw a transform. Choose your own length scale.
	void DrawTransform(const b2Transform& xf);
	
	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);
	
// rendering
	
	// clear color
	Color *backgroundColor = NULL;
	
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

// events
	
	/// overridden from GameObject to affect overlay
	void DispatchEvent( Event& event, bool callOnSelf=false, GameObjectCallback *forEachGameObject=NULL);
	
// hierarchy
	
	/// returns scene
	Scene* GetScene() { return this; }
	
};

SCRIPT_CLASS_NAME( Scene, "Scene" );

#endif /* Scene_hpp */
