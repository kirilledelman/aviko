#include "ImageResource.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

ImageResource::ImageResource( const char* originalKey, string& path, string& ext ) {
	
	// string key
	string okey = originalKey;
	
	// clear frame struct
	memset( &this->frame, 0, sizeof( ImageFrame ) );
	
	// if it's a data url
	if ( originalKey[ 0 ] == '#' ) {
		// convert from base64 using Image class
		Image* img = new Image();
		img->FromDataURL( okey );
		if ( img->image ) {
			this->frame.actualWidth = this->frame.locationOnTexture.w = img->image->base_w;
			this->frame.actualHeight = this->frame.locationOnTexture.h = img->image->base_h;
			this->image = img->image;
			img->image = NULL;
		}
		delete img;
		return;
	}
	
	// if it's a frame
	string::size_type colPos = okey.find_last_of( ':' );
	if ( colPos != string::npos ) {
		// separate name and frame
		string frame = okey.substr( colPos + 1 );
		okey = okey.substr( 0, colPos );
		
		// add main resource first
		ImageResource* res = app.textureManager.Get( okey.c_str() );
		this->mainResource = res;
		
		// locate needed frame
		ImageFramesIterator it = this->mainResource->frames.find( frame );
		if ( it == this->mainResource->frames.end() ) {
			printf( "%s - Frame %s not found.\n", GetScriptNameAndLine().c_str(), originalKey );
			this->error = ERROR_NOT_FOUND;
			return;
		}
		
		// copy frame info
		this->frame = it->second;
		return;
	}
	
	// if loading json + image
	if ( ext.compare( "json" ) == 0 ) {
		
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
					ArgValueVector frameNames;
					script.GetProperties( framesObj.value.objectValue, &frameNames, false, false, false );
					ArgValueVector::iterator it = frameNames.begin(), end = frameNames.end();
					string keyName;
					
					// for each frame
					while ( it != end ){
						
						// { frame:{x,y,w,h},rotated,trimmed,spriteSourceSize:{x,y,w,h},sourceSize:{w,h},pivot:{x,y} }
						ArgValue frameObj = script.GetProperty( (*it).value.stringValue->c_str(), framesObj.value.objectValue );
						
						// strip extension from key
						size_t extPos = (*it).value.stringValue->find_last_of( '.' );
						keyName = (extPos != string::npos ? (*it).value.stringValue->substr( 0, extPos ) : (*it).value.stringValue->c_str() );
						
						// populate
						ImageFrame frameInfo;
						
						// .rotated
						ArgValue rotated = script.GetProperty( "rotated", frameObj.value.objectValue );
						frameInfo.rotated = rotated.toBool();
						
						// .trimmed
						ArgValue trimmed = script.GetProperty( "trimmed", frameObj.value.objectValue );
						frameInfo.trimmed = trimmed.toBool();
						
						// .sourceSize
						ArgValue subObj = script.GetProperty( "sourceSize", frameObj.value.objectValue );
						if ( subObj.type == TypeObject ) {
							ArgValue w = script.GetProperty( "w", subObj.value.objectValue );
							ArgValue h = script.GetProperty( "h", subObj.value.objectValue );
							w.toNumber( frameInfo.actualWidth );
							h.toNumber( frameInfo.actualHeight );
						} else {
							printf( "%s - Error while loading \"%s.json\"", GetScriptNameAndLine().c_str(), path.c_str() );
							break;
						}

						// .frame
						subObj = script.GetProperty( "frame", frameObj.value.objectValue );
						if ( subObj.type == TypeObject ) {
							ArgValue x = script.GetProperty( "x", subObj.value.objectValue );
							ArgValue y = script.GetProperty( "y", subObj.value.objectValue );
							ArgValue w = script.GetProperty( "w", subObj.value.objectValue );
							ArgValue h = script.GetProperty( "h", subObj.value.objectValue );
							x.toNumber(frameInfo.locationOnTexture.x);
							y.toNumber(frameInfo.locationOnTexture.y);
							
							if ( frameInfo.rotated ) {
								w.toNumber( frameInfo.locationOnTexture.h );
								h.toNumber( frameInfo.locationOnTexture.w );
								frameInfo.trimWidth = frameInfo.actualWidth - frameInfo.locationOnTexture.h;
								frameInfo.trimHeight = frameInfo.actualHeight - frameInfo.locationOnTexture.w;
							} else {
								w.toNumber( frameInfo.locationOnTexture.w );
								h.toNumber( frameInfo.locationOnTexture.h );
								frameInfo.trimWidth = frameInfo.actualWidth - frameInfo.locationOnTexture.w;
								frameInfo.trimHeight = frameInfo.actualHeight - frameInfo.locationOnTexture.h;
							}

						} else {
							printf( "%s - Error while loading \"%s.json\"", GetScriptNameAndLine().c_str(), path.c_str() );
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
							printf( "%s - Error while loading \"%s.json\"", GetScriptNameAndLine().c_str(), path.c_str() );
							break;
						}
						
						// add to map
						this->frames.insert( make_pair( keyName, frameInfo ) );
						it++;
					}
					
				} else {
					printf( "%s - Error while loading \"%s.json\": object doesn't contain 'frames' property.\n", GetScriptNameAndLine().c_str(), path.c_str() );
				}
				// failed to parse
			} else {
				printf( "%s - Error while loading \"%s.json\": bad JSON\n", GetScriptNameAndLine().c_str(), path.c_str() );
			}
			
		}
		
		// reattach image extension
		path = path.substr( 0, path.length() - ext.length() - 1 );
		TryFileExtensions( path.c_str(), "png,jpg", ext );
		path = path + ext;
	}
	
	// load image
	SDL_Surface* surface = IMG_Load( path.c_str() );
	if ( surface == NULL ) {
		printf( "%s - Texture %s was not found\n", GetScriptNameAndLine().c_str(), path.c_str() );
		this->mainResource = NULL;
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
	GPU_SetWrapMode( this->image, GPU_WRAP_NONE, GPU_WRAP_NONE );
	
	// printf( "Loaded %s - %p\n", path.c_str(), this->image );
	
	// destroy surface
	SDL_FreeSurface( surface );
}


//
string ImageResource::ResolveKey( const char* ckey, string& fullpath, string& extension ) {
	
	// see if there's frame in ckey
	string okey = ckey;
	string::size_type colPos = okey.find_last_of( ':' );
	string frame;
	if ( colPos != string::npos ) {
		frame = okey.substr( colPos );
		okey = okey.substr( 0, colPos );
	}
	
	fullpath = ResolvePath( okey.c_str(), "json,png,jpg,jpeg", extension, app.texturesDirectory.c_str() );
	// strip .json extension
	if ( extension.compare( "json" ) == 0 ) {
		size_t cdl = app.currentDirectory.length();
		return fullpath.substr( cdl, fullpath.length() - ( cdl + extension.length() + 1 ) ) + frame;
	} else {
		// return resolved key
		return fullpath.substr( app.currentDirectory.length() ) + frame;
	}
	
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

/// load from png from memory
bool ImageResource::LoadFromMemory(void *p, int s ) {
    SDL_RWops* rwops = SDL_RWFromMem( p, s );
    SDL_Surface* surface = IMG_Load_RW( rwops, 1 );
    if ( !surface ) return false;
    this->image = GPU_CopyImageFromSurface( surface );
    SDL_FreeSurface( surface );
    if ( this->image ) {
    
        // init bounds
        this->frame.locationOnTexture = { 0, 0, (float) this->image->base_w, (float) this->image->base_h };
        this->frame.actualWidth = this->frame.locationOnTexture.w;
        this->frame.actualHeight = this->frame.locationOnTexture.h;
        
        // convert to image
        this->image->anchor_x = this->image->anchor_y = 0; // reset
        GPU_SetImageFilter( this->image, GPU_FILTER_NEAREST );
        GPU_SetSnapMode( this->image, GPU_SNAP_NONE );
        GPU_SetWrapMode( this->image, GPU_WRAP_NONE, GPU_WRAP_NONE );

        // clear error
        this->error = ERROR_NONE;
        return true;
    } else return false;
    
}

// destroy
ImageResource::~ImageResource() {

	//
	// printf( "Unloading image %s\n", this->key.c_str() );

	// unload
	if ( this->image ) {
		GPU_FreeTarget( this->image->target );
		GPU_FreeImage( this->image );
	}
	
}
