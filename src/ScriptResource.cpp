#include "ScriptResource.hpp"
#include "ScriptHost.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

ScriptResource::ScriptResource( const char* ckey ) {
	
	this->key = ckey;
	
	// read file
	size_t fsize = 0;
	string path;
	const char* buf = strlen( ckey ) ? ReadFile( ckey, "js", app.scriptsDirectory.c_str(), NULL, &path, &fsize ) : NULL;
	if ( !buf ) {
		this->error = ERROR_NOT_FOUND;
		return;
	}
	
	// compile
	this->compiledScript = JS_CompileScript( script.js, script.global_object, buf, fsize, path.c_str(), 0 );
	if ( this->compiledScript ) {
		// protect
		JS_AddNamedScriptRoot( script.js, &this->compiledScript, ckey );
	} else {
		this->error = ERROR_COMPILE;
	}
	free( (void*) buf );	
}

ScriptResource::~ScriptResource() {

	printf( "Unloading script %s\n", this->key.c_str() );

	// clean up
	if ( this->compiledScript && script.js ) {
		JS_RemoveScriptRoot( script.js, &this->compiledScript );
	}
}
