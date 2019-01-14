#ifndef RenderBehavior_hpp
#define RenderBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

// shader features mask
#define SHADER_BASE		0x0
#define SHADER_TEXTURE	0x1
#define SHADER_BLEND	0x2
#define SHADER_SLICE	0x4
#define SHADER_WAVE		0x8
#define SHADER_STIPPLE	0x10
#define SHADER_OUTLINE	0x20
#define SHADER_PARTICLE 0x40
#define SHADER_MAXVAL	0x80

class RigidBodyShape;
class RenderSpriteBehavior;

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
	
	/// return true, if mouse events going to c should be clipped by this render component
	virtual bool ClipsMouseEventsFor( GameObject* c );
	
	/// color
	Color *color = NULL;
	Color *addColor = NULL;
	
	// rendering pivot / offset ( range 0.0 - 1.0 )
	float pivotX = 0;
	float pivotY = 0;
	bool _pivotDirty = true;

	// texture padding
	int texturePad = 0;
    float offsetX=0;
    float offsetY=0;
    
    // alpha thresh (particles)
    float alphaThresh = 0;
	
	/// called after effect type of params change to reserve appropriate padding
	void UpdateTexturePad();
	
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
		Colorize = 8,
		Hue = 9,
		Saturation = 10,
		Luminosity = 11,
		Refract = 12,
		Cut = 13
	};
	
	BlendMode blendMode = BlendMode::Normal;
	
	Color *outlineColor = NULL;
	float outlineOffsetX = 0, outlineOffsetY = 0;
	float outlineRadius = 0;
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	/// creates color objects
	void AddDefaults();
	
	// garbage collector
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
// shaders
	
	/// sprite rendering shader variant (lets us enable shader with only required features turned on/off)
	typedef struct {
		Uint32 shader;
		GPU_ShaderBlock shaderBlock;
		int addColorUniform;
		int tileUniform; // 0,0 = no texture
		int texInfoUniform; // x,y,w,h of sprite on texture, in px
		int texSizeUniform; // w,h of whole texture in px, not incl texPad, or framing
		int texPadUniform; // empty margin added around texture in px
		int sliceUniform; // top, right, bottom, left slice margins in px, from respective sides, 0,0,0,0 - no slicing
		int sliceScaleUniform; // scale of actual sprite without pad in px
		int scrollOffsetUniform; // scroll
        int stippleUniform; // 0 = no stipple
		int stippleAlphaUniform;
		int backgroundUniform; // used with blending, or with particles, supplying background image
		int backgroundSizeUniform; // used with blending, or when rendering particles, supplying size of background tex
		int blendUniform; // 0 for default blending
		int outlineColorUniform;
		int outlineOffsetRadiusUniform;
        int alphaThreshUniform; // < 0 - particle mode
        
	} ShaderVariant;

	/// shader
	static ShaderVariant mainShader;

	/// draws target to blendTarget
	void _UpdateBlendTarget( GPU_Target* targ, GPU_Target** blendTarg );
	
    /// sets params for basic shader
    static void SelectBasicShader( GPU_Image* textured );
    
	/// applies current shader + params
	void SelectTexturedShader(
					 float tw = 0, float th = 0,
					 float u = 0, float v = 0, float w = 0, float h = 0,
					 float st = 0, float sr = 0, float sb = 0, float sl = 0,
					 float sw = 0, float sh = 0,
					 float tx = 1, float ty = 1,
                     float ox = 0, float oy = 0,
					 GPU_Image *image = NULL, GPU_Target* targ = NULL, GPU_Target** blendTarg = NULL );
	
	/// selects untextured shader
	void SelectUntexturedShader( GPU_Target* targ = NULL, GPU_Target** blendTarg = NULL );
	
	/// compiles main shader
	static void CompileMainShader();

	/// helper method for shader compilation
	static bool CompileShader( Uint32& outShader, GPU_ShaderBlock& outShaderBlock, const char* vertShader, const char* fragShader );

// shape from render
	
	virtual RigidBodyShape* MakeShape() { return NULL; }

};

SCRIPT_CLASS_NAME( RenderBehavior, "RenderBehavior" );

#endif /* RenderBehavior_hpp */
