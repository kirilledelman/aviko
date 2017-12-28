
#ifndef RigidBodyShape_hpp
#define RigidBodyShape_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "RenderShapeBehavior.hpp"
#include "Vector.hpp"

class RigidBodyBehavior;

///
class RigidBodyShape : public ScriptableClass {
public:
	
	RigidBodyBehavior* body = NULL;
	b2Fixture* fixture = NULL;
	
	void SetBody( RigidBodyBehavior* b );
	
	/// re-creates fixture, if dirty, and have body
	void UpdateFixture();
	
	/// params
	float density = 5;
	float friction = 0.25;
	float restitution = 0.1;
	bool isSensor = false;
	
	RenderShapeBehavior::ShapeType shapeType = RenderShapeBehavior::ShapeType::Circle;
	
	float radius = 2;
	float width = 1;
	float height = 1;
	b2Vec2 center = { 0, 0 };
	
	FloatVector* polyPoints = NULL;
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
