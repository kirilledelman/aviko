#ifndef SceneDebugDraw_hpp
#define SceneDebugDraw_hpp

#include "common.h"

#define MAX_DEBUG_POLY_VERTS 512

class SceneDebugDraw : public b2Draw {
public:

	/// buffer to hold body verts for drawing
	float verts[ MAX_DEBUG_POLY_VERTS * 2 ];
	
	/// helper to copy b2Vec2 vertices to verts buffer for drawing
	int FillVertsBuffer( const b2Vec2* vertices, int32 vertexCount );
	
	/// Draw a closed polygon provided in CCW order.
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	
	/// Draw a solid closed polygon provided in CCW order.
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	
	/// Draw a circle.
	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
	
	/// Draw a solid circle.
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	
	/// Draw a particle array
	void DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count);
	
	/// Draw a line segment.
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	
	/// Draw a transform. Choose your own length scale.
	void DrawTransform(const b2Transform& xf);
	
	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);
	
};

#endif /* SceneDebugDraw_hpp */
