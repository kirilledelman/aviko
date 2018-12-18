#ifndef FontResource_hpp
#define FontResource_hpp

#include "ResourceManager.hpp"

/* MARK:	-				Font resource
 
 Loads a TTF font. Key must be in the form "FONTNAME,ptSize"
 -------------------------------------------------------------------- */

class FontResource : public Resource {
public:
	
	TTF_Font* font = NULL;
	int size = 16;
	
	static string ResolveKey( const char* ckey, string& fullpath, string& extension );
	
    bool LoadFromMemory( void* p, int size );
    
	// init, destroy
	FontResource( const char* originalKey, const unsigned char* data );
	FontResource( const char* originalKey, string& path, string& ext );
	~FontResource();
};


#endif /* FontResource_hpp */
