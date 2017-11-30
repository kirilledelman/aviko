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
	
	/// holds x,y points for GPU_Polygon
	vector<float> polyPoints;
	
	/// line thickness
	float lineThickness = 2;
	
	/// if true, rectangles and circles are centered at 0, 0
	bool centered = true;
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
// methods
	
	/// render callback
	static void Render( RenderShapeBehavior* behavior, GPU_Target* target, Event* event );
	
	/// returns .points as new ArgValueVector (delete it after using)
	ArgValueVector* GetPoints();
	
	/// updates .points from ArgValueVector
	void SetPoints( ArgValueVector* );
	
};

SCRIPT_CLASS_NAME( RenderShapeBehavior, "RenderShape" );

#endif /* RenderShapeBehavior_hpp */
