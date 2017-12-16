#ifndef Tween_hpp
#define Tween_hpp

#include "common.h"
#include "ScriptableClass.hpp"

/// class for transitioning properties of objects
class Tween : public ScriptableClass {
public:

	// init
	Tween(){};
	Tween( ScriptArguments* args );
	~Tween();
	
// processing
	
	typedef enum {
		EaseNone,
		EaseIn,
		EaseOut,
		EaseInOut
	} EasingFunc;
	
	// all currently running tweens
	static unordered_set<Tween*> *activeTweens;
	
	/// iterate / increment currently running tweens
	static void ProcessActiveTweens( float dt, float udt );
	
	/// step tween - return true, if should be removed from active
	bool ProcessTween( float dt, float udt );
	
	// props
	
	bool _running = false;
	void running( bool r );
	bool running() { return _running; }

	bool useUnscaledTime = false;
	
	// tween target object
	void* target = NULL;
	
	/// transitioning properties
	vector<string> properties;
	void SetProperties( ArgValue val );
	ArgValue GetProperties();
	
	/// from
	vector<float> startValues;
	void SetStartValues( ArgValue val );
	ArgValue GetStartValues();
	
	/// to values
	vector<float> endValues;
	void SetEndValues( ArgValue val );
	ArgValue GetEndValues();
	
	/// transition duration in seconds
	float duration = 1.0;
	
	/// current time
	float time = 0;
	
	EasingFunc easingFunc = EaseNone;
	
	// script
	
	static void InitClass();
	
};

SCRIPT_CLASS_NAME( Tween, "Tween" );

#endif /* Tween_hpp */
