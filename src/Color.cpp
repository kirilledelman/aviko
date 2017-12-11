#include "Color.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


Color::Color( ScriptArguments* args ) {

	// add scriptObject
	script.NewScriptObject<Color>( this );
	
	// arguments?
	if ( args ) {
		
		// just call set method
		this->Set( *args );
		
	}
	
}

// copy constructor
Color::Color( Color& copyFrom ) {
	// copy from other
	this->rgba = copyFrom.rgba;
}

Color::Color() {}
Color::~Color() {}


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void Color::InitClass() {
	
	// create class
	script.RegisterClass<Color>( "ScriptableObject" );
	
	// properties
	script.AddProperty<Color>
	( "r",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->r; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.r = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->r = val;
		return val;
	} ));
	
	script.AddProperty<Color>
	( "g",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->g; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.g = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->g = val;
		return val;
	} ));

	script.AddProperty<Color>
	( "b",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->b; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.b = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->b = val;
		return val;
	} ));

	script.AddProperty<Color>
	( "a",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->a; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.a = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->a = val;
		return val;
	} ));
	
	script.AddProperty<Color>
	( "h",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		return clr->h;
	 }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		clr->SetHSV( val, clr->s, clr->v );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<Color>
	( "s",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		return clr->s;
	}),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		clr->SetHSV( clr->h, val, clr->v );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<Color>
	( "v",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		return clr->v;
	}),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->UpdateHSV();
		clr->SetHSV( clr->h, clr->s, val );
		return val;
	}), PROP_ENUMERABLE );

	script.AddProperty<Color>
	( "hex",
	 static_cast<ScriptStringCallback>([]( void* self, string val ) { return ((Color*) self)->GetHex( true ); }),
	 static_cast<ScriptStringCallback>([]( void* self, string val ) {
		Color* clr = (Color*) self;
		clr->SetHex( val.c_str() );
		return val;
	} ), PROP_ENUMERABLE );
	
	// functions
	
	/// change to floats
	script.DefineFunction<Color>
	( "set",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		 // validate params
		 const char* error = "usage: set( Float r, Float g, Float b[, Float a ] | String hex | Int rgba | Color copyFrom )";
		 Color* self = (Color*) o;
		
		 // if not a valid call report error
		 if ( !self->Set( sa ) ) {
			 script.ReportError( error );
			 return false;
		 }
		
		 return true;
	 }));
	
	script.DefineFunction<Color>
	( "toString",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		 static char buf[256];
		 Color* clr = (Color*) o;
		 sprintf( buf, "[object Color (%p): %f, %f, %f, %f]", o, clr->r, clr->g, clr->b, clr->a );
		 sa.ReturnString( buf );
		 return true;
	 }));
	
}


/* MARK:	-				Manipulation
 -------------------------------------------------------------------- */

void Color::UpdateHSV() {

	// update
	if ( this->_hsvDirty ) {
		
		if ( r >= 0 && g >= 0 && b >= 0 ) {
			float fCMax = max(max(r, g), b);
			float fCMin = min(min(r, g), b);
			float fDelta = fCMax - fCMin;
			
			if(fDelta > 0) {
				if(fCMax == r) {
					h = 60 * (fmod(((g - b) / fDelta), 6));
				} else if(fCMax == g) {
					h = 60 * (((b - r) / fDelta) + 2);
				} else if(fCMax == b) {
					h = 60 * (((r - g) / fDelta) + 4);
				}
				
				if( fCMax > 0 ) {
					s = fDelta / fCMax;
				} else {
					s = 0;
				}
				
				v = fCMax;
			} else {
				h = 0;
				s = 0;
				v = fCMax;
			}
			
			if(h < 0) h += 360;
			
			h /= 360.0f;
		} else {
			h = s = v = 0;
		}
	}
	
	this->_hsvDirty = false;
}

void Color::SetHSV( float hue, float sat, float val ) {
	
	h = hue = min( 1.0f, max( 0.0f, fmod( hue, 1.0f ) ) );
	s = sat = min( 1.0f, max( 0.0f, sat ) );
	v = val = min( 1.0f, max( 0.0f, val ) );
	
	hue *= 360.0f;
	
	float fC = val * sat;
	float fHPrime = fmod( hue / 60.0, 6);
	float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	float fM = val - fC;
	
	if(0 <= fHPrime && fHPrime < 1) {
		r = fC;
		g = fX;
		b = 0;
	} else if(1 <= fHPrime && fHPrime < 2) {
		r = fX;
		g = fC;
		b = 0;
	} else if(2 <= fHPrime && fHPrime < 3) {
		r = 0;
		g = fC;
		b = fX;
	} else if(3 <= fHPrime && fHPrime < 4) {
		r = 0;
		g = fX;
		b = fC;
	} else if(4 <= fHPrime && fHPrime < 5) {
		r = fX;
		g = 0;
		b = fC;
	} else if(5 <= fHPrime && fHPrime < 6) {
		r = fC;
		g = 0;
		b = fX;
	} else {
		r = 0;
		g = 0;
		b = 0;
	}
	
	r += fM;
	g += fM;
	b += fM;

	// updates RGB
	SetFloats( r, g, b );
	this->_hsvDirty = false;
}

void Color::SetInt( unsigned int val, bool withAlpha ) {
	
	// rrggbb
	if ( withAlpha ) {
		this->rgba.a = ( val & 0xFF );
		this->rgba.b = ( ( val >> 8 ) & 0xFF );
		this->rgba.g = ( ( val >> 16 ) & 0xFF );
		this->rgba.r = ( ( val >> 24 ) & 0xFF );
		// rrggbbaa
	} else {
		this->rgba.b = ( val & 0xFF );
		this->rgba.g = ( ( val >> 8 ) & 0xFF );
		this->rgba.r = ( ( val >> 16 ) & 0xFF );
	}
	
	// update
	this->SetInts( this->rgba.r, this->rgba.g, this->rgba.b, this->rgba.a );
	
}

bool Color::Set( ScriptArguments &sa ) {

	// one argument
	if ( sa.args.size() == 1 ) {
		
		// copy from another color
		if ( sa.args[ 0 ].type == TypeObject ) {
			
			Color* other = script.GetInstance<Color>( sa.args[ 0 ].value.objectValue );
			if ( other ) {
				this->rgba = other->rgba;
				this->r = other->r;
				this->g = other->g;
				this->b = other->b;
				this->a = other->a;
				this->_hsvDirty = true;
				return true;
			} else return false;
			
		// parse string
		} else if ( sa.args[ 0 ].type == TypeString ) {
			
			return this->SetHex( sa.args[ 0 ].value.stringValue->c_str() );
			
		} else if ( sa.args[ 0 ].type == TypeInt ) {
			
			this->SetInt( sa.args[ 0 ].value.intValue, false );
			
		} else if ( sa.args[ 0 ].type == TypeFloat ) {
			
			this->SetInt( static_cast<unsigned int>(sa.args[ 0 ].value.floatValue), true );
			
		} else return false;
		
	} else if ( sa.args.size() >= 3 ) {
		
		int R,G,B,A = this->a * 255;
		// ints
		if ( sa.args[ 0 ].type == TypeInt && sa.ReadArguments( 3, TypeInt, &R, TypeInt, &G, TypeInt, &B, TypeInt, &A ) ) {
			this->SetInts( R, G, B, A );
			
		// floats
		} else if ( sa.args[ 0 ].type == TypeFloat && sa.ReadArguments( 3, TypeFloat, &this->r, TypeFloat, &this->g, TypeFloat, &this->b, TypeFloat, &this->a ) ) {
			this->SetFloats( this->r, this->g, this->b, this->a );
		}
		
	} else return false;
	
	return true;
	
}

bool Color::SetHex( const char* s ) {
	
	string hex( s );
	unsigned long val = 0;
	char* ptr = NULL;
	
	// strip # or 0x
	if ( hex.length() >= 1 && hex[ 0 ] == '#' ) hex = hex.substr( 1 );
	else if ( hex.length() >= 2 && hex[ 0 ] == '0' && ( hex[ 1 ] == 'x' || hex[ 1 ] == 'X' ) ) hex = hex.substr( 3 );
	
	// convert from hex
	val = SDL_strtoul( hex.c_str(), &ptr, 16 );

	// short - 2 characters - solid color, same vals for all 3
	if ( hex.size() <= 2 ) {
		this->rgba.r = this->rgba.g = this->rgba.b = ( val & 0xFF );
		this->rgba.a = 255;
	// rrggbb
	} else if ( hex.size() == 6 ) {
		this->rgba.b = ( val & 0xFF );
		this->rgba.g = ( ( val >> 8 ) & 0xFF );
		this->rgba.r = ( ( val >> 16 ) & 0xFF );
	// rrggbbaa
	} else if ( hex.size() >= 8 ) {
		this->rgba.a = ( val & 0xFF );
		this->rgba.b = ( ( val >> 8 ) & 0xFF );
		this->rgba.g = ( ( val >> 16 ) & 0xFF );
		this->rgba.r = ( ( val >> 24 ) & 0xFF );
	}
	
	// update float part
	this->SetInts( this->rgba.r, this->rgba.g, this->rgba.b, this->rgba.a );
	
	// all good
	return true;

}

int Color::GetInt( bool withAlpha ) {
	if ( withAlpha ) {
		return ( ((Uint32) this->rgba.r) << 24 ) + ( ((Uint32) this->rgba.g) << 16 ) + ( ((Uint32) this->rgba.b) << 8 ) + ((Uint32) this->rgba.a);
	} else {
		return ( ((Uint32) this->rgba.r) << 16 ) + ( ((Uint32) this->rgba.g) << 8 ) + ((Uint32) this->rgba.b);
	}
}

string Color::GetHex( bool withAlpha ) {
	char str[ 16 ];
	unsigned int val =
		( this->rgba.a & 0xFF ) +
		( ( this->rgba.b & 0xFF ) << 8 ) +
		( ( this->rgba.g & 0xFF ) << 16 ) +
		( ( this->rgba.r & 0xFF ) << 24 );
	return string( SDL_uitoa( val, str, 16 ) );
}

void Color::SetInts( int red, int green, int blue, int alpha ) {
	this->rgba.r = max( 0, min( 255, red ));
	this->rgba.g = max( 0, min( 255, green ));
	this->rgba.b = max( 0, min( 255, blue ));
	this->rgba.a = max( 0, min( 255, alpha ));
	this->r = red / 255.0f;
	this->g = green / 255.0f;
	this->b = blue / 255.0f;
	this->a = alpha / 255.0f;
}

void Color::SetFloats( float red, float green, float blue, float alpha ) {
	this->r = red;
	this->g = green;
	this->b = blue;
	this->a = alpha;
	this->rgba.r = (Uint8) max( 0, min( 255, (int) (red * 255.0f) ));
	this->rgba.g = (Uint8) max( 0, min( 255, (int) (green * 255.0f) ));
	this->rgba.b = (Uint8) max( 0, min( 255, (int) (blue * 255.0f) ));
	this->rgba.a = (Uint8) max( 0, min( 255, (int) (alpha * 255.0f) ));
}
