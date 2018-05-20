#include "ScriptResource.hpp"
#include "ScriptHost.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

ScriptResource::ScriptResource( const char* originalKey, string& path, string& ext ) {
	// read file
	size_t fsize = 0;
	const char* buf = ReadFile( path.c_str(), &fsize );
	if ( !buf ) {
		this->error = ERROR_NOT_FOUND;
		return;
	}
	
	// save source
	this->source = buf;
	
	// compile
	this->compiledScript = JS_CompileScript( script.js, script.global_object, buf, fsize, path.c_str(), 0 );
	if ( this->compiledScript && !JS_IsExceptionPending( script.js ) ) {
		// protect
		JS_AddNamedScriptRoot( script.js, &this->compiledScript, originalKey );
	} else {
		this->error = ERROR_COMPILE;
		printf( "Compilation error in %s, check syntax.\n", path.c_str() );
		JS_ReportPendingException( script.js );		
	}
	free( (void*) buf );	
}

string ScriptResource::ResolveKey(const char *ckey, string &fullpath, string &extension ) {
	
	extension = "js";
	fullpath = ResolvePath( ckey, "js", app.scriptsDirectory.c_str() );
	return fullpath.substr( app.currentDirectory.length() );
	
}

ScriptResource::~ScriptResource() {

	//printf( "Unloading script %s\n", this->key.c_str() );

	// clean up
	if ( this->compiledScript && script.js ) {
		JS_RemoveScriptRoot( script.js, &this->compiledScript );
	}
}
