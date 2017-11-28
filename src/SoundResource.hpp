#ifndef SoundResource_hpp
#define SoundResource_hpp

#include "ResourceManager.hpp"

/* MARK:	-				Sound resource
 -------------------------------------------------------------------- */

class SoundResource : public Resource {
public:
	
	Mix_Chunk* sample = NULL;
	Mix_Music* music = NULL;
	
	// init, destroy
	SoundResource( const char* ckey );
	~SoundResource();
};


#endif /* SoundResource_hpp */
