#ifndef Image_hpp
#define Image_hpp

#include "common.h"
#include "GameObject.hpp"

/// class for drawing to texture
class Image : public ScriptableClass {
public:
	
	// render target
	GPU_Image* image = NULL;
	GPU_Image* mask = NULL;
	GPU_Target* blendTarget = NULL;
	
	// stored width / height
	int width = 0;
	int height = 0;
		
	/// force redraw this object each frame
	GameObject* autoDraw = NULL;
	GameObject* autoMask = NULL;
	bool autoMaskInverted = false;
	
	/// keeps track of last redraw
	int lastRedrawFrame = 0;
	
	/// helper to make/replace current image
	GPU_Image* MakeImage( bool makingMask=false );
	
	/// draw gameobject
	void Draw( GameObject* go, bool toMask=false, float x=0, float y=0, float angle=0, float scaleX=1, float scaleY=1 );
	
	/// applies mask to image
	void ApplyMask( bool inverted );
	
	/// save
	void Save( const char* filename );
	
	/// gets base64 encoded png
	bool ToDataURL( ArgValue& val );
	
	/// inits image from base64 encoded png
	bool FromDataURL( string &s );
	
	/// creates image from texture
	bool FromTexture( string &s );
	
	/// returns updated image
	GPU_Image* GetImage();
	
	// scripting
	
	static void InitClass();
	
	/// garbage collection callback
	void TraceProtectedObjects( vector<void **> &protectedObjects );
	
	// init/destroy
	Image();
	Image( ScriptArguments* args );
	~Image();
	
};

SCRIPT_CLASS_NAME( Image, "Image" );

#endif /* Image_hpp */
