#include "FontResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// create
FontResource::FontResource( const char* originalKey, string& path, string& ext ){

	// extract ,fontSize from the end of key
	string okey = originalKey;
	vector<string> parts = Resource::splitString( okey, "/" );
	string::size_type commaPos = parts.back().find_last_of( ',' );
	if ( commaPos == string::npos ) {
		this->error = ERROR_NOT_FOUND;
		printf( "Font path %s doesn't contain font size specifier.\n", originalKey );
		return;
	}
	string fsize = parts.back().substr( commaPos + 1 );
	try {
		this->size = stoi( fsize );
	} catch ( std::invalid_argument err ) {
		this->error = ERROR_NOT_FOUND;
		printf( "Font path %s font size specifier is invalid.\n", originalKey );
		return;
	}
	
	// load
	if ( access( path.c_str(), R_OK ) == -1 ) {
		printf( "Font %s was not found\n", path.c_str() );
		this->error = ERROR_NOT_FOUND;
	} else {
		this->font = TTF_OpenFont( path.c_str(), this->size );
		if ( !this->font ) {
			printf( "Font %s couldn't be loaded: %s\n", path.c_str(), TTF_GetError() );
			this->error = ERROR_COMPILE;
		}
	}
	if ( !this->error ) this->path = path;
	
}

string FontResource::ResolveKey(const char *ckey, string &fullpath, string &extension ) {
	
	// remove ",fontSize" from the end of key
	string okey = ckey;
	vector<string> parts = Resource::splitString( okey, "/" );
	string::size_type commaPos = parts.back().find_last_of( ',' );
	string sizeSpec = "";
	if ( commaPos != string::npos ) {
		sizeSpec = parts.back().substr( commaPos );
		parts.back() = parts.back().substr( 0, commaPos );
	}
	okey = Resource::concatStrings( parts, "/" );
	
	// get ttf
	extension = "ttf";
	fullpath = ResolvePath( okey.c_str(), "ttf", app.fontsDirectory.c_str() );
	return fullpath.substr( app.currentDirectory.length() ) + sizeSpec;
	
}

// release
FontResource::~FontResource (){

	// clean up
	if ( this->font ) TTF_CloseFont( this->font );
	
}
