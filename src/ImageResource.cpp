#include "ImageResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// create
ImageResource::ImageResource( const char* ckey ) {
	
	this->key = ckey;
	
	// split path into chunks
	bool startsWithSlash = ( ckey[ 0 ] == '/' );
	vector<string> parts = Resource::splitString( key, string( "/" ) );
	
	// get file extension
	string::size_type extPos = parts[ parts.size() - 1 ].find_last_of( '.' );
	string extension = "", filename = key, path, path2;
	
	// clear frame struct
	memset( &this->frame, 0, sizeof( ImageFrame ) );
	
	// filename included extension?
	if ( extPos != string::npos ) {
		
		// load as is
		filename = key.substr( 0, extPos );
		extension = key.substr( extPos );
		
	// filename without extension
	} else {
		
		// if more than one part
		if ( parts.size() > 1 ) {
			
			// if png file with full pathname doesn't exist
			path = app.currentDirectory + ( startsWithSlash ? "" : app.texturesDirectory ) + filename;
			if ( access( (path + ".png").c_str(), R_OK ) == -1 &&
				 access( (path + ".jpg").c_str(), R_OK ) == -1 ) {
				
				// but png and json without last path chunk do
				path2 = Resource::concatStrings( parts, "/", (int) parts.size() - 1 );
				path = app.currentDirectory + ( startsWithSlash ? "/" : app.texturesDirectory ) + path2;
				if ( ( access( (path + ".png").c_str(), R_OK ) != -1 || access( (path + ".jpg").c_str(), R_OK ) != -1 )&&
					access( (path + ".json").c_str(), R_OK ) != -1 ) {
					
					// prepend /
					if ( startsWithSlash ) path2 = "/" + path2;
					
					// add that resource first
					ImageResource* res = app.textureManager.Get( path2.c_str() );
					this->mainResource = res;
					
					// locate needed frame
					ImageFramesIterator it = this->mainResource->frames.find( parts[ parts.size() - 1 ] );
					if ( it == this->mainResource->frames.end() ) {						
						printf( "Frame %s not found.\n", filename.c_str() );
						this->error = ERROR_NOT_FOUND;
						return;
					}
					
					// copy frame info
					this->frame = it->second;
					return;
					
				}
				
			}
			
		}
		
		// load json from file
		path = app.currentDirectory + ( startsWithSlash ? "" : app.texturesDirectory ) + filename + ".json";
		FILE *f = fopen( (char*) path.c_str(), "r" );
		if ( f != NULL ) {
			// get file size
			fseek( f, 0, SEEK_END );
			size_t fsize = (size_t) ftell( f );
			fseek( f, 0, SEEK_SET );
			// read file in
			char *buf = (char*) malloc( sizeof( char ) * fsize + 1 );
			buf[ fsize ] = 0;
			fread(buf, sizeof(char), fsize, f );
			string jsonString = buf;
			free( buf );
			fclose( f );
			
			// evaluate json to object
			void *json = NULL;
			if ( ( json = script.ParseJSON( jsonString.c_str() ) ) ) {
				
				// { frames: {....} }
				ArgValue framesObj = script.GetProperty( "frames", json );
				if ( framesObj.type == TypeObject && framesObj.value.objectValue ) {
					// get names
					unordered_set<string> frameNames;
					script.GetPropertyNames( framesObj.value.objectValue, frameNames );
					unordered_set<string>::iterator it = frameNames.begin(), end = frameNames.end();
					string keyName;
					
					// for each frame
					while ( it != end ){
						
						// { frame:{x,y,w,h},rotated,trimmed,spriteSourceSize:{x,y,w,h},sourceSize:{w,h},pivot:{x,y} }
						ArgValue frameObj = script.GetProperty( (*it).c_str(), framesObj.value.objectValue );
						
						// strip extension from key
						size_t extPos = (*it).find_last_of( '.' );
						keyName = (extPos != string::npos ? (*it).substr( 0, extPos ) : (*it));
						
						// populate
						ImageFrame frameInfo;
						
						// .rotated
						ArgValue rotated = script.GetProperty( "rotated", frameObj.value.objectValue );
						frameInfo.rotated = rotated.toBool();
						
						// .frame
						ArgValue subObj = script.GetProperty( "frame", frameObj.value.objectValue );
						if ( subObj.type == TypeObject ) {
							ArgValue x = script.GetProperty( "x", subObj.value.objectValue );
							ArgValue y = script.GetProperty( "y", subObj.value.objectValue );
							ArgValue w = script.GetProperty( "w", subObj.value.objectValue );
							ArgValue h = script.GetProperty( "h", subObj.value.objectValue );
							x.toNumber(frameInfo.locationOnTexture.x);
							y.toNumber(frameInfo.locationOnTexture.y);
							w.toNumber( frameInfo.rotated ? frameInfo.locationOnTexture.h : frameInfo.locationOnTexture.w );
							h.toNumber( frameInfo.rotated ? frameInfo.locationOnTexture.w : frameInfo.locationOnTexture.h );
						} else {
							printf( "Error while loading \"%s.json\"", filename.c_str() );
							break;
						}
						
						// .sourceSize
						subObj = script.GetProperty( "sourceSize", frameObj.value.objectValue );
						if ( subObj.type == TypeObject ) {
							ArgValue w = script.GetProperty( "w", subObj.value.objectValue );
							ArgValue h = script.GetProperty( "h", subObj.value.objectValue );
							w.toNumber( frameInfo.actualWidth );
							h.toNumber( frameInfo.actualHeight );
						} else {
							printf( "Error while loading \"%s.json\"", filename.c_str() );
							break;
						}

						// .spriteSourceSize
						subObj = script.GetProperty( "spriteSourceSize", frameObj.value.objectValue );
						if ( subObj.type == TypeObject ) {
							ArgValue x = script.GetProperty( "x", subObj.value.objectValue );
							ArgValue y = script.GetProperty( "y", subObj.value.objectValue );
							x.toNumber( frameInfo.trimOffsetX );
							y.toNumber( frameInfo.trimOffsetY );
						} else {
							printf( "Error while loading \"%s.json\"", filename.c_str() );
							break;
						}
						
						// add to map
						this->frames.insert( make_pair( keyName, frameInfo ) );
						it++;
					}
				
				} else {
					printf( "Error while loading \"%s.json\": object doesn't contain 'frames' property.\n", filename.c_str() );
				}
			// failed to parse
			} else {
				printf( "Error while loading \"%s.json\": bad JSON\n", filename.c_str() );
			}
			
		}
		
		// add ext
		path = app.currentDirectory + ( startsWithSlash ? "" : app.texturesDirectory ) + filename;
		TryFileExtensions( path.c_str(), "png,jpg", extension );
		
	}
	
	// load from file
	path = app.currentDirectory + ( startsWithSlash ? "" : app.texturesDirectory ) + filename + extension;
	SDL_Surface* surface = IMG_Load( path.c_str() );
	if ( surface == NULL ) {
		printf( "%s was not found", path.c_str() );
		this->error = ERROR_NOT_FOUND;
		return;
	}
	
	// init bounds
	this->frame.locationOnTexture = { 0, 0, (float) surface->clip_rect.w, (float) surface->clip_rect.h };
	this->frame.actualWidth = this->frame.locationOnTexture.w;
	this->frame.actualHeight = this->frame.locationOnTexture.h;
	
	// convert to image
	this->image = GPU_CopyImageFromSurface( surface );
	this->image->anchor_x = this->image->anchor_y = 0; // reset
	GPU_SetImageFilter( this->image, GPU_FILTER_NEAREST );
	GPU_SetSnapMode( this->image, GPU_SNAP_NONE );
	
	printf( "Loaded %s - %p\n", path.c_str(), this->image );
	
	// destroy surface
	SDL_FreeSurface( surface );
	
}

/// adjust use count
void ImageResource::AdjustUseCount( int increment ) {
	// if this is a image map, forward call
	if ( this->mainResource ) this->mainResource->AdjustUseCount( increment );
	else Resource::AdjustUseCount( increment );
}

/// override base class
bool ImageResource::CanUnload() {
	// if this is a image map, forward call
	if ( this->mainResource ) return this->mainResource->CanUnload();
	else return Resource::CanUnload();
}

// destroy
ImageResource::~ImageResource() {

	//
	printf( "Unloading image %s\n", this->key.c_str() );

	// unload
	if ( this->image ) GPU_FreeImage( this->image );
	
}
