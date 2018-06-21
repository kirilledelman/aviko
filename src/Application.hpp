#ifndef Application_hpp
#define Application_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "Scene.hpp"
#include "Input.hpp"
#include "Controller.hpp"
#include "ImageResource.hpp"
#include "ScriptResource.hpp"
#include "FontResource.hpp"
#include "SoundResource.hpp"
#include "Tween.hpp"
#include "Sound.hpp"
#include "Image.hpp"

// for input capture workaround on Pi
#include <unistd.h>
#include <termios.h>
#include <poll.h>

// Main application controller
class Application : public ScriptableClass {
public:
	
	// init / destroy
	Application();
	Application( ScriptArguments* );
	~Application();
	
// JS
	
	/// registers classes for scripting
	void InitClass();
	
	/// used with garbage collection
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
// scene management
	
	/// current scene
	vector<Scene*> sceneStack;
	
	/// overlay
	GameObject* overlay = NULL;
	
// rendering
	
	/// display width
	Uint16 windowWidth = 800;

	/// display height
	Uint16 windowHeight = 480;
	
	/// screen width
	Uint16 screenWidth = 1024;
	
	/// screen height
	Uint16 screenHeight = 768;
	
	/// upscaling
	float windowScalingFactor = 2;
	
	bool windowResizable = true;
	
	/// computed in UpdateBackscreen
	float backscreenScale = 0.5;
	
	// draw debug data
	bool debugDraw = false;
		
	/// main display
	GPU_Target* screen = NULL;
	GPU_Rect screenRect;

	/// downscaled render target
	GPU_Image* backScreen = NULL;
	GPU_Target* blendTarget = NULL;
	GPU_Rect backScreenDstRect;
	GPU_Rect backScreenSrcRect;
	
	// (re) creates main rendering window
	void InitRender();
	
	// (re) creates back buffer
	void UpdateBackscreen();
	
	// callback to resize screen
	void WindowResized( Sint32 newWidth, Sint32 newHeight );
	
	// sends top resized and layout events
	void SendResizedEvents();
	
// input
	
	Input input;

	// used to consume stdin input
	struct termios _savedTerminal;
	
// clipboard

	// used only on Pi, as there's no system clipboard
	string clipboard;
	
// resource managers
	
	/// full path to executable + resource directory
	string currentDirectory = "";
	
	/// resources
	string configDirectory = "/config/";
	string texturesDirectory = "/textures/";
	string soundsDirectory = "/sounds/";
	string fontsDirectory = "/fonts/";
	string scriptsDirectory = "/scripts/";
	string defaultFontName = "Roboto-Regular";
	
	/// stores loaded textures
	ResourceManager<ImageResource> textureManager;
	
	/// stores fonts
	ResourceManager<FontResource> fontManager;
	
	/// stores compiled scripts
	ResourceManager<ScriptResource> scriptManager;
	
	/// stores compiled scripts
	ResourceManager<SoundResource> soundManager;
	
	/// unloads unused resources and calls garbage collector in script 
	void GarbageCollect();

// event queue
	
	// late events are run right before render (layout changes, dispatchLate )
	struct LateEvent {
		bool lateDispatch = false;
		bool bubbles = false;
		bool behaviorsOnly = false;
		ArgValueVector params;
		LateEvent( bool disp ) : lateDispatch( disp ) {};
	};
	typedef unordered_map<string,LateEvent> ObjectEventMap;
	typedef unordered_map<ScriptableClass*, ObjectEventMap> LateEventMap;
	
	LateEventMap lateEvents;
	LateEventMap lateEventsProcessing; // used to allow adding late events while dispatching late events
	
	/// true while InitObject is in progress
	bool isUnserializing = false;
	
	/// add / replace event to run right before render
	ArgValueVector* AddLateEvent( ScriptableClass* obj, const char* eventName, bool dispatch=false, bool bubbles=false, bool behaviorsOnly=false );
	
	/// remove late events for object (on destruction)
	void RemoveLateEvents( ScriptableClass* obj );
	
	// runs late events
	void RunLateEvents( int maxRepeats = 0 );
	
// running
	
	/// time scaling factor
	float timeScale = 1.0f;
	
	/// time since game loop started
	float time = 0;
	
	/// unscaled time since game loop started
	float unscaledTime = 0;
	
	/// time passed since last frame
	float deltaTime = 0;
	
	/// unscaled time passed since last frame
	float unscaledDeltaTime = 0;
	
	/// total frames rendered
	int frames = 0;
	
	/// current frames per second
	float fps = 0;
	
	/// set to false to exit next frame
	bool run = false;
	
	/// main game loop
    void GameLoop();
	
};

// Application -> script class "Application"
SCRIPT_CLASS_NAME( Application, "App" );

#endif
