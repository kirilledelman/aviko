#include "RenderBehavior.hpp"
#include "GameObject.hpp"

// static
RenderBehavior::SpriteShaderVariant RenderBehavior::shaders[ 8 ];


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

RenderBehavior::~RenderBehavior() { }


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
	( "tileX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->tileX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->tileX = val ); }) );
	
	script.AddProperty<RenderBehavior>
	( "tileY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((RenderBehavior*) b)->tileY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ( ((RenderBehavior*) b)->tileY = val ); }) );
	
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


int RenderBehavior::SelectShader( bool textured, bool rotated, float u, float v, float w, float h ){
	
	int shaderIndex = textured ? SHADER_TEXTURE : 0;
	
	if ( this->tileX != 1 || this->tileY != 1 ) shaderIndex |= SHADER_TILE;
	
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
	
	// tile
	if ( variant.tileUniform >= 0 ) {
		if ( rotated ) {
			params[ 0 ] = this->tileY;
			params[ 1 ] = this->tileX;
		} else {
			params[ 1 ] = this->tileY;
			params[ 0 ] = this->tileX;
		}
		GPU_SetUniformfv( variant.tileUniform, 2, 1, params );
	}
	
	// texInfo
	if ( variant.texInfoUniform >= 0 ) {
		params[ 0 ] = u;
		params[ 1 ] = v;
		params[ 2 ] = w;
		params[ 3 ] = h;
		GPU_SetUniformfv( variant.texInfoUniform, 4, 1, params );
	}
	
	return shaderIndex;
	
}

// init shaders
void RenderBehavior::InitShaders() {
	
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();
	bool glsles = ( renderer->shader_language == GPU_LANGUAGE_GLSLES );
	
	// create shader combinations w sets of different features
	
	char vertShader[ 4096 ];
	sprintf ( vertShader,
	"#version %d\n\
	%s\n\
	void main(void){\n\
		color = gpu_Color;\n\
		texCoord = vec2(gpu_TexCoord);\n\
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
	
	// params
	string tex =
	"uniform sampler2D tex;\n";
	string tile =
	"uniform vec2 tile;\n\
	uniform vec4 texInfo;\n";
	string stipple =
	"uniform int stipple; uniform int stippleAlpha;\n";
	string params[ 8 ];
	params[ SHADER_BASE ] = "";
	params[ SHADER_TEXTURE ] = tex;
	// params[ SHADER_TILE ] = tile;
	params[ SHADER_STIPPLE ] = stipple;
	params[ SHADER_STIPPLE | SHADER_TILE ] = stipple + tile;
	params[ SHADER_TILE | SHADER_TEXTURE ] = tile + tex;
	params[ SHADER_STIPPLE | SHADER_TEXTURE ] = stipple + tex;
	params[ SHADER_STIPPLE | SHADER_TILE | SHADER_TEXTURE ] = stipple + tile + tex;
	
	// features
	tile =
	"coord.x = mod((coord.x - texInfo.x) * tile.x, texInfo.z) + texInfo.x;\n\
	coord.y = mod((coord.y - texInfo.y) * tile.y, texInfo.w) + texInfo.y;\n";
	
	stipple =
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

	string readPixel = glsles ? "vec4 src = texture2D(tex, coord);\n" : "vec4 src = texture(tex, coord);\n";
	string readColor = "vec4 src = color;";
	string features[ 8 ];
	features[ SHADER_BASE ] = readColor;
	features[ SHADER_TEXTURE ] = readPixel;
	features[ SHADER_TILE | SHADER_TEXTURE ] = tile + readPixel;
	features[ SHADER_STIPPLE | SHADER_TEXTURE ] = readPixel + stipple;
	features[ SHADER_STIPPLE | SHADER_TILE | SHADER_TEXTURE ] = tile + readPixel + stipple;
	// features[ SHADER_TILE ] = tile + readColor;
	features[ SHADER_STIPPLE ] = readColor + stipple;
	// features[ SHADER_STIPPLE | SHADER_TILE ] = tile + readColor + stipple;
	
	/// sprite rendering shaders
	char fragShader[ 8192 ];
	for ( Uint8 i = 0; i < 8; i++ ){
		
		// skip ones with tile but without tex
		if ( ( i & SHADER_TILE ) && !( i & SHADER_TEXTURE ) ) continue;
		
		if ( glsles ) {
			sprintf( fragShader,
			"#version %d\n\
			precision mediump int;\nprecision mediump float;\n\
			varying mediump vec4 color;\n\
			varying vec2 texCoord;\n\
			uniform vec4 addColor;\n\
			%s\n\
			void main(void){\n\
				vec2 coord = texCoord;\n\
			%s\n\
				src = src * color;\n\
				if ( src.a == 0.0 ) discard;\n\
				gl_FragColor = src + addColor;\n\
			}", renderer->min_shader_version, params[ i ].c_str(), features[ i ].c_str() );
		} else {
			sprintf( fragShader,
			"#version %d\n\
			in vec4 color;\n\
			in vec2 texCoord;\n\
			in vec4 gl_FragCoord;\n\
			uniform vec4 addColor;\n\
			out vec4 fragColor;\n\
			%s\n\
			void main(void){\n\
				vec2 coord = texCoord;\n\
			%s\n\
				src = src * color;\n\
				if ( src.a == 0.0 ) discard;\n\
				fragColor = src + addColor;\n\
			}", renderer->min_shader_version, params[ i ].c_str(), features[ i ].c_str() );
	
		}
		
		// compile variant
		if ( CompileShader( shaders[ i ].shader, shaders[ i ].shaderBlock, vertShader, fragShader ) ) {
			shaders[ i ].tileUniform = GPU_GetUniformLocation( shaders[ i ].shader, "tile" );
			shaders[ i ].texInfoUniform = GPU_GetUniformLocation( shaders[ i ].shader, "texInfo" );
			shaders[ i ].addColorUniform = GPU_GetUniformLocation( shaders[ i ].shader, "addColor" );
			shaders[ i ].stippleUniform = GPU_GetUniformLocation( shaders[ i ].shader, "stipple" );
			shaders[ i ].stippleAlphaUniform = GPU_GetUniformLocation( shaders[ i ].shader, "stippleAlpha" );
		} else {
			printf ( "Shader error: %s\n", GPU_GetShaderMessage() );
			memset( &shaders[ i ], 0, sizeof( SpriteShaderVariant ) );
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
