#include "Application.hpp"
#include "RenderShapeBehavior.hpp"
#include "RenderSpriteBehavior.hpp"
#include "RenderTextBehavior.hpp"
#include "UIBehavior.hpp"
#include "SampleBehavior.hpp"
#include "TypedVector.hpp"

// from ScriptableClass.hpp
int ScriptableClass::asyncIndex = 0;
ScriptableClass::AsyncMap* ScriptableClass::scheduledAsyncs = NULL;
ScriptableClass::DebouncerMap* ScriptableClass::scheduledDebouncers = NULL;

// from Tween.hpp
unordered_set<Tween*> *Tween::activeTweens = NULL;


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
	
	// init containers
	ScriptableClass::scheduledAsyncs = new AsyncMap();
	ScriptableClass::scheduledDebouncers = new DebouncerMap();
	Tween::activeTweens = new unordered_set<Tween*>();
	
	// register classes
	this->InitClass();
	
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
	
	// printf( "(frames:%d, seconds:%f) average FPS: %f\n", this->frames, this->unscaledTime, ((float) this->frames / (float) this->unscaledTime) );
	
	// close mixer
	Mix_CloseAudio();
	
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


void Application::InitRender() {
	
	// init SDL_gpu library
	
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
	
	this->windowWidth = this->screen->base_w;
	this->windowHeight = this->screen->base_h;
	GPU_UnsetVirtualResolution( this->screen );
	
	// resizable
	SDL_SetWindowResizable( SDL_GL_GetCurrentWindow(), (SDL_bool) windowResizable );
	
#ifdef __MACOSX__
	// make windowed on desktop
	GPU_SetFullscreen( false, false );
#endif
	
}

void Application::WindowResized( Sint32 newWidth, Sint32 newHeight ) {
	if ( ( newWidth && newHeight ) && ( newWidth != this->windowWidth || newHeight != this->windowHeight ) ) {
		GPU_SetWindowResolution( newWidth, newHeight );
		this->windowWidth = newWidth;
		this->windowHeight = newHeight;
		GPU_UnsetVirtualResolution( this->screen );
		UpdateBackscreen();
		
		if ( app.sceneStack.size() ) app.sceneStack.back()->_camTransformDirty = true;
		
		// send event
		Event event( EVENT_RESIZED );
		event.scriptParams.AddIntArgument( newWidth / app.windowScalingFactor );
		event.scriptParams.AddIntArgument( newHeight / app.windowScalingFactor );
		this->CallEvent( event );
		
		// and send layout event to scene
		if ( sceneStack.size() ) {
			event.name = EVENT_LAYOUT;
			event.stopped = false;
			event.scriptParams.ResizeArguments( 0 );
			sceneStack.back()->DispatchEvent( event );
		}

	}
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
	if ( !GPU_AddDepthBuffer( this->backScreen->target ) ){
		printf( "Warning: depth buffer is not supported.\n" );
	}
	GPU_SetDepthTest( this->backScreen->target, true );
	GPU_SetDepthWrite( this->backScreen->target, true );
	GPU_SetDepthFunction( this->backScreen->target, GPU_ComparisonEnum::GPU_LEQUAL );
	this->backScreen->target->camera.z_near = -1024;
	this->backScreen->target->camera.z_far = 1024;
	
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


/// initialize class scripting
void Application::InitClass() {
	
	// register base class
	ScriptableClass::InitClass();
	
	// register class
	script.RegisterClass<Application>( "ScriptableObject", true );
	
	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "BlendMode", constants );
	script.SetProperty( "Normal", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_NORMAL ), constants );
	script.SetProperty( "PremultipliedAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_PREMULTIPLIED_ALPHA ), constants );
	script.SetProperty( "Multiply", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_MULTIPLY ), constants );
	script.SetProperty( "Add", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_ADD ), constants );
	script.SetProperty( "Subtract", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_SUBTRACT ), constants );
	script.SetProperty( "ModAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_MOD_ALPHA ), constants );
	script.SetProperty( "SetAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_SET_ALPHA ), constants );
	script.SetProperty( "Set", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_SET ), constants );
	script.SetProperty( "NormalKeepAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_NORMAL_KEEP_ALPHA ), constants );
	script.SetProperty( "NormalAddAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_NORMAL_ADD_ALPHA ), constants );
	script.SetProperty( "NormalFactorAlpha", ArgValue( GPU_BlendPresetEnum::GPU_BLEND_NORMAL_FACTOR_ALPHA ), constants );
	script.SetProperty( "CutAlpha", ArgValue( GPU_BLEND_CUT_ALPHA ), constants );
	script.FreezeObject( constants );
	
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
			// generate event
			Event event;
			event.name = EVENT_SCENECHANGED;
			event.scriptParams.AddObjectArgument( current ? current->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			app.CallEvent( event );
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
	("windowResizable",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.windowResizable; }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		SDL_SetWindowResizable( SDL_GL_GetCurrentWindow(), ((SDL_bool) (app.windowResizable = v)) );
		return v;
	}));
	
	script.AddProperty<Application>
	("windowWidth", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowWidth / app.windowScalingFactor; }) );
	
	script.AddProperty<Application>
	("windowHeight", static_cast<ScriptIntCallback>([](void* self, int v ) { return app.windowHeight / app.windowScalingFactor; }) );
	
	script.AddProperty<Application>
	("fullScreen",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return GPU_GetFullscreen(); }),
	 static_cast<ScriptBoolCallback>([](void* self, bool v){
		GPU_SetFullscreen( v, false );
		return v;
	}));
	
	script.AddProperty<Application>
	("isUnserializing",
	 static_cast<ScriptBoolCallback>([](void* self, bool v){ return app.isUnserializing; }));
	
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
			event.name = EVENT_SCENECHANGED;
			event.scriptParams.AddObjectArgument( oldScene ? oldScene->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			app.CallEvent( event );
		}
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
			event.name = EVENT_SCENECHANGED;
			event.scriptParams.AddObjectArgument( oldScene ? oldScene->scriptObject : NULL );
			event.scriptParams.AddObjectArgument( newScene ? newScene->scriptObject : NULL );
			app.CallEvent( event );
		}
		sa.ReturnObject( newScene ? newScene->scriptObject : NULL );
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
		w *= app.windowScalingFactor;
		h *= app.windowScalingFactor;
		app.WindowResized( max( 320, min( 4096, w ) ), max( 320, min( 4096, h ) ) );
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
	
	script.DefineGlobalFunction
	( "load",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		string filename;
		if ( !sa.ReadArguments( 1, TypeString, &filename ) ) {
			script.ReportError( "usage: load( String filename )" );
			return false;
		}
		// read file
		string path;
		const char* buf = ReadFile( filename.c_str(), "json", NULL, &path, NULL );
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
		
		// add other params, if given
		if ( sa.args.size() >= 2 ) stringifyArgs.AddArgument( sa.args[ 1 ] );
		if ( sa.args.size() >= 3 ) stringifyArgs.AddArgument( sa.args[ 2 ] );
		
		// call original stringify function and return result
		sa.ReturnValue( script.CallClassFunction( "JSON", jsonStringify.value.objectValue, stringifyArgs ) );
		return true;
	}));
	
	script.DefineGlobalFunction
	( "save",
	 static_cast<ScriptFunctionCallback>([](void*, ScriptArguments& sa ){
		void *obj = NULL;
		string filename;
		if ( !sa.ReadArguments( 2, TypeObject, &obj, TypeString, &filename ) ) {
			script.ReportError( "usage: save( Object obj, String filename )" );
			return false;
		}
		
		// call own stringify function
		ScriptArguments parseArgs;
		parseArgs.AddObjectArgument( obj );
		parseArgs.AddObjectArgument( NULL );
		parseArgs.AddStringArgument( "\t" );
		ArgValue ret = script.CallGlobalFunction( "stringify", parseArgs );
		if ( ret.type != TypeString ) {
			sa.ReturnBool( false );
			return true;
		}
		
		// save to file
		sa.ReturnBool( SaveFile( ret.value.stringValue->c_str(), ret.value.stringValue->length(), filename.c_str(), "json" ) );
		
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
		sa.ReturnString( string( event->name ) );
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
		
		if ( Event::eventStack.size() ) sa.ReturnString( string( Event::eventStack.back()->name ) );
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
		ArgValue ret;
		script.Execute( app.scriptManager.Get( path.c_str() ), thisObj ? thisObj : sa.GetThis(), &ret );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// global toInit serializer
	script.DefineGlobalFunction
	( "serialize",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj ) ){
			script.ReportError( "usage: serialize( Object object )" );
			return false;
		}
		ArgValue ret( initObj );
		ret = script.MakeInitObject( ret );
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

	// global toInit serializer
	script.DefineGlobalFunction
	( "clone",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		void* initObj = NULL;
		if ( !sa.ReadArguments( 1, TypeObject, &initObj ) ){
			script.ReportError( "usage: clone( Object object )" );
			return false;
		}
		ArgValue ret( initObj );
		ret = script.MakeInitObject( ret );
		void *def = ret.value.objectValue;
		ret.value.objectValue = script.InitObject( def );
		sa.ReturnValue( ret );
		return true;
	}) );
	
	// schedule a call
	script.DefineGlobalFunction
	( "async",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: async( Function handler [, Number delay ] )";
		void* handler = NULL;
		float delay = 0;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeFunction, &handler, TypeFloat, &delay ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		sa.ReturnInt( ScriptableClass::AddAsync( (void*) script.global_object, handler, delay ) );
		return true;
	}));
	
	// cancel a call
	script.DefineGlobalFunction
	( "cancelAsync",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelAsync( Int asyncId )";
		int index = 0;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeInt, &index ) ) {
			script.ReportError( error );
			return false;
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
		const char* error = "usage: debounce( String debounceId, Function handler [, Number delay ] )";
		void* handler = NULL;
		string name;
		float delay = 0;
		
		// validate
		if ( !sa.ReadArguments( 2, TypeString, &name, TypeFunction, &handler, TypeFloat, &delay ) ) {
			script.ReportError( error );
			return false;
		}
		// schedule call
		ScriptableClass::AddDebouncer( (void*) script.global_object, name, handler, delay );
		return true;
	}));
	
	// cancel a call
	script.DefineGlobalFunction
	( "cancelDebouncer",
	 static_cast<ScriptFunctionCallback>([](void* o, ScriptArguments& sa ){
		// validate params
		const char* error = "usage: cancelDebouncer( String debounceId )";
		string name;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeString, &name ) ) {
			script.ReportError( error );
			return false;
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
	
	// intern all strings
	const char* interns[] = {
		EVENT_SCENECHANGED, EVENT_UPDATE, EVENT_LATE_UPDATE, EVENT_ADDED,
		EVENT_REMOVED, EVENT_ADDED_TO_SCENE, EVENT_REMOVED_FROM_SCENE,
		EVENT_CHILDADDED, EVENT_CHILDREMOVED, EVENT_ACTIVE_CHANGED,
		EVENT_ATTACHED, EVENT_DETACHED, EVENT_RENDER, EVENT_KEYDOWN, EVENT_KEYUP,
		EVENT_KEYPRESS, EVENT_MOUSEDOWN, EVENT_MOUSEUP, EVENT_MOUSEMOVE, EVENT_MOUSEWHEEL,
		EVENT_CONTROLLERADDED, EVENT_CONTROLLERREMOVED,
		EVENT_JOYDOWN, EVENT_JOYUP, EVENT_JOYAXIS, EVENT_JOYHAT,
		EVENT_MOUSEOVER, EVENT_MOUSEOUT, EVENT_CLICK, EVENT_MOUSEUPOUTSIDE,
		EVENT_FOCUSCHANGED, EVENT_NAVIGATION, EVENT_TOUCH, EVENT_UNTOUCH,
		EVENT_FINISHED, EVENT_RESIZED, EVENT_AWAKE, EVENT_LAYOUT, EVENT_ERROR,
		EVENT_CHANGE, EVENT_LOG,
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
	GameObject::InitClass();
	Scene::InitClass();
	Image::InitClass();
	RigidBodyShape::InitClass();
	RigidBodyJoint::InitClass();
	Behavior::InitClass();
	RenderBehavior::InitClass();
	RenderBehavior::InitShaders();
	RenderShapeBehavior::InitClass();
	RenderSpriteBehavior::InitClass();
	RenderTextBehavior::InitClass();
	BodyBehavior::InitClass();
	RigidBodyBehavior::InitClass();
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
	
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
}


/* MARK:	-				Game loop
 -------------------------------------------------------------------- */


/// main application loop
void Application::GameLoop() {
	
	// setup
	this->run = true;
	Uint32 _time = 0, _timeFps = 0;
	Uint32 _quitPressedTime = 0;
	Event event;
	SDL_Event e;
	Scene* scene = NULL;

	// stdin capture
	char pollChar = 0;
	struct pollfd pollStruct = { .fd=STDIN_FILENO, .events=POLLIN|POLLRDBAND|POLLRDNORM|POLLPRI };
	
	// add default scene
	app.sceneStack.push_back( new Scene( NULL ) );
	
	// load and run "main.js" script
	ScriptResource* mainScript = scriptManager.Get( "/main.js" );
	if ( mainScript->error ) return; // bail on compilation or not found error
	script.Execute( mainScript );
	
	// add default "keyboard" controller to input
	input.AddKeyboardController();
	
	// init backscreen
	this->UpdateBackscreen();
	
	// main loop
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
		
		// perform asyncs & debounce calls
		ScriptableClass::ProcessScheduledCalls( unscaledDeltaTime );
		
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
			event.stopped = false; // reused
		}
		
		// update joysticks state
		SDL_JoystickUpdate();
		
		// handle system events
		while( SDL_PollEvent( &e ) != 0 ){
			
			// let input handle events
			input.HandleEvent( e );
			
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
			}

			// window events
			else if ( e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ) {
				// adjust buffers
				WindowResized( e.window.data1, e.window.data2 );
			}
		}
		
		// capture console input (RPi workaround)
		if ( poll( &pollStruct, 1, 0 ) == 1 ) read( STDIN_FILENO, &pollChar, 1 );
		
		// continue
		if ( scene ) {
		
			// late update
			event.name = EVENT_LATE_UPDATE;
			scene->DispatchEvent( event, true );
			event.stopped = false; // reused
		}
		
		// late events (scheduled by `fireLate` and `dispatchLate` / Application::AddLateEvent )
		RunLateEvents();
		
		// render scene graph
		if ( scene ) {
			// render to backscreen
			event.name = EVENT_RENDER;
			event.behaviorParam = this->backScreen->target;
			scene->Render( event );
			event.behaviorParam = NULL;
			event.stopped = false; // reused
		}
		
		// copy to main screen and flip
		GPU_ActivateShaderProgram(0, NULL);
		GPU_Clear( this->screen );
		GPU_BlitRect( this->backScreen, &this->backScreenSrcRect, this->screen, &this->backScreenDstRect );
		GPU_Flip( this->screen );
		
	}
	
	sceneStack.clear();
	script.GC();
	
	// exit requested
}


/* MARK:	-				Late events
 -------------------------------------------------------------------- */


/// add / replace event to run right before render, returns params member of LateEvent struct
ArgValueVector* Application::AddLateEvent( ScriptableClass* obj, const char* eventName, bool dispatch ) {
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
	
	// return result
	return &event->params;
	
}


/// remove late events for object (on destruction)
void Application::RemoveLateEvents( ScriptableClass* obj ) {
	LateEventMap::iterator it = lateEvents.find( obj );
	if ( it != lateEvents.end() ) lateEvents.erase( it );
}

// runs late events
void Application::RunLateEvents() {
	lateEventsProcessing = lateEvents; // copy
	lateEvents.clear();
	LateEventMap::iterator oit = lateEventsProcessing.begin(), oend = lateEventsProcessing.end();
	while ( oit != oend ) {
		ScriptableClass* obj = oit->first;
		ObjectEventMap& objMap = oit->second;
		ObjectEventMap::iterator it = objMap.begin(), end = objMap.end();
		while ( it != end ) {
			LateEvent& le = it->second;
			Event event( it->first.c_str() );
			// add params
			for ( size_t i = 0, np = le.params.size(); i < np; i++ ) event.scriptParams.AddArgument( le.params[ i ] );
			// call
			if ( le.lateDispatch ) {
				GameObject* go = (GameObject*) obj;
				if ( go ) go->DispatchEvent( event, true );
			} else {
				obj->ScriptableClass::CallEvent( event );
			}
			it++;
		}
		oit++;
	}
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
			const char* scriptPath = JS_GetScriptFilename( script.js, curScript );
			// find script
			unordered_map<string, ScriptResource*>::iterator it = app.scriptManager.map.begin();
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
			const char* scriptPath = JS_GetScriptFilename( script.js, curScript );
			// find script
			unordered_map<string, ScriptResource*>::iterator it = app.scriptManager.map.begin();
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
			}
		}
	}
	
	// split path into chunks
	string fileKey( filepath );
	if ( startsWithDot ) fileKey = fileKey.substr( startsWithTwoDots ? 2 : 1 );
	vector<string> parts = Resource::splitString( fileKey, string( "/" ) );
	
	// ensure first / is stripped
	if ( fileKey.c_str()[ 0 ] == '/' ) fileKey = fileKey.substr( 1 );

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
bool SaveFile( const char* data, size_t numBytes, const char* filepath, const char* ext ) {
	
	// path
	string path = ResolvePath( filepath, ext, NULL );
	
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
		char *buf = (char*) malloc( strlen( scriptPath ) + 32 );
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


