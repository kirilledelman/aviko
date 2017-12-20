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
	
	enum EasingType {
		EaseNone,
		EaseIn,
		EaseOut,
		EaseInOut
	};
	
	enum EasingFunc {
		EaseLinear,
		EaseSine,
		EaseQuad,
		EaseCubic,
		EaseQuart,
		EaseQuint,
		EaseCirc,
		EaseExpo,
		EaseBack,
		EaseElastic,
		EaseBounce
	};
	
	// all currently running tweens
	static unordered_set<Tween*> *activeTweens;
	
	/// iterate / increment currently running tweens
	static void ProcessActiveTweens( float dt, float udt );
	
	/// step tween - return true, if should be removed from active
	bool ProcessTween( float dt, float udt );
	
	// props
	
	bool _active = false;
	void active( bool r );
	bool active() { return _active; }

	// returns true if tween is viable to run
	bool _canRun();
	
	bool useUnscaledTime = false;
	
	// tween target object
	void* target = NULL;
	
	/// transitioning properties
	vector<string> properties;
	void SetProperties( ArgValue val );
	ArgValue GetProperties();
	
	/// custom update function
	ScriptFunctionObject callback;
	
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
	
	/// easing
	EasingType easeType = EasingType::EaseInOut;
	EasingFunc easeFunc = EasingFunc::EaseSine;
	
	static float Ease( EasingType type, EasingFunc func, float p );
	
	void Reverse();
	void Cut();
	
	// script
	
	static void InitClass();
	
};

SCRIPT_CLASS_NAME( Tween, "Tween" );

#endif /* Tween_hpp */
