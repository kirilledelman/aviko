#include "SoundResource.hpp"
#include "ScriptHost.hpp"
#include "Application.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


SoundResource::SoundResource( const char* originalKey, string& path, string& extension ) {
	
	// load
	if ( access( path.c_str(), R_OK ) == -1 ) {
		printf( "Sound %s was not found\n", path.c_str() );
		this->error = ERROR_NOT_FOUND;
		return;
	}
	
	// sample?
	if ( !extension.compare( "wav" ) || !extension.compare( "aiff" ) || !extension.compare( "riff" ) || !extension.compare( "ogg" ) ){
		this->sample = Mix_LoadWAV( path.c_str() );
		if( !this->sample ) {
			printf( "Sound %s couldn't be loaded: %s\n", path.c_str(), Mix_GetError() );
			this->error = ERROR_COMPILE;
		}
	// assume music
	} else {
		this->music = Mix_LoadMUS( path.c_str() );
		if( !this->music ) {
			printf( "Sound %s couldn't be loaded: %s\n", path.c_str(), Mix_GetError() );
			this->error = ERROR_COMPILE;
		}
	}
	
}

//
string SoundResource::ResolveKey( const char* ckey, string& fullpath, string& extension ) {
	
	fullpath = ResolvePath( ckey, "wav,aiff,riff,ogg,voc,mp3,midi,mod,flac", extension, app.soundsDirectory.c_str() );
	return fullpath.substr( app.currentDirectory.length() );
	
}



SoundResource::~SoundResource() {
	
	printf( "Unloading sound %s\n", this->key.c_str() );
	
	
}
