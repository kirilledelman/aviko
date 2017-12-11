#ifndef RenderBehavior_hpp
#define RenderBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

// shader features mask
#define SHADER_BASE		0x0
#define SHADER_TEXTURE	0x1
#define SHADER_TILE		0x2
#define SHADER_STIPPLE	0x4

/// rendering behaviors should inherit from this class and override getBounds method
class RenderBehavior : public Behavior {
public:
	
	// ui
	
	/// (overriden in child classes) returns local-space bounding box
	virtual GPU_Rect GetBounds(){ return GPU_Rect(); }
	
	/// returns true if screen space point is inside this GameObject/RenderBehavior
	virtual bool IsScreenPointInside( float x, float y, float* outLocalX, float* outLocalY );
	
	/// color
	Color *color = NULL;
	Color *addColor = NULL;
	
	/// tiling
	float tileX = 1;
	float tileY = 1;

	// stipple transparency
	float stipple = 0;
	
	// apply stipple pattern to alpha channel
	bool stippleAlpha = false;
	
	/// blend mode
	Uint8 blendMode = GPU_BLEND_NORMAL;
	
	/// sprite rendering shader variant (lets us enable shader with only required features turned on/off)
	typedef struct {
		Uint32 shader;
		GPU_ShaderBlock shaderBlock;
		int addColorUniform;
		int tileUniform;
		int texInfoUniform;
		int stippleUniform;
		int stippleAlphaUniform;
	} SpriteShaderVariant;
	
	///
	static SpriteShaderVariant shaders[ 8 ];

	/// applies current shader + params
	int SelectShader( bool textured, bool rotated = false, float u = 0, float v = 0, float w = 1, float h = 1 );

	/// resets shader to default
	void ResetShader() { GPU_ActivateShaderProgram( 0, NULL ); }
	
	/// creates common shaders
	static void InitShaders();
	
	/// helper method
	static bool CompileShader( Uint32& outShader, GPU_ShaderBlock& outShaderBlock, const char* vertShader, const char* fragShader );
	
};

#endif /* RenderBehavior_hpp */
