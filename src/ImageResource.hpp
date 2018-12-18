#ifndef ImageResource_hpp
#define ImageResource_hpp

#include "ResourceManager.hpp"

/* MARK:	-				Image resource
 
 key can be path/to/image.png|jpg - loads image
 or path/to/image - tries to load image.json + image.png, then just image.png
 
 JSON is from slice maps generated by http://free-tex-packer.com
 -------------------------------------------------------------------- */

/// Describes image frame on packed texture
typedef struct {
	GPU_Rect locationOnTexture = { 0, 0, 0, 0 };
	float trimOffsetX = 0, trimOffsetY = 0;
	float actualWidth = 0, actualHeight = 0;
	float trimWidth = 0, trimHeight = 0;
	bool rotated = false;
	bool trimmed = false;
} ImageFrame;

typedef unordered_map<string, ImageFrame> ImageFramesMap;
typedef unordered_map<string, ImageFrame>::iterator ImageFramesIterator;

class ImageResource : public Resource {
public:
	
	/// image frames (if sprite sheet)
	ImageFramesMap frames;
	
	/// this frame
	ImageFrame frame;
	
	/// image reference
	GPU_Image* image = NULL;
	
	/// in sprite sheets points to main resource (image is NULL)
	ImageResource* mainResource = NULL;
	
	// override use count
	void AdjustUseCount( int increment );
	bool CanUnload();
	
	static string ResolveKey( const char* ckey, string& fullpath, string& extension );
	
    bool LoadFromMemory(void *p, int size);
    
	// init, destroy
	ImageResource( const char* originalKey, string& path, string& ext );
	~ImageResource();
	
};


#endif /* ImageResource_hpp */
