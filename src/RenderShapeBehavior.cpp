#include "RenderShapeBehavior.hpp"
#include "ScriptHost.hpp"
#include "GameObject.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderShapeBehavior::RenderShapeBehavior( ScriptArguments* args ) : RenderShapeBehavior() {

	// add scriptObject
	script.NewScriptObject<RenderShapeBehavior>( this );
	
	// polypoints
	this->polyPoints = new TypedVector( NULL );
	ArgValue dv( "Float" );
	this->polyPoints->InitWithType( dv );
	this->polyPoints->lockedType = true;
	this->polyPoints->callback = static_cast<TypedVectorCallback>([this](TypedVector* fv){ this->_renderPointsDirty = true; });
	
	// add defaults
	RenderBehavior::AddDefaults();	
	
	// outline color
	Color* color = new Color( NULL );
	color->SetInts( 0, 0, 0, 255 );
	script.SetProperty( "outlineColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// read params
	int pShape = 0;
	vector<ArgValue>* pArray = NULL;
	float p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0;
	void *initObj = NULL;
	void *arrayObj = NULL;
	
	// if arguments are given
	if ( args &&
		( args->ReadArguments( 1, TypeObject, &initObj ) ||
		  args->ReadArguments( 1, TypeInt, &pShape, TypeFloat, &p1, TypeFloat, &p2, TypeFloat, &p3, TypeFloat, &p4, TypeFloat, &p5, TypeFloat, &p6 ) ||
		  args->ReadArguments( 1, TypeInt, &pShape, TypeArray, &pArray ) ||
		  args->ReadArguments( 1, TypeInt, &pShape, TypeObject, &arrayObj ) ) ) {
		
		// first is shape
		this->shapeType = (ShapeType) pShape;
		size_t numArgs = args->args.size();
		
		// second was object?
		if ( initObj ) {
			
			// use as init
			script.CopyProperties( initObj, this->scriptObject );
			
		} else {
			// extra arguments
			if ( this->shapeType == Line ) { /// line from 0, 0 to x / y
				this->x = this->y = 50;
				if ( numArgs >= 3 ) {
					this->x = p1; this->y = p2;
				}
			} else if ( this->shapeType == Arc ) { /// arc for a circle at x, y, radius, start->end angle
				this->radius = 25;
				this->startAngle = 0;
				this->endAngle = 90;
				if ( numArgs >= 3 ) {
					this->x = p1;
					this->y = p2;
					if ( numArgs >= 4 ) {
						this->radius = p3;
						if ( numArgs == 5 ) {
							this->endAngle = p4;
						} else if ( numArgs >= 6 ) {
							this->startAngle = p4;
							this->endAngle = p5;
						}
					}
				}
			} else if ( this->shapeType == Circle ) { /// circle with radius
				this->radius = 25;
				if ( numArgs >= 2 ) {
					this->radius = p1;
				}
			} else if ( this->shapeType == Ellipse ) { /// ellipse x wide, y tall
				this->x = 50; this->y = 25;
				if ( numArgs == 3 ) {
					this->x = p1;
					this->y = p2;
				}
			} else if ( this->shapeType == Sector ) { /// sector of a donut with radius, hole innerRadius, start->end angle
				this->radius = 25;
				this->innerRadius = 5;
				this->startAngle = 0;
				this->endAngle = 360;
				if ( numArgs >= 2 ) {
					this->radius = p1;
					if ( numArgs >= 3 ) {
						this->innerRadius = p2;
						if ( numArgs == 4 ) {
							this->endAngle = p3;
						} else if ( args->args.size() >= 5 ) {
							this->startAngle = p3;
							this->endAngle = p4;
						}
					} else {
						this->innerRadius = max( 0.f, this->radius - 10 );
					}
				}
			} else if ( this->shapeType == Triangle ) { /// triangle between x,y,x1,y1,x2,y2
				this->x = -10; this->y = 5;
				this->x1 = 10; this->y1 = 5;
				this->x2 = 0; this->y2 = -5;
				if ( numArgs >= 7 ) {
					this->x = p1;
					this->y = p2;
					this->x1 = p3;
					this->y1 = p4;
					this->x2 = p5;
					this->y2 = p6;
				}
			} else if ( this->shapeType == Rectangle ) { /// rectangle x wide, y tall
				this->x = 30; this->y = 20;
				this->centered = false;
				if ( numArgs >= 3 ) {
					this->x = p1;
					this->y = p2;
				}
			} else if ( this->shapeType == RoundedRectangle ) { /// rectangle x wide, y tall, radius corner
				this->x = 30; this->y = 20;
				this->centered = false;
				if ( numArgs >= 3 ) {
					this->x = p1;
					this->y = p2;
					if ( numArgs >= 4 ) {
						this->radius = p3;
					}
				}
			} else if ( this->shapeType == Polygon ) { /// polygon between polyPoints (x,y pairs)
				if ( pArray || arrayObj ) {
					this->polyPoints->Set( args->args[ 1 ] );
					this->_renderPointsDirty = filled;
				}
			} else if ( this->shapeType == Chain ) { /// un-closed polygon between polyPoints (x,y pairs)
				if ( pArray || arrayObj ) this->polyPoints->Set( args->args[ 1 ] );
			}
		}
	}
}

// init
RenderShapeBehavior::RenderShapeBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_RENDER, (BehaviorEventCallback) &RenderShapeBehavior::Render );
	
	// can render
	this->isRenderBehavior = true;
	
}

// destroy
RenderShapeBehavior::~RenderShapeBehavior() {
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void RenderShapeBehavior::InitClass() {

	// register class
	script.RegisterClass<RenderShapeBehavior>( "RenderBehavior" );
	
	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "Shape", constants );
	script.SetProperty( "None", ArgValue( (int) ShapeType::None ), constants );
	script.SetProperty( "Arc", ArgValue( (int) ShapeType::Arc ), constants );
	script.SetProperty( "Circle", ArgValue( (int) ShapeType::Circle ), constants );
	script.SetProperty( "Ellipse", ArgValue( (int) ShapeType::Ellipse ), constants );
	script.SetProperty( "Line", ArgValue( (int) ShapeType::Line ), constants );
	script.SetProperty( "Polygon", ArgValue( (int) ShapeType::Polygon ), constants );
	script.SetProperty( "Rectangle", ArgValue( (int) ShapeType::Rectangle ), constants );
	script.SetProperty( "RoundedRectangle", ArgValue( (int) ShapeType::RoundedRectangle ), constants );
	script.SetProperty( "Sector", ArgValue( (int) ShapeType::Sector ), constants );
	script.SetProperty( "Triangle", ArgValue( (int) ShapeType::Triangle ), constants );
	script.SetProperty( "Chain", ArgValue( (int) ShapeType::Chain ), constants );
	script.FreezeObject( constants );
	
	// properties
	
	script.AddProperty<RenderShapeBehavior>
	( "shape",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderShapeBehavior*) b)->shapeType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		RenderShapeBehavior* s = (RenderShapeBehavior*) b;
		s->shapeType = (ShapeType) val;
		s->_renderPointsDirty = ( val == Polygon && s->filled );
		return val;
	}) );
	
	script.AddProperty<RenderShapeBehavior>
	( "filled",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderShapeBehavior*) b)->filled; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		RenderShapeBehavior* rb = (RenderShapeBehavior*) b;
		rb->filled = val;
		rb->_renderPointsDirty = ( val && rb->shapeType == Polygon );
		return val;
	}) );
	
	script.AddProperty<RenderShapeBehavior>
	( "outlineColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderShapeBehavior*) b)->outlineColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderShapeBehavior* rs = (RenderShapeBehavior*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->outlineColor = other;
		} else {
			rs->outlineColor->Set( val );
		}
		return rs->outlineColor->scriptObject;
	}) );
	
	script.AddProperty<RenderShapeBehavior>
	( "radius",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->radius; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->radius = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "innerRadius",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->innerRadius; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->innerRadius = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "x",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->x; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->x = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "y",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->y; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->y = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "width",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->x; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->x = val ); }), PROP_ENUMERABLE );
	
	script.AddProperty<RenderShapeBehavior>
	( "height",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->y; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->y = val ); }), PROP_ENUMERABLE );
	
	script.AddProperty<RenderShapeBehavior>
	( "x1",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->x1; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->x1 = val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "y1",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->y1; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->y1 = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "x2",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->x2; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->x2 = val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "y2",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->y2; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->y2 = val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "startAngle",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->startAngle; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->startAngle = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "endAngle",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->endAngle; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->endAngle = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "lineThickness",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->lineThickness; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->lineThickness = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "centered",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderShapeBehavior*) b)->centered; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderShapeBehavior*) b)->centered = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "points",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderShapeBehavior*) b)->polyPoints->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderShapeBehavior* rb = (RenderShapeBehavior*) b;
		rb->polyPoints->Set( val );
		rb->_renderPointsDirty = true;
		return ArgValue( rb->polyPoints->scriptObject );
	}),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );
	
	// functions
	
	script.DefineFunction<RenderShapeBehavior>
	( "resize", // setSize( Number width, Number height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		RenderShapeBehavior* self = (RenderShapeBehavior*) obj;
		float x = 0, y = 0;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y ) ) {
			script.ReportError( "usage: resize( Number width, Number height )" );
			return false;
		}
		self->Resize( x, y );
		return true;
	}));
	
}

void RenderShapeBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	
	// points vector
	protectedObjects.push_back( &this->polyPoints->scriptObject );
	
	// call super
	RenderBehavior::TraceProtectedObjects( protectedObjects );
	
}

/* MARK:	-				UI
 -------------------------------------------------------------------- */


float DistanceToLine( float px, float py, float x1, float y1, float x2, float y2 ) {
	float A = px - x1;
	float B = py - y1;
	float C = x2 - x1;
	float D = y2 - y1;
	float dot = A * C + B * D;
	float len_sq = C * C + D * D;
	float param = -1;
	float xx, yy;
	if ( len_sq != 0 ) param = dot / len_sq;
	if ( param < 0 ) {
		xx = x1;
		yy = y1;
	} else if ( param > 1 ) {
		xx = x2;
		yy = y2;
	} else {
		xx = x1 + param * C;
		yy = y1 + param * D;
	}
	
	float dx = px - xx;
	float dy = py - yy;
	return sqrt(dx * dx + dy * dy);
}

bool InsideArc( float px, float py, float cx, float cy, float r, float a1, float a2 ) {
	a1 = fmod( a1, 360 );
	a2 = fmod( a2, 360 );
	if ( a1 > a2 ) { float tmp = a1; a1 = a2; a2 = tmp; }
	
	float dx = px - cx, dy = py - cy;
	float pr = ( dx * dx + dy * dy );
	if ( pr > r * r ) return false;
	
	float pa = atan2( dy, dx ) * RAD_TO_DEG;
	if ( pa < 0 && a1 > 0 ) pa += 360;
	return ( pa >= a1 && pa <= a2 );
	return true;
}

bool InsideEllipse( float px, float py, float cx, float cy, float rx, float ry ) {
	float q = rx / ry;
	px -= cx;
	py = ( py - cy ) * q;
	return ( sqrt( px * px + py * py ) <= rx );
}

bool InsidePolygon( float px, float py, float* points, int numPoints ) {
	
	bool result = false;
	for ( int i = 0, j = numPoints - 1; i < numPoints; j = i++ ) {
		float x1 = points[ i * 2 ];
		float y1 = points[ i * 2 + 1 ];
		float x2 = points[ j * 2 ];
		float y2 = points[ j * 2 + 1 ];
		
		if (( y1 > py ) != ( y2 > py ) &&
			( px < (x2 - x1) * (py - y1) / (y2-y1) + x1) ) {
			result = !result;
		}
	}
	return result;
}

bool RenderShapeBehavior::IsScreenPointInside( float screenX, float screenY, float* outLocalX, float* outLocalY ) {
	
	// transform global point to local space
	this->gameObject->ConvertPoint( screenX, screenY, *outLocalX, *outLocalY, false );
	
	// check if point is inside
	if( shapeType == ShapeType::Line ){
		
		return ( DistanceToLine( *outLocalX, *outLocalY, 0, 0, x, y ) <= lineThickness * 0.5 );
		
	} else if( shapeType == ShapeType::Arc ){
		
		return InsideArc( *outLocalX, *outLocalY, x, y, radius, startAngle, endAngle );
		
	} else if ( shapeType == ShapeType::Circle ) {
		
		return InsideEllipse( *outLocalX, *outLocalY, centered ? 0 : radius, centered ? 0 : radius, radius, radius );
		
	} else if ( shapeType == ShapeType::Ellipse ) {
		
		return InsideEllipse( *outLocalX, *outLocalY, centered ? 0 : x, centered ? 0 : y, x, y );
		
	} else if ( shapeType == ShapeType::Sector ) {
		
		return InsideArc( *outLocalX, *outLocalY, x, y, radius, startAngle, endAngle ) &&
			   !InsideArc( *outLocalX, *outLocalY, x, y, innerRadius, startAngle, endAngle );
		
	} else if ( shapeType == ShapeType::Triangle ) {
	
		static float triPoints[ 6 ];
		triPoints[ 0 ] = x; triPoints[ 1 ] = y;
		triPoints[ 2 ] = x1; triPoints[ 3 ] = y1;
		triPoints[ 4 ] = x2; triPoints[ 5 ] = y2;
		return InsidePolygon( *outLocalX, *outLocalY, triPoints, 3 );
		
	} else if ( shapeType == ShapeType::Rectangle || shapeType == ShapeType::RoundedRectangle ) {
		if ( centered ) {
			return ( *outLocalX >= -x * 0.5 && *outLocalX < x * 0.5 ) && ( *outLocalY >= -y * 0.5 && *outLocalY < y * 0.5 );
		} else {
			return ( *outLocalX >= 0 && *outLocalX < x ) && ( *outLocalY >= 0 && *outLocalY < y );
		}
	} else if ( shapeType == ShapeType::Polygon ) {
	
		vector<float>* dv = polyPoints->ToFloatVector();
		return InsidePolygon( *outLocalX, *outLocalY, dv->data(), polyPoints->GetLength() / 2 );
		
	}
	
	return false;
	
}

// returns sprite bounding box
GPU_Rect RenderShapeBehavior::GetBounds() {
	GPU_Rect rect = { 0, 0, 0, 0 };
	// check if point is inside
	if( shapeType == ShapeType::Line ){
		
		if ( x < 0 ) { rect.x = x; rect.w = -x; }
		else { rect.x = 0; rect.w = x; }
		if ( y < 0 ) { rect.y = y; rect.h = -y; }
		else { rect.y = 0; rect.h = y; }
		
	} else if( shapeType == ShapeType::Arc ){
		
		rect = { x - radius, y - radius, radius * 2, radius * 2 };
		
	} else if ( shapeType == ShapeType::Circle ) {
		
		if ( centered ) {
			rect = { -radius, -radius, radius * 2, radius * 2 };
		} else {
			rect = { 0, 0, radius * 2, radius * 2 };
		}
		
	} else if ( shapeType == ShapeType::Ellipse ) {
		
		if ( centered ) {
			rect = { -x, -y, x * 2, y * 2 };
		} else {
			rect = { 0, 0, x * 2, y * 2 };
		}
		
	} else if ( shapeType == ShapeType::Sector ) {
		
		rect = { x - radius, y - radius, radius * 2, radius * 2 };
		
	} else if ( shapeType == ShapeType::Triangle ) {
		
		float minX = x, minY = y, maxX = x, maxY = y;
		minX = fmin( minX, x1 ); minY = fmin( minY, y1 );
		maxX = fmax( maxX, x1 ); maxY = fmax( maxY, y1 );
		minX = fmin( minX, x2 ); minY = fmin( minY, y2 );
		maxX = fmax( maxX, x2 ); maxY = fmax( maxY, y2 );
		rect.x = minX; rect.y = minY;
		rect.w = maxX - minX; rect.h = maxY - minY;
		
	} else if ( shapeType == ShapeType::Rectangle || shapeType == ShapeType::RoundedRectangle ) {
		if ( centered ) {
			rect = { -x * 0.5f, -y * 0.5f, x, y };
		} else {
			rect = { 0, 0, x, y };
		}
	} else if ( shapeType == ShapeType::Polygon ) {
		
		vector<float>* dv = polyPoints->ToFloatVector();
		if ( dv->size() >= 2 ) {
			float minX = (*dv)[ 0 ], minY = (*dv)[ 1 ];
			float maxX = minX, maxY = minY;
			for ( size_t i = 2, np = dv->size(); i < np; i+=2 ){
				float xx = (*dv)[ i ], yy = (*dv)[ i + 1 ];
				minX = fmin( minX, xx ); minY = fmin( minY, yy );
				maxX = fmax( maxX, xx ); maxY = fmax( maxY, yy );
			}
			rect.x = minX; rect.y = minY;
			rect.w = maxX - minX; rect.h = maxY - minY;
		}
		
	}
	
	return rect;
}

void RenderShapeBehavior::Resize( float w, float h ) {
	
	// TODO - better resize for shapes
	this->x = w; this->y = h;
	
}

/* MARK:	-				To triangles
 -------------------------------------------------------------------- */

void RenderShapeBehavior::UpdatePoints() {

	_renderPointsDirty = false;
	if ( shapeType == Polygon && filled ) {
		// clear
		renderTris.clear();
		
		// modified http://www.flipcode.com/archives/Efficient_Polygon_Triangulation.shtml
		vector<float>& contour = *polyPoints->ToFloatVector();
		
		// allocate and initialize list of Vertices in polygon
		int n = (int) contour.size() / 2;
		if ( n < 3 ) return;
		int nv = n;
		unsigned short *V = new unsigned short[ n ];
		
		// we want a counter-clockwise polygon in V
		float area = 0.0f;
		for( int p = n-1, q = 0; q < n; p = q++ ) {
			area += contour[ p * 2 ] * contour[ q * 2 + 1 ] - contour[ q * 2 ] * contour[ p * 2 + 1 ];
		}
		area *= 0.5f;
		if ( 0.0f < area ) {
			for ( int v = 0; v < n; v++ ) V[ v ] = v;
		} else {
			for( int v = 0; v < n; v++) V[ v ] = ( n - 1 ) - v;
		}
		
		// remove nv-2 Vertices, creating 1 triangle every time */
		int count = 2 * nv; // error detection
		
		for( int m = 0, v = nv - 1; nv > 2; ) {
			
			if ( 0 >= ( count-- ) ) return;  // if we loop, it is probably a non-simple polygon
			
			// three consecutive vertices in current polygon, <u,v,w>
			int u = v  ; if (nv <= u) u = 0;// previous
			v = u+1; if (nv <= v) v = 0;	// new v
			int w = v+1; if (nv <= w) w = 0;// next
			
			// snip
			bool snip = true;
			float	Px, Py,
			Ax = contour[ V[ u ] * 2 ],
			Ay = contour[ V[ u ] * 2 + 1 ],
			Bx = contour[ V[ v ] * 2 ],
			By = contour[ V[ v ] * 2 + 1 ],
			Cx = contour[ V[ w ] * 2 ],
			Cy = contour[ V[ w ] * 2 + 1 ];
			if ( 0.0000000001f > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) {
				snip = false;
			} else {
				for ( int p = 0; p < nv; p++ ) {
					if( (p == u) || (p == v) || (p == w) ) continue;
					Px = contour[ V[ p ] * 2 ];
					Py = contour[ V[ p ] * 2 + 1 ];
					
					float ax = Cx - Bx, ay = Cy - By;
					float bx = Ax - Cx, by = Ay - Cy;
					float cx = Bx - Ax, cy = By - Ay;
					float apx = Px - Ax, apy = Py - Ay;
					float bpx = Px - Bx, bpy = Py - By;
					float cpx = Px - Cx, cpy = Py - Cy;
					float aCROSSbp = ax * bpy - ay * bpx;
					float cCROSSap = cx * apy - cy * apx;
					float bCROSScp = bx * cpy - by * cpx;
					if ( ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f)) ) {
						snip = false;
						break;
					}
				}
			}
			
			if ( snip ) {
				renderTris.push_back( V[ u ] );
				renderTris.push_back( V[ v ] );
				renderTris.push_back( V[ w ] );
				m++;
				
				// remove v from remaining polygon
				for( int s = v, t = v + 1; t < nv; s++, t++ ) V[s] = V[t]; nv--;
				
				// reset error detection counter
				count = 2 * nv;
			}
		}
		
		delete[] V;
	}
	
}


/* MARK:	-				Render
 -------------------------------------------------------------------- */


/// render callback
void RenderShapeBehavior::Render( RenderShapeBehavior* behavior, GPU_Target* target, Event* event ) {
	
	// setup
	GPU_SetLineThickness( behavior->lineThickness );
	
	// blend
	if ( behavior->blendMode == BlendMode::Cut ) {
		// cut alpha
		GPU_SetShapeBlendFunction( GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetShapeBlendEquation( GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	} else {
		// normal mode
		GPU_SetShapeBlendFunction( GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE_MINUS_SRC_ALPHA, GPU_FUNC_SRC_ALPHA, GPU_FUNC_ONE );
		GPU_SetShapeBlendEquation( GPU_EQ_ADD, GPU_EQ_ADD);
		
	}
	
	SDL_Color color = behavior->color->rgba;
	SDL_Color outlineColor = behavior->outlineColor->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	outlineColor.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0 && ( outlineColor.a == 0 && behavior->lineThickness == 0.0 ) ) return;
	
	// set textureless shader
	behavior->SelectUntexturedShader( target, (GPU_Target**) event->behaviorParam2 );
	
	// draw based on type
	GPU_Rect rect;
	switch( behavior->shapeType ){
		
		case ShapeType::Line:
			GPU_Line( target, 0, 0, behavior->x, behavior->y, color );
			break;
			
		case ShapeType::Arc:
			if ( behavior->filled ) {
				GPU_ArcFilled( target, behavior->x, behavior->y, behavior->radius, behavior->startAngle, behavior->endAngle, color );
				if ( outlineColor.a && behavior->lineThickness > 0.0 )
					GPU_Arc( target, behavior->x, behavior->y, behavior->radius, behavior->startAngle, behavior->endAngle, outlineColor );
			} else {
				GPU_Arc( target, behavior->x, behavior->y, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			}
			break;
			
		case ShapeType::Circle:
			if ( behavior->centered ) {
				if ( behavior->filled ) {
					GPU_CircleFilled( target, 0, 0, behavior->radius, color );
					if ( outlineColor.a && behavior->lineThickness > 0.0 )
						GPU_Circle( target, 0, 0, behavior->radius, outlineColor );
				} else {
					GPU_Circle( target, 0, 0, behavior->radius, color );
				}
			} else {
				if ( behavior->filled ) {
					GPU_CircleFilled( target, behavior->radius, behavior->radius, behavior->radius, color );
					if ( outlineColor.a && behavior->lineThickness > 0.0 )
						GPU_Circle( target, behavior->radius, behavior->radius, behavior->radius, outlineColor );
				} else {
					GPU_Circle( target, behavior->radius, behavior->radius, behavior->radius, color );
				}
			}
			break;
			
		case ShapeType::Ellipse:
			if ( behavior->centered ) {
				if ( behavior->filled ) {
					GPU_EllipseFilled( target, 0, 0, behavior->x, behavior->y, 0, color );
					if ( outlineColor.a && behavior->lineThickness > 0.0 )
						GPU_Ellipse( target, 0, 0, behavior->x, behavior->y, 0, outlineColor );
				} else {
					GPU_Ellipse( target, 0, 0, behavior->x, behavior->y, 0, color );
				}
			} else {
				if ( behavior->filled ) {
					GPU_EllipseFilled( target, behavior->x, behavior->y, behavior->x, behavior->y, 0, color );
					if ( outlineColor.a && behavior->lineThickness > 0.0 )
						GPU_Ellipse( target, behavior->x, behavior->y, behavior->x, behavior->y, 0, outlineColor );
				} else {
					GPU_Ellipse( target, behavior->x, behavior->y, behavior->x, behavior->y, 0, color );
				}
			}
			break;
			
		case ShapeType::Sector:
			if ( behavior->filled ) {
				GPU_SectorFilled( target, behavior->x, behavior->y, behavior->innerRadius, behavior->radius, behavior->startAngle, behavior->endAngle, color );
				if ( outlineColor.a && behavior->lineThickness > 0.0 )
					GPU_Sector( target, behavior->x, behavior->y, behavior->innerRadius, behavior->radius, behavior->startAngle, behavior->endAngle, outlineColor );
			} else {
				GPU_Sector( target, behavior->x, behavior->y, behavior->innerRadius, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			}
			break;
			
		case ShapeType::Triangle:
			if ( behavior->filled ) {
				GPU_TriFilled( target, behavior->x, behavior->y, behavior->x1, behavior->y1, behavior->x2, behavior->y2, color );
				if ( outlineColor.a && behavior->lineThickness > 0.0 ) {
					float fv[ 6 ] = { behavior->x, behavior->y, behavior->x1, behavior->y1, behavior->x2, behavior->y2 };
					GPU_Polyline( target, 3, fv, outlineColor, true );
				}
			} else {
				float fv[ 6 ] = { behavior->x, behavior->y, behavior->x1, behavior->y1, behavior->x2, behavior->y2 };
				GPU_Polyline( target, 3, fv, color, true );
			}
			break;
			
		case ShapeType::Rectangle:
			rect = { 0, 0, behavior->x, behavior->y };
			if ( behavior->centered ) {
				rect.x = -behavior->x * 0.5;
				rect.y = -behavior->y * 0.5;
			}
			if ( behavior->filled ) {
				GPU_RectangleFilled2( target, rect, color );
				if ( outlineColor.a && behavior->lineThickness > 0.0 )
					GPU_Rectangle2( target, rect, outlineColor );
			} else {
				GPU_Rectangle2( target, rect, color );
			}
			break;
			
		case ShapeType::RoundedRectangle:
			rect = { 0, 0, behavior->x, behavior->y };
			if ( behavior->centered ) {
				rect.x = -behavior->x * 0.5;
				rect.y = -behavior->y * 0.5;
			}
			if ( behavior->filled ) {
				GPU_RectangleRoundFilled2( target, rect, behavior->radius, color );
				if ( outlineColor.a && behavior->lineThickness > 0.0 )
					GPU_RectangleRound2( target, rect, behavior->radius, outlineColor );
			} else {
				GPU_RectangleRound2( target, rect, behavior->radius, color );
			}
			break;
			
		case ShapeType::Chain: // line segments, not closed
		case ShapeType::Polygon:
			
			if ( behavior->shapeType == Polygon && behavior->filled ) {
				// decompose into triangle strips
				if ( behavior->_renderPointsDirty ) behavior->UpdatePoints();

				// setup
				target->color = color;
				target->use_color = true;
				
				// render
				vector<float>* points = behavior->polyPoints->ToFloatVector();
				GPU_PrimitiveBatch( NULL, target,
								   GPU_TRIANGLES,
								   points->size() / 2, points->data(),
								   (int) behavior->renderTris.size(), behavior->renderTris.data(), GPU_BATCH_XY );
				
				// unset color
				target->color = { 255, 255, 255, 255 };
				target->use_color = false;
				if ( outlineColor.a && behavior->lineThickness > 0.0 )
					GPU_Polyline( target, (int) points->size() / 2, points->data(), outlineColor, ( behavior->shapeType == Polygon ) );
				
			} else {
				vector<float>* fv = behavior->polyPoints->ToFloatVector();
				GPU_Polyline( target, (int) fv->size() / 2, fv->data(), color, ( behavior->shapeType == Polygon ) );
			}
			break;
		
		default:
			break;
	}
	
}


/* MARK:	-				Shape from render
 -------------------------------------------------------------------- */


RigidBodyShape* RenderShapeBehavior::MakeShape() {

	// TODO
	return NULL;
}

