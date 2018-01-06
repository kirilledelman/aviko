#include <sys/param.h>
#include "Application.hpp"

// singleton application and scripting instances
ScriptHost script;
Application app;

// application entry point
int main( int argc, char* args[] ) {
	
	// if given an argument, use it as a path to base folder
	if ( argc >= 2 && strlen( args[ 1 ] ) > 0 ) {
		char resolved[MAXPATHLEN];
		// if starts with /, it's absolute path
		if ( args[ 1 ][ 0 ] == '/' ) {
			realpath( args[ 1 ], resolved );
		// otherwise, relative to current directory
		} else {
			string temp = app.currentDirectory + "/" + args[ 1 ];
			realpath( temp.c_str(), resolved );
		}
		app.currentDirectory = resolved;
	}
	
    // run game
	app.GameLoop();
	
    // done
    return 0;
	
}
