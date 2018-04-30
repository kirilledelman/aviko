#include "RenderBehavior.hpp"
#include "GameObject.hpp"

// static
RenderBehavior::SpriteShaderVariant RenderBehavior::shaders[ 32 ];


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
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return ( ((RenderBehavior*) b)->blendMode = (GPU_BlendPresetEnum) val ); }) );
	
	script.AddProperty<RenderBehavior>
	( "stipple",
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->stipple; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->stipple = max( 0.0f, min( 1.0f, val ))); }) );
	
	script.AddProperty<RenderBehavior>
	( "stippleAlpha",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((RenderBehavior*) b)->stippleAlpha; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((RenderBehavior*) b)->stippleAlpha = val ); }) );
	
	script.AddProperty<RenderBehavior>
	( "pivotX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotX = val ); }) );
	
	script.AddProperty<RenderBehavior>
	( "pivotY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->pivotY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->pivotY = val ); }) );

}


/* MARK:	-				Shader methods
 -------------------------------------------------------------------- */

int RenderBehavior::SelectTexturedShader(
		float tw, float th,
		float u, float v, float w, float h,
		float st, float sr, float sb, float sl,
		float sx, float sy,
		float tx, float ty, GPU_Target* targ ){

	int shaderIndex = SHADER_TEXTURE;
	
	if ( tx != 1 || ty != 1 ) shaderIndex |= SHADER_TILE;
	if ( this->stipple != 0 || this->stippleAlpha ) shaderIndex |= SHADER_STIPPLE;
	if ( st != 0 || sr != 0 || sl != 0 || sb != 0 ) shaderIndex |= SHADER_SLICE;
	
	SpriteShaderVariant &variant = shaders[ shaderIndex ];
	
	if ( !variant.shader ) {
		printf( "Shader %d not compiled!\n", shaderIndex );
	}
	
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
		GPU_SetUniformi( variant.texPadUniform, this->texturePad );
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
	
	 // TODO - add render target as texture, set size, blend mode
										 
	return shaderIndex;
	
}

int RenderBehavior::SelectUntexturedShader( GPU_Target* targ ) {
	
	int shaderIndex = 0;
	
	if ( this->stipple != 0 || this->stippleAlpha ) shaderIndex |= SHADER_STIPPLE;
	
	SpriteShaderVariant &variant = shaders[ shaderIndex ];
	
	if ( !variant.shader ) {
		printf( "Shader %d not compiled!\n", shaderIndex );
	}
	
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
	
	// TODO - add render target as texture, set size, blend mode
 
	return shaderIndex;
	
}

// init shaders
void RenderBehavior::InitShaders() {
	
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();
	bool glsles = ( renderer->shader_language == GPU_LANGUAGE_GLSLES );
	
	// vertex shader
	char vertShader[ 4096 ];
	sprintf ( vertShader,
	"#version %d\n\
	%s\n\
	void main(void){\n\
		color = gpu_Color;\n\
        texCoord = vec2( gpu_TexCoord.x, gpu_TexCoord.y );\n\
		gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 0.0, 1.0);\n\
	}",
	renderer->min_shader_version,
    glsles ?
	"precision mediump int;\nprecision mediump float;\n\
	attribute vec2 gpu_Vertex;\n\
	attribute vec2 gpu_TexCoord;\n\
	attribute mediump vec4 gpu_Color;\n\
	uniform mat4 gpu_ModelViewProjectionMatrix;\n\
	varying mediump vec4 color;\n\
	varying vec2 texCoord;\n"
	 :
	"in vec2 gpu_Vertex;\n\
	in vec2 gpu_TexCoord;\n\
	in vec4 gpu_Color;\n\
	uniform mat4 gpu_ModelViewProjectionMatrix;\n\
	out vec4 color;\n\
	out vec2 texCoord;\n");
	
	// create shader combinations w sets of different features
	
	// shader permutations
	string params[ 32 ];
	string features[ 32 ];
	string funcs[ 32 ];
	string post[ 32 ];
	for ( int i = 0; i < 32; i++ ) {
		if ( i & SHADER_TEXTURE ) {
			params[ i ] +=
			"uniform sampler2D tex;\n\
			uniform vec2 texSize;\n\
			uniform vec4 texInfo;\n\
			uniform int texPad;";
			funcs[ i ] =
			"vec4 readPixel( vec2 uv ){\n\
				uv = uv * texSize - texInfo.xy;\n\
				if ( texPad > 0 ){\n\
					float texPad2 = 2.0 * texPad;\n\
					uv.x = uv.x * ( ( texInfo.z + texPad2 ) / texInfo.z ) - texPad;\n\
					uv.y = uv.y * ( ( texInfo.w + texPad2 ) / texInfo.w ) - texPad;\n\
				}\n\
				if ( uv.x < 0.0 || uv.x >= texInfo.z || uv.y < 0.0 || uv.y >= texInfo.w ) return vec4(0.0,0.0,0.0,0.0);\n";
			
			// sliced
			if ( i & SHADER_SLICE ) {
				params[ i ] +=
				"uniform vec4 slice;\n\
				uniform vec2 sliceScale;";
				funcs[ i ] +=
				"vec2 suv = uv * sliceScale;\n\
				vec2 actSize = texInfo.zw * sliceScale;\n\
				vec2 sliceRightBottom = vec2( texInfo.z - slice.y, texInfo.w - slice.z );\n\
				vec2 actRightBottom = vec2( actSize.x - slice.y, actSize.y - slice.z );\n\
				vec2 sliceMid = vec2 ( sliceRightBottom.x - slice.w, sliceRightBottom.y - slice.x );\n\
				vec2 actMid = vec2 ( actRightBottom.x - slice.w, actRightBottom.y - slice.x );\n";
				
				// with tiling
				if ( i & SHADER_TILE ) {
					params[ i ] +=
					"uniform vec2 tile;";
					funcs[ i ] +=
					"if ( suv.x < slice.w ) uv.x = suv.x;\n\
					else if ( suv.x >= actRightBottom.x ) uv.x = suv.x - actRightBottom.x + sliceRightBottom.x;\n\
					else uv.x = slice.w + mod( tile.x * sliceMid.x * ( suv.x - slice.w ) / actMid.x, sliceMid.x );\n\
					if ( suv.y < slice.x ) uv.y = suv.y;\n\
					else if ( suv.y >= actRightBottom.y ) uv.y = suv.y - actRightBottom.y + sliceRightBottom.y;\n\
					else uv.y = slice.x + mod( tile.y * sliceMid.y * ( suv.y - slice.x ) / actMid.y, sliceMid.y );\n";
				// slicing without tiling
				} else {
					funcs[ i ] +=
					"if ( suv.x < slice.w ) uv.x = suv.x;\n\
					else if ( suv.x >= actRightBottom.x ) uv.x = suv.x - actRightBottom.x + sliceRightBottom.x;\n\
					else uv.x = slice.w + sliceMid.x * ( suv.x - slice.w ) / actMid.x;\n\
					if ( suv.y < slice.x ) uv.y = suv.y;\n\
					else if ( suv.y >= actRightBottom.y ) uv.y = suv.y - actRightBottom.y + sliceRightBottom.y;\n\
					else uv.y = slice.x + sliceMid.y * ( suv.y - slice.x ) / actMid.y;\n";
				}
				
			// not sliced, but tiled
			} else if ( i & SHADER_TILE ) {
				params[ i ] +=
				"uniform vec2 tile;";
				funcs[ i ] +=
				"if ( tile.x != 1.0 ) uv.x = mod( uv.x * tile.x, texInfo.z );\n\
				if ( tile.y != 1.0 ) uv.y = mod( uv.y * tile.y, texInfo.w );\n";
			}
			
			// finish read pixel func
			funcs[ i ] += glsles ?
			"return texture2D( tex, ( uv + texInfo.xy ) / texSize );\n}\n" :
			"return texture( tex, ( uv + texInfo.xy ) / texSize );\n}\n";
			
			// read pixel color
			features[ i ] +=
			"src = readPixel( coord ) * color + addColor;\n";
		} else {
			features[ i ] +=
			"src = color + addColor;";
		}
		
		
		if ( i & SHADER_BLEND ) {
			params[ i ] +=
			"uniform sampler2D background;\n\
			uniform vec2 backgroundSize;\n\
			uniform int blendMode;";
			features[ i ] +=
			""; // TODO
		}
		
		if ( i & SHADER_STIPPLE ) {
			params[ i ] +=
			"uniform int stipple;\n\
			uniform int stippleAlpha;\n";
			features[ i ] +=
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
			if ( stippleValue < limit ) discard;";
		}

	}
	
	/// pixel shaders
	char fragShader[ 16384 ];
	for ( Uint8 i = 0; i < 32; i++ ){
		
		// skip ones with tile, or slice but without tex
		if ( ( i & SHADER_TILE || i & SHADER_SLICE ) && !( i & SHADER_TEXTURE ) ) continue;
		
		if ( glsles ) {
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
				%s\n\
				gl_FragColor = src;\n\
			}", renderer->min_shader_version, params[ i ].c_str(), funcs[ i ].c_str(), features[ i ].c_str(), post[ i ].c_str() );
		} else {
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
				%s\n\
				fragColor = src;\n\
			}", renderer->min_shader_version, params[ i ].c_str(), funcs[ i ].c_str(), features[ i ].c_str(), post[ i ].c_str() );
	
		}
		
		// compile variant
		if ( CompileShader( shaders[ i ].shader, shaders[ i ].shaderBlock, vertShader, fragShader ) ) {
			shaders[ i ].addColorUniform = GPU_GetUniformLocation( shaders[ i ].shader, "addColor" );
			shaders[ i ].tileUniform = GPU_GetUniformLocation( shaders[ i ].shader, "tile" );
			shaders[ i ].texInfoUniform = GPU_GetUniformLocation( shaders[ i ].shader, "texInfo" );
			shaders[ i ].texSizeUniform = GPU_GetUniformLocation( shaders[ i ].shader, "texSize" );
			shaders[ i ].texPadUniform = GPU_GetUniformLocation( shaders[ i ].shader, "texPad" );
			shaders[ i ].sliceUniform = GPU_GetUniformLocation( shaders[ i ].shader, "slice" );
			shaders[ i ].sliceScaleUniform = GPU_GetUniformLocation( shaders[ i ].shader, "sliceScale" );
			shaders[ i ].stippleUniform = GPU_GetUniformLocation( shaders[ i ].shader, "stipple" );
			shaders[ i ].stippleAlphaUniform = GPU_GetUniformLocation( shaders[ i ].shader, "stippleAlpha" );
			shaders[ i ].backgroundUniform = GPU_GetUniformLocation( shaders[ i ].shader, "background" );
			shaders[ i ].backgroundSizeUniform = GPU_GetUniformLocation( shaders[ i ].shader, "backgroundSize" );
			shaders[ i ].blendUniform = GPU_GetUniformLocation( shaders[ i ].shader, "blendMode" );
		} else {
			printf ( "Shader error: %s\nin shaders[%d]:\n%s\n%s\n", GPU_GetShaderMessage(), i, fragShader, vertShader );
			memset( &shaders[ i ], 0, sizeof( SpriteShaderVariant ) );
			exit(1);
		}
	}
	
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
