#include "Color.hpp"
#include "Tween.hpp"
#include "Application.hpp"


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
	script.RegisterClass<Color>( "Color" );
	
	// properties
	script.AddProperty<Color>
	( "r",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->r; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.r = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->r = val;
		clr->Notify();
		return val;
	} ));
	
	script.AddProperty<Color>
	( "g",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->g; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.g = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->g = val;
		clr->Notify();
		return val;
	} ));

	script.AddProperty<Color>
	( "b",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->b; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.b = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->b = val;
		clr->Notify();
		return val;
	} ));

	script.AddProperty<Color>
	( "a",
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) { return ((Color*) self)->a; }),
	 static_cast<ScriptFloatCallback>([]( void* self, float val ) {
		Color* clr = (Color*) self;
		clr->rgba.a = min( 255, max( 0, (int) (val * 255.0f) ));
		clr->a = val;
		clr->Notify();
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
		clr->Notify();
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
		clr->Notify();
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
		clr->Notify();
		return val;
	}), PROP_ENUMERABLE );

	script.AddProperty<Color>
	( "hex",
	 static_cast<ScriptStringCallback>([]( void* self, string val ) { return ((Color*) self)->GetHex(); }),
	 static_cast<ScriptStringCallback>([]( void* self, string val ) {
		Color* clr = (Color*) self;
		clr->SetHex( val.c_str() );
		clr->Notify();
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
		 self->Notify();
		 return true;
	 }));
	
	/// change to floats
	script.DefineFunction<Color>
	( "hsv",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: hsv( Float hue, Float saturation, Float value )";
		Color* self = (Color*) o;
		float h = 0, s = 0, v = 0;
		if ( !sa.ReadArguments( 3, TypeFloat, &h, TypeFloat, &s, TypeFloat, &v ) ){
			script.ReportError( error );
			return false;
		}
		
		// apply
		self->SetHSV( h, s, v );
		self->Notify();
		return true;
	}));
	
	script.DefineFunction<Color>
	( "rgbaTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: rgbaTo( Number red, Number green, Number blue, Number alpha, [ Float duration, [ Int easeType, [ Int easeFunc ]]] )";
		float r, g, b, a, dur = 1;
		int etype = (int) Tween::EaseOut, efunc = (int) Tween::EaseSine;
		Color* self = (Color*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 4, TypeFloat, &r, TypeFloat, &g, TypeFloat, &b, TypeFloat, &a, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}

		// stop previous tweens
		Tween::StopTweens( self->scriptObject );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 4 );
		t->properties[ 0 ] = "r";
		t->properties[ 1 ] = "g";
		t->properties[ 2 ] = "b";
		t->properties[ 3 ] = "a";
		t->startValues.resize( 4 );
		t->startValues[ 0 ] = self->r;
		t->startValues[ 1 ] = self->g;
		t->startValues[ 2 ] = self->b;
		t->startValues[ 3 ] = self->a;
		t->endValues.resize( 4 );
		t->endValues[ 0 ] = r;
		t->endValues[ 1 ] = g;
		t->endValues[ 2 ] = b;
		t->endValues[ 3 ] = a;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<Color>
	( "hexTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: hexTo( String rgba, [ Float duration, [ Int easeType, [ Int easeFunc ]]] )";
		float r, g, b, a, dur = 1;
		string hex;
		int etype = (int) Tween::EaseOut, efunc = (int) Tween::EaseSine;
		Color* self = (Color*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeString, &hex, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// convert
		Color::FromHex( hex, r, g, b, a );
		
		// stop previous tweens
		Tween::StopTweens( self->scriptObject );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 4 );
		t->properties[ 0 ] = "r";
		t->properties[ 1 ] = "g";
		t->properties[ 2 ] = "b";
		t->properties[ 3 ] = "a";
		t->startValues.resize( 4 );
		t->startValues[ 0 ] = self->r;
		t->startValues[ 1 ] = self->g;
		t->startValues[ 2 ] = self->b;
		t->startValues[ 3 ] = self->a;
		t->endValues.resize( 4 );
		t->endValues[ 0 ] = r;
		t->endValues[ 1 ] = g;
		t->endValues[ 2 ] = b;
		t->endValues[ 3 ] = a;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<Color>
	( "hsvTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: hsvTo( Number hue, Number saturation, Number value, [ Float duration, [ Int easeType, [ Int easeFunc ]]] )";
		float h, s, v, dur = 1;
		int etype = (int) Tween::EaseOut, efunc = (int) Tween::EaseSine;
		Color* self = (Color*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 3, TypeFloat, &h, TypeFloat, &s, TypeFloat, &v, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop previous tweens
		Tween::StopTweens( self->scriptObject );
		self->_hsvDirty = true;
		self->UpdateHSV();
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 3 );
		t->properties[ 0 ] = "h";
		t->properties[ 1 ] = "s";
		t->properties[ 2 ] = "v";
		t->startValues.resize( 3 );
		t->startValues[ 0 ] = self->h;
		t->startValues[ 1 ] = self->s;
		t->startValues[ 2 ] = self->v;
		t->endValues.resize( 3 );
		t->endValues[ 0 ] = h;
		t->endValues[ 1 ] = s;
		t->endValues[ 2 ] = v;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<Color>
	( "stopMotion",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		Color* self = (Color*) go;
		Tween::StopTweens( self->scriptObject );
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

void Color::SetInt( int val, bool withAlpha ) {
	
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
		// this->rgba.a = 255;
	}
	
	// update
	this->SetInts( this->rgba.r, this->rgba.g, this->rgba.b, this->rgba.a );
	
}

void Color::FromInt( int val, float &r, float &g, float &b) {
	float m = 1;
	if ( val < 0 ) { m = -1; val = -val; }
	b = m * ( val & 0xFF ) / 255.0f;
	g = m * ( ( val >> 8 ) & 0xFF ) / 255.0f;
	r = m * ( ( val >> 16 ) & 0xFF ) / 255.0f;
}

void Color::FromHex( string& hex, float &r, float &g, float &b, float& a ) {
	unsigned long val = 0;
	size_t len = hex.length();
	char* ptr = NULL;
	
	// strip # or 0x
	if ( len >= 1 && hex[ 0 ] == '#' ) {
		if ( len == 1 ) hex = "";
		else hex = hex.substr( 1 );
	}
	else if ( hex.length() >= 2 && hex[ 0 ] == '0' && ( hex[ 1 ] == 'x' || hex[ 1 ] == 'X' ) ) {
		if ( len == 2 ) hex = "";
		else hex = hex.substr( 3 );
	}
	
	// convert from hex
	len = hex.length();
	val = SDL_strtoul( hex.c_str(), &ptr, 16 );
	
	// short - 2 characters - solid color, same vals for all 3
	if ( len <= 2 ) {
		r = g = b = ( val & 0xFF ) / 255.0;
		a = 1;
	// rrggbb
	} else if ( len == 6 ) {
		b = ( val & 0xFF ) / 255.0;
		g = ( ( val >> 8 ) & 0xFF ) / 255.0;
		r = ( ( val >> 16 ) & 0xFF ) / 255.0;
		// rrggbbaa
	} else if ( len >= 8 ) {
		a = ( val & 0xFF ) / 255.0;
		b = ( ( val >> 8 ) & 0xFF ) / 255.0;
		g = ( ( val >> 16 ) & 0xFF ) / 255.0;
		r = ( ( val >> 24 ) & 0xFF ) / 255.0;
	}

}

bool Color::Set( ScriptArguments &sa ) {

	this->_hsvDirty = true;
	
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
				return true;
			} else return false;
			
		// parse string
		} else if ( sa.args[ 0 ].type == TypeString ) {
			
			return this->SetHex( sa.args[ 0 ].value.stringValue->c_str() );
			
		} else if ( sa.args[ 0 ].type == TypeInt || sa.args[ 0 ].type == TypeFloat ) {
			
			int v = 0;
			sa.args[ 0 ].toInt( v );
			this->SetInt( v, false );
			
		} else return false;
		
	} else if ( sa.args.size() >= 3 ) {
		
		float _r = 0, _g = 0, _b = 0, _a = 1;
		sa.args[ 0 ].toNumber( _r );
		sa.args[ 1 ].toNumber( _g );
		sa.args[ 2 ].toNumber( _b );
		if ( sa.args.size() >= 4 ) sa.args[ 3 ].toNumber( _a );
		this->SetFloats( _r, _g, _b, _a );
		
	} else return false;
	
	return true;
	
}

void Color::Set( ArgValue &val ) {
	
	this->_hsvDirty = true;
	
	// copy from another color
	if ( val.type == TypeObject ) {
		
		Color* other = script.GetInstance<Color>( val.value.objectValue );
		if ( other ) {
			this->rgba = other->rgba;
			this->r = other->r;
			this->g = other->g;
			this->b = other->b;
			this->a = other->a;
		}
		
	// parse string
	} else if ( val.type == TypeString ) {
		
		this->SetHex( val.value.stringValue->c_str() );
		
	} else if ( val.type == TypeInt || val.type == TypeFloat ) {
	
		int v = 0;
		val.toInt( v );
		if ( v < 0 ) {
			Color::FromInt( v, this->r, this->g, this->b );
			this->rgba = { 0, 0, 0, 0 };
		} else {
			this->SetInt( v, false );
		}
	} else if ( val.type == TypeArray ) {
		
		if ( val.value.arrayValue->size() >= 1 ) val.value.arrayValue->at( 0 ).toNumber( this->r );
		if ( val.value.arrayValue->size() >= 2 ) val.value.arrayValue->at( 1 ).toNumber( this->g );
		if ( val.value.arrayValue->size() >= 3 ) val.value.arrayValue->at( 2 ).toNumber( this->b );
		if ( val.value.arrayValue->size() >= 4 ) val.value.arrayValue->at( 3 ).toNumber( this->a );
		this->rgba.r = (Uint8) max( 0, min( 255, (int) ( this->r * 255.0f) ));
		this->rgba.g = (Uint8) max( 0, min( 255, (int) ( this->g * 255.0f) ));
		this->rgba.b = (Uint8) max( 0, min( 255, (int) ( this->b * 255.0f) ));
		this->rgba.a = (Uint8) max( 0, min( 255, (int) ( this->a * 255.0f) ));
		
	}
	
}

bool Color::SetHex( const char* s ) {
	
	string hex( s );
	Color::FromHex( hex, this->r, this->g, this->b, this->a );
	
	// update int part
	this->SetFloats( this->r, this->g, this->b, this->a );
	
	// all good
	return true;

}

string Color::GetHex() {
	char str[ 16 ];
	unsigned int val =
		( this->rgba.a & 0xFF ) +
		(( this->rgba.b & 0xFF ) << 8 ) +
		(( this->rgba.g & 0xFF ) << 16 ) +
		(( this->rgba.r & 0xFF ) << 24 );
	string s = "00000000" + string( SDL_uitoa( val, str, 16 ) );
	return s.substr( s.length() - 8 );
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
	this->_hsvDirty = true;
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
	this->_hsvDirty = true;
	
}


/* MARK:	-				Notification
 -------------------------------------------------------------------- */


void Color::Notify() {
	
	if ( callback ) callback( this );
	
	/*Event e( EVENT_CHANGE );
	ArgValueVector* args = app.AddLateEvent( this, EVENT_CHANGE );
	this->CallEvent( e );*/
	
}












