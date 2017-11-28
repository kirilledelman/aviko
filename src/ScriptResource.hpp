#ifndef ScriptResource_hpp
#define ScriptResource_hpp

#include "ResourceManager.hpp"

class ScriptResource : public Resource {
public:
	
	JSScript* compiledScript = NULL;
	
	// init, destroy
	ScriptResource( const char* ckey );
	~ScriptResource();
	
};

#endif /* ScriptResource_hpp */
