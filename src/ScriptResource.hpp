#ifndef ScriptResource_hpp
#define ScriptResource_hpp

#include "ResourceManager.hpp"

class ScriptResource : public Resource {
public:
	
	JSScript* compiledScript = NULL;
	
	static string ResolveKey( const char* ckey, string& fullpath, string& extension );
	
	// init, destroy
	ScriptResource( const char* originalKey, string& path, string& ext );
	~ScriptResource();
	
};

#endif /* ScriptResource_hpp */
