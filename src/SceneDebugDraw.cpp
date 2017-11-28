#include "SceneDebugDraw.hpp"
#include "Application.hpp"

/// Draw a closed polygon provided in CCW order.
void SceneDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	
	vertexCount = FillVertsBuffer( vertices, vertexCount );
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Polygon( app.backScreen->target, vertexCount, this->verts, clr);
	
}

/// Draw a solid closed polygon provided in CCW order.
void SceneDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	
	vertexCount = FillVertsBuffer( vertices, vertexCount );
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_PolygonFilled( app.backScreen->target, vertexCount, this->verts, clr);

}

/// Draw a circle.
void SceneDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
	
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Circle( app.backScreen->target, center.x, center.y, radius, clr);
	
}

/// Draw a solid circle.
void SceneDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
	
	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_CircleFilled( app.backScreen->target, center.x, center.y, radius, clr);
	
}

/// Draw a line segment.
void SceneDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {

	SDL_Color clr;
	clr.r = color.r * 255;
	clr.g = color.g * 255;
	clr.b = color.b * 255;
	clr.a = 128;
	GPU_Line( app.backScreen->target, p1.x, p1.y, p2.x, p2.y, clr);
	
}

/// Draw a transform. Choose your own length scale.
/// @param xf a transform.
void SceneDebugDraw::DrawTransform(const b2Transform& xf) {
}

void SceneDebugDraw::DrawParticles(const b2Vec2 *centers, float32 radius, const b2ParticleColor *colors, int32 count) {
}

void SceneDebugDraw::DrawPoint(const b2Vec2 &p, float32 size, const b2Color &color) {
}

int SceneDebugDraw::FillVertsBuffer( const b2Vec2* vertices, int32 vertexCount ) {

	int i;
	for ( i = 0; i < vertexCount && i < MAX_DEBUG_POLY_VERTS; i++ ) {
		this->verts[ i * 2 ] = vertices[ i ].x;
		this->verts[ i * 2 + 1 ] = vertices[ i ].y;
	}
	
	return ( i );
}
