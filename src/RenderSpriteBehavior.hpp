#ifndef RenderSpriteBehavior_hpp
#define RenderSpriteBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"
#include "Image.hpp"

class ImageResource;

class RenderSpriteBehavior: public RenderBehavior {
public:
	
	// init, destroy
	RenderSpriteBehavior( ScriptArguments* args );
	RenderSpriteBehavior( const char* imageResourceName );
	RenderSpriteBehavior();
	~RenderSpriteBehavior();
	
	/// texture for drawing
	ImageResource* imageResource = NULL;
	
	/// Image for drawing
	Image* imageInstance = NULL;
	
	/// image width from texture frame
	float width = 0;
	
	/// image height from texture frame
	float height = 0;
	
	/// slice texture
	GPU_Rect slice = { 0, 0, 0, 0 }; // x, y, w, h -> top, right, bottom, left
		
	/// flip horizontally
	bool flipX = false;

	/// flip vertically
	bool flipY = false;
		
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
// ui
	
	GPU_Rect GetBounds();
	
	/// UIs without layout handler will call this on gameObject's render component
	void Resize( float w, float h );
	
// methods
		
	/// render callback
	static void Render( RenderSpriteBehavior* behavior, GPU_Target* target, Event* event );
	
};

SCRIPT_CLASS_NAME( RenderSpriteBehavior, "RenderSprite" );

#endif /* RenderSpriteBehavior_hpp */
