#include "Application.hpp"
#include "RenderShapeBehavior.hpp"
#include "RenderSpriteBehavior.hpp"
#include "RenderTextBehavior.hpp"
#include "RenderParticlesBehavior.hpp"
#include "UIBehavior.hpp"
#include "ParticleGroupBehavior.hpp"
#include "SampleBehavior.hpp"
#include "TypedVector.hpp"

// from ScriptableClass.hpp
int ScriptableClass::asyncIndex = 0;
ScriptableClass::AsyncMap* ScriptableClass::scheduledAsyncs = NULL;
ScriptableClass::DebouncerMap* ScriptableClass::scheduledDebouncers = NULL;

// from Tween.hpp
unordered_set<Tween*> *Tween::activeTweens = NULL;

// from common.h
size_t debugObjectsCreated = 0;
size_t debugObjectsDestroyed = 0;
unordered_map<string,size_t> debugEventsDispatched;

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// application initialization
Application::Application() {
	
    // read current working directory
    char *cwd = (char*) malloc( 1024 );
    getcwd( cwd, 1024 );
	this->currentDirectory = string( cwd );
	// printf( "Current working directory is %s\n", cwd );
    free( cwd );
	
    // get current mode
    if( SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf( "Couldn't init SDL video: %s\n", SDL_GetError() );
        exit( 1 );
    }
    
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode( 0, &current );
    
    // init GPU
    this->screen = GPU_Init( this->screenWidth, this->screenHeight, GPU_DEFAULT_INIT_FLAGS );
    if ( this->screen == NULL ) {
        GPU_ErrorObject err = GPU_PopErrorCode();
        printf( "Failed to initialize SDL_gpu library: %s.\n", err.details );
        exit( 1 );
    }
    
    this->windowWidth = this->screen->base_w;
    this->windowHeight = this->screen->base_h;
    GPU_UnsetVirtualResolution( this->screen );
    GPU_UnsetClip( this->screen );
    GPU_UnsetViewport( this->screen );
    GPU_SetShapeBlendMode( GPU_BlendPresetEnum::GPU_BLEND_NORMAL );
    if ( !GPU_AddDepthBuffer( this->screen ) ){
        printf( "Warning: depth buffer is not supported.\n" );
    }
    GPU_SetDepthTest( this->screen, true );
    GPU_SetDepthWrite( this->screen, true );
    GPU_SetDepthFunction( this->screen, GPU_ComparisonEnum::GPU_LEQUAL );
    this->screen->camera.z_near = -1024;
    this->screen->camera.z_far = 1024;
    this->screen->camera.use_centered_origin = false;
    GPU_EnableCamera( this->screen, false );
    
    SDL_SetWindowResizable( SDL_GL_GetCurrentWindow(), (SDL_bool) windowResizable );
    
#if !defined(RASPBERRY_PI) && !defined(ORANGE_PI)
     // make windowed on desktop
     GPU_SetFullscreen( false, false );
#endif
    
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
    
	// init containers
	ScriptableClass::scheduledAsyncs = new AsyncMap();
	ScriptableClass::scheduledDebouncers = new DebouncerMap();
	Tween::activeTweens = new unordered_set<Tween*>();
	
    // register classes
	this->InitClass();
    
	// overlay
	this->overlay = new GameObject( NULL );
	this->overlay->SetZ( 1000 );
	this->overlay->orphan = false;
    
	// begin terminal capture
	struct termios newt;
	tcgetattr(STDIN_FILENO, &_savedTerminal);
	memcpy( &newt, &_savedTerminal, sizeof(struct termios) );
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
    
}

Application::Application( ScriptArguments* ) {

	script.ReportError( "Application can't be created using 'new'. Only one instance is available as global 'app' property." );
	
}

Application::~Application() {
	
	printf( "(frames:%d, seconds:%f) average FPS: %f\n", this->frames, this->unscaledTime, ((float) this->frames / (float) this->unscaledTime) );
	
	// destroy window
	GPU_Quit();
	
	// quit SDL subsystems
	SDL_Quit();
	
	// shut down script engine
	script.Shutdown();
	
	// delete async
	delete ScriptableClass::scheduledAsyncs;
	delete ScriptableClass::scheduledDebouncers;	
	delete Tween::activeTweens;
	
	// stop terminal capture
	tcsetattr( STDIN_FILENO, TCSANOW, &_savedTerminal );
}


/* MARK:	-				Rendering init / update
 -------------------------------------------------------------------- */


/// window resizing callback
void Application::WindowResized( Sint32 newWidth, Sint32 newHeight ) {
	if ( ( newWidth && newHeight ) && ( fixedWindowResolution || newWidth != this->windowWidth || newHeight != this->windowHeight ) ) {

		// resize window
		GPU_SetWindowResolution( newWidth, newHeight );
		GPU_UnsetVirtualResolution( this->screen );
		
		// if not fixed size, update window size,
		if ( !fixedWindowResolution ) {
			this->windowWidth = this->screen->base_w;
			this->windowHeight = this->screen->base_h;
		}
		
		// update backscreen, or its scale
		UpdateBackscreen();
		
		if ( app.sceneStack.size() ) app.sceneStack.back()->_camTransformDirty = true;

		// notify script
		this->SendResizedEvents();

	}
}

void Application::SendResizedEvents() {
	
	// send event
	Event event( EVENT_RESIZED );
	event.scriptParams.AddIntArgument( (float) this->windowWidth / app.windowScalingFactor );
	event.scriptParams.AddIntArgument( (float) this->windowHeight / app.windowScalingFactor );
	this->CallEvent( event );
	
	// and send layout event to scene
	if ( sceneStack.size() ) {
		event.name = EVENT_LAYOUT;
		event.stopped = false;
		event.behaviorsOnly = true;
		event.scriptParams.ResizeArguments( 0 );
		sceneStack.back()->DispatchEvent( event, true );
	}
}

void Application::UpdateBackscreen() {
	
	// actual size
	int ww = this->windowWidth / this->windowScalingFactor,
		hh = this->windowHeight / this->windowScalingFactor;

    // if resize / recreate is needed
	if ( !this->backScreen || this->backScreen->base_w != ww || this->backScreen->base_h != hh ) {
		
		// if already have a back screen
		if ( this->backScreen ) {
			// clean up
			GPU_FreeTarget( this->backScreen->target );
			GPU_FreeImage( this->backScreen );
		}
		
		// clear blend target
		if ( this->blendTarget ) {
			GPU_FreeTarget( this->blendTarget );
			GPU_FreeImage( this->blendTarget->image );
			this->blendTarget = NULL;
		}
		
		// create backscreen
		this->backScreen = GPU_CreateImage( ww, hh, GPU_FORMAT_RGB );
		this->backScreenSrcRect = { 0, 0, (float) this->backScreen->base_w, (float) this->backScreen->base_h };
		GPU_UnsetImageVirtualResolution( this->backScreen );
		GPU_SetImageFilter( this->backScreen, GPU_FILTER_NEAREST );
		GPU_SetSnapMode( this->backScreen, GPU_SNAP_NONE );
		GPU_SetAnchor( this->backScreen, 0, 0 );
		GPU_LoadTarget( this->backScreen );
		GPU_SetShapeBlending( true );
		GPU_SetBlendMode( this->backScreen, GPU_BlendPresetEnum::GPU_BLEND_NORMAL );
		GPU_SetShapeBlendMode( GPU_BlendPresetEnum::GPU_BLEND_NORMAL );
		if ( !GPU_AddDepthBuffer( this->backScreen->target ) ){
			printf( "Warning: depth buffer is not supported.\n" );
		}
		GPU_SetDepthTest( this->backScreen->target, true );
		GPU_SetDepthWrite( this->backScreen->target, true );
		GPU_SetDepthFunction( this->backScreen->target, GPU_ComparisonEnum::GPU_LEQUAL );
        GPU_EnableCamera( this->backScreen->target, false );
	}
	
	// update sizes to center small screen inside large
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


/// initialize class scripting
void Application::InitClass() {
	
	// register base class
	ScriptableClass::InitClass();
	
	// register class
	script.RegisterClass<Application>( "ScriptableObject", true );
	
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
		
		Event event;

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
						// pop
						app.sceneStack.pop_back();
					}
					//  add at end
					app.sceneStack.push_back( newScene );
				}
				
			// setting scene to null, removes all scenes from stack
			} else {
				// clear
				app.sceneStack.clear();
			}

			// call removed event on current
			if ( current ) {
				event.name = EVENT_REMOVED;
				event.behaviorParam = current;
				event.stopped = false;
				current->CallEvent( event );
			}
			
			// generate events
			event.name = EVENT_SCENECHANGED;
			event.stopped = false;
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( current ? current->scriptObject : NULL );
			app.CallEvent( event );
			
			// layout as well
			if ( newScene ) {
				event.stopped = false;
				event.scriptParams.ResizeArguments( 0 );
				event.behaviorParam = newScene;
				event.name = EVENT_ADDED;
				newScene->CallEvent( event );
				
				event.name = EVENT_LAYOUT;
				event.stopped = false;
				newScene->DispatchEvent( event );
			}
		}
		return val;
	 })
    );
	
	script.AddProperty<Application>
	("sceneStack",
	static_cast<ScriptArrayCallback>([](void* self, ArgValueVector* val ){
		ArgValueVector* v = new ArgValueVector();
		for( size_t i = 0; i < app.sceneStack.size(); i++ ){
			v->emplace_back( app.sceneStack[ i ]->scriptObject );
		}
		return v;
	}) );
	
	script.AddProperty<Application>
	("overlay",
	 static_cast<ScriptObjectCallback>([](void* self, void* val ){ return app.overlay->scriptObject; }) );
	
	script.AddProperty<Application>
	("timeScale",
	 static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.timeScale; }),
	 static_cast<ScriptFloatCallback>([](void* self, float v ) { return app.timeScale = max( 0.0f, v ); }));
	
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
	("windowResizable",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.windowResizable; }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		app.windowResizable = v;
#if !defined(RASPBERRY_PI) && !defined(ORANGE_PI)
		SDL_SetWindowResizable( SDL_GL_GetCurrentWindow(), (SDL_bool) app.windowResizable );
#endif
		return v;
	}));
	
	script.AddProperty<Application>
	("fixedWindowResolution",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.fixedWindowResolution; }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ app.fixedWindowResolution = v; return v; }));
	
	script.AddProperty<Application>
	("windowWidth", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowWidth / app.windowScalingFactor; }) );
	
	script.AddProperty<Application>
	("windowHeight", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowHeight / app.windowScalingFactor; }) );
	
	script.AddProperty<Application>
	("screenWidth", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.screenWidth; }) );

	script.AddProperty<Application>
	("screenHeight", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.screenHeight; }) );
	
	script.AddProperty<Application>
	("fullScreen",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return GPU_GetFullscreen(); }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		GPU_SetFullscreen( v, false );
		return v;
	}));
	
	script.AddProperty<Application>
	("debugDraw",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.debugDraw; }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		return (app.debugDraw = v);
	}));
	
	script.AddProperty<Application>
	("isUnserializing",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.isUnserializing; }));
	
	// platform dependent clipboard
	script.AddProperty<Application>
	("clipboard",
	 static_cast<ScriptStringCallback>([](void* self, string v){
		if ( SDL_HasClipboardText() ) return string( SDL_GetClipboardText() );
		return string("");
	}),
	 static_cast<ScriptStringCallback>([](void* self, string v){ SDL_SetClipboardText( v.c_str() ); return v; }),
	 PROP_NOSTORE | PROP_ENUMERABLE );
	
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
		
		Scene* oldScene = app.sceneStack.size() ? app.sceneStack.back() : NULL;
		
		// check if it's already in the stack
		vector<Scene*>::iterator it, end = app.sceneStack.end();
		it = find( app.sceneStack.begin(), app.sceneStack.end(), newScene );
		if ( it != end ) {
			// move it to end
			app.sceneStack.erase( it );
			app.sceneStack.push_back( newScene );
		// not in stack
		} else {
			// add at end
			app.sceneStack.push_back( newScene );
		}
		
		// generate event
		if ( newScene != oldScene ) {
			
			Event event;
			
			if ( oldScene ) {
				event.name = EVENT_REMOVED;
				event.behaviorParam = oldScene;
				oldScene->CallEvent( event );
			}
			
			event.stopped = false;
			event.name = EVENT_ADDED;
			event.behaviorParam = newScene;
			newScene->CallEvent( event );
			
			event.stopped = false;
			event.name = EVENT_SCENECHANGED;
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( oldScene ? oldScene->scriptObject : NULL );
			app.CallEvent( event );
		}
		
		sa.ReturnObject( obj );
		return true;
	}) );
	
	script.DefineFunction<Application>
	( "popScene",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		// optional one argument
		void* obj = NULL;
		Scene* newScene = NULL;
		if ( sa.ReadArguments( 1, TypeObject, &obj ) ){
			// has to be scene
			newScene = script.GetInstance<Scene>( obj );
			if ( obj || !newScene ) {
				script.ReportError( "usage: app.popScene( [ Scene popToScene ] )" );
				return false;
			}
		}
		Scene* oldScene = app.sceneStack.size() ? app.sceneStack.back() : NULL;
		
		// pop up to this scene
		if ( newScene ) {
			// unwind up to newScene
			for ( int i = (int) app.sceneStack.size() - 1; i >= 0; i-- ) {
				Scene* s = app.sceneStack[ i ];
				if ( s == newScene ) {
					app.sceneStack.resize( i + 1 );
					break;
				}
			}
		// pop one
		} else if ( app.sceneStack.size() ){
			app.sceneStack.pop_back();
			newScene = app.sceneStack.size() ? app.sceneStack.back() : NULL;
		}
		
		// generate event
		if ( newScene != oldScene ) {
			Event event;
			
			if ( oldScene ) {
				event.name = EVENT_REMOVED;
				event.behaviorParam = oldScene;
				oldScene->CallEvent( event );
			}
			
			event.stopped = false;
			event.name = EVENT_ADDED;
			event.behaviorParam = newScene;
			newScene->CallEvent( event );
			
			event.stopped = false;
			event.name = EVENT_SCENECHANGED;
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( oldScene ? oldScene->scriptObject : NULL );
			app.CallEvent( event );
		}
		sa.ReturnObject( oldScene ? oldScene->scriptObject : NULL );
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
		app.windowScalingFactor = max( 0.1f, min( 8.0f, sc ) );
		w = max( 128, min( 4096, (int) floor( w * app.windowScalingFactor ) ) );
		h = max( 128, min( 4096, (int) floor( h * app.windowScalingFactor ) ) );
		// force update sizes
		app.windowWidth = w;
		app.windowHeight = h;
		app.WindowResized( w, h );
		app.UpdateBackscreen();
		return true;
	}) );
	
	// add to global namespace
	script.NewScriptObject<Application>( this );
	script.AddGlobalNamedObject( "App", this->scriptObject );

    // extensions
	
	script.DefineClassFunction
	( "String", "positionToIndex", false,
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa ){
		int pos;
		string *str = ((ArgValue*) p)->value.stringValue;
		if ( !sa.ReadArguments( 1, TypeInt, &pos ) ) {
			script.ReportError( "usage: String.positionToIndex( Int position )" );
			return false;
		}
		
		// return character index
		sa.ReturnInt( StringPositionToIndex( str->c_str(), pos ) );
		return true;
	}));
	
	script.DefineClassFunction
	( "String", "indexToPosition", false,
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa ){
		int pos;
		string *str = ((ArgValue*) p)->value.stringValue;
		if ( !sa.ReadArguments( 1, TypeInt, &pos ) ) {
			script.ReportError( "usage: indexToPosition( Int index )" );
			return false;
		}
		
		// return character position
		sa.ReturnInt( StringIndexToPosition( str->c_str(), pos ) );
		return true;
	}));
	
	script.DefineClassFunction
	( "String", "positionLength", false,
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa ){
		string *str = ((ArgValue*) p)->value.stringValue;
		// return character length
		sa.ReturnInt( StringPositionLength( str->c_str() ));
		return true;
	}));
	
	script.DefineClassFunction
	( "Object", "getProperties", true,
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments& sa ){
		ArgValueVector ret;
		void* obj = NULL;
		const char* error = "usage: getProperties( Object object, [ Boolean useSerializeMask, [ Boolean includeReadOnly, [ Boolean includeFunctions ] ] ] )";
		bool useSerializeMask = false,
			 includeReadOnly = false,
		     includeFunctions = false;
		// read params
		if ( sa.args.size() >= 1 ) {
			if ( sa.args[ 0 ].type == TypeArray ) obj = sa.args[ 0 ].arrayObject;
			else if ( sa.args[ 0 ].type == TypeObject ) obj = sa.args[ 0 ].value.objectValue;
			else if ( sa.args[ 0 ].type == TypeFunction ) obj = sa.args[ 0 ].value.objectValue;
			else obj = script.GetTypePrototypeObject( sa.args[ 0 ].type );
			sa.ReadArgumentsFrom( 1, 0, TypeBool, &useSerializeMask, TypeBool, &includeReadOnly, TypeBool, &includeFunctions );
		}
		// fail
		if ( obj == NULL ) {
			script.ReportError( error );
			// sa.ReturnArray( ret );
			return false;
		}
		// read props
		script.GetProperties( obj, &ret, useSerializeMask, includeReadOnly, includeFunctions );
		sa.ReturnArray( ret );
		return true;
	}));
	
	script.DefineGlobalFunction
	( "getTextureFrames",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename;
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( "usage: getTextureFrames( String textureName )" );
			return false;
		}
		
		// load texture
		ImageResource* img = app.textureManager.Get( filename.c_str() );
		if ( img->error ) {
			sa.ReturnBool( false );
		} else {
			// get frames
			ArgValueVector r;
			ImageFramesMap::iterator it = img->frames.begin(), end = img->frames.end();
			while( it != end ) { r.emplace_back( it->first.c_str() ); it++; }
			sa.ReturnArray( r );
		}
		
		return true;
	}));
	
	script.DefineGlobalFunction
	( "fileExists",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename;
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( "usage: fileExists( String filename )" );
			return false;
		}
		// check path
		string path = ResolvePath( filename.c_str(), NULL, NULL );
		if ( access( path.c_str(), R_OK ) == 0 ) {
			// file exists - check if file or dir
			struct stat buf;
			stat( path.c_str(), &buf );
			sa.ReturnString( S_ISDIR(buf.st_mode) ? "directory" : "file" );
		} else sa.ReturnBool( false );
		return true;
	}));
	
	script.DefineGlobalFunction
	( "listDirectory",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename, startPath = "";
		ArgValueVector *extensions = NULL;
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( "usage: listDirectory( String filename, [ Array allowedFileExtensions, [ String startPath ] ] )" );
			return false;
		}
		if ( sa.args.size() >= 2 && sa.args[ 1 ].type == TypeArray ) extensions = sa.args[ 1 ].value.arrayValue;
		if ( sa.args.size() == 3 && sa.args[ 2 ].type == TypeString ) startPath = *sa.args[ 2 ].value.stringValue;
		ArgValueVector ret;

		
		// start path
		if ( filename.length() && ( filename[ 0 ] == '/' || filename[ 0 ] == '.' ) ) startPath = "";
		else if ( startPath.length() && startPath[ startPath.length() - 1 ] != '/' ) startPath.append( "/" );
		
		// list
		string curPath = startPath + filename;
		string path = ResolvePath( curPath.c_str(), NULL, NULL );
		string filter;
		struct stat buf;
		
		// if current path exists
		if ( access( path.c_str(), R_OK ) == 0 ) {
			stat( path.c_str(), &buf );
			// is not a directory ending with /
			if ( !( path[ path.length() - 1 ] == '/' && S_ISDIR( buf.st_mode ) ) ){
				// return empty set
				sa.ReturnArray( ret );
				return true;
			}
		// path doesn't exist,
		} else {
			// find last /
			size_t lastSlash = path.find_last_of( "/" );
			if ( lastSlash != string::npos && lastSlash != path.length() - 1 ) {
				// check if path before slash exists
				filter = path.substr( lastSlash + 1 );
				path = path.substr( 0, lastSlash );
				if ( access( path.c_str(), R_OK ) != 0 ) {
					// return empty set
					sa.ReturnArray( ret );
					return true;
				}
			}
		}
		
		// for each file in directory
		struct dirent *ent;
		DIR *dir = opendir( path.c_str() );
		if ( dir != NULL ) {
			while ( ( ent = readdir( dir ) ) != NULL ) {
				
				filename = ent->d_name;
				
				// skip . and ..
				if ( filename.compare( "." ) == 0 || filename.compare( ".." ) == 0 ) continue;
				
				// if not a directory, and allowed extensions are supplied
				string full = path + "/" + filename;
				stat( full.c_str(), &buf );
				bool isDir = S_ISDIR( buf.st_mode );
				if ( !isDir && extensions ) {
					// for each allowed extension
					bool allowed = false;
					for ( size_t i = 0, ne = extensions->size(); i < ne; i++ ) {
						if ( (*extensions)[ i ].type != TypeString ) continue;
						// match extension
						if ( filename.length() > (*extensions)[ i ].value.stringValue->length() ) {
							string ext = string( "." );
							ext.append( (*extensions)[ i ].value.stringValue->c_str() );
							if ( filename.substr( filename.length() - ext.length() ).compare( ext.c_str() ) == 0 ) {
								allowed = true; break;
							}
						}
					}
					// no extension matched, skip entry
					if ( !allowed ) continue;
				}
				
				// if no filter, or matches filter, add to list
				if ( !filter.length() || ( filename.length() >= filter.length() && filename.substr( 0, filter.length() ).compare( filter ) == 0 ) ) {
					if ( isDir ) filename.append( "/" );
					ret.push_back( filename.c_str() );
				}
			}
			closedir (dir);
		}
		
		// done
		sa.ReturnArray( ret );
		return true;
	}));
	
	script.DefineGlobalFunction
	( "load",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename;
		bool parse = false;
		if ( !sa.ReadArguments( 1, TypeString, &filename, TypeBool, &parse ) ) {
			script.ReportError( "usage: load( String filename, [ Boolean parseJSON=false ] )" );
			return false;
		}
		// read file
		string path;
		const char* buf = ReadFile( filename.c_str(), "json", NULL, &path, NULL );
		if ( !buf ) {
			script.ReportError( "load: file '%s' not found", path.c_str() );
			return false;
		}
		// if parse + ends with .json
		if ( parse ) {
			// call own parse function
			ScriptArguments parseArgs;
			parseArgs.AddStringArgument( buf );
			ArgValue ret = script.CallClassFunction( "JSON", "parse", parseArgs );
			sa.ReturnValue( ret );
		} else {
			// as string
			sa.ReturnString( buf );
		}
		free( (void*) buf );
		return true;
	}));
	
	// patched stringify function
	ArgValue jsonStringify = script.GetClassProperty( "JSON", "stringify" );
	script.DefineGlobalFunction
	( "stringify",
	 static_cast<ScriptFunctionCallback>([jsonStringify](void*, ScriptArguments& sa ){
		
		// call modify stringify's params
		ScriptArguments stringifyArgs;
		
		// read first param
		if ( sa.args.size() >= 1 ) {
			// convert it to init object format, suitable for deserialization later
			ArgValue val = script.MakeInitObject( sa.args[ 0 ] );
			stringifyArgs.AddArgument( val.toValue() );
		}
		
		// second param should be a boolean, indicating whether to pretty print
		if ( sa.args.size() >= 2 ) {
			if ( sa.args[ 1 ].type == TypeBool ) {
				if ( sa.args[ 1 ].value.boolValue ) {
					stringifyArgs.AddBoolArgument( false );
					stringifyArgs.AddStringArgument( "\t" );
				}
			} else {
				script.ReportError( "usage: stringify( Object object, [Boolean prettyPrint] )" );
				return false;
			}
		}
		
		// call original stringify function and return result
		sa.ReturnValue( script.CallClassFunction( "JSON", jsonStringify.value.objectValue, stringifyArgs ) );
		return true;
	}));
	
	script.DefineGlobalFunction
	( "save",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		void *obj = NULL;
		string str;
		bool overwrite = false;
		string filename;
		if ( !sa.ReadArguments( 2, TypeObject, &obj, TypeString, &filename, TypeBool, &overwrite ) ) {
			if ( !sa.ReadArguments( 2, TypeString, &str, TypeString, &filename, TypeBool, &overwrite ) ) {
				script.ReportError( "usage: save( Object obj | String str, String filename, [ Boolean overwrite=false ] )" );
				return false;
			}
		}
		
		// if object, serialize
		ArgValue ret;
		if ( obj ) {
			// call own stringify function
			ScriptArguments parseArgs;
			parseArgs.AddObjectArgument( obj );
			parseArgs.AddBoolArgument( true );
			ret = script.CallGlobalFunction( "stringify", parseArgs );
			if ( ret.type != TypeString ) {
				sa.ReturnBool( false );
				return true;
			}
		// just save string
		} else {
			ret.type = TypeString;
			ret.value.stringValue = new string( str );
		}
		
		// save to file
		bool existsOverwrite = overwrite;
		bool result = SaveFile( ret.value.stringValue->c_str(), ret.value.stringValue->length(), filename.c_str(), "json", &existsOverwrite );
		
		// return bool, or 'exists'
		if ( result || overwrite ) {
			sa.ReturnBool( result );
		} else if ( !overwrite && existsOverwrite ) {
			sa.ReturnString( "exists" );
		}
		
		// all good
		return true;
	}));
	
	//
	script.DefineGlobalFunction
	( "stopEvent",
	 static_cast<ScriptFunctionCallback>([]( void*, ScriptArguments& sa ) {
		string evtName;
		if ( !sa.ReadArguments( 0, TypeString, &evtName ) ){
			script.ReportError( "usage: stopEvent( [ String eventName ] )" );
			return false;
		}
		
		// find event
		Event* event = NULL;
		if ( evtName.length() ) {
			for ( size_t i = Event::eventStack.size(); i > 0; i-- ) {
				// find by name
				if ( evtName.compare( Event::eventStack[ i - 1 ]->name ) == 0 ){
					event = Event::eventStack[ i - 1 ];
					break;
				}
			}
			// not found?
			if ( !event ) {
				sa.ReturnBool( false );
				return true;
			}
		// no name specified - top of stack
		} else {
			event = Event::eventStack.back();
		}
		
		// stop event
		event->stopped = true;
		
		// return stopped event name
		sa.ReturnString( event->name );
		return true;
	}) );
	
	// stops all events on stack (used for input/ui)
	script.DefineGlobalFunction
	( "stopAllEvents",
	 static_cast<ScriptFunctionCallback>([]( void*, ScriptArguments& sa ) {
		ArgValueVector* eventNames = new ArgValueVector();
		for ( size_t i = 0; i < Event::eventStack.size(); i++ ) {
			Event::eventStack[ i ]->stopped = true;
			eventNames->emplace_back( Event::eventStack[ i ]->name );
		}
		sa.ReturnArray( *eventNames );
		delete eventNames;
		return true;
	}) );
	
	script.DefineGlobalFunction
	( "currentEventName",
	 static_cast<ScriptFunctionCallback>([]( void*, ScriptArguments& sa ) {
		
		if ( Event::eventStack.size() ) sa.ReturnString( Event::eventStack.back()->name );
		else sa.ReturnNull();
		
		return true;
	}) );
	
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
		ScriptResource* sr = app.scriptManager.Get( path.c_str() );
		if ( sr->error == ERROR_NONE ) {
			ArgValue ret;
			script.Execute( sr, thisObj ? thisObj : sa.GetThis(), &ret );
			sa.ReturnValue( ret );
			return true;
		} else {
			// printf( "returning false\n");
			return false;
		}
	}) );
	
	// when serializing, serializeMask stops property from being serialized, (also properties starting w __, or objects w .serializeable === false)
	script.DefineGlobalFunction
	( "serialize",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		bool force = false;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj, TypeBool, &force ) ){
			script.ReportError( "usage: serialize( Object object, [ Boolean force ] )" );
			return false;
		}
		ArgValue ret( initObj );
		ret = script.MakeInitObject( ret, force );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// global init deserializer
	script.DefineGlobalFunction
	( "unserialize",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj ) ){
			script.ReportError( "usage: unserialize( Object initObject )" );
			return false;
		}
		ArgValue ret( script.InitObject( initObj ) );
		sa.ReturnValue( ret );
		return true;
	}) );

	// when cloning, serializeMask stops property from being cloned, cloneMask makes property copied verbatim (not cloned recursively)
	script.DefineGlobalFunction
	( "clone",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		void* overrides = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj, TypeObject, &overrides ) ){
			script.ReportError( "usage: clone( Object object,[ Object initObject ] )" );
			return false;
		}
		ArgValue ret( initObj );
		ret = script.MakeInitObject( ret, true, true );
		void *def = ret.value.objectValue;
		ret.value.objectValue = script.InitObject( def, true );
		if ( overrides ) script.CopyProperties( overrides, ret.value.objectValue );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// schedule a call
	script.DefineGlobalFunction
	( "async",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: async( Function handler [, Number delay, [ Boolean useUnscaledTime ] ] )";
		void* handler = NULL;
		float delay = 0;
		bool unscaled = false;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeFunction, &handler, TypeFloat, &delay, TypeBool, &unscaled ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		sa.ReturnInt( ScriptableClass::AddAsync( (void*) script.global_object, handler, delay, unscaled ) );
		return true;
	}));
	
	// cancel a call
	script.DefineGlobalFunction
	( "cancelAsync",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelAsync( Int asyncId )";
		int index = -1;
		
		// one param? cancel single one
		if ( sa.args.size() >= 1 ) {
			if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
				script.ReportError( error );
				return false;
			}
		}
		
		// cancel scheduled call
		sa.ReturnBool( ScriptableClass::CancelAsync( (void*) script.global_object, index ) );
		return true;
	}));
	
	// schedule a call
	script.DefineGlobalFunction
	( "debounce",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: debounce( String debounceId, Function handler [, Number delay, [ Boolean useUnscaledTime ] ] )";
		void* handler = NULL;
		string name;
		float delay = 0;
		bool unscaled = false;
		
		// validate
		if ( !sa.ReadArguments( 2, TypeString, &name, TypeFunction, &handler, TypeFloat, &delay, TypeBool, &unscaled ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		ScriptableClass::AddDebouncer( (void*) script.global_object, name, handler, delay, unscaled );
		return true;
	}));
	
	// cancel a call
	script.DefineGlobalFunction
	( "cancelDebouncer",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelDebouncer( String debounceId )";
		string name;
		
		// one param? cancel single one
		if ( sa.args.size() >= 1 ) {
			if ( !sa.ReadArguments( 1, TypeString, &name ) ) {
				script.ReportError( error );
				return false;
			}
		}
		
		// cancel scheduled call
		sa.ReturnBool( ScriptableClass::CancelDebouncer( (void*) script.global_object, name ) );
		
		return true;
	}));
	
	// global clean up call
	script.DefineGlobalFunction
	( "gc",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		app.GarbageCollect();
		return true;
	}) );

	script.DefineGlobalFunction
	( "quit",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		app.run = false;
		return true;
	}) );
	
	// dump stack
	script.DefineGlobalFunction
	( "stackTrace",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		const char* error = "usage: stackTrace( Boolean asObject )";
		bool asObj = false;
		if ( !sa.ReadArguments( 0, TypeBool, &asObj ) ) {
			script.ReportError( error );
			return false;
		}
		StackDescription* desc = JS::DescribeStack( script.js, 128 );
		ArgValueVector ret;
		string sret;
		for ( unsigned i = 0; i < desc->nframes; i++ ) {
			FrameDescription* frame = &desc->frames[ i ];
			string scriptFileName = JS_GetScriptFilename( script.js, frame->script );
			scriptFileName.erase( scriptFileName.begin(), scriptFileName.begin() + app.currentDirectory.size() );
			if ( asObj ) {
				void* obj = script.NewObject();
				script.SetProperty( "function", ArgValue( (void*) frame->fun ), obj );
				script.SetProperty( "line", ArgValue( (int) frame->lineno ), obj );
				script.SetProperty( "script", scriptFileName.c_str(), obj );
				ret.emplace_back( obj );
			} else {
				ArgValue funcName = script.GetProperty( "name", frame->fun );
				sret.append( funcName.value.stringValue->length() ? funcName.value.stringValue->c_str() : "anonymous" );
				sret.append( "() @ " );
				sret.append( scriptFileName.c_str() );
				sret.append( ":" );
				static char buf[ 8 ];
				sprintf( buf, "%u", frame->lineno );
				sret.append( buf );
				if ( i < desc->nframes - 1 ) sret.append( "\n" );
			}
		}
		JS::FreeStackDescription( script.js, desc );
		// done
		if ( asObj ) sa.ReturnArray( ret );
		else sa.ReturnString( sret );
		return true;
	}));
	
	// intern all strings
	const char* interns[] = {
		EVENT_SCENECHANGED, EVENT_UPDATE, EVENT_ADDED,
		EVENT_REMOVED, EVENT_ADDEDTOSCENE, EVENT_REMOVEDFROMSCENE,
		EVENT_CHILDADDED, EVENT_CHILDREMOVED, EVENT_ACTIVECHANGED,
		EVENT_ATTACHED, EVENT_DETACHED, EVENT_RENDER, EVENT_KEYDOWN, EVENT_KEYUP,
		EVENT_KEYPRESS, EVENT_MOUSEDOWN, EVENT_MOUSEUP, EVENT_MOUSEMOVE, EVENT_MOUSEWHEEL,
		EVENT_CONTROLLERADDED, EVENT_CONTROLLERREMOVED,
		EVENT_JOYDOWN, EVENT_JOYUP, EVENT_JOYAXIS, EVENT_JOYHAT,
		EVENT_MOUSEOVER, EVENT_MOUSEOUT, EVENT_CLICK, EVENT_MOUSEUPOUTSIDE,
		EVENT_FOCUSCHANGED, EVENT_NAVIGATION, EVENT_TOUCH, EVENT_UNTOUCH,
		EVENT_FINISHED, EVENT_RESIZED, EVENT_AWAKE, EVENT_LAYOUT, EVENT_ERROR,
		EVENT_CHANGE, EVENT_LOG, EVENT_DESTROYED,
		NULL
	};
	for ( size_t i = 0; interns[ i ] != NULL; i++ ) {
		script.InternString( interns[ i ] );
	}
	
	// call registration functions for all classes
	input.InitClass(); // single instance
	Controller::InitClass();
    TypedVector::InitClass();
    Color::InitClass();
    Sound::InitClass();
    Tween::InitClass();
    Behavior::InitClass();
	GameObject::InitClass();
	Scene::InitClass();
	Image::InitClass();
	RigidBodyShape::InitClass();
	RigidBodyJoint::InitClass();
    ParticleSystem::InitClass();
	RenderBehavior::InitClass();
	RenderShapeBehavior::InitClass();
	RenderSpriteBehavior::InitClass();
	RenderTextBehavior::InitClass();
    RenderParticlesBehavior::InitClass();
	BodyBehavior::InitClass();
	RigidBodyBehavior::InitClass();
    ParticleGroupBehavior::InitClass();
	UIBehavior::InitClass();
	SampleBehavior::InitClass();
    
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
	// printf( "GC performed\n" );
}

void Application::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	// protect scenes stack
	for ( size_t i = 0, ns = this->sceneStack.size(); i < ns; i++ ){
		protectedObjects.push_back( &sceneStack[ i ]->scriptObject );
	}
	
	// protect overlay
	protectedObjects.push_back( &overlay->scriptObject );
	
	// add debouncers and asyncs
	AsyncMap::iterator it = scheduledAsyncs->begin();
	while ( it != scheduledAsyncs->end() ) {
		ScheduledCallList &list = it->second;
		ScheduledCallList::iterator lit = list.begin();
		while ( lit != list.end() ) {
			ScheduledCall& sched = *lit;
			protectedObjects.push_back( &sched.func.funcObject );
			if ( sched.func.thisObject ) protectedObjects.push_back( &sched.func.thisObject );
			lit++;
		}
		it++;
	}
	DebouncerMap::iterator dit = scheduledDebouncers->begin();
	while( dit != scheduledDebouncers->end() ) {
		unordered_map<string, ScheduledCall> &debouncers = dit->second;
		unordered_map<string, ScheduledCall>::iterator dbit = debouncers.begin();
		while ( dbit != debouncers.end() ) {
			ScheduledCall& sched = dbit->second;
			protectedObjects.push_back( &sched.func.funcObject );
			if ( sched.func.thisObject ) protectedObjects.push_back( &sched.func.thisObject );
			dbit++;
		}
		dit++;
	}
	
	// protect params of late events
	LateEventMap::iterator oit = lateEvents.begin(), oend = lateEvents.end();
	while ( oit != oend ) {
		ScriptableClass* obj = oit->first;
		ObjectEventMap& objMap = oit->second;
		ObjectEventMap::iterator it = objMap.begin(), end = objMap.end();
		protectedObjects.push_back( &obj->scriptObject );
		while ( it != end ) {
			LateEvent& le = it->second;
			for ( size_t i = 0, np = le.params.size(); i < np; i++ ) {
				// no arrays of object, but whatever, TODO? recursive add to protect?
				if ( le.params[ i ].type == TypeObject || le.params[ i ].type == TypeFunction ) {
					protectedObjects.push_back( &( le.params[ i ].value.objectValue ) );
				}
			}
			it++;
		}
		oit++;
	}
	
	// protect running tweens
	unordered_set<Tween*>::iterator tit = Tween::activeTweens->begin();
	while( tit != Tween::activeTweens->end() ) {
		if ( (*tit)->_active ) protectedObjects.push_back( &(*tit)->scriptObject );
		tit++;
	}
	
	// protect playing sounds
	unordered_set<Sound*>::iterator sit = Sound::activeSounds.begin(), send = Sound::activeSounds.end();
	while ( sit != send ) {
		protectedObjects.push_back( &(*sit)->scriptObject );
		sit++;
	}
	
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
}


/* MARK:	-				Game loop
 -------------------------------------------------------------------- */


/// main application loop
void Application::GameLoop() {
	
	// setup
	this->run = true;
	Uint32 _time = 0, _timeFps = 0, _t = 0, _rt = 0;
	Uint32 _quitPressedTime = 0;
	Event event;
	SDL_Event e;
	Scene* scene = NULL;
    Uint32 benchmark = 0;

	// stdin capture
	char pollChar = 0;
	struct pollfd pollStruct = { .fd=STDIN_FILENO, .events=POLLIN|POLLRDBAND|POLLRDNORM|POLLPRI };
	
	// add default scene
	app.sceneStack.push_back( new Scene( NULL ) );
	
	// load and run "main.js" script
	ScriptResource* mainScript = scriptManager.Get( "main.js" );
	if ( mainScript->error ) {
		mainScript = scriptManager.Get( "/main.js" ); // in root dir
		if ( mainScript->error ) return; // bail on compilation or not found error
	}
	script.Execute( mainScript );
	
	// add default "keyboard" controller to input
	input.AddKeyboardController();
	
	// init backscreen
	this->UpdateBackscreen();
	
	// initial resized and layout events
	this->SendResizedEvents();
	
	// main loop
	while( run ) {
		
		// get current scene
		scene = sceneStack.size() ? sceneStack.back() : NULL;
		
		// read time / delta time
		_time = SDL_GetTicks();
		this->unscaledDeltaTime = (float) _time * 0.001f - this->unscaledTime;
		this->deltaTime = this->unscaledDeltaTime * this->timeScale;
		this->time += this->deltaTime;
		this->unscaledTime += this->unscaledDeltaTime;
		
		// compute fps
		if ( ++this->frames % 100 == 0 ) {
			this->fps = 100.0f / (( _time - _timeFps ) * 0.001f );
			_timeFps = _time;
			
		}
		
		// advance tweens
		Tween::ProcessActiveTweens( deltaTime, unscaledDeltaTime );
		
		// if have active scene
		if ( scene ) {
		
			// simulate physics
			scene->SimulatePhysics();

			// update
			event.name = EVENT_UPDATE;
			event.scriptParams.ResizeArguments( 0 );
			event.scriptParams.AddFloatArgument( this->deltaTime );
			scene->DispatchEvent( event, true );
			event.skipObject = NULL;
			event.stopped = false; // reused
		}
        
		// update joysticks state
		SDL_JoystickUpdate();
		
		// handle system events
		while( SDL_PollEvent( &e ) != 0 ){
			
			// let input handle events
			if ( !benchmark ) input.HandleEvent( e );
			
			// exit
			if ( e.type == SDL_QUIT ) {
				run = false; break;
				
			// escape key down for 2+ sec
			} else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE ) {
				if ( e.key.repeat == 0 ) {
					_quitPressedTime = _time;
				} else if ( e.key.repeat && ( _time - _quitPressedTime ) > 2000 ) {
					run = false; break;
				}
            
            // benchmark
            } else if ( e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F1 ) {
                benchmark = (benchmark+1) % 10;
                debugEventsDispatched.clear();
            }
            

			// window events
			else if ( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ) {
				// adjust buffers
				WindowResized( e.window.data1, e.window.data2 );
			}
		}
		
		// capture console input (RPi workaround)
		if ( poll( &pollStruct, 1, 0 ) == 1 ) read( STDIN_FILENO, &pollChar, 1 );
        
		// perform asyncs & debounce calls
		ScriptableClass::ProcessScheduledCalls( unscaledDeltaTime );
		
		// late events (scheduled by `fireLate` and `dispatchLate` / Application::AddLateEvent )
		RunLateEvents();
        
        _rt = SDL_GetTicks();

		// clear with scene's bg color
		GPU_ClearColor( this->screen, scene->backgroundColor->rgba );
        GPU_ResetProjection();
        
        if ( benchmark ) {
            
            static ImageResource* clown = app.textureManager.Get( "clown" );
            GPU_ClearColor( this->backScreen->target, scene->backgroundColor->rgba );
            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PushMatrix();
            float *p = GPU_GetProjection();
            GPU_MatrixIdentity( p );
            GPU_MatrixOrtho( p, 0, this->backScreen->target->w, 0, this->backScreen->target->h, -1024, 1024 );
            
            // 1000 blits
            if ( benchmark == 2 ){
                _t = SDL_GetTicks();
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_Blit( clown->image, &clown->frame.locationOnTexture, this->backScreen->target, 4 * (i % 100), 100 + i / 10 );
                }
                debugEventsDispatched[ "1000 Blits" ] = SDL_GetTicks() - _t;
            // 1000 circles
            } else if ( benchmark == 3 ){
                _t = SDL_GetTicks();
                static SDL_Color clr = { 0, 255, 0, 255 };
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_CircleFilled(this->backScreen->target, 20 + 4 * (i % 100), 100 + i / 10, 32, clr );
                    clr.b = (clr.b + 3) % 255;
                }
                debugEventsDispatched[ "1000 Circles" ] = SDL_GetTicks() - _t;
            // 1000 rectangles
            } else if ( benchmark == 4 ){
                _t = SDL_GetTicks();
                static SDL_Color clr = { 0, 0, 255, 255 };
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_RectangleFilled(this->backScreen->target,
                                        20 + 4 * (i % 100), 100 + i / 10,
                                        52 + 4 * (i % 100), 132 + i / 10, clr);
                    clr.r = (clr.r + 3) % 255;
                }
                debugEventsDispatched[ "1000 Rects" ] = SDL_GetTicks() - _t;
            // 1000 mixed
            } else if ( benchmark == 5 ){
                _t = SDL_GetTicks();
                static SDL_Color clr = { 0, 0, 255, 255 };
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_RectangleFilled(this->backScreen->target,
                                        20 + 4 * (i % 100), 100 + i / 10,
                                        52 + 4 * (i % 100), 132 + i / 10, clr);
                    
                    clr.r = (clr.r + 3) % 255;
                    GPU_Blit( clown->image, &clown->frame.locationOnTexture, this->backScreen->target, 20 + 4 * (i % 100), 100 + i / 10 );
                }
                debugEventsDispatched[ "1000 Rects+Blits" ] = SDL_GetTicks() - _t;
            } else if ( benchmark == 6 ){
                _t = SDL_GetTicks();
                static SDL_Color clr = { 0, 0, 255, 255 };
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_RectangleFilled(this->backScreen->target,
                                        20 + 4 * (i % 100), 100 + i / 10,
                                        52 + 4 * (i % 100), 132 + i / 10, clr);
                    clr.r = (clr.r + 3) % 255;
                }
                for ( int i = 0; i < 1000; i++ ) {
                    GPU_Blit( clown->image, &clown->frame.locationOnTexture, this->backScreen->target, 20 + 4 * (i % 100), 100 + i / 10 );
                }
                debugEventsDispatched[ "1000 Rects,Blits" ] = SDL_GetTicks() - _t;
            }
            
            GPU_MatrixMode( GPU_PROJECTION );
            GPU_PopMatrix();
            
        } else {
            
            // render scene graph
            if ( scene ) {
                // render to backscreen
                event.name = EVENT_RENDER;
                event.behaviorParam = this->backScreen->target;
                event.behaviorParam2 = &this->blendTarget;
                scene->Render( event );
                event.behaviorParam = NULL;
                event.skipObject = NULL;
                event.stopped = false; // reused
            }
        }
        
		// copy to main screen and flip
        GPU_DeactivateShaderProgram();//GPU_ActivateShaderProgram(0, NULL);
		GPU_BlitRect( this->backScreen, &this->backScreenSrcRect, this->screen, &this->backScreenDstRect );

        debugEventsDispatched[ "RENDER" ] = SDL_GetTicks() - _rt;
        if ( this->debugDraw ) {
            _t = SDL_GetTicks();
            this->DebugDraw();
            debugEventsDispatched[ "DebugDraw" ] = SDL_GetTicks() - _t;
        }
        GPU_Flip( this->screen );

	}
	
	sceneStack.clear();
	script.GC();
	
	// close mixer
	Mix_CloseAudio();	
	
	// exit requested
}

/* MARK:    -                Debug draw
 -------------------------------------------------------------------- */

void Application::DebugDraw(){
    // debug info
    static char debugText[2048];
    static string evts;
    evts = "";
    auto it = debugEventsDispatched.begin(), end = debugEventsDispatched.end();
    int wrap = 0;
    while (it != end) {
        evts.append( it->first );
        evts.append( ":" );
        sprintf( debugText, "%lu ", it->second );
        evts.append( debugText );
        if ( !(++wrap % 4) ) evts.append( "\n" );
        it++;
    }
    sprintf( debugText,
            "FPS: %.1f\nScriptObjects (created - destroyed): %lu - %lu = %lu\n%s\n",
            this->fps,
            debugObjectsCreated, debugObjectsDestroyed, debugObjectsCreated - debugObjectsDestroyed,
            evts.c_str() );
    
    // render
    static FontResource* font = fontManager.Get( "Roboto,12" );
    static SDL_Color clr = { 255, 0, 0, 255 };
    SDL_Surface* surf = TTF_RenderText_Blended_Wrapped( font->font, debugText, clr, 500 );
    GPU_Image* surfImg = GPU_CopyImageFromSurface( surf );
    SDL_FreeSurface( surf );
    surfImg->anchor_x = surfImg->anchor_y = 0;
    GPU_Rect rect = { 0,0,0,0 };
    rect.w = surfImg->base_w;
    rect.h = surfImg->base_h;
    GPU_Blit(surfImg, &rect, this->screen, 0, 0 );
    GPU_FreeImage( surfImg );
}

/* MARK:	-				Late events
 -------------------------------------------------------------------- */


/// add / replace event to run right before render, returns params member of LateEvent struct
ArgValueVector* Application::AddLateEvent( ScriptableClass* obj, const char* eventName, bool dispatch, bool bubbles, bool behaviorsOnly ) {
	LateEvent* event = NULL;
	
	// find existing for this object
	string ename( eventName );
	ObjectEventMap& eventMap = lateEvents[ obj ];
	ObjectEventMap::iterator it = eventMap.find( ename );
	
	// found existing, return it
	if ( it != eventMap.end() ) {
		event = &(it->second);
		event->lateDispatch = dispatch;
	} else {
		// otherwise emplace new
		auto p = eventMap.emplace( ename, dispatch );
		event = &(p.first->second);
	}
	
	event->behaviorsOnly = behaviorsOnly;
	event->bubbles = bubbles;
	
	// return result
	return &event->params;
	
}

/// remove late events for object (on destruction)
void Application::RemoveLateEvents( ScriptableClass* obj ) {
	LateEventMap::iterator it = lateEvents.find( obj );
	if ( it != lateEvents.end() ) lateEvents.erase( it );
	it = lateEventsProcessing.find( obj );
	if ( it != lateEventsProcessing.end() ) lateEventsProcessing.erase( it );
}

// runs late events
void Application::RunLateEvents( int maxRepeats ) {
	lateEventsProcessing = lateEvents; // copy
	lateEvents.clear();
	LateEventMap::iterator oit = lateEventsProcessing.begin(), oend = lateEventsProcessing.end();
	size_t numProcessed = 0;
	while ( oit != oend ) {
		ScriptableClass* obj = oit->first;
		ObjectEventMap& objMap = oit->second;
		ObjectEventMap::iterator it = objMap.begin(), end = objMap.end();
		while ( it != end ) {
			if ( obj->scriptObject ) {
				LateEvent& le = it->second;
				Event event( it->first.c_str() );
				event.bubbles = le.bubbles;
				event.behaviorsOnly = le.behaviorsOnly;
				// add params
				for ( size_t i = 0, np = le.params.size(); i < np; i++ ) event.scriptParams.AddArgument( le.params[ i ] );
				// call
				if ( le.lateDispatch ) {
					GameObject* go = (GameObject*) obj;
					if ( go ) go->DispatchEvent( event, true );
				} else {
					obj->ScriptableClass::CallEvent( event );
				}
			}
			it++;
			numProcessed++;
		}
		oit++;
	}
	lateEventsProcessing.clear();
	// run until no new events, or max iterations
	if ( numProcessed && maxRepeats > 0 ) RunLateEvents( maxRepeats - 1 );
}

/* MARK:	-				Global funcs ( from common.h )
 -------------------------------------------------------------------- */

/// if filepath starts with . uses current script location as start point, otherwise, app currentDirectory
/// if filepath starts with /, ignores optionalSubPath
/// otherwise if optionalSubPath specified, checks is startingPath/optionalSubPath/file/path exists & returns it
/// if not, returns startingPath/file/path
string ResolvePath( const char* filepath, const char* ext, const char* optionalSubPath ) {
	
	bool startsWithSlash = ( filepath[ 0 ] == '/' );
	bool startsWithDot = ( filepath[ 0 ] == '.' );
	bool startsWithTwoDots = ( filepath[ 1 ] == '.' );
	string startingPath = app.currentDirectory;
	if ( startsWithDot ) {
		// get current script
		unsigned lineNumber = 0;
		JSScript* curScript = NULL;
		if ( JS_DescribeScriptedCaller( script.js, &curScript, &lineNumber ) ) {
			const char* sp = JS_GetScriptFilename( script.js, curScript );
			string scriptPath = sp ? sp : app.currentDirectory;
			string shortPath = scriptPath;
			shortPath = shortPath.substr( app.currentDirectory.length() );
			// find script
			ScriptResource* sr = app.scriptManager.Get( shortPath.c_str() );
			if ( sr->error ) {
				unordered_map<string, ScriptResource*>::iterator it = app.scriptManager.map.begin();
				while ( it != app.scriptManager.map.end() ) {
					if ( it->second->path.compare( scriptPath ) == 0 ) {
						sr = it->second;
						break;
					}
					it++;
				}
			}
			if ( !sr->error ) {
				// remove filename from current script
				vector<string> parts = Resource::splitString( sr->path, "/" );
				parts.pop_back();
				if ( startsWithTwoDots ) parts.pop_back();
				startingPath = Resource::concatStrings( parts, "/" );
			}
		}
	}
	
	// split path into chunks
	string fileKey( filepath );
	if ( startsWithDot ) fileKey = fileKey.substr( startsWithTwoDots ? 2 : 1 );
	vector<string> parts = Resource::splitString( fileKey, string( "/" ) );
	
	// append file extension, if missing
	string::size_type extPos = parts[ parts.size() - 1 ].find_last_of( '.' );
	if ( extPos == string::npos && extPos > 0 && ext ) fileKey += string( "." ) + ext;
	
	// ensure first / is stripped
	if ( fileKey.c_str()[ 0 ] == '/' ) fileKey = fileKey.substr( 1 );
	
	string path = startingPath;
	
	// if given optionalSubpath
	if ( optionalSubPath && !startsWithSlash && !startsWithDot ) {
		path += optionalSubPath + fileKey;
		if ( access( path.c_str(), R_OK ) == 0 ) return path;
		// no? reset
		path = startingPath;
	}
	
	// done
	return path + string( "/" ) + fileKey;
}

string ResolvePath( const char* filepath, const char* commaSeparatedExtensions, string& extension, const char* optionalSubPath ) {
	
	bool startsWithSlash = ( filepath[ 0 ] == '/' );
	bool startsWithDot = ( filepath[ 0 ] == '.' );
	bool startsWithTwoDots = ( filepath[ 1 ] == '.' );
	string startingPath = app.currentDirectory;
	if ( startsWithDot ) {
		// get current script
		unsigned lineNumber = 0;
		JSScript* curScript = NULL;
		if ( JS_DescribeScriptedCaller( script.js, &curScript, &lineNumber ) ) {
			string scriptPath = JS_GetScriptFilename( script.js, curScript );
			vector<string> parts = Resource::splitString( scriptPath, "/" );
			parts.pop_back();
			if ( startsWithTwoDots ) parts.pop_back();
			startingPath = Resource::concatStrings( parts, "/" );
			// find script
			/*unordered_map<string, ScriptResource*>::iterator it = app.scriptManager.map.begin();
			while ( it != app.scriptManager.map.end() ) {
				if ( it->second->path.compare( scriptPath ) == 0 ) {
					// remove filename from current script
					vector<string> parts = Resource::splitString( it->second->path, "/" );
					parts.pop_back();
					if ( startsWithTwoDots ) parts.pop_back();
					startingPath = Resource::concatStrings( parts, "/" );
					break;
				}
				it++;
			}*/
		}
	}
	
	// split path into chunks
	string fileKey( filepath );
	if ( startsWithDot ) fileKey = fileKey.substr( startsWithTwoDots ? 2 : 1 );
	vector<string> parts = Resource::splitString( fileKey, string( "/" ) );
	
	// ensure first / is stripped
	if ( fileKey.c_str()[ 0 ] == '/' ) {
		fileKey = fileKey.substr( 1 );
		parts.erase( parts.begin() );
	}

	// given set of extensions
	vector<string> extensions = Resource::splitString( string( commaSeparatedExtensions ), "," );
	size_t startIndex = 0;
	
	// if already have extension
	string::size_type extPos = parts.back().find_last_of( '.' );
	if ( extPos != string::npos ) {
		// set start index to it
		vector<string>::iterator it = find( extensions.begin(), extensions.end(), parts.back().substr( extPos + 1 ) );
		startIndex = ( it == extensions.end() ? 0 : ( it - extensions.begin() ) );
		// strip it
		parts.back() = parts.back().substr( 0, extPos );
		fileKey = Resource::concatStrings( parts, "/" );
	}
	
	// try each extension
	string path;
	for ( size_t i = startIndex, ne = extensions.size(); i < ne; i++ ) {
		extension = extensions[ i ];
		if ( optionalSubPath && !startsWithSlash && !startsWithDot ) {
			path = startingPath + optionalSubPath + fileKey + string( "." ) + extension;
			if ( access( path.c_str(), R_OK ) == 0 ) return path;
		}

		path = startingPath + string( "/" ) + fileKey + string( "." ) + extension;
		if ( access( path.c_str(), R_OK ) == 0 ) return path;
	}
	
	return path;
}

const char* ReadFile( const char* absoluteFilepath, size_t *fileSize ) {
	
	// load file
	FILE *f = fopen( absoluteFilepath, "r" );
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
	
	if ( fileSize != NULL ) *fileSize = fsize;
	return buf;
	
}

/// read file from path, returns buffer on success, or NULL. Call free(buf) after using
const char* ReadFile( const char* filepath, const char* ext, const char* optionalSubPath, string* finalPath, size_t *fileSize ) {
	
	string path = ResolvePath( filepath, ext, optionalSubPath );
	
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
/// if existsOverwrite is supplied:
//		if file exists, and initial value of *existsOverwrite == true, file will be overwritten, returns true
//		if file exists, and initial value of *existsOverwrite == false, will set existsOverwrite = true, and return false
// if existsOverwrite == NULL and file exists, will not overwrite and return false
bool SaveFile( const char* data, size_t numBytes, const char* filepath, const char* ext, bool* existsOverwrite=NULL ) {
	
	// path
	string path = ResolvePath( filepath, ext, NULL );
	
	// check if exists
	if ( access( path.c_str(), R_OK ) == 0 ) {
		if ( existsOverwrite != NULL ) {
			// overwrite is false
			if ( *existsOverwrite == false ) {
				*existsOverwrite = true; // mark exists, fail
				return false;
			}
		} else return false;
	}
	
	// create subfolders leading up to file
	vector<string> parts = Resource::splitString( path.substr( app.currentDirectory.length() + 1 ), string( "/" ) );
	string subDir;
	for ( int i = 0; i < parts.size() - 1; i++ ) {
		subDir = app.currentDirectory + "/" + parts[ i ];
		if ( access( subDir.c_str(), R_OK ) == -1 ) {
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

bool DeleteFile( const char* filepath ) {
	string path = ResolvePath( filepath, NULL, NULL );
	if ( access( path.c_str(), R_OK ) != -1 ) {
		unlink( path.c_str() );
		return true;
	}
	return false;
}

/// returns currently executing script pathname:lineNumber
string GetScriptNameAndLine() {
	unsigned lineNumber = 0;
	JSScript* curScript = NULL;
	if ( JS_DescribeScriptedCaller( script.js, &curScript, &lineNumber ) ) {
		const char* scriptPath = JS_GetScriptFilename( script.js, curScript );
		static char buf[ 512 ];
		sprintf( buf, "%s:%u", scriptPath, lineNumber );
		return string( buf );
	}
	return "";
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
	if ( angleDegrees != 0 ) GPU_MatrixRotate( mat, angleDegrees, 0, 0, 1 );
	if ( scaleX != 0 || scaleY != 0 ) GPU_MatrixScale( mat, scaleX, scaleY, 1 );
	return mat;
}

void MatrixSkew( float* result, float sx, float sy ) {
	if(result == NULL) return;
	float A[16];
	A[0] = 1; A[1] = tan(sy * DEG_TO_RAD); A[2] = 0; A[3] = 0;
	A[4] = tan(sx * DEG_TO_RAD); A[5] = 1; A[6] = 0; A[7] = 0;
	A[8] = 0; A[9] = 0; A[10] = 1; A[11] = 0;
	A[12] = 0; A[13] = 0; A[14] = 0; A[15] = 1;
	GPU_MultiplyAndAssign(result, A);
}

string HexStr( Uint32 w, size_t hex_len ) {
	static const char* digits = "0123456789ABCDEF";
	string rc( hex_len, '0' );
	for ( size_t i = 0, j = ( hex_len-1 ) * 4 ; i < hex_len; ++i, j-=4 ) {
		rc[ i ] = digits[ ( w >> j ) & 0x0f ];
	}
	return rc;
}

// finds pos utf+8 character in string, or -1 if out of bounds
int StringPositionToIndex( const char* str, int pos ) {

	if ( pos < 0 ) return 0;
	
	const char *current = str;
	size_t characterPos = 0;
	
	while ( *current != 0 ) {
		if ( characterPos == pos ) break;
		// decode utf-8
		if ( (*current & 0x80) != 0 ) {
			if ( (*current & 0xE0) == 0xC0 ) {
				current += 1;
			} else if ( (*current & 0xF0) == 0xE0 ) {
				current += 2;
			} else if ( (*current & 0xF8) == 0xF0 ) {
				current += 3;
			} else if ( (*current & 0xFC) == 0xF8 ) {
				current += 4;
			} else if ( (*current & 0xFE) == 0xFC ) {
				current += 5;
			}
			// ascii
		}
		current++;
		characterPos++;
		
	}
	return (int)( current - str );
	
}

// converts index to position
int StringIndexToPosition( const char* str, int index ) {
	
	if ( index < 0 ) return 0;
	
	const char *current = str;
	size_t characterPos = 0;
	size_t characterIndex = 0;
	
	while ( *current != 0 ) {
		
		// decode utf-8
		if ( (*current & 0x80) != 0 ) {
			if ( (*current & 0xE0) == 0xC0 ) {
				current += 1;
			} else if ( (*current & 0xF0) == 0xE0 ) {
				current += 2;
			} else if ( (*current & 0xF8) == 0xF0 ) {
				current += 3;
			} else if ( (*current & 0xFC) == 0xF8 ) {
				current += 4;
			} else if ( (*current & 0xFE) == 0xFC ) {
				current += 5;
			}
			// ascii
		}
		characterIndex = current - str;
		if ( characterIndex >= index ) return (int) characterPos;
		current++;
		characterPos++;
		
	}
	return (int) characterPos;

}

int StringPositionLength( const char* str ) {
	
	const char *current = str;
	size_t characterPos = 0;
	
	while ( *current != 0 ) {
		// decode utf-8
		if ( (*current & 0x80) != 0 ) {
			if ( (*current & 0xE0) == 0xC0 ) {
				current += 1;
			} else if ( (*current & 0xF0) == 0xE0 ) {
				current += 2;
			} else if ( (*current & 0xF8) == 0xF0 ) {
				current += 3;
			} else if ( (*current & 0xFC) == 0xF8 ) {
				current += 4;
			} else if ( (*current & 0xFE) == 0xFC ) {
				current += 5;
			}
		}
		current++;
		characterPos++;
	}
	return (int) characterPos;
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

string base64_encode( unsigned char const* bytes_to_encode, unsigned int in_len ) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			
			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i) {
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';
		
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		
		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];
		
		while((i++ < 3))
			ret += '=';
		
	}
	
	return ret;
	
}

string base64_decode( string const& encoded_string ) {
	size_t in_len = encoded_string.size();
	size_t i = 0;
	size_t j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;
	
	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
			
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			
			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}
	
	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;
		
		for (j = 0; j <4; j++)
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
		
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
		
		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}
	
	return ret;
}

// exec command line util
string ExecCommand( const char* cmd ) {
	// exec
	FILE* pipe = popen(cmd, "r");
	if (!pipe) return "";
	char buffer[128];
	std::string result = "";
	while( !feof( pipe ) ) {
		if( fgets( buffer, 128, pipe ) != NULL ) {
			result += buffer;
		}
	}
	pclose(pipe);
	
	// truncate newline from end
	size_t resultSize = result.size();
	if ( resultSize > 0 && result.c_str()[ resultSize - 1 ] == '\n' ) result.resize( resultSize - 1 );
	return result;
}


