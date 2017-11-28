#include "SoundResource.hpp"
#include "ScriptHost.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

SoundResource::SoundResource( const char* ckey ) {
	
	this->key = ckey;
	
	// split path into chunks
	bool startsWithSlash = ( ckey[ 0 ] == '/' );
	vector<string> parts = Resource::splitString( key, string( "/" ) );
	
	// get file extension
	string::size_type extPos = parts.back().find_last_of( '.' );
	string extension = "", filename = key, path;
	
	// filename included extension?
	if ( extPos != string::npos ) {
		
		// load as is
		filename = key.substr( 0, extPos );
		extension = key.substr( extPos );
		path = app.currentDirectory + ( startsWithSlash ? "" : app.soundsDirectory ) + filename + extension;
	
	// no ext
	} else {
		// try different ones
		path = app.currentDirectory + ( startsWithSlash ? "" : app.soundsDirectory ) + filename;
		if ( !TryFileExtensions( path.c_str(), "wav,aiff,riff,ogg,voc,mp3,midi,mod,flac", extension ) ){
			printf( "Sound %s was not found\n", path.c_str() );
			this->error = ERROR_NOT_FOUND;
			return;
		}
	}
	
	// load
	path = app.currentDirectory + ( startsWithSlash ? "" : app.soundsDirectory ) + filename + extension;
	if ( access( path.c_str(), R_OK ) == -1 ) {
		printf( "Sound %s was not found\n", path.c_str() );
		this->error = ERROR_NOT_FOUND;
		return;
	}
	
	// sample?
	if ( !extension.compare( ".wav" ) || !extension.compare( ".aiff" ) || !extension.compare( ".riff" ) || !extension.compare( ".ogg" ) ){
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

SoundResource::~SoundResource() {
	
	printf( "Unloading sound %s\n", this->key.c_str() );
	
	
}
