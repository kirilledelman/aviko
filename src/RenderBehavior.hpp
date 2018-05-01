#ifndef RenderBehavior_hpp
#define RenderBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

// shader features mask
#define SHADER_BASE		0x0
#define SHADER_TEXTURE	0x1
#define SHADER_BLEND	0x2
#define SHADER_SLICE	0x4
#define SHADER_TILE		0x8
#define SHADER_STIPPLE	0x10

/// rendering behaviors should inherit from this class and override getBounds method
class RenderBehavior : public Behavior {
public:
	
	// init, destroy
	RenderBehavior( ScriptArguments* args );
	RenderBehavior();
	~RenderBehavior();
	
	// ui
	
	/// (overriden in child classes) returns local-space bounding box
	virtual GPU_Rect GetBounds(){ return GPU_Rect(); }
	
	/// returns true if screen space point is inside this GameObject/RenderBehavior
	virtual bool IsScreenPointInside( float x, float y, float* outLocalX, float* outLocalY );
	
	/// UIs without layout handler will call this on gameObject's render component
	virtual void Resize( float w, float h ) {};
	
	/// color
	Color *color = NULL;
	Color *addColor = NULL;
	
	// rendering pivot / offset ( range 0.0 - 1.0 )
	float pivotX = 0;
	float pivotY = 0;
	bool _pivotDirty = true;

	// texture padding
	int texturePad = 0;
	
	// stipple transparency
	float stipple = 0;
	
	// apply stipple pattern to alpha channel
	bool stippleAlpha = false;
	
	/// blend modes
	enum BlendMode {
		Normal = 0,
		Add = 1,
		Subtract = 2,
		Multiply = 3,
		Screen = 4,
		Burn = 5,
		Dodge = 6,
		Invert = 7,
		Hue = 8,
		Saturation = 9,
		Luminosity = 10,
		Refract = 11,
		Cut = 12
	};
	
	BlendMode blendMode = BlendMode::Normal;
	
	/// sprite rendering shader variant (lets us enable shader with only required features turned on/off)
	typedef struct {
		Uint32 shader;
		GPU_ShaderBlock shaderBlock;
		int addColorUniform;
		int tileUniform;
		int texInfoUniform; // x,y,w,h of sprite on texture, in px
		int texSizeUniform; // w,h of whole texture in px, not incl texPad
		int texPadUniform; // empty margin added around texture in px
		int sliceUniform; // top, right, bottom, left slice margins in px, from respective sides
		int sliceScaleUniform; // scale of actual sprite without pad in px
		int stippleUniform;
		int stippleAlphaUniform;
		int backgroundUniform;
		int backgroundSizeUniform;
		int blendUniform;
	} SpriteShaderVariant;
	
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	/// creates color objects
	void AddDefaults();
	
// shaders
	
	///
	static SpriteShaderVariant shaders[ 32 ];

	/// applies current shader + params
	int SelectTexturedShader(
					 float tw = 0, float th = 0,
					 float u = 0, float v = 0, float w = 0, float h = 0,
					 float st = 0, float sr = 0, float sb = 0, float sl = 0,
					 float sw = 0, float sh = 0,
					 float tx = 1, float ty = 1,
					 GPU_Target* targ = NULL, GPU_Target** blendTarg = NULL );
	
	/// draws target to blendTarget
	void _UpdateBlendTarget( GPU_Target* targ, GPU_Target** blendTarg );
	
	/// selects untextured shader
	int SelectUntexturedShader( GPU_Target* targ = NULL, GPU_Target** blendTarg = NULL );
	
	/// resets shader to default
	void ResetShader() { GPU_ActivateShaderProgram( 0, NULL ); }
	
	/// creates common shaders
	static void InitShaders();
	
	/// helper method
	static bool CompileShader( Uint32& outShader, GPU_ShaderBlock& outShaderBlock, const char* vertShader, const char* fragShader );
	
};

SCRIPT_CLASS_NAME( RenderBehavior, "RenderBehavior" );

#endif /* RenderBehavior_hpp */
