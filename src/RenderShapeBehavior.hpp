#ifndef RenderShapeBehavior_hpp
#define RenderShapeBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"

class RenderShapeBehavior: public RenderBehavior {
public:
	
	// init, destroy
	RenderShapeBehavior( ScriptArguments* args );
	RenderShapeBehavior();
	~RenderShapeBehavior();
	
	typedef enum {
		/// line from 0, 0 to x / y
		Line,
		/// arc for a circle at x, y, radius, start->end angle
		Arc,
		/// circle with radius
		Circle,
		/// ellipse x wide, y tall
		Ellipse,
		/// sector of a donut with radius, hole innerRadius, start->end angle
		Sector,
		/// triangle between x,y,x1,y1,x2,y2
		Triangle,
		/// rectangle x wide, y tall
		Rectangle,
		/// rectangle x wide, y tall, radius corner
		RoundedRectangle,
		/// polygon between polyPoints (x,y pairs)
		Polygon,
		///
		None
	} ShapeType;
	
	/// shape type
	ShapeType shapeType = Circle;
	
	/// fill shape
	bool filled = false;
	
	/// center circles etc.
	bool centered = true;
	
	/// radius used for circles, ellipses, arcs, sectors
	float radius = 10;
	
	/// innerRadius used for sectors
	float innerRadius = 5;
	
	/// x or width
	float x = 0;
	
	/// y or height
	float y = 0;
	
	/// x for triangle's second point
	float x1 = 0;
	
	/// y for triangle's second point
	float y1 = 0;

	/// x for triangle's third point
	float x2 = 0;
	
	/// y for triangle's third point
	float y2 = 0;
	
	/// start angle for arcs
	float startAngle = 0;
	
	/// end angle for arcs
	float endAngle = 359.999;
	
	/// holds x,y point pairs for GPU_Polygon
	vector<float> polyPoints;
	
	/// line thickness
	float lineThickness = 3;
		
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
// methods
	
	/// render callback
	static void Render( RenderShapeBehavior* behavior, GPU_Target* target, Event* event );
	
	/// overridden from RenderBehavior
	bool IsScreenPointInside( float screenX, float screenY, float* outLocalX, float* outLocalY );
	
	/// returns .points as new ArgValueVector (delete it after using)
	ArgValueVector* GetPoints();
	
	/// updates .points from ArgValueVector
	void SetPoints( ArgValueVector* );
	
};

float DistanceToLine( float px, float py, float x1, float y1, float x2, float y2 );
bool InsideArc( float px, float py, float cx, float cy, float r, float a1, float a2 );
bool InsideEllipse( float px, float py, float cx, float cy, float rx, float ry );
bool InsidePolygon( float px, float py, float* points, int numPoints );


SCRIPT_CLASS_NAME( RenderShapeBehavior, "RenderShape" );

#endif /* RenderShapeBehavior_hpp */
