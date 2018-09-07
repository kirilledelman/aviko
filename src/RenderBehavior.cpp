#include "RenderBehavior.hpp"
#include "GameObject.hpp"
#include "RigidBodyShape.hpp"

// static
RenderBehavior::ShaderVariant RenderBehavior::shaders[ SHADER_MAXVAL ];


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

/// no 'new RenderBehavior()'
RenderBehavior::RenderBehavior( ScriptArguments* ) { script.ReportError( "RenderBehavior can't be created using 'new'." ); }

/// default constructor
RenderBehavior::RenderBehavior() { };

/// called by child classes to add default objects
void RenderBehavior::AddDefaults() {
	
	// create color object
	Color *color = new Color( NULL );
	script.SetProperty( "color", ArgValue( color->scriptObject ), this->scriptObject );
	
	// create addColor object
	color = new Color( NULL );
	color->SetInts( 0, 0, 0, 0 );
	script.SetProperty( "addColor", ArgValue( color->scriptObject ), this->scriptObject );
		
}

RenderBehavior::~RenderBehavior() {
	
}


/* MARK:	-				UI
 -------------------------------------------------------------------- */


/// returns true if screen space point x, y is inside this RenderBehavior. By default, just checks GetBounds rectangle
bool RenderBehavior::IsScreenPointInside( float x, float y, float* outLocalX, float* outLocalY ) {
	
	// transform global point to local space
	this->gameObject->ConvertPoint( x, y, *outLocalX, *outLocalY, false );
	
	// in bounds?
	GPU_Rect bounds = this->GetBounds();
	return ( *outLocalX >= bounds.x && *outLocalX < ( bounds.x + bounds.w ) ) &&
	( *outLocalY >= bounds.y && *outLocalY < ( bounds.y + bounds.h ) );

}


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void RenderBehavior::InitClass() {

	// register class
	script.RegisterClass<RenderBehavior>( "Behavior", true );

	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "BlendMode", constants );
	script.SetProperty( "Normal", ArgValue( RenderBehavior::BlendMode::Normal ), constants );
	script.SetProperty( "Add", ArgValue( RenderBehavior::BlendMode::Add ), constants );
	script.SetProperty( "Subtract", ArgValue( RenderBehavior::BlendMode::Subtract ), constants );
	script.SetProperty( "Multiply", ArgValue( RenderBehavior::BlendMode::Multiply ), constants );
	script.SetProperty( "Screen", ArgValue( RenderBehavior::BlendMode::Screen ), constants );
	script.SetProperty( "Burn", ArgValue( RenderBehavior::BlendMode::Burn ), constants );
	script.SetProperty( "Dodge", ArgValue( RenderBehavior::BlendMode::Dodge ), constants );
	script.SetProperty( "Invert", ArgValue( RenderBehavior::BlendMode::Invert ), constants );
	script.SetProperty( "Hue", ArgValue( RenderBehavior::BlendMode::Hue ), constants );
	script.SetProperty( "Color", ArgValue( RenderBehavior::BlendMode::Colorize ), constants );
	script.SetProperty( "Luminosity", ArgValue( RenderBehavior::BlendMode::Luminosity ), constants );
	script.SetProperty( "Saturation", ArgValue( RenderBehavior::BlendMode::Saturation ), constants );
	script.SetProperty( "Refract", ArgValue( RenderBehavior::BlendMode::Refract ), constants );
	script.SetProperty( "Cut", ArgValue( RenderBehavior::BlendMode::Cut ), constants );
	script.FreezeObject( constants );
	
	// properties
	script.AddProperty<RenderBehavior>
	( "color",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderBehavior*) b)->color->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderBehavior* rs = (RenderBehavior*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->color = other;
		} else {
			rs->color->Set( val );
		}
		return rs->color->scriptObject;
	}) );
	
	script.AddProperty<RenderBehavior>
	( "addColor",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((RenderBehavior*) b)->addColor->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		RenderBehavior* rs = (RenderBehavior*) b;
		if ( val.type == TypeObject ) {
			// replace if it's a color
			Color* other = script.GetInstance<Color>( val.value.objectValue );
			if ( other ) rs->addColor = other;
		} else {
			rs->addColor->Set( val );
		}
		return rs->addColor->scriptObject;
	}) );
	
	script.AddProperty<RenderBehavior>
	( "blendMode",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ((RenderBehavior*) b)->blendMode; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderBehavior*) b)->blendMode = (RenderBehavior::BlendMode) val ); }) );
	
	script.AddProperty<RenderBehavior>
	( "stipple",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->stipple; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->stipple = max( 0.0f, min( 1.0f, val ))); }) );
	
	script.AddProperty<RenderBehavior>
	( "stippleAlpha",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderBehavior*) b)->stippleAlpha; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderBehavior*) b)->stippleAlpha = val ); }) );
	
	// clear shaders structure
	memset( &shaders, 0, sizeof( shaders ) );

}


/* MARK:	-				Padding
 -------------------------------------------------------------------- */


void RenderBehavior::UpdateTexturePad() {
	texturePad = outlineColor->rgba.a ? 2 * ( fabs( outlineRadius ) + fmax( fabs( outlineOffsetX ), fabs( outlineOffsetY ) ) ) : 0;
}


/* MARK:	-				Shader methods
 -------------------------------------------------------------------- */


void RenderBehavior::_UpdateBlendTarget( GPU_Target *targ, GPU_Target **blendTarg ) {
	
	if ( targ && targ->image && blendTarg ) {
		
		// create new if none avail
		if ( !*blendTarg ) {
			GPU_Image* img = GPU_CreateImage( targ->base_w, targ->base_h, targ->image->format );
			if ( img ) {
				GPU_UnsetImageVirtualResolution( img );
				GPU_SetImageFilter( img, GPU_FILTER_NEAREST );
				GPU_SetSnapMode( img, GPU_SNAP_NONE );
				GPU_SetAnchor( img, 0, 0 );
				GPU_LoadTarget( img );
				if ( img->target ) {
					*blendTarg = img->target;
				} else {
					GPU_FreeImage( img );
				}
			}
		}
		// if have target
		if ( *blendTarg ) {
			// push view matrices
			GPU_MatrixMode( GPU_PROJECTION );
			GPU_PushMatrix();
			GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
			GPU_MatrixMode( GPU_MODELVIEW );
			GPU_PushMatrix();
			GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
			// draw target
			GPU_Rect srcRect = { 0, 0, (float) targ->base_w, (float) targ->base_h };
			GPU_Blit( targ->image, &srcRect, *blendTarg, 0, 0 );
			// pop
			GPU_MatrixMode( GPU_MODELVIEW );
			GPU_PopMatrix();
			GPU_MatrixMode( GPU_PROJECTION );
			GPU_PopMatrix();
		}
		
	}
}

size_t RenderBehavior::SelectTexturedShader(
		float tw, float th,
		float u, float v, float w, float h,
		float st, float sr, float sb, float sl,
		float sx, float sy,
		float tx, float ty,
		GPU_Image *image, GPU_Target* targ, GPU_Target** blendTarg ){

	size_t shaderIndex = SHADER_TEXTURE;
	
	if ( tx != 1 || ty != 1 ) shaderIndex |= SHADER_TILE;
	if ( this->stipple != 0 || this->stippleAlpha ) shaderIndex |= SHADER_STIPPLE;
	if ( st != 0 || sr != 0 || sl != 0 || sb != 0 ) shaderIndex |= SHADER_SLICE;
	if ( this->blendMode != BlendMode::Normal && this->blendMode != BlendMode::Cut ) {
		shaderIndex |= SHADER_BLEND;
		this->_UpdateBlendTarget( targ, blendTarg );
	}
	if ( this->outlineRadius != 0 || this->outlineOffsetX != 0 || this->outlineOffsetY != 0 ) shaderIndex |= SHADER_OUTLINE;
	
	ShaderVariant &variant = shaders[ shaderIndex ];
	if ( !variant.shader ) variant = CompileShaderWithFeatures( shaderIndex );
	
	//
	
	// activate shader
	GPU_ActivateShaderProgram( variant.shader, &variant.shaderBlock );
	
	// set params
	float params[ 4 ];
	
	// addColor
	
	if ( variant.addColorUniform >= 0 ) {
		params[ 0 ] = this->addColor->r;
		params[ 1 ] = this->addColor->g;
		params[ 2 ] = this->addColor->b;
		params[ 3 ] = this->addColor->a;
		GPU_SetUniformfv( variant.addColorUniform, 4, 1, params );
	}
	
	// stipple
	if ( variant.stippleUniform >= 0 || variant.stippleAlphaUniform >= 0 ) {
		GPU_SetUniformi( variant.stippleUniform, 10000 * ( 1.0 - this->stipple ) );
		GPU_SetUniformi( variant.stippleAlphaUniform, this->stippleAlpha ? 1 : 0 );
	}
	
	// tile
	if ( variant.tileUniform >= 0 ) {
		params[ 1 ] = ty;
		params[ 0 ] = tx;
		GPU_SetUniformfv( variant.tileUniform, 2, 1, params );
	}
	
	// pad
	if ( variant.texPadUniform >= 0 ) {
		GPU_SetUniformf( variant.texPadUniform, (float) this->texturePad );
	}
	
	// texture size
	if ( variant.texSizeUniform >= 0 ) {
		params[ 0 ] = tw;
		params[ 1 ] = th;
		GPU_SetUniformfv( variant.texSizeUniform, 2, 1, params );
	}
	
	// sprite on texture
	if ( variant.texInfoUniform >= 0 ) {
		params[ 0 ] = u;
		params[ 1 ] = v;
		params[ 2 ] = w;
		params[ 3 ] = h;
		GPU_SetUniformfv( variant.texInfoUniform, 4, 1, params );
	}
	
	// slicing
	if ( variant.sliceUniform >= 0 ) {
		params[ 0 ] = st;
		params[ 1 ] = sr;
		params[ 2 ] = sb;
		params[ 3 ] = sl;
		GPU_SetUniformfv( variant.sliceUniform, 4, 1, params );
	}
	
	// slicing: sprite scaling
	if ( variant.sliceScaleUniform >= 0 ) {
		params[ 0 ] = sx;
		params[ 1 ] = sy;
		GPU_SetUniformfv( variant.sliceScaleUniform, 2, 1, params );
	}
	
	// blend mode
	if ( variant.blendUniform >= 0 ) {
		GPU_SetUniformi( variant.blendUniform, this->blendMode );
	}

	// background
	if ( variant.backgroundUniform >= 0 && blendTarg && *blendTarg ) {
		GPU_SetShaderImage( (*blendTarg)->image, variant.backgroundUniform, 1 );
	}
	
	// background size
	if ( variant.backgroundSizeUniform >= 0 ) {
		params[ 0 ] = targ->base_w;
		params[ 1 ] = targ->base_h;
		GPU_SetUniformfv( variant.backgroundSizeUniform, 2, 1, params );
	}
	
	// outline color
	if ( variant.outlineColorUniform >= 0 ) {
		params[ 0 ] = this->outlineColor->r;
		params[ 1 ] = this->outlineColor->g;
		params[ 2 ] = this->outlineColor->b;
		params[ 3 ] = this->outlineColor->a;
		GPU_SetUniformfv( variant.outlineColorUniform, 4, 1, params );
	}
	
	// outline params
	if ( variant.outlineOffsetRadiusUniform >= 0 ) {
		params[ 0 ] = this->outlineOffsetX;
		params[ 1 ] = this->outlineOffsetY;
		params[ 2 ] = this->outlineRadius;
		GPU_SetUniformfv( variant.outlineOffsetRadiusUniform, 3, 1, params );
	}
	
	return shaderIndex;
	
}

size_t RenderBehavior::SelectUntexturedShader( GPU_Target* targ, GPU_Target** blendTarg ) {
	
	size_t shaderIndex = 0;
	
	if ( this->stipple != 0 || this->stippleAlpha ) shaderIndex |= SHADER_STIPPLE;
	if ( this->blendMode != BlendMode::Normal && this->blendMode != BlendMode::Cut ) {
		shaderIndex |= SHADER_BLEND;
		this->_UpdateBlendTarget( targ, blendTarg );
	}
	
	ShaderVariant &variant = shaders[ shaderIndex ];
	if ( !variant.shader ) variant = CompileShaderWithFeatures( shaderIndex );
	
	// activate shader
	GPU_ActivateShaderProgram( variant.shader, &variant.shaderBlock );
	
	// set params
	float params[ 4 ];
	
	// addColor
	if ( variant.addColorUniform >= 0 ) {
		params[ 0 ] = this->addColor->r;
		params[ 1 ] = this->addColor->g;
		params[ 2 ] = this->addColor->b;
		params[ 3 ] = this->addColor->a;
		GPU_SetUniformfv( variant.addColorUniform, 4, 1, params );
	}
	
	// stipple
	if ( variant.stippleUniform >= 0 || variant.stippleAlphaUniform >= 0 ) {
		GPU_SetUniformi( variant.stippleUniform, 10000 * ( 1.0 - this->stipple ) );
		GPU_SetUniformi( variant.stippleAlphaUniform, this->stippleAlpha ? 1 : 0 );
	}
	
	// blend mode
	if ( variant.blendUniform >= 0 ) {
		GPU_SetUniformi( variant.blendUniform, this->blendMode );
	}
	
	// background
	if ( variant.backgroundUniform >= 0 && blendTarg && *blendTarg ) {
		GPU_SetShaderImage( (*blendTarg)->image, variant.backgroundUniform, 1 );
	}
	
	// background size
	if ( variant.backgroundSizeUniform >= 0 ) {
		params[ 0 ] = targ->base_w;
		params[ 1 ] = targ->base_h;
		GPU_SetUniformfv( variant.backgroundSizeUniform, 2, 1, params );
	}
 
	return shaderIndex;
	
}

// compiles shader with features
RenderBehavior::ShaderVariant& RenderBehavior::CompileShaderWithFeatures( size_t featuresMask ) {
	
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();
	bool glsles = ( renderer->shader_language == GPU_LANGUAGE_GLSLES );
	
	// assemble fragment shader source
	char fragShader[ 32768 ];
	char vertShader[ 8192 ];
	
	string params;
	string features;
	string funcs;
	string vertParams;
	string vertFeatures;
	
	// has texture
	if ( featuresMask & SHADER_TEXTURE ) {
		params +=
		"uniform sampler2D tex;\n\
		uniform vec2 texSize;\n\
		uniform vec4 texInfo;\n\
		uniform float texPad;";
		funcs =
		"vec4 readPixel( vec2 uv ){\n\
			uv = uv - texInfo.xy;\n\
			if ( texPad > 0.0 ){\n\
				float texPad2 = 2.0 * texPad;\n\
				uv.x = uv.x * ( ( texInfo.z + texPad2 ) / texInfo.z ) - texPad;\n\
				uv.y = uv.y * ( ( texInfo.w + texPad2 ) / texInfo.w ) - texPad;\n\
			}\n\
			if ( uv.x < 0.0 || uv.x >= texInfo.z || uv.y < 0.0 || uv.y >= texInfo.w ) return vec4(0.0,0.0,0.0,0.0);\n";
		
		// sliced
		if ( featuresMask & SHADER_SLICE ) {
			params +=
			"uniform vec4 slice;\n\
			uniform vec2 sliceScale;";
			funcs +=
			"vec2 suv = uv * sliceScale;\n\
			vec2 actSize = texInfo.zw * sliceScale;\n\
			vec2 sliceRightBottom = vec2( texInfo.z - slice.y, texInfo.w - slice.z );\n\
			vec2 actRightBottom = vec2( actSize.x - slice.y, actSize.y - slice.z );\n\
			vec2 sliceMid = vec2 ( sliceRightBottom.x - slice.w, sliceRightBottom.y - slice.x );\n\
			vec2 actMid = vec2 ( actRightBottom.x - slice.w, actRightBottom.y - slice.x );\n";
			
			// with tiling
			if ( featuresMask & SHADER_TILE ) {
				params +=
				"uniform vec2 tile;";
				funcs +=
				"if ( suv.x < slice.w ) uv.x = suv.x;\n\
				else if ( suv.x >= actRightBottom.x ) uv.x = suv.x - actRightBottom.x + sliceRightBottom.x;\n\
				else uv.x = slice.w + mod( tile.x * sliceMid.x * ( suv.x - slice.w ) / actMid.x, sliceMid.x );\n\
				if ( suv.y < slice.x ) uv.y = suv.y;\n\
				else if ( suv.y >= actRightBottom.y ) uv.y = suv.y - actRightBottom.y + sliceRightBottom.y;\n\
				else uv.y = slice.x + mod( tile.y * sliceMid.y * ( suv.y - slice.x ) / actMid.y, sliceMid.y );\n";
			// sliced without tiling
			} else {
				funcs +=
				"if ( suv.x < slice.w ) uv.x = suv.x;\n\
				else if ( suv.x >= actRightBottom.x ) uv.x = suv.x - actRightBottom.x + sliceRightBottom.x;\n\
				else uv.x = slice.w + sliceMid.x * ( suv.x - slice.w ) / actMid.x;\n\
				if ( suv.y < slice.x ) uv.y = suv.y;\n\
				else if ( suv.y >= actRightBottom.y ) uv.y = suv.y - actRightBottom.y + sliceRightBottom.y;\n\
				else uv.y = slice.x + sliceMid.y * ( suv.y - slice.x ) / actMid.y;\n";
			}
			
		// not sliced, but tiled
		} else if ( featuresMask & SHADER_TILE ) {
			params +=
			"uniform vec2 tile;";
			funcs +=
			"if ( tile.x != 1.0 ) uv.x = mod( uv.x * tile.x, texInfo.z );\n\
			if ( tile.y != 1.0 ) uv.y = mod( uv.y * tile.y, texInfo.w );\n";
		}
		
		// finish read pixel func
		funcs += glsles ?
		"return texture2D( tex, ( uv + texInfo.xy ) / texSize );\n}\n" :
		"return texture( tex, ( uv + texInfo.xy ) / texSize );\n}\n";
		
		// read pixel color
		features +=
		( featuresMask & SHADER_OUTLINE ) ?
		"src = readPixel( coord * texSize );\n" :
		"src = readPixel( coord * texSize ) * color + addColor;\n";
	} else {
		features +=
		"src = color + addColor;";
	}
	
	// effects
	if ( featuresMask & SHADER_OUTLINE ) {
		params +=
		"uniform vec4 outlineColor;\n\
		uniform vec3 outlineOffsetRadius;\n";
		
		funcs +=
		"vec4 outlineSample( vec2 coord ){\n\
			vec4 result = readPixel( coord ); \n\
			float radius = abs( outlineOffsetRadius.z );\n\
			float numRings = radius > 1.0 ? 2.0 : 1.0;\n\
			vec4 r1 = vec4( 0.0, 0.0, 0.0, 0.0 ),\n\
				 r2 = vec4( 0.0, 0.0, 0.0, 0.0 );\n\
			float ringStep = radius / numRings; \n\
			if ( numRings >= 1.0 ) { \n\
				result = max( result, readPixel( coord + vec2( 0.0, -ringStep ) ) );\n\
				result = max( result, readPixel( coord + vec2( ringStep, 0.0 ) ) );\n\
				result = max( result, readPixel( coord + vec2( 0.0, ringStep ) ) );\n\
				result = max( result, readPixel( coord + vec2( -ringStep, 0.0 ) ) );\n\
			}\n\
			if ( numRings >= 2.0 ) { \n\
				float ringRadius = radius * 0.35; \n\
				result = max( result, readPixel( coord + vec2( -ringRadius, -ringRadius ) ) );\n\
				result = max( result, readPixel( coord + vec2( ringRadius, ringRadius ) ) );\n\
				result = max( result, readPixel( coord + vec2( -ringRadius, ringRadius ) ) );\n\
				result = max( result, readPixel( coord + vec2( ringRadius, -ringRadius ) ) );\n\
			}\n\
			return result;\n\
		}";
		features +=
		"vec4 smp = outlineSample( coord * texSize - outlineOffsetRadius.xy );\n\
		src.rgb = mix( outlineColor.rgb, src.rgb * color.rgb, src.a ) + addColor.rgb;\n\
		if ( outlineOffsetRadius.z >= 0.0 ) src.a = max( smp.a * outlineColor.a, src.a ) * color.a;\n\
		else src.a = max( smp.a * outlineColor.a, 0.0 ) * ( 1.0 - src.a ) * color.a;";
	}
	
	// blending with background modes
	if ( featuresMask & SHADER_BLEND ) {
		params +=
		"uniform sampler2D background;\n\
		uniform vec2 backgroundSize;\n\
		uniform int blendMode;";
		funcs +=
		"vec3 rgb2hsv(vec3 c) {\n\
		vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n\
		vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n\
		vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n\
		float d = q.x - min(q.w, q.y);\n\
		float e = 1.0e-10;\n\
		return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n\
		} \n\
		vec3 hsv2rgb(vec3 c) { \n\
		vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0); \n\
		vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www); \n\
		return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y); \n\
		}";
		features +=
		"vec2 fragCoord = gl_FragCoord.xy;\n\
		if ( blendMode == 12 ) fragCoord += 255.0 * vec2( src.r - 0.5, src.b - 0.5 ) * src.b; \n";
		features += glsles ?
		"vec4 bg = texture2D( background, fragCoord / backgroundSize  );\n" :
		"vec4 bg = texture( background, fragCoord / backgroundSize );\n";
		features +=
		"if ( blendMode == 1 ) src.rgb += bg.rgb; // add \n\
		else if ( blendMode == 2 ) src.rgb = bg.rgb - src.rgb; // sub \n\
		else if ( blendMode == 3 ) src.rgb = bg.rgb * src.rgb; // mul \n\
		else if ( blendMode == 4 ) src.rgb = vec3(1.0,1.0,1.0) - (vec3(1.0,1.0,1.0) - bg.rgb) * (vec3(1.0,1.0,1.0) - src.rgb); // screen \n\
		else if ( blendMode == 5 ) src.rgb = vec3(1.0,1.0,1.0) - (vec3(1.0,1.0,1.0) - bg.rgb) / src.rgb; // burn \n\
		else if ( blendMode == 6 ) src.rgb = bg.rgb / (vec3(1.0,1.0,1.0) - src.rgb); // dodge\n\
		else if ( blendMode == 7 ) { // invert \n\
		src.rgb = ( vec3(1.0,1.0,1.0) - bg.rgb );\n\
		} else if ( blendMode == 8 ) { // colorize \n\
		vec3 bgHSV = rgb2hsv( bg.rgb ); \n\
		vec3 srcHSV = rgb2hsv( src.rgb ); \n\
		src.rgb = hsv2rgb( vec3( srcHSV.x, srcHSV.y, bgHSV.z ) );\n\
		} else if ( blendMode == 9 ) { // hue \n\
		vec3 bgHSV = rgb2hsv( bg.rgb ); \n\
		vec3 srcHSV = rgb2hsv( src.rgb ); \n\
		src.rgb = hsv2rgb( vec3( srcHSV.x, bgHSV.y, bgHSV.z ) );\n\
		} else if ( blendMode == 10 ) { // sat \n\
		vec3 bgHSV = rgb2hsv( bg.rgb ); \n\
		vec3 srcHSV = rgb2hsv( src.rgb ); \n\
		src.rgb = hsv2rgb( vec3( bgHSV.x, srcHSV.y, bgHSV.z ) );\n\
		} else if ( blendMode == 11 ) { // lum \n\
		vec3 bgHSV = rgb2hsv( bg.rgb ); \n\
		vec3 srcHSV = rgb2hsv( src.rgb ); \n\
		src.rgb = hsv2rgb( vec3( bgHSV.x, bgHSV.y, srcHSV.z ) );\n\
		} else if ( blendMode == 12 ) src.rgb = bg.rgb; // refract \n\
		\n";
	}
	
	// stippling
	if ( featuresMask & SHADER_STIPPLE ) {
		params +=
		"uniform int stipple;\n\
		uniform int stippleAlpha;\n";
		features +=
		"int index = int(mod(gl_FragCoord.x * 16.0, 64.0) / 16.0) + int(mod(gl_FragCoord.y * 16.0, 64.0) / 16.0) * 4;\n\
		int limit = 0;\n\
		int stippleValue = stipple; \n\
		if ( stippleAlpha > 0 ) stippleValue = 16 * int( src.a * float(stipple) / 15.0 );\n\
		if (index == 0) limit = 625;\n\
		else if (index == 1) limit = 5625;\n\
		else if (index == 2) limit = 1875;\n\
		else if (index == 3) limit = 6875;\n\
		else if (index == 4) limit = 8125;\n\
		else if (index == 5) limit = 3125;\n\
		else if (index == 6) limit = 9375;\n\
		else if (index == 7) limit = 4375;\n\
		else if (index == 8) limit = 2500;\n\
		else if (index == 9) limit = 7500;\n\
		else if (index == 10) limit = 1250;\n\
		else if (index == 11) limit = 6250;\n\
		else if (index == 12) limit = 10000;\n\
		else if (index == 13) limit = 5000;\n\
		else if (index == 14) limit = 8750;\n\
		else if (index >= 15) limit = 3750;\n\
		if ( stippleValue < limit ) discard;\n";
	}
	
	// vertex shader
	
	// put it together
	if ( glsles ) {
		sprintf ( vertShader,
		"#version %d\n\
		precision mediump int;\nprecision mediump float;\n\
		attribute vec2 gpu_Vertex;\n\
		attribute vec2 gpu_TexCoord;\n\
		attribute mediump vec4 gpu_Color;\n\
		uniform mat4 gpu_ModelViewProjectionMatrix;\n\
		varying mediump vec4 color;\n\
		varying vec2 texCoord;\n\
		%s \n\
		void main(void){\n\
			color = gpu_Color;\n\
			texCoord = vec2( gpu_TexCoord.x, gpu_TexCoord.y );\n\
			gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 0.0, 1.0);\n\
			%s \n\
		}", renderer->min_shader_version, vertParams.c_str(), vertFeatures.c_str() );

		sprintf( fragShader,
		"#version %d\n\
		precision mediump int;\nprecision mediump float;\n\
		varying mediump vec4 color;\n\
		varying vec2 texCoord;\n\
		uniform vec4 addColor;\n\
		%s\n\
		%s\n\
		void main(void){\n\
			vec4 src = vec4( 0.0, 0.0, 0.0, 0.0 );\n\
			vec2 coord = texCoord;\n\
			%s\n\
			gl_FragColor = src;\n\
		}", renderer->min_shader_version, params.c_str(), funcs.c_str(), features.c_str() );
	} else {
		sprintf ( vertShader,
		"#version %d\n\
		precision mediump int;\nprecision mediump float;\n\
		in vec2 gpu_Vertex;\n\
		in vec2 gpu_TexCoord;\n\
		in vec4 gpu_Color;\n\
		uniform mat4 gpu_ModelViewProjectionMatrix;\n\
		out vec4 color;\n\
		out vec2 texCoord;\n\
		%s \n\
		void main(void){\n\
			color = gpu_Color;\n\
			texCoord = vec2( gpu_TexCoord.x, gpu_TexCoord.y );\n\
			gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 0.0, 1.0);\n\
			%s \n\
		}", renderer->min_shader_version, vertParams.c_str(), vertFeatures.c_str() );
		
		sprintf( fragShader,
		"#version %d\n\
		in vec4 color;\n\
		in vec2 texCoord;\n\
		in vec4 gl_FragCoord;\n\
		uniform vec4 addColor;\n\
		out vec4 fragColor;\n\
		%s\n\
		%s\n\
		void main(void){\n\
			vec4 src = vec4( 0.0, 0.0, 0.0, 0.0 );\n\
			vec2 coord = texCoord;\n\
			%s\n\
			fragColor = src;\n\
		}", renderer->min_shader_version, params.c_str(), funcs.c_str(), features.c_str() );

	}
		
	// compile variant
	ShaderVariant& variant = shaders[ featuresMask ];
	if ( CompileShader( variant.shader, variant.shaderBlock, vertShader, fragShader ) ) {
		variant.addColorUniform = GPU_GetUniformLocation( variant.shader, "addColor" );
		variant.tileUniform = GPU_GetUniformLocation( variant.shader, "tile" );
		variant.texInfoUniform = GPU_GetUniformLocation( variant.shader, "texInfo" );
		variant.texSizeUniform = GPU_GetUniformLocation( variant.shader, "texSize" );
		variant.texPadUniform = GPU_GetUniformLocation( variant.shader, "texPad" );
		variant.sliceUniform = GPU_GetUniformLocation( variant.shader, "slice" );
		variant.sliceScaleUniform = GPU_GetUniformLocation( variant.shader, "sliceScale" );
		variant.stippleUniform = GPU_GetUniformLocation( variant.shader, "stipple" );
		variant.stippleAlphaUniform = GPU_GetUniformLocation( variant.shader, "stippleAlpha" );
		variant.backgroundUniform = GPU_GetUniformLocation( variant.shader, "background" );
		variant.backgroundSizeUniform = GPU_GetUniformLocation( variant.shader, "backgroundSize" );
		variant.blendUniform = GPU_GetUniformLocation( variant.shader, "blendMode" );
		variant.outlineColorUniform = GPU_GetUniformLocation( variant.shader, "outlineColor" );
		variant.outlineOffsetRadiusUniform = GPU_GetUniformLocation( variant.shader, "outlineOffsetRadius" );
	} else {
		printf ( "Shader error: %s\nin shaders[%zu]:\n%s\n%s\n", GPU_GetShaderMessage(), featuresMask, fragShader, vertShader );
		exit(1);
	}
	
	return variant;
	
}

/// helper method
bool RenderBehavior::CompileShader( Uint32& outShader, GPU_ShaderBlock& outShaderBlock, const char* vertShader, const char* fragShader ){
	
	// vertex shader
	Uint32 v = GPU_CompileShader( GPU_VERTEX_SHADER, vertShader);
	if ( !v ) {
		printf( "Failed to compile shader: %s\n", GPU_GetShaderMessage() );
		return false;
	}
	
	// frag shader
	Uint32 f = GPU_CompileShader( GPU_FRAGMENT_SHADER, fragShader);
	if ( !f ) {
		printf( "Failed to compile shader: %s\n", GPU_GetShaderMessage() );
		return false;
	}
	
	// success
	outShader = GPU_LinkShaders(v, f);
	if ( !outShader ) {
		printf( "Failed to link, fragment shader: %s\n\n", fragShader );
		return false;
	}
	outShaderBlock = GPU_LoadShaderBlock( outShader, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix" );
	return true;
}

