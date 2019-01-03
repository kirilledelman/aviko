#include "RigidBodyShape.hpp"
#include "RigidBodyBehavior.hpp"
#include "GameObject.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


RigidBodyShape::RigidBodyShape( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<RigidBodyShape>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );
	
	// polypoints
	this->polyPoints = new TypedVector( NULL );
	ArgValue dv( "Float" );
	this->polyPoints->InitWithType( dv );
	this->polyPoints->callback = static_cast<TypedVectorCallback>([this](TypedVector* fv){ this->UpdateFixture(); });
	
	// read params
	int pShape = 0;
	vector<ArgValue>* pArray = NULL;
	float p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0;
	void *initObj = NULL, *arrayObj = NULL;
	
	// if arguments are given
	if ( args &&
		( args->ReadArguments( 1, TypeObject, &initObj ) ||
		 args->ReadArguments( 1, TypeInt, &pShape, TypeFloat, &p1, TypeFloat, &p2, TypeFloat, &p3, TypeFloat, &p4, TypeFloat, &p5, TypeFloat, &p6 ) ||
		 args->ReadArguments( 1, TypeInt, &pShape, TypeArray, &pArray ) ||
		 args->ReadArguments( 1, TypeInt, &pShape, TypeObject, &arrayObj ) ) ) {
			
			// first is shape
			this->shapeType = (RenderShapeBehavior::ShapeType) pShape;
			size_t numArgs = args->args.size();
			
			// second was object?
			if ( initObj ) {
				
				// use as init
				script.CopyProperties( initObj, this->scriptObject );
				
			} else {
				// extra arguments
				if ( this->shapeType == RenderShapeBehavior::ShapeType::Line ) { /// line from 0, 0 to x / y
					this->width = this->height = 50;
					if ( numArgs >= 3 ) {
						this->width = p1; this->height = p2;
					}
				} else if ( this->shapeType == RenderShapeBehavior::ShapeType::Circle ) { /// circle with radius
					this->radius = 25;
					if ( numArgs >= 2 ) {
						this->radius = p1;
					}
				} else if ( this->shapeType == RenderShapeBehavior::ShapeType::Rectangle ) { /// rectangle x wide, y tall
					this->width = 30; this->height = 20;
					if ( numArgs >= 3 ) {
						this->width = p1;
						this->height = p2;
					}
				} else if ( this->shapeType == RenderShapeBehavior::ShapeType::Polygon || this->shapeType == RenderShapeBehavior::ShapeType::Chain ) { /// polygon or un-closed polygon between polyPoints (x,y pairs)
					if ( pArray || arrayObj ) {
						this->polyPoints->Set( args->args[ 1 ] );
					}
				}
			}
		}
    
}

RigidBodyShape::~RigidBodyShape() {
    
    this->ClearShapesList();
    
}


/* MARK:	-				Scripting
 -------------------------------------------------------------------- */


void RigidBodyShape::InitClass() {

	// register class
	script.RegisterClass<RigidBodyShape>( NULL );

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
    ( "gameObject",
     static_cast<ScriptObjectCallback>([](void *b, void* val ){
        RigidBodyShape* rb = (RigidBodyShape*) b;
        if ( rb->body && rb->body->gameObject ) return rb->body->gameObject->scriptObject;
        return (void*) NULL;
    }));
	
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
	} ), PROP_ENUMERABLE );
	
	script.AddProperty<RigidBodyShape>
	( "height",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->height * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->height = fmax( 0.01f, val * WORLD_TO_BOX2D_SCALE );
		rb->UpdateFixture();
		return val;
	} ), PROP_ENUMERABLE );
	
	script.AddProperty<RigidBodyShape>
	( "x",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->width * BOX2D_TO_WORLD_SCALE; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->width = fmax( 0.01f, val * WORLD_TO_BOX2D_SCALE );
		rb->UpdateFixture();
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "y",
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
		for ( size_t i = 0, nf = rb->fixtures.size(); i < nf; i++ ) rb->fixtures[ i ]->SetSensor( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "friction",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->friction; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->friction = val;
		for ( size_t i = 0, nf = rb->fixtures.size(); i < nf; i++ ) rb->fixtures[ i ]->SetFriction( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "bounce",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->restitution; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->restitution = val;
		for ( size_t i = 0, nf = rb->fixtures.size(); i < nf; i++ ) rb->fixtures[ i ]->SetRestitution( val );
		return val;
	} ) );
	
	script.AddProperty<RigidBodyShape>
	( "density",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RigidBodyShape*) b)->density; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		RigidBodyShape* rb = (RigidBodyShape*) b;
		rb->density = val;
		for ( size_t i = 0, nf = rb->fixtures.size(); i < nf; i++ ) rb->fixtures[ i ]->SetDensity( val );
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

	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
	
}

/* MARK:    -                Stored contact
 -------------------------------------------------------------------- */

void RigidBodyShape::AddContactWith( RigidBodyShape* other, b2Vec2 point, b2Vec2 normal ){
    StoredFixtureContact& r = this->storedContacts[ other ];
    r.point = point;
    r.normal = normal;
}

void RigidBodyShape::RemoveContactWith( RigidBodyShape* other ){
    this->storedContacts.erase( other );
}

bool RigidBodyShape::CheckContactWith(RigidBodyShape *other, b2Vec2 point, b2Vec2 normal){
    unordered_map<RigidBodyShape*, StoredFixtureContact>::iterator it = this->storedContacts.find( other );
    if ( it == this->storedContacts.end() ) return false;
    // update pos/normal
    it->second.normal = normal;
    it->second.point = point;
    return true;
}


/* MARK:	-				Shape splitting

	Code adopted / modified from algorithm by Antoan Angelov
	http://www.emanueleferonato.com/2011/09/12/create-non-convex-complex-shapes-with-box2d/
 
 -------------------------------------------------------------------- */


#define SHAPE_SPLIT_TOLERANCE 0.1f

inline bool _isOnLine( b2Vec2& pp, b2Vec2& p1, b2Vec2& p2 ) {
	if ( fabs( p1.x - p2.x ) > SHAPE_SPLIT_TOLERANCE ) {
		float a = ( p2.y - p1.y ) / ( p2.x - p1.x ),
		possibleY = a * ( pp.x - p1.x ) + p1.y,
		diff = ( possibleY > pp.y ) ? possibleY - pp.y : pp.y - possibleY;
		return (diff < SHAPE_SPLIT_TOLERANCE);
	}
	return ( fabs( pp.x - p1.x ) < SHAPE_SPLIT_TOLERANCE );
}

inline bool _isOnSegment( b2Vec2& pp, b2Vec2& p1, b2Vec2& p2 ) {
	bool b1 = ( ( ( p1.x + SHAPE_SPLIT_TOLERANCE ) >= pp.x ) && pp.x >= p2.x - SHAPE_SPLIT_TOLERANCE ) || ( ( ( p1.x - SHAPE_SPLIT_TOLERANCE ) <= pp.x ) && pp.x <= p2.x + SHAPE_SPLIT_TOLERANCE);
	bool b2 = ( ( ( p1.y + SHAPE_SPLIT_TOLERANCE ) >= pp.y ) && pp.y >= p2.y - SHAPE_SPLIT_TOLERANCE ) || ( ( ( p1.y - SHAPE_SPLIT_TOLERANCE ) <= pp.y ) && pp.y <= p2.y + SHAPE_SPLIT_TOLERANCE);
	return ( ( b1 && b2 ) && _isOnLine( pp, p1, p2 ) );
}

bool _hitRay( b2Vec2& p1, b2Vec2& p2, b2Vec2& p3, b2Vec2& p4, b2Vec2& out ) {
	float	t1 = p3.x - p1.x,
	t2 = p3.y - p1.y,
	t3 = p2.x - p1.x,
	t4 = p2.y - p1.y,
	t5 = p4.x - p3.x,
	t6 = p4.y - p3.y,
	t7 = t4 * t5 - t3 * t6, a;
	a = ( t5 * t2 - t6 * t1 ) / t7;
	b2Vec2 pp( p1.x + a * t3, p1.y + a * t4 );
	bool b1 = _isOnSegment( p2, p1, pp );
	bool b2 = _isOnSegment( pp, p3, p4 );
	if ( b1 && b2 ) {
		out.Set( pp.x, pp.y );
		return true;
	}
	return false;
}

// splits concave shape defined by verticesVec into multiple convex shapes, puts result in second param
bool _splitShapes( vector<b2Vec2>& verticesVec, vector<vector<b2Vec2>>& figsVec ) {
	vector<b2Vec2> vec1, vec2;
	float d = 0, t = 0, dx = 0, dy = 0, minLen = 0;
	int i = 0, n = 0, j = 0, i2 = 0, i3 = 0, j1 = 0, j2 = 0, k = 0, h = 0;
	b2Vec2 v, hitV;
	list<vector<b2Vec2>> queue;
	size_t queueSize = 1;
	bool isConvex;
	figsVec.clear();
 
	// detect and reverse counterclockwise
	for ( i = 0, n = (int) verticesVec.size(); i < n; i++ ) {
		i2 = (i + 1) % n;
		b2Vec2 &p1 = verticesVec[ i ];
		b2Vec2 &p2 = verticesVec[ i2 ];
		d += ( p1.x * p2.y - p2.x * p1.y );
	}
	if ( d < 0 ) reverse( verticesVec.begin(), verticesVec.end() );
	
	// start with given points
	queue.push_back( verticesVec );
	while ( queueSize ) {
		vector<b2Vec2> &vec = queue.front();
		isConvex = true;
		for ( i = 0, n = (int) vec.size(); i < n; i++ ) {
			i2 = ( i < n-1 ) ? i + 1 : i + 1 - n;
			i3 = ( i < n-2 ) ? i + 2 : i + 2 - n;
			b2Vec2 &p1 = vec[ i ];
			b2Vec2 &p2 = vec[ i2 ];
			b2Vec2 &p3 = vec[ i3 ];
			d = p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.y * p2.x - p2.y * p3.x - p3.y * p1.x;
			if ( d < 0 ) {
				isConvex = false;
				minLen = numeric_limits<float>::max();
				for ( j = 0; j < n; j++ ) {
					if ( j != i && j != i2 ) {
						j1 = j;
						j2 = ( j < n - 1 ) ? j + 1 : 0;
						b2Vec2 &v1 = vec[ j1 ];
						b2Vec2 &v2 = vec[ j2 ];
						if ( _hitRay( p1, p2, v1, v2, v ) ) {
							dx = p2.x - v.x;
							dy = p2.y - v.y;
							t = dx * dx + dy * dy;
							if ( t < minLen ) {
								h = j1;
								k = j2;
								hitV = v;
								minLen = t;
							}
						}
					}
				}
				if ( minLen == numeric_limits<float>::max() ) {
					// error
					return false;
				}
				vec1.clear(); vec2.clear();
				j1 = h; j2 = k;
				b2Vec2 &v1 = vec[ j1 ];
				b2Vec2 &v2 = vec[ j2 ];
				if ( fabs( hitV.x - v2.x ) > SHAPE_SPLIT_TOLERANCE || fabs( hitV.y - v2.y ) > SHAPE_SPLIT_TOLERANCE ) {
					vec1.push_back( hitV );
				}
				if ( fabs( hitV.x - v1.x ) > SHAPE_SPLIT_TOLERANCE || fabs( hitV.y - v1.y ) > SHAPE_SPLIT_TOLERANCE ) {
					vec2.push_back( hitV );
				}
				h = -1;
				k = i;
				while ( true ) {
					if ( k != j2 ) {
						vec1.push_back( vec[ k ] );
					} else {
						if ( h < 0 || h >= n ) {
							// error
							return false;
						}
						if ( !_isOnSegment( v2, vec[ h ], p1 ) ) {
							vec1.push_back( vec[ k ] );
						}
						break;
					}
					h = k;
					if ( k - 1 < 0 ) {
						k = n - 1;
					} else {
						k--;
					}
				}
				reverse( vec1.begin(), vec1.end() );
				h = -1;
				k = i2;
				while ( true ) {
					if ( k != j1 ) {
						vec2.push_back( vec[ k ] );
					} else {
						if ( h < 0  || h >= n ) {
							// error
							return false;
						}
						if ( k == j1 && !_isOnSegment( v1, vec[ h ], p2 ) ) {
							vec2.push_back( vec[ k ] );
						}
						break;
					}
					h = k;
					if ( k + 1 > n - 1 ) {
						k = 0;
					} else {
						k++;
					}
				}
				queue.push_back( vec1 );
				queue.push_back( vec2 );
				queue.erase( queue.begin() );
				queueSize++;
				break;
			}
		}
		if ( isConvex ) {
			figsVec.push_back( queue.front() );
			queue.erase( queue.begin() );
			queueSize--;
		}
	}
 
	return true;
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
	if ( this->fixtures.size() ) {
		for ( size_t i = 0, nf = this->fixtures.size(); i < nf; i++ ) {
			b2Fixture* fix = this->fixtures[ i ];
			b2Body *fixtureBody = fix->GetBody();
			fixtureBody->DestroyFixture( fix );
		}
		this->fixtures.clear();
	}
	
	// create fixture
	if ( this->body && this->body->body ) {
		
		// make shape
		static b2PolygonShape polyShape;
		if ( shapeType == RenderShapeBehavior::ShapeType::Polygon && polyPoints->GetLength() > 2 ) {
			polyShape.m_centroid.Set( center.x, center.y );
			vector<b2Vec2> points;
			polyPoints->ToVec2Vector( points );
			// split into multiple convex shapes
			if ( splitConcave ) {
				static vector<vector<b2Vec2>> split;
				if ( _splitShapes( points, split ) ) {
					for ( size_t i = 0, ns = split.size(); i < ns; i++ ) {
						vector<b2Vec2>& sh = split[ i ];
						polyShape.Set( sh.data(), (int) sh.size() );
						this->fixtures.push_back( this->body->body->CreateFixture( &polyShape, density ) );
					}
				}
				
			// no need to split
			} else {
				polyShape.Set( points.data(), (int) points.size() );
				this->fixtures.push_back( this->body->body->CreateFixture( &polyShape, density ) );
			}
		
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Circle ){
			static b2CircleShape circleShape;
			circleShape.m_p.Set( center.x, center.y );
			circleShape.m_radius = radius;
			this->fixtures.push_back( this->body->body->CreateFixture( &circleShape, density ) );
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Rectangle ) {
			b2Vec2 offs;
			offs.x = width * 0.5 - center.x;
			offs.y = height * 0.5 - center.y;
			polyShape.SetAsBox( width * 0.5, height * 0.5, offs, 0 );
			this->fixtures.push_back( this->body->body->CreateFixture( &polyShape, density ) );
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Chain ) {
			static b2ChainShape chainShape;
			vector<b2Vec2> points;
			polyPoints->ToVec2Vector( points );
			chainShape.CreateChain( points.data(), (int) points.size() );
			for ( int32 i = 0; i < chainShape.m_count; i++ ) chainShape.m_vertices[ i ] += center;
			this->fixtures.push_back( this->body->body->CreateFixture( &chainShape, density ) );
		} else if ( shapeType == RenderShapeBehavior::ShapeType::Line ) {
			static b2EdgeShape edgeShape;
			edgeShape.Set( center, b2Vec2( width, height ) );
			this->fixtures.push_back( this->body->body->CreateFixture( &edgeShape, density ) );
		}
		
		// set params on all fixtures
		for ( size_t i = 0, nf = this->fixtures.size(); i < nf; i++ ) {
			b2Fixture* fix = this->fixtures[ i ];
			fix->SetRestitution( restitution );
			fix->SetFriction( friction );
			fix->SetSensor( isSensor );
			fix->SetDensity( density );
			fix->SetUserData( this );
		}
	}
	
}



/// creates an array of shapes
void RigidBodyShape::MakeShapesList() {
    
    // clean up
    this->ClearShapesList();
    
    // make shape
    b2PolygonShape* polyShape = NULL;
    if ( shapeType == RenderShapeBehavior::ShapeType::Polygon && polyPoints->GetLength() > 2 ) {
        vector<b2Vec2> points;
        polyPoints->ToVec2Vector( points );
        // split into multiple convex shapes
        if ( splitConcave ) {
            static vector<vector<b2Vec2>> split;
            if ( _splitShapes( points, split ) ) {
                for ( size_t i = 0, ns = split.size(); i < ns; i++ ) {
                    vector<b2Vec2>& sh = split[ i ];
                    polyShape = new b2PolygonShape();
                    polyShape->m_centroid.Set( center.x, center.y );
                    polyShape->Set( sh.data(), (int) sh.size() );
                    this->shapes.push_back( polyShape );
                }
            }
            
            // no need to split
        } else {
            polyShape = new b2PolygonShape();
            polyShape->m_centroid.Set( center.x, center.y );
            polyShape->Set( points.data(), (int) points.size() );
            this->shapes.push_back( polyShape );
        }
        
    } else if ( shapeType == RenderShapeBehavior::ShapeType::Circle ){
        b2CircleShape* circleShape = new b2CircleShape();
        circleShape->m_p.Set( center.x, center.y );
        circleShape->m_radius = radius;
        this->shapes.push_back( circleShape );
    } else if ( shapeType == RenderShapeBehavior::ShapeType::Rectangle ) {
        b2Vec2 offs;
        offs.x = width * 0.5 - center.x;
        offs.y = height * 0.5 - center.y;
        polyShape = new b2PolygonShape();
        polyShape->SetAsBox( width * 0.5, height * 0.5, offs, 0 );
        this->shapes.push_back( polyShape );
    }
    
}

void RigidBodyShape::ClearShapesList() {
    // clean up shapes
    for ( size_t i = 0, nf = this->shapes.size(); i < nf; i++ ) {
        b2Shape* shp = this->shapes[ i ];
        delete shp;
    }
    this->shapes.clear();
}
