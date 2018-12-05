#ifndef RigidBodyJoint_hpp
#define RigidBodyJoint_hpp

#include "common.h"
#include "ScriptableClass.hpp"

class RigidBodyBehavior;

class RigidBodyJoint : public ScriptableClass {
public:

	// default type
	b2JointType jointType = b2JointType::e_revoluteJoint;
	
	// joint
	b2Joint* joint = NULL;
	
	// bodies
	RigidBodyBehavior* body = NULL;
	RigidBodyBehavior* otherBody = NULL;

	// params
	bool collideConnected = false;
	float maxForce = 100000;
	
	b2Vec2 anchorA = { 0, 0 };
	b2Vec2 anchorB = { 0, 0 };
	float motorSpeed = 0;
	bool enableLimit = false;
	float lowerLimit = 0, upperLimit = 0;
	b2Vec2 axis = { 0, 1 };
	float distance = 25;
	float damping = 0.1;
	float frequency = 5;
	b2RevoluteJointDef revoluteDef;
	b2PrismaticJointDef prismaticDef;
	b2MouseJointDef mouseDef;
	b2DistanceJointDef distanceDef;
	b2WeldJointDef weldDef;
    b2WheelJointDef wheelDef;
	
	
	// rebuilds joint
	void UpdateJoint();
	
	void SetBody( RigidBodyBehavior* b );
	void SetOtherBody( RigidBodyBehavior* b );
	
	// script
	
	static void InitClass();
	
	virtual void TraceProtectedObjects( vector<void**> &protectedObjects );
	
	// init/destroy
	RigidBodyJoint( ScriptArguments* );
	RigidBodyJoint();
	~RigidBodyJoint();
	
};

SCRIPT_CLASS_NAME( RigidBodyJoint, "Joint" );

#endif /* RigidBodyJoint_hpp */
