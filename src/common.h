// system includes and STL
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <regex>
#include <unordered_map>
#include <list>
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
#include <SDL2_gpu.framework/Headers/SDL_gpu.h>
// #include <OpenGL/gl.h>
#else
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_net.h>
#include <SDL2/SDL_gpu.h>
// #include <gles.h> // check OPi makefile
#endif

// physics library
#include <Box2D/Box2D.h>
#define BOX2D_VELOCITY_ITERATIONS 5
#define BOX2D_POSITION_ITERATIONS 5
#define WORLD_TO_BOX2D_SCALE 0.1f
#define BOX2D_TO_WORLD_SCALE 10.0f
#define RAD_TO_DEG (180.0f / 3.1415f)
#define DEG_TO_RAD (3.1415f / 180.0f)
#define RAND() (((float) rand() / (RAND_MAX)) + 1)

// Spidermonkey Javascript
#include <jsapi.h>
#include <jsdbgapi.h>
using namespace JS;

// global funcs (defined at the end of Application.cpp)
size_t HashString( const char* p );
float* MatrixCompose( float* mat, float x, float y, float angleDegrees, float scaleX, float scaleY );
void MatrixSkew( float* result, float sx, float sy );
string ResolvePath( const char* filepath, const char* ext, const char* optionalSubPath );
string ResolvePath( const char* filepath, const char* commaSeparatedExtensions, string& extension, const char* optionalSubPath );
const char* ReadFile( const char* filepath, const char* ext, const char* optionalSubPath, string* finalPath, size_t *fileSize );
const char* ReadFile( const char* absoluteFilepath, size_t *fileSize );
bool SaveFile( const char* data, size_t numBytes, const char* filepath, const char* ext, bool* existsOverwrite );
bool DeleteFile( const char* filepath );
bool TryFileExtensions( const char* filePath, const char* commaSeparatedExtensions, string &outExtension );
string HexStr( Uint32 w, size_t hex_len );
int StringPositionToIndex( const char* str, int pos );
int StringIndexToPosition( const char* str, int index );
int StringPositionLength( const char* str );
string base64_encode( unsigned char const*, unsigned int len );
string base64_decode( string const& s );
string GetScriptNameAndLine();
string ExecCommand( const char* cmd );

// built in resources - added using http://www.fourmilab.ch/xd/
extern unsigned char RobotoRegular[];
extern int RobotoRegular_size;

/// built-in event types
#define EVENT_SCENECHANGED "sceneChanged"

// GameObject events
#define EVENT_UPDATE "update"
#define EVENT_ADDED "added"
#define EVENT_REMOVED "removed"
#define EVENT_CHILDADDED "childAdded"
#define EVENT_CHILDREMOVED "childRemoved"
#define EVENT_ADDEDTOSCENE "addedToScene"
#define EVENT_REMOVEDFROMSCENE "removedFromScene"
#define EVENT_ACTIVECHANGED "activeChanged"
#define EVENT_NAMECHANGED "nameChanged"
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

// Physics events
#define EVENT_TOUCH "touch"
#define EVENT_UNTOUCH "untouch"

// Misc events
#define EVENT_FINISHED "finished"
#define EVENT_RESIZED "resized"
#define EVENT_AWAKE "awake"
#define EVENT_LAYOUT "layout"
#define EVENT_ERROR "error"
#define EVENT_LOG "log"
#define EVENT_CHANGE "change"

// additional blending mode
#define GPU_BLEND_CUT_ALPHA 16

// global forward declarations
class ScriptArguments;
class ScriptHost;
class Application;

// global variables
extern ScriptHost script;
extern Application app;
