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
#include "Sound.hpp"
#include "Image.hpp"

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
	
// scene management
	
	/// current scene
	vector<Scene*> sceneStack;
	
// rendering
	
	/// display width
	Uint16 windowWidth = 800;

	/// display height
	Uint16 windowHeight = 480;
	
	/// upscaling
	float windowScalingFactor = 2;
	
	/// computed in UpdateBackscreen
	float backscreenScale = 0.5;
		
	/// main display
	GPU_Target* screen = NULL;
	GPU_Rect screenRect;

	/// downscaled render target
	GPU_Image* backScreen = NULL;
	GPU_Rect backScreenDstRect;
	GPU_Rect backScreenSrcRect;
	
	// (re) creates main rendering window
	void InitRender();
	
	// (re) creates back buffer
	void UpdateBackscreen();
	
// input
	
	Input input;

// resource managers
	
	/// full path to executable + resource directory
	string currentDirectory = "";
	
	/// resources
	string configDirectory = "/config/";
	string texturesDirectory = "/textures/";
	string soundsDirectory = "/sounds/";
	string fontsDirectory = "/fonts/";
	string scriptsDirectory = "/scripts/";
	string defaultFontName = "default";
	
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

// running
	
	/// time scaling factor
	float timeScale = 1.0f;
	
	/// time since game loop started
	float time = 0;
	
	/// unscaled time since game loop started
	float unscaledTime = 0;
	
	/// time passed since last frame
	float deltaTime;
	
	/// unscaled time passed since last frame
	float unscaledDeltaTime;
	
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
