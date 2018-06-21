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
	
	// defaults
	this->massData.center.Set( 0, 0 );
	this->massData.mass = 10;
	this->massData.I = 0;
	
	// obj argument - init object
	void *initObj = NULL;
	if ( args && args->ReadArguments( 1, TypeObject, &initObj ) ) {
		script.CopyProperties( initObj, this->scriptObject );
	}
}

// init
RigidBodyBehavior::RigidBodyBehavior() : BodyBehavior::BodyBehavior() {}

// destroy
RigidBodyBehavior::~RigidBodyBehavior() {
	
	// clean up
	this->ReplaceShapes( NULL );
	
	// unlink joints
	while ( this->joints.size() ) this->joints[ 0 ]->SetBody( NULL );
	while ( this->otherJoints.size() ) this->otherJoints[ 0 ]->SetOtherBody( NULL );
	
	// remove body
	this->RemoveBody();
	
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void RigidBodyBehavior::InitClass() {
	
	// register class
	script.RegisterClass<RigidBodyBehavior>( "BodyBehavior" );
	
	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "BodyType", constants );
	script.SetProperty( "Dynamic", ArgValue( (int) b2BodyType::b2_dynamicBody ), constants );
	script.SetProperty( "Static", ArgValue( (int) b2BodyType::b2_staticBody ), constants );
	script.SetProperty( "Kinematic", ArgValue( (int) b2BodyType::b2_kinematicBody ), constants );
	script.FreezeObject( constants );
	
	// properties
	
	script.AddProperty<RigidBodyBehavior>
	( "type",
	 static_cast<ScriptIntCallback>([]( void* p, int val ) { return ((RigidBodyBehavior*)p)->bodyType; }),
	 static_cast<ScriptIntCallback>([]( void* p, int val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->bodyType = (b2BodyType) min( b2BodyType::b2_dynamicBody + 1, max( 0, val ) );
		if ( rb->body ) rb->body->SetType( rb->bodyType );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "anglularDamping",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) { return ((RigidBodyBehavior*)p)->angularDamping; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->angularDamping = val;
		if ( rb->body ) rb->body->SetAngularDamping( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "linearDamping",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) { return ((RigidBodyBehavior*)p)->linearDamping; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->angularDamping = val;
		if ( rb->body ) rb->body->SetLinearDamping( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "velocityX",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		return rb->GetVelocity().x;
	}),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		b2Vec2 vel = rb->GetVelocity();
		vel.x = val;
		rb->SetVelocity( vel );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "velocityY",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		return rb->GetVelocity().y;
	}),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		b2Vec2 vel = rb->GetVelocity();
		vel.y = val;
		rb->SetVelocity( vel );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "angularVelocity",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		return rb->GetAngularVelocity();
	}),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->SetAngularVelocity( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "fixedRotation",
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) { return ((RigidBodyBehavior*)p)->fixedRotation; }),
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->fixedRotation = val;
		if ( rb->body ) rb->body->SetFixedRotation( val );
		return val;
	}));

	script.AddProperty<RigidBodyBehavior>
	( "bullet",
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) { return ((RigidBodyBehavior*)p)->bullet; }),
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->bullet = val;
		if ( rb->body ) rb->body->SetBullet( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "canSleep",
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) { return ((RigidBodyBehavior*)p)->canSleep; }),
	 static_cast<ScriptBoolCallback>([]( void* p, bool val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->canSleep = val;
		if ( rb->body ) rb->body->SetSleepingAllowed( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "mass",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		if ( rb->body ) rb->body->GetMassData( &rb->massData );
		return rb->massData.mass;
	 }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->massData.mass = val;
		if ( rb->body ) rb->body->SetMassData( &rb->massData );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "massCenterX",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		if ( rb->body ) rb->body->GetMassData( &rb->massData );
		return rb->massData.center.x * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->massData.center.x = val * WORLD_TO_BOX2D_SCALE;
		if ( rb->body ) rb->body->SetMassData( &rb->massData );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "massCenterY",
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		if ( rb->body ) rb->body->GetMassData( &rb->massData );
		return rb->massData.center.y * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([]( void* p, float val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		rb->massData.center.y = val * WORLD_TO_BOX2D_SCALE;
		if ( rb->body ) rb->body->SetMassData( &rb->massData );
		return val;
	}));
	
	script.AddProperty<RigidBodyBehavior>
	( "shape",
	 static_cast<ScriptObjectCallback>([]( void* p, void* val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		if ( rb->shapes.size() ) return rb->shapes.back()->scriptObject;
		return (void*) NULL;
	 }),
	 static_cast<ScriptObjectCallback>([]( void* p, void* val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		RigidBodyShape* rbs = script.GetInstance<RigidBodyShape>( val );
		if ( val && !rbs ) {
			script.ReportError( "Body .shape can only be set to BodyShape instance, or null." );
			return (void*) NULL;
		}
		rb->ReplaceShapes( rbs );
		return val;
	 }), PROP_ENUMERABLE | PROP_NOSTORE);
	
	script.AddProperty<RigidBodyBehavior>
	( "shapes",
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((RigidBodyBehavior*) go)->GetShapesVector(); }),
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((RigidBodyBehavior*) go)->SetShapesVector( in ); }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE
	 );
	
	script.AddProperty<RigidBodyBehavior>
	( "joint",
	 static_cast<ScriptObjectCallback>([]( void* p, void* val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		if ( rb->joints.size() ) return rb->joints.back()->scriptObject;
		return (void*) NULL;
	}),
	 static_cast<ScriptObjectCallback>([]( void* p, void* val ) {
		RigidBodyBehavior* rb = (RigidBodyBehavior*)p;
		RigidBodyJoint* j = script.GetInstance<RigidBodyJoint>( val );
		if ( val && !j ) {
			script.ReportError( "Body .joint can only be set to Joint instance, or null." );
			return (void*) NULL;
		}
		rb->ReplaceJoints( j );
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE);
	
	script.AddProperty<RigidBodyBehavior>
	( "joints",
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((RigidBodyBehavior*) go)->GetJointsVector( false ); }),
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((RigidBodyBehavior*) go)->SetJointsVector( in, false ); }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE
	 );
	
	script.AddProperty<RigidBodyBehavior>
	( "otherJoints",
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((RigidBodyBehavior*) go)->GetJointsVector( true ); }),
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((RigidBodyBehavior*) go)->SetJointsVector( in, true ); }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE
	 );
	
	script.AddProperty<RigidBodyBehavior>
	( "numShapes",
	 static_cast<ScriptIntCallback>([]( void* p, int val ) { return ((RigidBodyBehavior*)p)->shapes.size(); }));
	
	script.AddProperty<RigidBodyBehavior>
	( "numJoints",
	 static_cast<ScriptIntCallback>([]( void* p, int val ) { return ((RigidBodyBehavior*)p)->joints.size(); }));
	
	// functions
	
	script.DefineFunction<RigidBodyBehavior>
	("addShape",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		void *shp = NULL;
		RigidBodyShape* shape = NULL;
		if ( sa.args.size() ) {
			if ( !sa.ReadArguments( 1, TypeObject, &shp ) || !( shape = script.GetInstance<RigidBodyShape>( shp ) ) ){
				script.ReportError( "usage: addShape( [ BodyShape instance ] )" );
				return false;
			}
		} else {
			shape = new RigidBodyShape( NULL );
		}
		// add and return it
		shape->SetBody( self );
		sa.ReturnObject( shape->scriptObject );
		return true;
	} ));
	
	script.DefineFunction<RigidBodyBehavior>
	("removeShape",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		void *shp = NULL;
		RigidBodyShape* shape = NULL;
		int index = -1;
		const char *error = "usage: removeShape( BodyShape instance | Int index )";
		if ( sa.ReadArguments( 1, TypeObject, &shp ) ){
			shape = script.GetInstance<RigidBodyShape>( shp );
			if ( !shape ) {
				script.ReportError( error );
				return false;
			}
		} else if ( sa.ReadArguments( 1, TypeInt, &index ) ) {
			if ( index < 0 || index >= self->shapes.size() ) {
				// index out of range, return null
				sa.ReturnObject( NULL );
				return true;
			}
			shape = self->shapes[ index ];
		}
		shape->SetBody( NULL );
		return true;
	} ));
	
	script.DefineFunction<RigidBodyBehavior>
	("getShape",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		int index = -1;
		if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
			script.ReportError( "usage: getShape( Int index )" );
			return false;
		}
		if ( index < 0 || index >= self->shapes.size() ) sa.ReturnObject( NULL );
		else sa.ReturnObject( self->shapes[ index ]->scriptObject );
		return true;
	} ));
	
	script.DefineFunction<RigidBodyBehavior>
	("addJoint",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		void *j = NULL;
		RigidBodyJoint* joint = NULL;
		if ( sa.args.size() ) {
			if ( !sa.ReadArguments( 1, TypeObject, &j ) || !( joint = script.GetInstance<RigidBodyJoint>( j ) ) ){
				script.ReportError( "usage: addJoint( [ Joint instance ] )" );
				return false;
			}
		} else {
			joint = new RigidBodyJoint( NULL );
		}
		// add and return it
		joint->SetBody( self );
		sa.ReturnObject( joint->scriptObject );
		return true;
	} ));
	
	script.DefineFunction<RigidBodyBehavior>
	("removeJoint",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		void *j = NULL;
		RigidBodyJoint* joint = NULL;
		int index = -1;
		const char *error = "usage: removeJoint( Joint instance | Int index )";
		if ( sa.ReadArguments( 1, TypeObject, &j ) ){
			joint = script.GetInstance<RigidBodyJoint>( j );
			if ( !joint ) {
				script.ReportError( error );
				return false;
			}
		} else if ( sa.ReadArguments( 1, TypeInt, &index ) ) {
			if ( index < 0 || index >= self->joints.size() ) {
				// index out of range, return null
				sa.ReturnObject( NULL );
				return true;
			}
			joint = self->joints[ index ];
		}
		joint->SetBody( NULL );
		return true;
	} ));
	
	script.DefineFunction<RigidBodyBehavior>
	("getJoint",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		RigidBodyBehavior* self = (RigidBodyBehavior*) p;
		int index = -1;
		if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
			script.ReportError( "usage: getJoint( Int index )" );
			return false;
		}
		if ( index < 0 || index >= self->joints.size() ) sa.ReturnObject( NULL );
		else sa.ReturnObject( self->joints[ index ]->scriptObject );
		return true;
	} ));

	
}

void RigidBodyBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	
	// shapes
	for ( size_t i = 0, nf = shapes.size(); i < nf; i++ ) {
		protectedObjects.push_back( &shapes[ i ]->scriptObject );
	}
	
	// joints
	for ( size_t i = 0, nf = joints.size(); i < nf; i++ ) {
		protectedObjects.push_back( &joints[ i ]->scriptObject );
	}
	for ( size_t i = 0, nf = otherJoints.size(); i < nf; i++ ) {
		protectedObjects.push_back( &otherJoints[ i ]->scriptObject );
	}
	
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
	
}


/* MARK:	-				Velocity and impulse
 -------------------------------------------------------------------- */


b2Vec2 RigidBodyBehavior::GetVelocity() {
	if ( this->body ) this->velocity = this->body->GetLinearVelocity();
	return this->velocity * BOX2D_TO_WORLD_SCALE;
}

void RigidBodyBehavior::SetVelocity( b2Vec2 vel ) {
	this->velocity = vel * WORLD_TO_BOX2D_SCALE;
	if ( this->body ) this->body->SetLinearVelocity( vel );
}

float RigidBodyBehavior::GetAngularVelocity() {
	if ( this->body ) this->angularVelocity = this->body->GetAngularVelocity();
	return this->angularVelocity;
}

void RigidBodyBehavior::SetAngularVelocity( float v ) {
	this->angularVelocity = v;
	if ( this->body ) this->body->SetAngularVelocity( v );
}

void RigidBodyBehavior::Impulse( b2Vec2 impulse, b2Vec2 point ){
	if ( this->body ) {
		this->body->ApplyLinearImpulse( impulse * WORLD_TO_BOX2D_SCALE, point * WORLD_TO_BOX2D_SCALE, true );
	}
}


/* MARK:	-				Sync body <-> object
 -------------------------------------------------------------------- */


/// copies body transform to game object
void RigidBodyBehavior::SyncObjectToBody() {
	
	if ( !this->body || !this->live ) return;
	
	// copy from body
	b2Vec2 pos = this->body->GetPosition();
	float angle = (float) this->body->GetAngle() * RAD_TO_DEG;
	
	if ( isnan( pos.x) || isnan( pos.y ) ) {
		printf( "RigidBodyBehavior::SyncObjectToBody pos NAN\n" );
		return;
	}
	
	pos *= BOX2D_TO_WORLD_SCALE;
	
	// construct world transform matrix for object
	GPU_MatrixIdentity( this->gameObject->_worldTransform );
	GPU_MatrixTranslate( this->gameObject->_worldTransform, pos.x, pos.y, this->gameObject->_z );
	if ( angle != 0 ) GPU_MatrixRotate( this->gameObject->_worldTransform, angle, 0, 0, 1 );
	if ( this->gameObject->_scale.x != 1 || this->gameObject->_scale.y != 1 ) GPU_MatrixScale( this->gameObject->_worldTransform, this->gameObject->_scale.x, this->gameObject->_scale.y, 1 );
	if ( this->gameObject->_skew.x != 1 || this->gameObject->_skew.y != 1 ) MatrixSkew( this->gameObject->_worldTransform, this->gameObject->_skew.x, this->gameObject->_skew.y );
	this->gameObject->_worldTransformDirty = false;
	this->gameObject->_localCoordsAreDirty = this->gameObject->_inverseWorldDirty = this->gameObject->_transformDirty = true;
		
}

/// converts game object's local transform to body
void RigidBodyBehavior::SyncBodyToObject() {

	// parent's world transform times local transform = this object world transform
	if ( this->gameObject->parent ) GPU_MatrixMultiply( this->gameObject->_worldTransform, this->gameObject->parent->WorldTransform(), this->gameObject->Transform() );
	this->gameObject->_worldTransformDirty = false;
	
	// extract pos, rot, scale
	b2Vec2 pos, scale;
	float angle;
	this->gameObject->DecomposeTransform( this->gameObject->_worldTransform, pos, angle, scale );
	
	// update body
	pos *= WORLD_TO_BOX2D_SCALE;
	if ( this->body ) {
		this->body->SetTransform( pos, angle * DEG_TO_RAD );
		if ( !this->body->IsAwake() ) this->body->SetAwake( true );
	}
	
}

/// just sets body transform
void RigidBodyBehavior::SetBodyTransform( b2Vec2 pos, float angleInRad ) {
	if ( !this->body ) return;
	this->body->SetTransform( pos, angleInRad );
}

/// set body position
void RigidBodyBehavior::SetBodyPosition( b2Vec2 pos ) {
	if ( !this->body ) return;
	this->body->SetTransform( pos, this->body->GetAngle() );
}

/// set body angle
void RigidBodyBehavior::SetBodyAngle( float angleInRad ) {
	if ( !this->body ) return;
	this->body->SetTransform( this->body->GetPosition(), angleInRad );
}

// gets body transform
void RigidBodyBehavior::GetBodyTransform( b2Vec2& pos, float& angle ) {
	if ( !this->body ) return;
	pos = this->body->GetPosition();
	pos *= BOX2D_TO_WORLD_SCALE;
	angle = this->body->GetAngle();
}


/* MARK:	-				Shapes
 -------------------------------------------------------------------- */


void RigidBodyBehavior::ReplaceShapes( RigidBodyShape* rbs ) {
	vector<RigidBodyShape*>::iterator it = shapes.begin();
	while( it != shapes.end() ) {
		RigidBodyShape* shp = *it;
		if ( shp != rbs ) {
			it = shapes.erase( it );
			shp->SetBody( NULL );
		} else it++;
	}
	if ( rbs ) rbs->SetBody( this );
}

// returns shapes vector
ArgValueVector* RigidBodyBehavior::GetShapesVector() {
	ArgValueVector* vec = new ArgValueVector();
	size_t nc = this->shapes.size();
	vec->resize( nc );
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*vec)[ i ];
		val.type = TypeObject;
		val.value.objectValue = this->shapes[ i ]->scriptObject;
	}
	return vec;
}

/// overwrites shapes
ArgValueVector* RigidBodyBehavior::SetShapesVector( ArgValueVector* in ) {
	while( this->shapes.size() ) this->shapes[ 0 ]->SetBody( NULL );
	// add shapes
	size_t nc = in->size();
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*in)[ i ];
		RigidBodyShape* go = script.GetInstance<RigidBodyShape>( val.value.objectValue );
		if ( go ) go->SetBody( this );
	}
	return in;
}

/// calls MakeShape on render
bool RigidBodyBehavior::MakeShapeFromRender() {
	if ( !this->gameObject || !this->gameObject->render ) return false;
	RigidBodyShape* rbs = this->gameObject->render->MakeShape();
	if ( rbs ) {
		this->ReplaceShapes( rbs );
		return true;
	} else return false;
}

/* MARK:	-				Joints
 -------------------------------------------------------------------- */

/// replace all joints with one or NULL
void RigidBodyBehavior::ReplaceJoints( RigidBodyJoint* rbs ) {
	vector<RigidBodyJoint*>::iterator it = joints.begin();
	while( it != joints.end() ) {
		RigidBodyJoint* shp = *it;
		if ( shp != rbs ) {
			it = joints.erase( it );
			shp->SetBody( NULL );
		} else it++;
	}
	if ( rbs ) rbs->SetBody( this );
}

// returns shapes vector
ArgValueVector* RigidBodyBehavior::GetJointsVector( bool other ) {
	ArgValueVector* vec = new ArgValueVector();
	vector<RigidBodyJoint*> &list = ( other ? otherJoints : joints );
	size_t nc = list.size();
	vec->resize( nc );
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*vec)[ i ];
		val.type = TypeObject;
		val.value.objectValue = list[ i ]->scriptObject;
	}
	return vec;
}

/// overwrites joints
ArgValueVector* RigidBodyBehavior::SetJointsVector( ArgValueVector* in, bool other ) {
	vector<RigidBodyJoint*> &list = ( other ? otherJoints : joints );
	while( list.size() ) list[ 0 ]->SetBody( NULL );
	// add joints
	size_t nc = in->size();
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*in)[ i ];
		RigidBodyJoint* go = script.GetInstance<RigidBodyJoint>( val.value.objectValue );
		if ( go ) go->SetBody( this );
	}
	return in;
}


/* MARK:	-				Attached / removed / active
 -------------------------------------------------------------------- */


/// overrides behavior active setter
void RigidBodyBehavior::EnableBody( bool e ) {

	// body can't exist without shapes
	e = ( e && this->body != NULL && shapes.size() > 0 );
	this->live = e;
	
	// update flag
	if ( this->body ) {
		this->body->SetActive( e );
	}
	
	// update all joints
	for ( size_t i = 0, nf = joints.size(); i < nf; i++ ) {
		joints[ i ]->UpdateJoint();
	}
	for ( size_t i = 0, nf = otherJoints.size(); i < nf; i++ ) {
		otherJoints[ i ]->UpdateJoint();
	}
	
}

void RigidBodyBehavior::AddBody( Scene *scene ) {
	
	// ignore
	if ( !scene || this->body ) return;

	b2BodyDef bodyDef;
	bodyDef.angle = gameObject->_angle * DEG_TO_RAD;
	bodyDef.position = gameObject->GetWorldPosition();
	bodyDef.position *= WORLD_TO_BOX2D_SCALE;
	bodyDef.userData = this;
	bodyDef.linearDamping = linearDamping;
	bodyDef.angularDamping = angularDamping;
	bodyDef.allowSleep = canSleep;
	bodyDef.active = false;
	bodyDef.bullet = bullet;
	bodyDef.fixedRotation = fixedRotation;
	bodyDef.type = bodyType;
	
	// create body
	this->body = scene->world->CreateBody( &bodyDef );
	this->body->SetMassData( &massData );
	
	// add shapes/fixtures
	for ( size_t i = 0, nf = shapes.size(); i < nf; i++ ) {
		shapes[ i ]->UpdateFixture();
	}
	
	// make active, if gameObject and behavior are active
	this->EnableBody( this->gameObject->active() && this->_active );
	
}

void RigidBodyBehavior::RemoveBody() {
	
	// remove body
	if ( this->body ) {
		// clear fixtures from shapes
		for ( size_t i = 0, nf = shapes.size(); i < nf; i++ ) {
			shapes[ i ]->fixtures.clear();
		}
		this->body->GetWorld()->DestroyBody( this->body );
	}
	this->body = NULL;
	
	this->EnableBody( false );
	
}

