#include "FontResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// create
FontResource::FontResource( const char* ckey ) {

	this->key = ckey;
		
	// split path into chunks
	bool startsWithSlash = ( ckey[ 0 ] == '/' );
	vector<string> parts = Resource::splitString( key, string( "/" ) );
	
	// extract ,fontSize from the end of key
	string::size_type commaPos = parts.back().find_last_of( ',' );
	if ( commaPos == string::npos ) {
		this->error = ERROR_NOT_FOUND;
		printf( "Font path %s doesn't contain font size specifier.\n", ckey );
		return;
	}
	string fsize = parts.back().substr( commaPos + 1 );
	try {
		this->size = stoi( fsize );
	} catch ( std::invalid_argument err ) {
		this->error = ERROR_NOT_FOUND;
		printf( "Font path %s font size specifier is invalid.\n", ckey );
		return;
	}
	parts.back() = parts.back().substr( 0, commaPos );
	key = Resource::concatStrings( parts, "/" );
	
	// get file extension
	string::size_type extPos = parts.back().find_last_of( '.' );
	string extension = "", filename = key, path;
	
	// filename included extension?
	if ( extPos != string::npos ) {
		
		// load as is
		filename = key.substr( 0, extPos );
		extension = key.substr( extPos );
		
	// filename without extension
	} else {
	
		// add ext
		extension = ".ttf";
		
	}
	
	// load
	path = app.currentDirectory + ( startsWithSlash ? "" : app.fontsDirectory ) + filename + extension;
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
	
}

// release
FontResource::~FontResource (){

	printf( "Unloading font %s,%d\n", this->key.c_str(), this->size );
	
	// clean up
	if ( this->font ) TTF_CloseFont( this->font );
	
}
