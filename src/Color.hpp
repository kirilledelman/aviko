#ifndef Color_hpp
#define Color_hpp

#include "common.h"
#include "ScriptableClass.hpp"

/// class for specifying color in script
class Color : public ScriptableClass {
public:
	
	// SDL color ( 0 - 255 )
	SDL_Color rgba = { 255, 255, 255, 255 };
	
	// floats 0-1 allow negative values
	float r = 1;
	float g = 1;
	float b = 1;
	float a = 1;
	
	// hsv
	bool _hsvDirty = true;
	float h = 0;
	float s = 0;
	float v = 0;

	bool Set( ScriptArguments& sa );
	void Set( ArgValue &val );
	
	/// set color as R, G, B, A 0-255 values
	void SetInts( int red, int green, int blue, int alpha=255 );
	
	/// set color as r, g, b, a, 0-1.0 values (allows negative)
	void SetFloats( float red, float green, float blue, float alpha=1.0f );
	
	/// set color as h, s, v
	void SetHSV( float hue, float sat, float val );
	
	/// sets color from integer
	void SetInt( int clr, bool withAlpha );
	
	static void FromInt( int clr, float &r, float &g, float &b );
	static void FromHex( string& clr, float &r, float &g, float &b, float &a );
	
	/// returns hexadecimal RRGGBBAA integer value as string
	string GetHex();
	
	/// returns hexadecimal RRGGBB(AA) integer value as string
	bool SetHex( const char* hex );
	
	/// updates .h.s.v properties from SDL_rgba
	void UpdateHSV();
	
	// scripting
	
	static void InitClass();
	
	
	// init/destroy
	Color();
	Color( ScriptArguments* args );
	Color( Color& copyFrom );
	~Color();
	
};

SCRIPT_CLASS_NAME( Color, "Color" );

#endif /* Color_hpp */
