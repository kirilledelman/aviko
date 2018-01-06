#ifndef SoundResource_hpp
#define SoundResource_hpp

#include "ResourceManager.hpp"

/* MARK:	-				Sound resource
 -------------------------------------------------------------------- */

class SoundResource : public Resource {
public:
	
	Mix_Chunk* sample = NULL;
	Mix_Music* music = NULL;
	
	static string ResolveKey( const char* ckey, string& fullpath, string& extension );
	
	// init, destroy
	SoundResource( const char* originalKey, string& path, string& ext );
	~SoundResource();
};


#endif /* SoundResource_hpp */
