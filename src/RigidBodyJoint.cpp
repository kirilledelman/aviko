#include "RigidBodyJoint.hpp"
#include "RigidBodyBehavior.hpp"
#include "GameObject.hpp"
#include "Scene.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


RigidBodyJoint::RigidBodyJoint( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<RigidBodyJoint>( this );
	
	// arguments?
	if ( args ) {
		
		int type = (int) b2JointType::e_unknownJoint;
		void *obj = NULL;
		RigidBodyBehavior* beh = NULL;
		if ( args->ReadArguments( 1, TypeInt, &type ) ) {
			
			// params vary based on joint type
			jointType = (b2JointType) type;
			
			// JOINT_MOUSE - expects x, y world anchor point
			if ( jointType == b2JointType::e_mouseJoint ) {
				args->ReadArgumentsFrom( 1, 2, TypeFloat, &mouseDef.target.x, TypeFloat, &mouseDef.target.y );
				mouseDef.target *= WORLD_TO_BOX2D_SCALE;
				
			// JOINT_REVOLUTE, JOINT_PRISMATIC, JOINT_DISTANCE - expects world pin point x, y | body local pin x, y, other body local pin x, y, followed by other body
			} else if ( jointType == b2JointType::e_revoluteJoint ||
					   jointType == b2JointType::e_prismaticJoint ||
					   jointType == b2JointType::e_distanceJoint ) {
				b2Vec2 p1 = { 0, 0 }, p2 = { 0, 0 };
				// if given two points + body - these are local anchors
				if ( args->ReadArgumentsFrom( 1, 5, TypeFloat, &p1.x, TypeFloat, &p1.y, TypeFloat, &p2.x, TypeFloat, &p2.y, TypeObject, &obj ) ){
					beh = script.GetInstance<RigidBodyBehavior>( obj );
					if ( obj && !beh ) {
						GameObject* go = script.GetInstance<GameObject>( obj );
						if ( go ) beh = (RigidBodyBehavior*) go->body;
					}
					// set them as local anchors
					anchorA.Set( p1.x * WORLD_TO_BOX2D_SCALE, p1.y * WORLD_TO_BOX2D_SCALE );
					anchorB.Set( p2.x * WORLD_TO_BOX2D_SCALE, p2.y * WORLD_TO_BOX2D_SCALE );
				// given one point + other body - global coords
				} else if ( args->ReadArgumentsFrom( 1, 3, TypeFloat, &p1.x, TypeFloat, &p1.y, TypeObject, &obj ) ) {
					beh = script.GetInstance<RigidBodyBehavior>( obj );
					if ( obj && !beh ) {
						GameObject* go = script.GetInstance<GameObject>( obj );
						if ( go ) beh = (RigidBodyBehavior*) go->body;
					}
					// body provided
					if ( beh && beh->gameObject ){
						// set anchorA to nan ( means convert localAnchorB to world and then to local when attaching )
						anchorA.Set( nan(NULL), nan(NULL) );
						beh->gameObject->ConvertPoint( p1.x, p1.y, anchorB.x, anchorB.y, false );
						anchorB *= WORLD_TO_BOX2D_SCALE;
					}
				}
				this->SetOtherBody( beh );
			
			// JOINT_WELD
			} else if ( jointType == b2JointType::e_weldJoint ) {
				b2Vec2 p1 = { 0, 0 };
				// one point + body, this point is anchorB
				if ( args->ReadArgumentsFrom( 1, 3, TypeFloat, &p1.x, TypeFloat, &p1.y, TypeObject, &obj ) ) {
					anchorA.Set( 0, 0 );
					anchorB = p1 * WORLD_TO_BOX2D_SCALE;
				// just other body, weld at current position
				} else if ( args->ReadArgumentsFrom( 1, 1, TypeObject, &obj ) ) {
					// set anchorA to nan ( means convert localAnchorB to world and then to local when attaching )
					anchorA.Set( nan(NULL), nan(NULL) );
					anchorB.Set( 0, 0 );
				}
				// ensure body
				beh = script.GetInstance<RigidBodyBehavior>( obj );
				if ( obj && !beh ) {
					GameObject* go = script.GetInstance<GameObject>( obj );
					if ( go ) beh = (RigidBodyBehavior*) go->body;
				}
				this->SetOtherBody( beh );
			}
			
		}
	}
}

RigidBodyJoint::~RigidBodyJoint() {
	
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void RigidBodyJoint::InitClass() {
	
	// register class
	script.RegisterClass<RigidBodyJoint>( "ScriptableObject" );
	
	script.SetGlobalConstant( "JOINT_MOUSE", (int) b2JointType::e_mouseJoint );
	script.SetGlobalConstant( "JOINT_REVOLUTE", (int) b2JointType::e_revoluteJoint );
	script.SetGlobalConstant( "JOINT_PRISMATIC", (int) b2JointType::e_prismaticJoint );
	script.SetGlobalConstant( "JOINT_DISTANCE", (int) b2JointType::e_distanceJoint );
	script.SetGlobalConstant( "JOINT_WELD", (int) b2JointType::e_weldJoint );
	
	// props
	
	script.AddProperty<RigidBodyJoint>
	( "type",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((RigidBodyJoint*) b)->jointType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		val = max( 0, min( (int) b2JointType::e_motorJoint, val ));
		rb->jointType = (b2JointType) val;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "body",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RigidBodyJoint*) b)->body ? ((RigidBodyJoint*) b)->body->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		RigidBodyBehavior* beh = script.GetInstance<RigidBodyBehavior>( val );
		if ( !beh ) {
			GameObject* go = script.GetInstance<GameObject>( val );
			beh = (RigidBodyBehavior*)(go ? go->body : NULL);
			if (beh && beh->scriptClassName[ 0 ] != 'B' ) beh = NULL; // hacky
		}
		rb->SetBody( beh );
		return rb->body ? rb->body->scriptObject : NULL;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "otherBody",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RigidBodyJoint*) b)->otherBody ? ((RigidBodyJoint*) b)->otherBody->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		RigidBodyBehavior* beh = script.GetInstance<RigidBodyBehavior>( val );
		if ( !beh ) {
			GameObject* go = script.GetInstance<GameObject>( val );
			beh = (RigidBodyBehavior*)(go ? go->body : NULL);
			if (beh && beh->scriptClassName[ 0 ] != 'B' ) beh = NULL; // hacky
		}
		rb->SetOtherBody( beh );
		return rb->otherBody ? rb->otherBody->scriptObject : NULL;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "collide",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RigidBodyJoint*) b)->collideConnected; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->collideConnected = val;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "anchorX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->anchorA.x * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->anchorA.x = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "anchorY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->anchorA.y * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->anchorA.y = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "otherAnchorX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->anchorB.x * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->anchorB.x = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "otherAnchorY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->anchorB.y * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->anchorB.y = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "limit",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RigidBodyJoint*) b)->enableLimit; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->enableLimit = val;
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->EnableLimit( val );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->EnableLimit( val );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "lowerLimit",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->lowerLimit; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->lowerLimit = val;
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->SetLimits( rb->lowerLimit * DEG_TO_RAD, rb->upperLimit * DEG_TO_RAD );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->SetLimits( rb->lowerLimit * WORLD_TO_BOX2D_SCALE, rb->upperLimit * WORLD_TO_BOX2D_SCALE );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "upperLimit",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->upperLimit; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->upperLimit = val;
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->SetLimits( rb->lowerLimit * DEG_TO_RAD, rb->upperLimit * DEG_TO_RAD );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->SetLimits( rb->lowerLimit * WORLD_TO_BOX2D_SCALE, rb->upperLimit * WORLD_TO_BOX2D_SCALE );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "motorSpeed",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->motorSpeed; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->motorSpeed = val;
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->SetMotorSpeed( val * DEG_TO_RAD );
					((b2RevoluteJoint*)rb->joint)->EnableMotor( val != 0 );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->SetMotorSpeed( val * DEG_TO_RAD );
					((b2PrismaticJoint*)rb->joint)->EnableMotor( val != 0 );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "maxForce",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->maxForce; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->maxForce = fmax( 0, val );
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->SetMaxMotorTorque( val );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->SetMaxMotorForce( val );
					break;
				case b2JointType::e_mouseJoint:
					((b2MouseJoint*)rb->joint)->SetMaxForce( val );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "damping",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->damping; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->damping = fmax( 1, val );
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_weldJoint:
					((b2WeldJoint*)rb->joint)->SetDampingRatio( val );
					break;
				case b2JointType::e_distanceJoint:
					((b2DistanceJoint*)rb->joint)->SetDampingRatio( val );
					break;
				case b2JointType::e_mouseJoint:
					((b2MouseJoint*)rb->joint)->SetDampingRatio( val );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "frequency",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->frequency; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->frequency = fmax( 1, val );
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_weldJoint:
					((b2WeldJoint*)rb->joint)->SetFrequency( val );
					break;
				case b2JointType::e_distanceJoint:
					((b2DistanceJoint*)rb->joint)->SetFrequency( val );
					break;
				case b2JointType::e_mouseJoint:
					((b2MouseJoint*)rb->joint)->SetFrequency( val );
					break;
				default: break;
			}
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "mouseTargetX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->mouseDef.target.x * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->mouseDef.target.x = val * WORLD_TO_BOX2D_SCALE;
		if ( rb->jointType == b2JointType::e_mouseJoint && rb->joint ) {
			((b2MouseJoint*)rb->joint)->SetTarget( rb->mouseDef.target );
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "mouseTargetY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->mouseDef.target.y * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->mouseDef.target.y = val * WORLD_TO_BOX2D_SCALE;
		if ( rb->jointType == b2JointType::e_mouseJoint && rb->joint ) {
			((b2MouseJoint*)rb->joint)->SetTarget( rb->mouseDef.target );
		}
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "axisX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->axis.x; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->axis.x = val;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "axisY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->axis.y; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->axis.y = val;
		rb->UpdateJoint();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyJoint>
	( "distance",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyJoint*) b)->distance; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyJoint* rb = (RigidBodyJoint*) b;
		rb->distance = val;
		if ( rb->jointType == b2JointType::e_distanceJoint && rb->joint ) {
			((b2DistanceJoint*)rb->joint)->SetLength( fmax( 1, rb->distance * WORLD_TO_BOX2D_SCALE ) );
		}
		return val;
	} ) );
	
	// functions
	
	script.DefineFunction<RigidBodyJoint>
	("setAxis",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* self = (RigidBodyJoint*) p;
		float x = 0, y = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ){
			script.ReportError( "usage: setAxis( Number x, Number y )" );
			return false;
		}
		self->axis.Set( x, y );
		self->UpdateJoint();
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	("setAnchor",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* self = (RigidBodyJoint*) p;
		float x = 0, y = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ){
			script.ReportError( "usage: setAnchor( Number x, Number y )" );
			return false;
		}
		self->anchorA.Set( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE );
		self->UpdateJoint();
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	("setOtherAnchor",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* self = (RigidBodyJoint*) p;
		float x = 0, y = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ){
			script.ReportError( "usage: setOtherAnchor( Number x, Number y )" );
			return false;
		}
		self->anchorB.Set( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE );
		self->UpdateJoint();
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	("setMouseTarget",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* self = (RigidBodyJoint*) p;
		float x = 0, y = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ){
			script.ReportError( "usage: setMouseTarget( Number x, Number y )" );
			return false;
		}
		self->mouseDef.target.Set( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE );
		if ( self->jointType == b2JointType::e_mouseJoint && self->joint ) {
			((b2MouseJoint*)self->joint)->SetTarget( self->mouseDef.target );
		}
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	("setLimits",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* rb = (RigidBodyJoint*) p;
		float a = 0, b = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &a, TypeFloat, &b ) ){
			script.ReportError( "usage: setLimits( Number lower, Number upper )" );
			return false;
		}
		rb->lowerLimit = a;
		rb->upperLimit = b;
		if ( rb->joint ) {
			switch ( rb->jointType ) {
				case b2JointType::e_revoluteJoint:
					((b2RevoluteJoint*)rb->joint)->SetLimits( rb->lowerLimit * DEG_TO_RAD, rb->upperLimit * DEG_TO_RAD );
					break;
				case b2JointType::e_prismaticJoint:
					((b2PrismaticJoint*)rb->joint)->SetLimits( rb->lowerLimit * WORLD_TO_BOX2D_SCALE, rb->upperLimit * WORLD_TO_BOX2D_SCALE );
					break;
				default: break;
			}
		}
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	("destroy",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyJoint* self = (RigidBodyJoint*) p;
		self->SetBody( NULL );
		self->SetOtherBody( NULL );
		return true;
	}));
	
	script.DefineFunction<RigidBodyJoint>
	( "toString",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ) {
		static char buf[512];
		RigidBodyJoint* self = (RigidBodyJoint*) o;
		if ( !self ) {
			sprintf( buf, "[RigidBodyJoint prototype]" );
		} else {
			static const char* jointNames[] = {
				"Unknown",
				"Revolute",
				"Prismatic",
				"Distance",
				"Pulley",
				"Mouse",
				"Gear",
				"Wheel",
				"Weld",
				"Friction",
				"Rope",
				"Motor"
			};
			sprintf( buf, "[Joint (%s) %p]", jointNames[ self->jointType ], self );
		}
		sa.ReturnString( buf );
		return true;
	}));
	
}

void RigidBodyJoint::TraceProtectedObjects( vector<void**> &protectedObjects ) {

	if ( this->body ) protectedObjects.push_back( &this->body->scriptObject );
	if ( this->otherBody ) protectedObjects.push_back( &this->otherBody->scriptObject );
	
}


/* MARK:	-				Attachment
 -------------------------------------------------------------------- */


// set first body
void RigidBodyJoint::SetBody( RigidBodyBehavior* newBody ) {
	
	// if different
	if ( newBody != this->body && !( this->otherBody && newBody == this->otherBody ) ) {
		
		// if had body
		RigidBodyBehavior* oldBody = this->body;
		if ( oldBody ) {
			
			// find this object in old list of joints
			vector<RigidBodyJoint*> *oldList = &oldBody->joints;
			vector<RigidBodyJoint*>::iterator
			listEnd = oldList->end(),
			it = find( oldList->begin(), listEnd, this );
			
			// remove from list
			if ( it != listEnd ) oldList->erase( it );
			
			// clear
			this->body = NULL;
			
		}
		
		// set body
		this->body = newBody;
		
		// add to new body
		if ( newBody ) newBody->joints.push_back( this );
		
		// recreate
		this->UpdateJoint();
		
	}
	
}

void RigidBodyJoint::SetOtherBody( RigidBodyBehavior* newBody ) {

	// if different
	if ( newBody != this->otherBody && !( this->body && newBody == this->body ) ) {
		
		// if had body
		RigidBodyBehavior* oldBody = this->otherBody;
		if ( oldBody ) {
			
			// find this object in old list of joints
			vector<RigidBodyJoint*> *oldList = &oldBody->otherJoints;
			vector<RigidBodyJoint*>::iterator
			listEnd = oldList->end(),
			it = find( oldList->begin(), listEnd, this );
			
			// remove from list
			if ( it != listEnd ) oldList->erase( it );
			
			// clear
			this->otherBody = NULL;
			
		}
		
		// set body
		this->otherBody = newBody;
		
		// add to new body
		if ( newBody ) newBody->otherJoints.push_back( this );
		
		// recreate
		this->UpdateJoint();
		
	}
	
}


/* MARK:	-				Joint
 -------------------------------------------------------------------- */


// destroys and remakes the joint
void RigidBodyJoint::UpdateJoint() {
	
	// destroy old
	if ( this->joint ) {
		b2Body* b = this->joint->GetBodyA();
		if ( !b ) b = this->joint->GetBodyB();
		if ( b ) b->GetWorld()->DestroyJoint( this->joint );
		this->joint = NULL;
	}
	
	// create new
	if ( body && body->live && ( this->jointType == b2JointType::e_mouseJoint || ( otherBody && otherBody->live ) ) ) {
		
		Scene* scene = body->gameObject->GetScene();
		b2World* world = body->body->GetWorld();
		b2JointDef *jointDef = NULL;
		
		// if anchorA is nan
		if ( isnan( anchorA.x ) ) {
			// convert anchorB to global, then back to local anchorA
			if ( body->gameObject && otherBody->gameObject ) {
				float gx = 0, gy = 0;
				otherBody->gameObject->ConvertPoint( anchorB.x * BOX2D_TO_WORLD_SCALE, anchorB.y * BOX2D_TO_WORLD_SCALE, gx, gy, true );
				body->gameObject->ConvertPoint( gx, gy, anchorA.x, anchorA.y, false );
				anchorA *= WORLD_TO_BOX2D_SCALE;
			} else {
				anchorA.Set( 0, 0 );
			}
		}
		
		// JOINT_MOUSE
		if ( jointType == b2JointType::e_mouseJoint ) {
			
			mouseDef.bodyB = body->body;
			mouseDef.bodyA = scene->groundBody;
			mouseDef.maxForce = this->maxForce;
			mouseDef.dampingRatio = damping;
			mouseDef.frequencyHz = frequency;
			jointDef = &mouseDef;
			
		// JOINT_REVOLUTE
		} else if ( jointType == b2JointType::e_revoluteJoint ) {
			
			revoluteDef.bodyA = body->body;
			revoluteDef.bodyB = otherBody->body;
			revoluteDef.enableLimit = enableLimit;
			revoluteDef.lowerAngle = DEG_TO_RAD * lowerLimit;
			revoluteDef.upperAngle = DEG_TO_RAD * upperLimit;
			revoluteDef.enableMotor = ( motorSpeed != 0 );
			revoluteDef.motorSpeed = motorSpeed * DEG_TO_RAD;
			revoluteDef.maxMotorTorque = maxForce;
			revoluteDef.localAnchorA = anchorA;
			revoluteDef.localAnchorB = anchorB;
			jointDef = &revoluteDef;
			
		// JOINT_PRISMATIC
		} else if ( jointType == b2JointType::e_prismaticJoint ) {
			
			prismaticDef.bodyA = body->body;
			prismaticDef.bodyB = otherBody->body;
			prismaticDef.enableLimit = enableLimit;
			prismaticDef.lowerTranslation = lowerLimit * WORLD_TO_BOX2D_SCALE;
			prismaticDef.upperTranslation = upperLimit * WORLD_TO_BOX2D_SCALE;
			prismaticDef.enableMotor = ( motorSpeed != 0 );
			prismaticDef.motorSpeed = motorSpeed;
			prismaticDef.maxMotorForce = maxForce;
			prismaticDef.localAxisA = axis;
			prismaticDef.localAxisA.Normalize();
			prismaticDef.localAnchorA = anchorA;
			prismaticDef.localAnchorB = anchorB;
			jointDef = &prismaticDef;
		
		// JOINT_DISTANCE
		} else if ( jointType == b2JointType::e_distanceJoint ) {
			
			distanceDef.bodyA = body->body;
			distanceDef.bodyB = otherBody->body;
			distanceDef.localAnchorA = anchorA;
			distanceDef.localAnchorB = anchorB;
			distanceDef.length = distance * WORLD_TO_BOX2D_SCALE;
			distanceDef.dampingRatio = damping;
			distanceDef.frequencyHz = frequency;
			jointDef = &distanceDef;
			
		// JOINT_WELD
		} else if ( jointType == b2JointType::e_weldJoint ) {
			
			weldDef.bodyA = body->body;
			weldDef.bodyB = otherBody->body;
			weldDef.localAnchorA = anchorA;
			weldDef.localAnchorB = anchorB;
			weldDef.referenceAngle = otherBody->body->GetAngle() - body->body->GetAngle();
			weldDef.dampingRatio = damping;
			weldDef.frequencyHz = frequency;
			jointDef = &weldDef;
			
		}
		
		// construct
		if ( jointDef ) {
			
			jointDef->collideConnected = collideConnected;
			this->joint = world->CreateJoint( jointDef );
			
		}
		
	}
	
}

