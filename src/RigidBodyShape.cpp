#include "RigidBodyShape.hpp"
#include "RigidBodyBehavior.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


RigidBodyShape::RigidBodyShape( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<RigidBodyShape>( this );
	
	// add vector
	this->polyPoints = new FloatVector( NULL );
	this->polyPoints->notify = true;
	this->polyPoints->callback = static_cast<FloatVectorCallback>([this](FloatVector* fv){ this->UpdateFixture(); });
	
	// arguments?
	if ( args ) {
		
		
		
	}
}

RigidBodyShape::~RigidBodyShape() {}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void RigidBodyShape::InitClass() {

	// register class
	script.RegisterClass<RigidBodyShape>( "ScriptableObject" );

	// props
	
	script.AddProperty<RigidBodyShape>
	( "type",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((RigidBodyShape*) b)->shapeType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->shapeType = (RenderShapeBehavior::ShapeType) val;
		rb->UpdateFixture();
		return val;
	} ) );

	script.AddProperty<RigidBodyShape>
	( "body",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RigidBodyShape*) b)->body ? ((RigidBodyShape*) b)->body->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		RigidBodyBehavior* beh = script.GetInstance<RigidBodyBehavior>( val );
		rb->SetBody( beh );
		return ((RigidBodyShape*) b)->body ? ((RigidBodyShape*) b)->body->scriptObject : NULL;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	("category", //
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		ArgValue ret;
		BodyBehavior::BitsToValue( ((RigidBodyShape*) p)->categoryBits, ret );
		return ret;
	}),
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		((RigidBodyShape*) p)->categoryBits = BodyBehavior::ValueToBits( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyShape>
	("mask", //
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		ArgValue ret;
		BodyBehavior::BitsToValue( ~((RigidBodyShape*) p)->maskBits, ret );
		return ret;
	}),
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		((RigidBodyShape*) p)->maskBits = ~BodyBehavior::ValueToBits( val );
		return val;
	}));
	
	script.AddProperty<RigidBodyShape>
	( "centerX",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->center.x * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->center.x = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "centerY",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->center.y * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->center.y = val * WORLD_TO_BOX2D_SCALE;
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "radius",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->radius * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->radius = fmax( 0.01f, val * WORLD_TO_BOX2D_SCALE );
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "width",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->width * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->width = fmax( 0.01f, val * WORLD_TO_BOX2D_SCALE );
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "height",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->height * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->height = fmax( 0.01f, val * WORLD_TO_BOX2D_SCALE );
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "sensor",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RigidBodyShape*) b)->isSensor; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->isSensor = val;
		if ( rb->fixture ) rb->fixture->SetSensor( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "friction",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->friction; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->friction = val;
		if ( rb->fixture ) rb->fixture->SetFriction( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "bounce",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->restitution; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->restitution = val;
		if ( rb->fixture ) rb->fixture->SetRestitution( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "density",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->density; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->density = val;
		if ( rb->fixture ) rb->fixture->SetDensity( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "points",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RigidBodyShape*) b)->polyPoints->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->polyPoints->Set( val );
		rb->UpdateFixture();
		return ArgValue( rb->polyPoints->scriptObject );
	 }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );

	
}

void RigidBodyShape::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	
	// body
	if ( this->body ) protectedObjects.push_back( &this->body->scriptObject );
	
	// points vector
	protectedObjects.push_back( &this->polyPoints->scriptObject );
	
}


/* MARK:	-				Fixture
 -------------------------------------------------------------------- */


// set a new parent for object
void RigidBodyShape::SetBody( RigidBodyBehavior* newBody ) {
	
	// if parent is different
	if ( newBody != this->body ) {
		
		// if had body
		RigidBodyBehavior* oldBody = this->body;
		if ( oldBody ) {
			
			// find this object in old list of shapes
			vector<RigidBodyShape*> *oldList = &oldBody->shapes;
			vector<RigidBodyShape*>::iterator
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
		if ( newBody ) newBody->shapes.push_back( this );

		// recreate
		this->UpdateFixture();
		
	}
}

void RigidBodyShape::UpdateFixture() {

	// destroy old
	if ( this->fixture ) {
		b2Body *fixtureBody = this->fixture->GetBody();
		fixtureBody->DestroyFixture( this->fixture );
		this->fixture = NULL;
	}
	
	// create fixture
	if ( this->body && this->body->body ) {
		
		// make shape
		static b2PolygonShape polyShape;
		static b2ChainShape chainShape;
		static b2CircleShape circleShape;
		static b2EdgeShape edgeShape;
		b2Shape* currentShape = NULL;
		if ( shapeType == RenderShapeBehavior::ShapeType::Circle ){
			circleShape.m_p.Set( center.x, center.y );
			circleShape.m_radius = radius;
			currentShape = &circleShape;
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Rectangle ) {
			b2Vec2 offs;
			offs.x = width * 0.5 - center.x;
			offs.y = height * 0.5 - center.y;
			polyShape.SetAsBox( width * 0.5, height * 0.5, offs, 0 );
			currentShape = &polyShape;
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Polygon && polyPoints->vec.GetLength() > 4 ) {
			polyShape.m_centroid.Set( center.x, center.y );
			vector<b2Vec2> points;
			polyPoints->ToVec2Vector( points );
			polyShape.Set( points.data(), (int) points.size() );
			currentShape = &polyShape;
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Chain ) {
			vector<b2Vec2> points;
			polyPoints->ToVec2Vector( points );
			chainShape.CreateChain( points.data(), (int) points.size() );
			for ( int32 i = 0; i < chainShape.m_count; i++ ) chainShape.m_vertices[ i ] += center;
			currentShape = &chainShape;
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Line ) {
			edgeShape.Set( center, b2Vec2( width, height ) );
			currentShape = &edgeShape;
		}
		
		// create fixture
		this->fixture = this->body->body->CreateFixture( currentShape, density );
		this->fixture->SetRestitution( restitution );
		this->fixture->SetFriction( friction );
		this->fixture->SetSensor( isSensor );
		this->fixture->SetDensity( density );
		this->fixture->SetUserData( this );
	}
	
}



