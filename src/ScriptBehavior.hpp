#ifndef ScriptBehavior_hpp
#define ScriptBehavior_hpp

#include "common.h"
#include "Behavior.hpp"

class ScriptBehavior : public Behavior {
public:
	
	// init, destroy
	ScriptBehavior( ScriptArguments* args );
	ScriptBehavior();
	~ScriptBehavior();
	
	// scripting
	
	/// script resource
	ScriptResource* scriptResource = NULL;
	
	/// registers class for scripting
	static void InitClass();
	
	// events
		
};

#endif /* ScriptBehavior_hpp */
