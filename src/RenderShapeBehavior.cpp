#include "RenderShapeBehavior.hpp"
#include "ScriptHost.hpp"
#include "GameObject.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
RenderShapeBehavior::RenderShapeBehavior( ScriptArguments* args ) : RenderShapeBehavior() {

	// add scriptObject
	script.NewScriptObject<RenderShapeBehavior>( this );
	
	// create color object
	Color *color = new Color( NULL );
	script.SetProperty( "color", ArgValue( color->scriptObject ), this->scriptObject );

	// create addColor object
	color = new Color( NULL );
	color->SetInts( 0, 0, 0, 0 );
	script.SetProperty( "addColor", ArgValue( color->scriptObject ), this->scriptObject );
	
	// read params
	int pShape = 0;
	vector<ArgValue>* pArray = NULL;
	float p1 = 0, p2 = 0, p3 = 0, p4 = 0, p5 = 0, p6 = 0;
	
	// if arguments are given
	if ( args &&
		( args->ReadArguments( 1, TypeInt, &pShape, TypeFloat, &p1, TypeFloat, &p2, TypeFloat, &p3, TypeFloat, &p4, TypeFloat, &p5, TypeFloat, &p6 ) ||
		  args->ReadArguments( 1, TypeInt, &pShape, TypeArray, &pArray ) ) ) {
		// first is shape
		this->shapeType = (ShapeType) pShape;
		size_t numArgs = args->args.size();
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
			if ( numArgs >= 3 ) {
				this->x = p1;
				this->y = p2;
			}
		} else if ( this->shapeType == RoundedRectangle ) { /// rectangle x wide, y tall, radius corner
			this->x = 30; this->y = 20;
			if ( numArgs >= 3 ) {
				this->x = p1;
				this->y = p2;
				if ( numArgs >= 4 ) {
					this->radius = p3;
				}
			}
		} else if ( this->shapeType == Polygon ) { /// polygon between polyPoints (x,y pairs)
			if ( pArray ) {
				this->SetPoints( pArray );
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
	script.RegisterClass<RenderShapeBehavior>( "Behavior" );
	
	// constants
	
	script.SetGlobalConstant( "SHAPE_NONE", (int) ShapeType::None );
	script.SetGlobalConstant( "SHAPE_ARC", (int) ShapeType::Arc );
	script.SetGlobalConstant( "SHAPE_CIRCLE", (int) ShapeType::Circle );
	script.SetGlobalConstant( "SHAPE_ELLIPSE", (int) ShapeType::Ellipse );
	script.SetGlobalConstant( "SHAPE_LINE", (int) ShapeType::Line );
	script.SetGlobalConstant( "SHAPE_POLYGON", (int) ShapeType::Polygon );
	script.SetGlobalConstant( "SHAPE_RECTANGLE", (int) ShapeType::Rectangle );
	script.SetGlobalConstant( "SHAPE_ROUNDED_RECTANGLE", (int) ShapeType::RoundedRectangle );
	script.SetGlobalConstant( "SHAPE_SECTOR", (int) ShapeType::Sector );
	script.SetGlobalConstant( "SHAPE_TRIANGLE", (int) ShapeType::Triangle );
	
	// properties
	
	script.AddProperty<RenderShapeBehavior>
	( "shape",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderShapeBehavior*) b)->shapeType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderShapeBehavior*) b)->shapeType = (ShapeType) val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "filled",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderShapeBehavior*) b)->filled; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderShapeBehavior*) b)->filled = val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "color",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderShapeBehavior*) b)->color->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		 // replace if it's a color
		 Color* other = script.GetInstance<Color>(val);
		 if ( other ) ((RenderShapeBehavior*) b)->color = other;
		 return ((RenderShapeBehavior*) b)->color->scriptObject;
	 }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "addColor",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ return ((RenderShapeBehavior*) b)->addColor->scriptObject; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		// replace if it's a color
		Color* other = script.GetInstance<Color>(val);
		if ( other ) ((RenderShapeBehavior*) b)->addColor = other;
		return ((RenderShapeBehavior*) b)->addColor->scriptObject;
	}) );
	
	script.AddProperty<RenderShapeBehavior>
	( "blendMode",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderShapeBehavior*) b)->blendMode; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderShapeBehavior*) b)->blendMode = (GPU_BlendPresetEnum) val ); }) );
	
	script.AddProperty<RenderShapeBehavior>
	( "stipple",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->stipple; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->stipple = max( 0.0f, min( 1.0f, val ))); }) );
	
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
	( "thickness",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderShapeBehavior*) b)->lineThickness; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderShapeBehavior*) b)->lineThickness = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "centered",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderShapeBehavior*) b)->centered; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderShapeBehavior*) b)->centered = val ); }) );

	script.AddProperty<RenderShapeBehavior>
	( "points",
	 static_cast<ScriptArrayCallback>([](void *b, ArgValueVector* in ){ return ((RenderShapeBehavior*) b)->GetPoints(); }),
	 static_cast<ScriptArrayCallback>([](void *b, ArgValueVector* in ){ ((RenderShapeBehavior*) b)->SetPoints( in ); return in; }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );
	
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
	
		return InsidePolygon( *outLocalX, *outLocalY, polyPoints.data(), (int) polyPoints.size() / 2 );
		
	}
	
	return false;
	
}



/* MARK:	-				Geometry
 -------------------------------------------------------------------- */


/// returns .points as new ArgValueVector (delete it after using)
ArgValueVector* RenderShapeBehavior::GetPoints() {
	ArgValueVector* out = new ArgValueVector();
	size_t np = this->polyPoints.size();
	out->resize( np );
	for ( size_t i = 0; i < np; i += 2 ) {
		out->at( i ) = ArgValue( this->polyPoints[ i ] );
		out->at( i + 1 ) = ArgValue( this->polyPoints[ i + 1 ] );
	}
	return out;
}

/// updates .points from ArgValueVector
void RenderShapeBehavior::SetPoints( ArgValueVector *in ) {
	size_t np = in ? in->size() : 0;
	if ( np % 2 ) { np++; in->emplace_back( 0.f ); }
	this->polyPoints.resize( np );
	float fval = 0;
	for ( size_t i = 0; i < np; i++ ) {
		if ( !in->at( i ).toNumber(fval) ) fval = 0;
		this->polyPoints.at( i ) = fval;
	}
}


/* MARK:	-				Render
 -------------------------------------------------------------------- */


/// render callback
void RenderShapeBehavior::Render( RenderShapeBehavior* behavior, GPU_Target* target, Event* event ) {
	
	// setup
	GPU_SetLineThickness( behavior->lineThickness );
	
	// blend
	if ( behavior->blendMode <= GPU_BLEND_NORMAL_FACTOR_ALPHA ) {
		// normal mode
		GPU_SetShapeBlendMode( (GPU_BlendPresetEnum) behavior->blendMode );
		// special mode
	} else if ( behavior->blendMode == GPU_BLEND_CUT_ALPHA ) {
		// cut alpha
		GPU_SetShapeBlendFunction( GPU_FUNC_ZERO, GPU_FUNC_DST_ALPHA, GPU_FUNC_ONE, GPU_FUNC_ONE );
		GPU_SetShapeBlendEquation( GPU_EQ_ADD, GPU_EQ_REVERSE_SUBTRACT);
	}
	
	SDL_Color color = behavior->color->rgba;
	color.a *= behavior->gameObject->combinedOpacity;
	if ( color.a == 0.0 ) return;
	
	// set shader
	behavior->SelectShader( false );
	
	// draw based on type
	GPU_Rect rect;
	switch( behavior->shapeType ){
		
		case ShapeType::Line:
			GPU_Line( target, 0, 0, behavior->x, behavior->y, color );
			break;
			
		case ShapeType::Arc:
			if ( behavior->filled ) {
				GPU_ArcFilled( target, behavior->x, behavior->y, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			} else {
				GPU_Arc( target, behavior->x, behavior->y, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			}
			break;
			
		case ShapeType::Circle:
			if ( behavior->centered ) {
				if ( behavior->filled ) {
					GPU_CircleFilled( target, 0, 0, behavior->radius, color );
				} else {
					GPU_Circle( target, 0, 0, behavior->radius, color );
				}
			} else {
				if ( behavior->filled ) {
					GPU_CircleFilled( target, behavior->radius, behavior->radius, behavior->radius, color );
				} else {
					GPU_Circle( target, behavior->radius, behavior->radius, behavior->radius, color );
				}
			}
			break;
			
		case ShapeType::Ellipse:
			if ( behavior->centered ) {
				if ( behavior->filled ) {
					GPU_EllipseFilled( target, 0, 0, behavior->x, behavior->y, 0, color );
				} else {
					GPU_Ellipse( target, 0, 0, behavior->x, behavior->y, 0, color );
				}
			} else {
				if ( behavior->filled ) {
					GPU_EllipseFilled( target, behavior->x, behavior->y, behavior->x, behavior->y, 0, color );
				} else {
					GPU_Ellipse( target, behavior->x, behavior->y, behavior->x, behavior->y, 0, color );
				}
			}
			break;
			
		case ShapeType::Sector:
			if ( behavior->filled ) {
				GPU_SectorFilled( target, behavior->x, behavior->y, behavior->innerRadius, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			} else {
				GPU_Sector( target, behavior->x, behavior->y, behavior->innerRadius, behavior->radius, behavior->startAngle, behavior->endAngle, color );
			}
			break;
			
		case ShapeType::Triangle:
			if ( behavior->filled ) {
				GPU_TriFilled( target, behavior->x, behavior->y, behavior->x1, behavior->y1, behavior->x2, behavior->y2, color );
			} else {
				GPU_Tri( target, behavior->x, behavior->y, behavior->x1, behavior->y1, behavior->x2, behavior->y2, color );
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
			} else {
				GPU_RectangleRound2( target, rect, behavior->radius, color );
			}
			break;
			
		case ShapeType::Polygon:
			if ( behavior->filled ) {
				GPU_PolygonFilled( target, (unsigned int) behavior->polyPoints.size() / 2, behavior->polyPoints.data(), color );
			} else {
				GPU_Polygon( target, (unsigned int) behavior->polyPoints.size() / 2, behavior->polyPoints.data(), color );
			}
			break;
		
		default:
			break;
	}
	
}

