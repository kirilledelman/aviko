// system includes and STL
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <math.h>
#include <functional>
using namespace std;


// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>

// SDL additional libraries
#ifdef __MACOSX__
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <SDL2_net/SDL_net.h>
#else
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#endif

// SDL gpu
#include "SDL_gpu.h"

// physics library
#include <Box2D/Box2D.h>
#define BOX2D_VELOCITY_ITERATIONS 5
#define BOX2D_POSITION_ITERATIONS 5
#define RAD_TO_DEG (180.0f / 3.1415f)
#define DEG_TO_RAD (3.1415f / 180.0f)
#define RAND() (((float) rand() / (RAND_MAX)) + 1)

// Spidermonkey Javascript
#include <jsapi.h>
using namespace JS;

// global funcs (defined at the end of Application.cpp)
size_t HashString( const char* p );
float* MatrixCompose( float* mat, float x, float y, float angleDegrees, float scaleX, float scaleY );
const char* ReadFile( const char* filepath, const char* ext, const char* requiredSubPath, const char* optionalSubPath, string* finalPath, size_t *fileSize );
bool SaveFile( const char* data, size_t numBytes, const char* filepath, const char* ext, const char* subPath );
bool TryFileExtensions( const char* filePath, const char* commaSeparatedExtensions, string &outExtension );
string HexStr( Uint32 w, size_t hex_len );

/// built-in event types

// GameObject events
#define EVENT_UPDATE "update"
#define EVENT_LATE_UPDATE "lateUpdate"
#define EVENT_ADDED "added"
#define EVENT_REMOVED "removed"
#define EVENT_ADDED_TO_SCENE "addedToScene"
#define EVENT_REMOVED_FROM_SCENE "removedFromScene"
#define EVENT_ACTIVE_CHANGED "activeChanged"
#define EVENT_ATTACHED "attached"
#define EVENT_DETACHED "detached"
#define EVENT_RENDER "render"

// Input events
#define EVENT_KEYDOWN "keyDown"
#define EVENT_KEYUP "keyUp"
#define EVENT_KEYPRESS "keyPress"
#define EVENT_MOUSEDOWN "mouseDown"
#define EVENT_MOUSEUP "mouseUp"
#define EVENT_MOUSEMOVE "mouseMove"
#define EVENT_MOUSEWHEEL "mouseWheel"
#define EVENT_CONTROLLERADDED "controllerAdded"
#define EVENT_CONTROLLERREMOVED "controllerRemoved"
#define EVENT_JOYDOWN "joyDown"
#define EVENT_JOYUP "joyUp"
#define EVENT_JOYAXIS "joyAxis"
#define EVENT_JOYHAT "joyHat"

// additional UIBehavior events
#define EVENT_MOUSEOVER "mouseOver"
#define EVENT_MOUSEOUT "mouseOut"
#define EVENT_CLICK "click"
#define EVENT_MOUSEUPOUTSIDE "mouseUpOutside"
#define EVENT_FOCUSCHANGED "focusChanged"
#define EVENT_NAVIGATION "navigation"

// Input extra key defines
#define KEY_MOUSE_BUTTON (SDL_NUM_SCANCODES + 1)
#define KEY_JOY_BUTTON (SDL_NUM_SCANCODES + 2)
#define KEY_JOY_HAT_X (SDL_NUM_SCANCODES + 3)
#define KEY_JOY_HAT_Y (SDL_NUM_SCANCODES + 4)
#define KEY_JOY_AXIS (SDL_NUM_SCANCODES + 5)
#define KEY_MOUSE_WHEEL (SDL_NUM_SCANCODES + 6)
#define KEY_MOUSE_WHEEL_X (SDL_NUM_SCANCODES + 7)

// Sound events
#define EVENT_FINISHED "finished"

// global forward declarations
class ScriptArguments;
class ScriptHost;
class Application;

// global variables
extern ScriptHost script;
extern Application app;
