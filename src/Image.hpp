#ifndef Image_hpp
#define Image_hpp

#include "common.h"
#include "GameObject.hpp"

/// class for drawing to texture
class Image : public ScriptableClass {
public:
	
	// render target
	GPU_Image* image = NULL;
	GPU_Target* blendTarget = NULL;
	
	// stored width / height
	int width = 0;
	int height = 0;
	bool _sizeDirty = true;
	
	// drawing offsets
	float x = 0;
	float y = 0;
	float angle = 0;
	float scaleX = 1;
	float scaleY = 1;
	
	/// force redraw this object each frame
	GameObject* autoDraw = NULL;
	
	/// keeps track of last redraw
	int lastRedrawFrame = 0;
	
	/// helper to make/replace current image
	bool MakeImage();
	
	/// draw gameobject
	void Draw( GameObject* go );
	
	/// save
	void Save( const char* filename );
	
	/// gets base64 encoded png
	bool ToDataURL( ArgValue& val );
	
	/// inits image from base64 encoded png
	bool FromDataURL( string &s );
	
	/// returns updated image
	GPU_Image* GetImage();
	
	// scripting
	
	static void InitClass();
	
	// init/destroy
	Image();
	Image( ScriptArguments* args );
	~Image();
	
};

SCRIPT_CLASS_NAME( Image, "Image" );

#endif /* Image_hpp */
