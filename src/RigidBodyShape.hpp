
#ifndef RigidBodyShape_hpp
#define RigidBodyShape_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "RenderShapeBehavior.hpp"

class RigidBodyBehavior;

///
class RigidBodyShape : public ScriptableClass {
public:
	
	RigidBodyBehavior* body = NULL;
	vector<b2Fixture*> fixtures;
	
	void SetBody( RigidBodyBehavior* b );
	
	/// re-creates fixture, if dirty, and have body
	void UpdateFixture();
	
	// override body collision settings
	uint32 categoryBits = 0x0;
	uint32 maskBits = 0xFFFFFFFF;
	
	/// params
	float density = 5;
	float friction = 0.5;
	float restitution = 0.2;
	bool isSensor = false;
	
	/// will split concave polys in multiple fixtures
	bool splitConcave = true;
	
	RenderShapeBehavior::ShapeType shapeType = RenderShapeBehavior::ShapeType::Circle;
	
	float radius = 2;
	float width = 1;
	float height = 1;
	b2Vec2 center = { 0, 0 };
	
	TypedVector* polyPoints = NULL;
	ArgValueVector* GetPoints();
	void SetPoints( ArgValue& );
	
	// scripting
	
	static void InitClass();
	
	// garbage collector
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
	// init/destroy
	RigidBodyShape();
	RigidBodyShape( ScriptArguments* args );
	~RigidBodyShape();
	
};

SCRIPT_CLASS_NAME( RigidBodyShape, "BodyShape" );

#endif /* RigidBodyShape_hpp */
