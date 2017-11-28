#include "Application.hpp"
#include "RenderShapeBehavior.hpp"
#include "RenderSpriteBehavior.hpp"
#include "RenderTextBehavior.hpp"
#include "UIBehavior.hpp"
#include "SampleBehavior.hpp"
#include "ScriptBehavior.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// application initialization
Application::Application() {
	
    // read current working directory
    char *cwd = (char*) malloc( 1024 );
    getcwd( cwd, 1024 );
	this->currentDirectory = string( cwd );
    free( cwd );
	
	// init SDL_gpu
	this->InitRender();
	
	// init joysticks
	SDL_Init( SDL_INIT_JOYSTICK );
	SDL_JoystickEventState( SDL_ENABLE );
	
	// init image loading library
	if( !( IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG ) & ( IMG_INIT_JPG | IMG_INIT_PNG ) ) ){
		printf( "Window could not be created! SDL_Error: %s\n", IMG_GetError() );
		exit( 1 );
	}
	
	// init audio subsystem
	if ( SDL_Init( SDL_INIT_AUDIO ) < 0) {
		printf( "Couldn't init audio: %s\n", SDL_GetError() );
		exit( 1 );
	}
	
	// init audio mixer
	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) {
		printf( "Couldn't init audio mixer.\n" );
		exit( 1 );
	}
	Mix_AllocateChannels( 32 );
	
	// init font loader
	if( TTF_Init() == -1 ) {
		printf("Couldn't init TTF library: %s\n", TTF_GetError());
		exit( 1 );
	}
	
	// register classes
	this->InitClass();
	
}

Application::Application( ScriptArguments* ) {

	script.ReportError( "Application can't be created using 'new'. Only one instance is available as global 'app' property." );
	
}

Application::~Application() {
	
	printf( "(frames:%d, seconds:%f) average FPS: %f\n", this->frames, this->unscaledTime, ((float) this->frames / (float) this->unscaledTime) );
	
	// close mixer
	Mix_CloseAudio();
	
	// destroy window
	GPU_Quit();
	
	// quit SDL subsystems
	SDL_Quit();
	
	// shut down script engine
	script.Shutdown();
	
}

/* MARK:	-				Rendering init / update
 -------------------------------------------------------------------- */

void Application::InitRender() {
	
	// if window exists - this is a re-initialization
	if ( this->screen ) {
		
		GPU_SetWindowResolution( this->windowWidth, this->windowHeight );
		
	// init SDL_gpu library
	} else {
		
		// get current mode
		if( SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf( "Couldn't init SDL video: %s\n", SDL_GetError() );
			exit( 1 );
		}
		SDL_DisplayMode current;
		SDL_GetCurrentDisplayMode( 0, &current );
		
		// init GPU
		this->screen = GPU_Init( current.w, current.h, GPU_DEFAULT_INIT_FLAGS );
		if ( this->screen == NULL ) {
			GPU_ErrorObject err = GPU_PopErrorCode();
			printf( "Failed to initialize SDL_gpu library: %s.\n", err.details );
			exit( 1 );
		}
	}
	this->windowWidth = this->screen->base_w;
	this->windowHeight = this->screen->base_h;
	GPU_UnsetVirtualResolution( this->screen );
	
#ifdef __MACOSX__
	// make windowed on desktop
	GPU_SetFullscreen( false, false );
#endif
	
}

void Application::UpdateBackscreen() {
	
	// if already have a back screen
	if ( this->backScreen ) {
		// clean up
		GPU_FreeTarget( this->backScreen->target );
		GPU_FreeImage( this->backScreen );
	}
	
	// create backscreen
	this->backScreen = GPU_CreateImage( this->windowWidth / this->windowScalingFactor, this->windowHeight / this->windowScalingFactor, GPU_FORMAT_RGBA );
	this->backScreenSrcRect = { 0, 0, (float) this->backScreen->base_w, (float) this->backScreen->base_h };
	GPU_UnsetImageVirtualResolution( this->backScreen );
	GPU_SetImageFilter( this->backScreen, GPU_FILTER_NEAREST );
	GPU_SetSnapMode( this->backScreen, GPU_SNAP_NONE );
	GPU_LoadTarget( this->backScreen );
	
	// set up sizes to center small screen inside large
	float hscale = (float) this->screen->base_w / (float) this->windowWidth;
	float vscale = (float) this->screen->base_h / (float) this->windowHeight;
	float minScale = min( hscale, vscale );
	this->screenRect = { 0, 0, (float) this->screen->base_w, (float) this->screen->base_h };
	this->backScreenDstRect = {
		0.5f * ( (float) this->screen->base_w - minScale * this->windowWidth ),
		0.5f * ( (float) this->screen->base_h - minScale * this->windowHeight ),
		minScale * this->windowWidth,
		minScale * this->windowHeight };
	
	// scale for mouse position calculation
	this->backscreenScale = min( (float) this->backScreen->base_w / (float) this->backScreenDstRect.w,
								(float) this->backScreen->base_h / (float) this->backScreenDstRect.h );
	
}


/* MARK:	-				Script
 -------------------------------------------------------------------- */


// Application -> script class "Application"
SCRIPT_CLASS_NAME( Application, "App" );

/// initialize class scripting
void Application::InitClass() {
	
	// register base class
	ScriptableClass::InitClass();
	
	// register class
	script.RegisterClass<Application>( NULL, true );
	
	// constants
	
	script.SetGlobalConstant( "BLEND_NORMAL", GPU_BlendPresetEnum::GPU_BLEND_NORMAL );
	script.SetGlobalConstant( "BLEND_PREMULTIPLIED_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_PREMULTIPLIED_ALPHA );
	script.SetGlobalConstant( "BLEND_MULTIPLY", GPU_BlendPresetEnum::GPU_BLEND_MULTIPLY );
	script.SetGlobalConstant( "BLEND_ADD", GPU_BlendPresetEnum::GPU_BLEND_ADD );
	script.SetGlobalConstant( "BLEND_SUBTRACT", GPU_BlendPresetEnum::GPU_BLEND_SUBTRACT );
	script.SetGlobalConstant( "BLEND_MOD_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_MOD_ALPHA );
	script.SetGlobalConstant( "BLEND_SET_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_SET_ALPHA );
	script.SetGlobalConstant( "BLEND_SET", GPU_BlendPresetEnum::GPU_BLEND_SET );
	script.SetGlobalConstant( "BLEND_NORMAL_KEEP_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_NORMAL_KEEP_ALPHA );
	script.SetGlobalConstant( "BLEND_NORMAL_ADD_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_NORMAL_ADD_ALPHA );
	script.SetGlobalConstant( "BLEND_NORMAL_FACTOR_ALPHA", GPU_BlendPresetEnum::GPU_BLEND_NORMAL_FACTOR_ALPHA );
	
	// properties
	script.AddProperty<Application>
	("scene",
	 static_cast<ScriptObjectCallback>([](void* self, void* val ){
		return app.sceneStack.size() ? app.sceneStack.back()->scriptObject : NULL; }) ,
	 static_cast<ScriptObjectCallback>([](void* self, void* val ){
		// get current and new scene objects
		Scene* current = app.sceneStack.size() ? app.sceneStack.back() : NULL;
		Scene* newScene = script.GetInstance<Scene>( (JSObject*) val );
		vector<Scene*>::iterator it, end = app.sceneStack.end();
		
		// error if object is not scene
		if ( !newScene && val ) {
			script.ReportError( ".scene assignment: Object is not Scene instance" );
			return val;
		}
		
		// different scene
		if ( current != newScene ) {
			// new scene is provided
			if ( newScene ) {
				// check if it's already in the stack
				it = find( app.sceneStack.begin(), end, newScene );
				if ( it != end ) {
					// move it to end
					app.sceneStack.erase( it );
					app.sceneStack.push_back( newScene );
				// not in stack
				} else {
					// replace current (last)
					if ( current ) {
						// unprotect and pop
						script.ProtectObject( &current->scriptObject, false );
						app.sceneStack.pop_back();
					}
					// protect and add at end
					app.sceneStack.push_back( newScene );
					script.ProtectObject( &newScene->scriptObject, true );
				}
				
			// setting scene to null, removes all scenes from stack
			} else {
				// unprotect all
				it = app.sceneStack.begin();
				while ( it != end ) {
					script.ProtectObject( &(*it)->scriptObject, false );
					it++;
				}
				// clear
				app.sceneStack.clear();
			}
		}
		return val;
	 })
    );
	
	script.AddProperty<Application>
	("time", static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.time; }) );
	
	script.AddProperty<Application>
	("unscaledTime", static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.unscaledTime; }) );
	
	script.AddProperty<Application>
	("deltaTime", static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.deltaTime; }) );

	script.AddProperty<Application>
	("unscaledDeltaTime", static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.unscaledDeltaTime; }) );

	script.AddProperty<Application>
	("frames", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.frames; }) );

	script.AddProperty<Application>
	("fps", static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.fps; }) );

	script.AddProperty<Application>
	("windowScaling",
	 static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.windowScalingFactor; }),
	 static_cast<ScriptFloatCallback>([](void* self, float v ) {
		v = max( 0.1f, min( 8.0f, v ) );
		app.windowScalingFactor = v;
		app.UpdateBackscreen();
		return v; })
	 );
	
	script.AddProperty<Application>
	("windowWidth", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowWidth; }) );
	
	script.AddProperty<Application>
	("windowHeight", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowHeight; }) );
	
	script.AddProperty<Application>
	("fullScreen",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return GPU_GetFullscreen(); }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		GPU_SetFullscreen( v, false );
		return v;
	}));
	
	// functions
	
	script.DefineFunction<Application>
	( "pushScene",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		void* obj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &obj ) ) {
			script.ReportError( "usage: app.pushScene( Scene scene )" );
			return false;
		}
		
		// get scene
		Scene* newScene = script.GetInstance<Scene>( obj );
		if ( !obj || !newScene ) {
			script.ReportError( "app.pushScene: expecting Scene parameter" );
			return false;
		}
		
		// check if it's already in the stack
		vector<Scene*>::iterator it, end = app.sceneStack.end();
		it = find( app.sceneStack.begin(), app.sceneStack.end(), newScene );
		if ( it != end ) {
			// move it to end
			app.sceneStack.erase( it );
			app.sceneStack.push_back( newScene );
		// not in stack
		} else {
			// protect and add at end
			app.sceneStack.push_back( newScene );
			script.ProtectObject( &newScene->scriptObject, true );
		}
		return true;
	}) );
	
	script.DefineFunction<Application>
	( "popScene",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		// optional one argument
		void* obj = NULL;
		sa.ReadArguments( 1, TypeObject, &obj );
		
		// has to be scene
		Scene* newScene = script.GetInstance<Scene>( obj );
		if ( obj || !newScene ) {
			script.ReportError( "usage: app.popScene( [ Scene popToScene ] )" );
			return false;
		}
		
		// pop up to this scene
		if ( newScene ) {
			// unwind up to newScene
			for ( int i = (int) app.sceneStack.size() - 1; i >= 0; i-- ) {
				Scene* s = app.sceneStack[ i ];
				if ( s == newScene ) {
					app.sceneStack.resize( i + 1 );
					break;
				}
				script.ProtectObject( &s->scriptObject, false );
			}
		// pop one
		} else if ( app.sceneStack.size() ){
			newScene = app.sceneStack.back();
			script.ProtectObject( &newScene->scriptObject, false );
			app.sceneStack.pop_back();
		}
		
		return true;
	}) );
	
	script.DefineFunction<Application>
	( "setWindowSize",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		int w = 0, h = 0;
		float sc = app.windowScalingFactor;
		if ( !sa.ReadArguments( 2, TypeInt, &w, TypeInt, &h, TypeFloat, &sc ) ) {
			script.ReportError( "usage: app.setWindowSize( Int width, Int height, [ Float windowScalingFactor ] )" );
			return false;
		}
		app.windowWidth = max( 320, min( 4096, w ) );
		app.windowHeight = max( 320, min( 4096, h ) );
		app.windowScalingFactor = max( 0.1f, min( 8.0f, sc ) );
		app.InitRender();
		app.UpdateBackscreen();
		return true;
	}) );
	
	script.DefineFunction<Application>
	( "quit",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		app.run = false;
		return true;
	}) );
	
	// add to global namespace
	script.NewScriptObject<Application>( this );
	script.AddGlobalNamedObject( "app", this->scriptObject );
	
	// extensions
	
	script.DefineClassFunction
	( "JSON", "load", true,
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename;
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( "usage: JSON.load( String filename )" );
			return false;
		}
		// read file
		string path;
		const char* buf = ReadFile( filename.c_str(), "json", NULL, NULL, &path, NULL );
		if ( !buf ) {
			script.ReportError( "JSON.load: file '%s' not found", filename.c_str() );
			return false;
		}
		
		// call own parse function
		ScriptArguments parseArgs;
		parseArgs.AddStringArgument( buf );
		free( (void*) buf );
		sa.ReturnValue( script.CallClassFunction( "JSON", "parse", parseArgs ) );
		return true;
	}));
	
	// patched stringify function
	ArgValue jsonStringify = script.GetClassProperty( "JSON", "stringify" );
	script.DefineClassFunction
	( "JSON", "stringify", true,
	 static_cast<ScriptFunctionCallback>([jsonStringify](void*, ScriptArguments& sa ){
		
		// call modify stringify's params
		ScriptArguments stringifyArgs;
		
		// read first param
		if ( sa.args.size() >= 1 ) {
			// convert it to init object format, suitable for deserialization later
			ArgValue val = script.MakeInitObject( sa.args[ 0 ] );
			stringifyArgs.AddArgument( val.toValue() );
		}
		
		// add other params, if given
		if ( sa.args.size() >= 2 ) stringifyArgs.AddArgument( sa.args[ 1 ] );
		if ( sa.args.size() >= 3 ) stringifyArgs.AddArgument( sa.args[ 2 ] );
		
		// call original stringify function and return result
		sa.ReturnValue( script.CallClassFunction( "JSON", jsonStringify.value.objectValue, stringifyArgs ) );
		return true;
	}));
	
	script.DefineClassFunction
	( "JSON", "save", true,
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		void *obj = NULL;
		string filename;
		if ( !sa.ReadArguments( 2, TypeObject, &obj, TypeString, &filename ) ) {
			script.ReportError( "usage: JSON.save( Object obj, String filename )" );
			return false;
		}
		
		// call own stringify function
		ScriptArguments parseArgs;
		parseArgs.AddObjectArgument( obj );
		parseArgs.AddObjectArgument( NULL );
		parseArgs.AddStringArgument( "\t" );
		ArgValue ret = script.CallClassFunction( "JSON", "stringify", parseArgs );
		if ( ret.type != TypeString ) {
			sa.ReturnBool( false );
			return true;
		}
		
		// save to file
		sa.ReturnBool( SaveFile( ret.value.stringValue->c_str(), ret.value.stringValue->length(), filename.c_str(), "json", NULL ) );
		
		// all good
		return true;
	}));
	
	// eval( string, thisObj )
	script.DefineGlobalFunction
	( "eval",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* thisObj = NULL;
		string code;
		if ( !sa.ReadArguments( 1, TypeString, &code, TypeObject, &thisObj ) ){
			script.ReportError( "usage: eval( String script, [ Object thisObject ] )" );
			return false;
		}
		// eval
		sa.ReturnValue( script.Evaluate( code.c_str(), NULL, thisObj ) );
		return true;
	}) );
	
	// eval( string, thisObj )
	script.DefineGlobalFunction
	( "include",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* thisObj = NULL;
		string path;
		if ( !sa.ReadArguments( 1, TypeString, &path, TypeObject, &thisObj ) ){
			script.ReportError( "usage: include( String scriptResource, [ Object thisObject ] )" );
			return false;
		}
		
		ArgValue ret;
		script.Execute( app.scriptManager.Get( path.c_str() ), thisObj ? thisObj : sa.GetThis(), &ret );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// global toInit serializer
	script.DefineGlobalFunction
	( "toInit",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj ) ){
			script.ReportError( "usage: toInit( Object object )" );
			return false;
		}
		ArgValue ret( initObj );
		ret = script.MakeInitObject( ret );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// global init deserializer
	script.DefineGlobalFunction
	( "init",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj ) ){
			script.ReportError( "usage: init( Object initObject )" );
			return false;
		}
		ArgValue ret( script.InitObject( initObj ) );
		sa.ReturnValue( ret );
		return true;
	}) );

	// global clean up call
	script.DefineGlobalFunction
	( "gc",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		app.GarbageCollect();
		return true;
	}) );

	// call registration functions for all classes
	input.InitClass(); // single instance
	Controller::InitClass();
	Color::InitClass();
	Sound::InitClass();
	GameObject::InitClass();
	Scene::InitClass();
	Image::InitClass();
	Behavior::InitClass();
	RenderBehavior::InitShaders();
	RenderShapeBehavior::InitClass();
	RenderSpriteBehavior::InitClass();
	RenderTextBehavior::InitClass();
	RigidBodyBehavior::InitClass();
	UIBehavior::InitClass();
	SampleBehavior::InitClass();
	ScriptBehavior::InitClass();
	
}

void Application::GarbageCollect() {
	// call garbage collection in Spidermonkey
	script.GC();
	
	// call clean up in each resource manager
	textureManager.UnloadUnusedResources();
	fontManager.UnloadUnusedResources();
	scriptManager.UnloadUnusedResources();
	soundManager.UnloadUnusedResources();
	
	// done
	printf( "GC performed\n" );
}


/* MARK:	-				Game loop
 -------------------------------------------------------------------- */


/// main application loop
void Application::GameLoop() {
	
	// load and run "main.js" script
	script.Execute( scriptManager.Get( "/main.js" ) );
	
	// add default "keyboard" controller to input
	input.AddKeyboardController();
	
	// init backscreen
	this->UpdateBackscreen();
	
	// main loop
	run = true;
	Uint32 _time, _timeFps = 0;
	Event event;
	SDL_Event e;
	Scene* scene;

	while( run ) {
		
		// get current scene
		scene = sceneStack.size() ? sceneStack.back() : NULL;
		
		// read time / delta time
		_time = SDL_GetTicks();
		this->unscaledDeltaTime = (float) _time * 0.001f - this->time;
		this->deltaTime = this->unscaledDeltaTime * this->timeScale;
		this->time += this->deltaTime;
		this->unscaledTime += this->unscaledDeltaTime;
		
		// compute fps
		if ( ++this->frames % 100 == 0 ) {
			this->fps = 100.0f / (( _time - _timeFps ) * 0.001f );
			_timeFps = _time;
		}
		
		// if have active scene
		if ( scene ) {
		
			// simulate physics
			scene->SimulatePhysics();

			// update
			event.SetName( EVENT_UPDATE );
			scene->DispatchEvent( event, true );
		}
		
		SDL_JoystickUpdate();
		
		// handle system events
		while( SDL_PollEvent( &e ) != 0 ){
			// let input handle events
			input.HandleEvent( e );
			
			// Exit on escape
			if ( e.type == SDL_QUIT || ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE ) ) { run = false; break; }

		}
		
		// continue
		if ( scene ) {
		
			// late update
			event.SetName( EVENT_LATE_UPDATE );
			scene->DispatchEvent( event, true );
			
			// render to backscreen
			event.SetName( EVENT_RENDER );
			event.behaviorParam = this->backScreen->target;
			scene->Render( event );
			event.behaviorParam = NULL;
			
		}
		
		// copy to main screen and flip
		GPU_ActivateShaderProgram(0, NULL);
		GPU_Clear( this->screen );
		GPU_BlitRect( this->backScreen, &this->backScreenSrcRect, this->screen, &this->backScreenDstRect );
		GPU_Flip( this->screen );
		
	}
	
	// exit requested
}


/* MARK:	-				Global funcs ( from common.h )
 -------------------------------------------------------------------- */

/// read file from path, returns buffer on success, or NULL. Call free(buf) after using
/// tries to read from working directory / requiredSubpath if given / filepath, then directory / requiredSubpath if given / optionalSubpath / filepath before giving up
/// if filepath starts with /, ignores subPaths
const char* ReadFile( const char* filepath, const char* ext, const char* requiredSubPath, const char* optionalSubPath, string* finalPath, size_t *fileSize ) {
	
	// split path into chunks
	string key( filepath );
	bool startsWithSlash = ( filepath[ 0 ] == '/' );
	vector<string> parts = Resource::splitString( key, string( "/" ) );
	
	// get file extension
	string::size_type extPos = parts[ parts.size() - 1 ].find_last_of( '.' );
	
	// filename without extension?
	if ( extPos == string::npos ) key += string( "." ) + ext;
	
	// check if exists
	string path = app.currentDirectory + ( startsWithSlash ? "" : ( requiredSubPath ? requiredSubPath : "/" ) ) + key;
	if ( access( path.c_str(), R_OK ) == -1 ) {
		if ( optionalSubPath ) {
			path = app.currentDirectory + string(optionalSubPath) + key;
			if ( access( path.c_str(), R_OK ) != -1 ) {
				return NULL;
			}
		} else return NULL;
	}
	
	// load file
	FILE *f = fopen( (char*) path.c_str(), "r" );
	size_t fsize = 0;
	char *buf = NULL;
	if ( f != NULL ) {
		// get file size
		fseek( f, 0, SEEK_END );
		fsize = (size_t) ftell( f );
		fseek( f, 0, SEEK_SET );
		// read file in
		buf = (char*) malloc( sizeof( char ) * fsize + 1 );
		buf[ fsize ] = 0;
		fread(buf, sizeof(char), fsize, f );
		fclose( f );
	} else return NULL;
	
	// return
	if ( fileSize != NULL ) *fileSize = fsize;
	if ( finalPath != NULL ) *finalPath = path;
	return buf;
}

/// saves file to "optional/path/to/filename.optionalExt". If subPath is provided, start path is working directory / subPath / filepath, otherwise, start point is working directory
/// if filepath starts with /, ignores subPath
bool SaveFile( const char* data, size_t numBytes, const char* filepath, const char* ext, const char* subPath ) {
	
	// split path into chunks
	string key( filepath );
	bool startsWithSlash = ( filepath[ 0 ] == '/' );
	vector<string> parts = Resource::splitString( key, string( "/" ) );
	
	// get file extension
	string::size_type extPos = parts[ parts.size() - 1 ].find_last_of( '.' );
	
	// filename without extension?
	if ( extPos == string::npos ) key += string( "." ) + ext;
	
	// path
	string path = app.currentDirectory + ( startsWithSlash ? "" : ( subPath ? subPath : "/" ) ) + key;
	
	// create subfolders leading up to file
	parts = Resource::splitString( path, string( "/" ) );
	string subDir;
	for ( int i = 0; i < parts.size() - 1; i++ ) {
		subDir = "/" + parts[ i ];
		if ( access( path.c_str(), R_OK ) == -1 ) {
			// try to make directory
			if ( mkdir( subDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) == -1 ){
				printf( "Unable to create directory %s for saving.\n", subDir.c_str() );
				return false;
			}
		}
	}
	
	// create and save file
	FILE *f = fopen( (char*) path.c_str(), "w+" );
	if ( f != NULL ) {
		size_t bytesWritten = fwrite( (void*) data, sizeof(char), numBytes, f );
		fclose( f );
		return (bytesWritten > 0);
	}
	
	return false;
}

/// tries to locate a file with any given extensions. On success sets outExtension = ".ext", returns true
/// example: TryFileExtensions( "/path/to/file", "png,jpg", extension )
bool TryFileExtensions( const char* filePath, const char* commaSeparatedExtensions, string &outExtension ){
	vector<string> extArray = Resource::splitString( commaSeparatedExtensions, "," );
	string path;
	for ( size_t i = 0, ne = extArray.size(); i < ne; i++ ){
		path = filePath;
		path.append( "." );
		path.append( extArray[ i ] );
		if ( access( path.c_str(), R_OK ) != -1 ) {
			outExtension = "." + extArray[ i ];
			return true;
		}
	}
	return false;
}

/// quick and simple hashing function
size_t HashString( const char* p ) {
	size_t result = 0;
	const size_t prime = 31;
	for (size_t i = 0; p[ i ] != 0; ++i) {
		result = p[i] + (result * prime);
	}
	return result;
}

/// compose matrix helper
float* MatrixCompose( float* mat, float x, float y, float angleDegrees, float scaleX, float scaleY ) {
	GPU_MatrixIdentity( mat );
	GPU_MatrixTranslate( mat, x, y, 0 );
	GPU_MatrixRotate( mat, angleDegrees, 0, 0, 1 );
	GPU_MatrixScale( mat, scaleX, scaleY, 1 );
	return mat;
}

string HexStr( Uint32 w, size_t hex_len ) {
	static const char* digits = "0123456789ABCDEF";
	string rc( hex_len, '0' );
	for ( size_t i = 0, j = ( hex_len-1 ) * 4 ; i < hex_len; ++i, j-=4 ) {
		rc[ i ] = digits[ ( w >> j ) & 0x0f ];
	}
	return rc;
}

