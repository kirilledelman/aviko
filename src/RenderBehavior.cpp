#include "RenderBehavior.hpp"
#include "GameObject.hpp"
#include "RigidBodyShape.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

// static
RenderBehavior::ShaderVariant RenderBehavior::mainShader;


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

/// no 'new RenderBehavior()'
RenderBehavior::RenderBehavior( ScriptArguments* ) { script.ReportError( "RenderBehavior can't be created using 'new'." ); }

/// default constructor
RenderBehavior::RenderBehavior() {
};

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

RenderBehavior::~RenderBehavior() {}


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

bool RenderBehavior::ClipsMouseEventsFor( GameObject* c ) { return false; }

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
	
	// compile main
    RenderBehavior::CompileMainShader();

}

void RenderBehavior::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	
	// colors
	protectedObjects.push_back( &this->color->scriptObject );
	protectedObjects.push_back( &this->addColor->scriptObject );
	if ( this->outlineColor ) protectedObjects.push_back( &this->outlineColor->scriptObject );
	
	// call super
	Behavior::TraceProtectedObjects( protectedObjects );
	
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
            float *p = GPU_GetProjection();
            GPU_MatrixIdentity( p );
            GPU_MatrixOrtho( p, 0, targ->w, 0, targ->h, -1024, 1024 );
			GPU_MatrixMode( GPU_MODELVIEW );
			GPU_PushMatrix();
			GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
			// draw target
            RenderBehavior::SelectBasicShader( targ->image );
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

void RenderBehavior::SelectTexturedShader(
		float tw, float th,
		float u, float v, float w, float h,
		float st, float sr, float sb, float sl,
		float sx, float sy,
		float tx, float ty,
        float ox, float oy,
		GPU_Image *image, GPU_Target* targ, GPU_Target** blendTarg ){

	// set params
	float params[ 4 ];
	
	// addColor
    params[ 0 ] = this->addColor->r;
    params[ 1 ] = this->addColor->g;
    params[ 2 ] = this->addColor->b;
    params[ 3 ] = this->addColor->a;
    GPU_SetUniformfv( mainShader.addColorUniform, 4, 1, params );
	
	// stipple
    GPU_SetUniformi( mainShader.stippleUniform, 10000 * ( 1.0 - this->stipple ) );
    GPU_SetUniformi( mainShader.stippleAlphaUniform, this->stippleAlpha ? 1 : 0 );
	
	// tile
    params[ 1 ] = ty;
    params[ 0 ] = tx;
    GPU_SetUniformfv( mainShader.tileUniform, 2, 1, params );
	
    // thresh
    GPU_SetUniformf( mainShader.alphaThreshUniform, (float) this->alphaThresh );
    
	// pad
    GPU_SetUniformf( mainShader.texPadUniform, (float) this->texturePad );
	
	// texture size
    params[ 0 ] = tw;
    params[ 1 ] = th;
    GPU_SetUniformfv( mainShader.texSizeUniform, 2, 1, params );
	
	// sprite on texture
    params[ 0 ] = u;
    params[ 1 ] = v;
    params[ 2 ] = w;
    params[ 3 ] = h;
    GPU_SetUniformfv( mainShader.texInfoUniform, 4, 1, params );
	
	// slicing
    params[ 0 ] = st;
    params[ 1 ] = sr;
    params[ 2 ] = sb;
    params[ 3 ] = sl;
    GPU_SetUniformfv( mainShader.sliceUniform, 4, 1, params );

	// slicing: sprite scaling
    params[ 0 ] = sx;
    params[ 1 ] = sy;
    GPU_SetUniformfv( mainShader.sliceScaleUniform, 2, 1, params );
	
    // scroll
    params[ 0 ] = ox;
    params[ 1 ] = oy;
    GPU_SetUniformfv( mainShader.scrollOffsetUniform, 2, 1, params );
    
	// blend mode
    GPU_SetUniformi( mainShader.blendUniform, this->blendMode );
	
	// background
    if ( this->blendMode != BlendMode::Normal && this->blendMode != BlendMode::Cut ) {
        this->_UpdateBlendTarget( targ, blendTarg );
        if ( *blendTarg ) GPU_SetShaderImage( (*blendTarg)->image, mainShader.backgroundUniform, 1 );
    }
	
	// background size
    params[ 0 ] = targ->base_w;
    params[ 1 ] = targ->base_h;
    GPU_SetUniformfv( mainShader.backgroundSizeUniform, 2, 1, params );
	
	// outline color
    params[ 0 ] = this->outlineColor->r;
    params[ 1 ] = this->outlineColor->g;
    params[ 2 ] = this->outlineColor->b;
    params[ 3 ] = this->outlineColor->a;
    GPU_SetUniformfv( mainShader.outlineColorUniform, 4, 1, params );
	
	// outline params
    params[ 0 ] = this->outlineOffsetX;
    params[ 1 ] = this->outlineOffsetY;
    params[ 2 ] = this->outlineRadius;
    GPU_SetUniformfv( mainShader.outlineOffsetRadiusUniform, 3, 1, params );
	
}

void RenderBehavior::SelectUntexturedShader( GPU_Target* targ, GPU_Target** blendTarg ) {
	
	// set params
	float params[ 4 ];
	
    if ( this->blendMode != BlendMode::Normal && this->blendMode != BlendMode::Cut ) {
        this->_UpdateBlendTarget( targ, blendTarg );
        if ( *blendTarg ) GPU_SetShaderImage( (*blendTarg)->image, mainShader.backgroundUniform, 1 );
    }
    
    // background size
    params[ 0 ] = targ->base_w;
    params[ 1 ] = targ->base_h;
    GPU_SetUniformfv( mainShader.backgroundSizeUniform, 2, 1, params );
    
    // untex
    params[ 1 ] = params[ 0 ] = 0;
    GPU_SetUniformfv( mainShader.tileUniform, 2, 1, params );
    
    // addColor
    params[ 0 ] = this->addColor->r;
    params[ 1 ] = this->addColor->g;
    params[ 2 ] = this->addColor->b;
    params[ 3 ] = this->addColor->a;
    GPU_SetUniformfv( mainShader.addColorUniform, 4, 1, params );
	
	// stipple
    GPU_SetUniformi( mainShader.stippleUniform, 10000 * ( 1.0 - this->stipple ) );
    GPU_SetUniformi( mainShader.stippleAlphaUniform, this->stippleAlpha ? 1 : 0 );
	
	// blend mode
    GPU_SetUniformi( mainShader.blendUniform, this->blendMode );
	
	
}

void RenderBehavior::SelectBasicShader(GPU_Image* image) {
    
    // activate
    // GPU_ActivateShaderProgram( mainShader.shader, &mainShader.shaderBlock );

    // set params
    float params[ 4 ];
    
    // addColor
    params[ 0 ] = params[ 1 ] = params[ 2 ] = params[ 3 ] = 0;
    GPU_SetUniformfv( mainShader.addColorUniform, 4, 1, params );
    
    // stipple
    GPU_SetUniformi( mainShader.stippleUniform, 0 );
    GPU_SetUniformi( mainShader.stippleAlphaUniform, 0 );
    
    if ( image ) {
        
        params[ 1 ] = params[ 0 ] = 1.0;
        GPU_SetUniformfv( mainShader.tileUniform, 2, 1, params );
        
        // thresh
        GPU_SetUniformf( mainShader.alphaThreshUniform, 0 );
        
        // pad
        GPU_SetUniformf( mainShader.texPadUniform, 0 );
        
        // texture size
        params[ 0 ] = image->base_w;
        params[ 1 ] = image->base_h;
        GPU_SetUniformfv( mainShader.texSizeUniform, 2, 1, params );
        
        // sprite on texture
        params[ 0 ] = 0;
        params[ 1 ] = 0;
        params[ 2 ] = image->base_w;
        params[ 3 ] = image->base_h;
        GPU_SetUniformfv( mainShader.texInfoUniform, 4, 1, params );
        
        // slicing
        params[ 0 ] = params[ 1 ] = params[ 2 ] = params[ 3 ] = 0;
        GPU_SetUniformfv( mainShader.sliceUniform, 4, 1, params );
        
        // slicing: sprite scaling
        params[ 0 ] = params[ 1 ] = 1;
        GPU_SetUniformfv( mainShader.sliceScaleUniform, 2, 1, params );
        
        // scroll
        params[ 0 ] = params[ 1 ] = 0;
        GPU_SetUniformfv( mainShader.scrollOffsetUniform, 2, 1, params );
        
    } else {
        
        params[ 1 ] = params[ 0 ] = 0.0;
        GPU_SetUniformfv( mainShader.tileUniform, 2, 1, params );

    }
    
    // blend mode
    GPU_SetUniformi( mainShader.blendUniform, 0 );
    
    // outline params
    params[ 0 ] = params[ 1 ] = params[ 2 ] = 0;
    GPU_SetUniformfv( mainShader.outlineOffsetRadiusUniform, 3, 1, params );
    
}

// compiles shader with features
void RenderBehavior::CompileMainShader() {
	
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();
	bool glsles = ( renderer->shader_language == GPU_LANGUAGE_GLSLES );
	
	// assemble fragment shader source
	char fragShader[ 65536 ];
	char vertShader[ 8192 ];
	
    string vertParams;
    string vertFeatures;
    string params =
    "uniform sampler2D tex;\n\
    uniform vec2 texSize;\n\
    uniform vec4 texInfo;\n\
    uniform vec2 scrollOffset;\n\
    uniform vec2 tile;\n\
    uniform float alphaThresh;\n\
    uniform float texPad;\n\
    uniform vec4 slice;\n\
    uniform vec2 sliceScale;\n\
    uniform sampler2D background;\n\
    uniform vec2 backgroundSize;\n\
    uniform int blendMode;\n\
    uniform int stipple;\n\
    uniform int stippleAlpha;\n\
    uniform vec4 outlineColor;\n\
    uniform vec3 outlineOffsetRadius;\n";
    params += glsles ?
            "#define SAMPLE(a,b) texture2D(a,b)\n":
            "#define SAMPLE(a,b) texture(a,b)\n";
    string funcs = "\n\
    vec4 readPixel( sampler2D _tex, vec2 uv ){\n\
        bool outside = ( uv.x < texPad || uv.y < texPad || uv.x > texInfo.z - texPad || uv.y > texInfo.w - texPad );\n\
        uv = uv * tile - texInfo.xy;\n\
        uv.x = uv.x * ( ( texInfo.z + texPad * 2.0 ) / texInfo.z ) - texPad;\n\
        uv.y = uv.y * ( ( texInfo.w + texPad * 2.0 ) / texInfo.w ) - texPad;\n\
        // if ( uv.x < 0.0 || uv.x >= texInfo.z || uv.y < 0.0 || uv.y >= texInfo.w ) return vec4(0.0,0.0,0.0,0.0);\n\
        uv = vec2( mod( uv.x + scrollOffset.x * tile.x, texInfo.z ),\n\
            mod( uv.y  + scrollOffset.y * tile.y, texInfo.w ) );\n\
        if ( outside ) return vec4(0.0,0.0,0.0,0.0);\n\
        if ( slice != vec4( 0.0, 0.0, 0.0, 0.0 ) ) { // slice \n\
            vec2 suv = uv * sliceScale;\n\
            vec2 actSize = texInfo.zw * sliceScale;\n\
            vec2 sliceRightBottom = vec2( texInfo.z - slice.y, texInfo.w - slice.z );\n\
            vec2 actRightBottom = vec2( actSize.x - slice.y, actSize.y - slice.z );\n\
            vec2 sliceMid = vec2 ( sliceRightBottom.x - slice.w, sliceRightBottom.y - slice.x );\n\
            vec2 actMid = vec2 ( actRightBottom.x - slice.w, actRightBottom.y - slice.x );\n\
            if ( suv.x < slice.w ) uv.x = suv.x;\n\
            else if ( suv.x >= actRightBottom.x ) uv.x = suv.x - actRightBottom.x + sliceRightBottom.x;\n\
            else uv.x = slice.w + mod( tile.x * sliceMid.x * ( suv.x - slice.w ) / actMid.x, sliceMid.x );\n\
            if ( suv.y < slice.x ) uv.y = suv.y;\n\
            else if ( suv.y >= actRightBottom.y ) uv.y = suv.y - actRightBottom.y + sliceRightBottom.y;\n\
            else uv.y = slice.x + mod( tile.y * sliceMid.y * ( suv.y - slice.x ) / actMid.y, sliceMid.y );\n\
        }\n\
        return SAMPLE( _tex, ( uv + texInfo.xy ) / texSize );\n}\n";
    
    funcs += "\n\
    vec4 outlineSample( vec2 coord ){\n\
        vec4 result = readPixel( tex, coord ); \n\
        float radius = abs( outlineOffsetRadius.z );\n\
        float numRings = radius > 1.0 ? 2.0 : 1.0;\n\
        vec4 r1 = vec4( 0.0, 0.0, 0.0, 0.0 ),\n\
        r2 = vec4( 0.0, 0.0, 0.0, 0.0 );\n\
        float ringStep = radius / numRings; \n\
        if ( numRings >= 1.0 ) { \n\
            result = max( result, readPixel( tex, coord + vec2( 0.0, -ringStep ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( ringStep, 0.0 ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( 0.0, ringStep ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( -ringStep, 0.0 ) ) );\n\
        }\n\
        if ( numRings >= 2.0 ) { \n\
            float ringRadius = radius * 0.35; \n\
            result = max( result, readPixel( tex, coord + vec2( -ringRadius, -ringRadius ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( ringRadius, ringRadius ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( -ringRadius, ringRadius ) ) );\n\
            result = max( result, readPixel( tex, coord + vec2( ringRadius, -ringRadius ) ) );\n\
        }\n\
        if ( alphaThresh > 0.0 ) result.a = step( alphaThresh, result.a );\n\
        return result;\n\
    }\n";
    funcs += "\n\
    vec3 rgb2hsv(vec3 c) {\n\
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
    string features =
    "if ( alphaThresh < 0.0 ) { // particles \n\
        if ( tile != vec2( 0.0, 0.0 ) ) { // texture \n\
            vec4 part = SAMPLE( tex, coord );\n\
            if ( backgroundSize.x > 0.0 ) src = readPixel( background, vec2( -gl_FragCoord.y, gl_FragCoord.x ) ) * vec4( color.r, color.g, color.b, part.a ) + addColor;\n\
            else src = readPixel( background, gl_FragCoord.xy ) * vec4( color.r, color.g, color.b, part.a ) + addColor;\n\
        } else {\n\
            src = SAMPLE( tex, coord ) * vec4( color.r, color.g, color.b, 1.0 ) + addColor;\n\
        }\n\
    } else {\n\
        if ( tile != vec2( 0.0, 0.0 ) ) { // texture \n\
            if ( alphaThresh > 0.0 ) {\n\
                src = readPixel( tex, coord * texSize ) * vec4(color.r, color.g, color.b, 1.0) + addColor;\n\
                src.a = step( alphaThresh, src.a ) * color.a;\n\
            } else {\n\
                if ( outlineOffsetRadius != vec3 ( 0.0, 0.0, 0.0 ) ) src = readPixel( tex, coord * texSize );\n\
                else src = readPixel( tex, coord * texSize ) * color + addColor;\n\
            }\n\
            if ( outlineOffsetRadius != vec3 ( 0.0, 0.0, 0.0 ) ) { \n\
                src = readPixel( tex, coord * texSize );\n\
                vec4 smp = outlineSample( coord * texSize - outlineOffsetRadius.xy );\n\
                src.rgb = mix( outlineColor.rgb, src.rgb * color.rgb, src.a ) + addColor.rgb;\n\
                if ( outlineOffsetRadius.z >= 0.0 ) src.a = max( smp.a * outlineColor.a, src.a ) * color.a;\n\
                else src.a = max( smp.a * outlineColor.a, 0.0 ) * ( 1.0 - src.a ) * color.a;\n\
            }\n\
        } else {\n\
            src = color + addColor;\n\
        }\n\
    }\n";
	
    features +=
    "if ( blendMode > 0 ) {\n\
		vec2 fragCoord = gl_FragCoord.xy;\n\
		if ( blendMode == 12 ) fragCoord += 255.0 * vec2( src.r - 0.5, src.b - 0.5 ) * src.b; \n\
        vec4 bg = SAMPLE( background, fragCoord / backgroundSize );\n\
		if ( blendMode == 1 ) src.rgb += bg.rgb; // add \n\
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
    } \n";
	
	// stippling
    features +=
	"if ( stipple > 0 || stippleAlpha > 0 ) {\n\
		int index = int(mod(gl_FragCoord.x * 16.0, 64.0) / 16.0) + int(mod(gl_FragCoord.y * 16.0, 64.0) / 16.0) * 4;\n\
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
		if ( stippleValue < limit ) discard;\n\
    }\n";
	
	// vertex shader
	
	// put it together
	if ( glsles ) {
		sprintf ( vertShader,
		"#version %d\n\
		precision mediump int;\n\
        precision mediump float;\n\
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
		precision mediump int;\n\
        precision mediump float;\n\
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
		
	// compile
	if ( CompileShader( mainShader.shader, mainShader.shaderBlock, vertShader, fragShader ) ) {
        mainShader.scrollOffsetUniform = GPU_GetUniformLocation( mainShader.shader, "scrollOffset" );
        mainShader.backgroundUniform = GPU_GetUniformLocation( mainShader.shader, "background" );
        mainShader.backgroundSizeUniform = GPU_GetUniformLocation( mainShader.shader, "backgroundSize" );
		mainShader.addColorUniform = GPU_GetUniformLocation( mainShader.shader, "addColor" );
		mainShader.tileUniform = GPU_GetUniformLocation( mainShader.shader, "tile" );
		mainShader.texInfoUniform = GPU_GetUniformLocation( mainShader.shader, "texInfo" );
		mainShader.texSizeUniform = GPU_GetUniformLocation( mainShader.shader, "texSize" );
		mainShader.texPadUniform = GPU_GetUniformLocation( mainShader.shader, "texPad" );
        mainShader.sliceUniform = GPU_GetUniformLocation( mainShader.shader, "slice" );
		mainShader.sliceScaleUniform = GPU_GetUniformLocation( mainShader.shader, "sliceScale" );
		mainShader.stippleUniform = GPU_GetUniformLocation( mainShader.shader, "stipple" );
		mainShader.stippleAlphaUniform = GPU_GetUniformLocation( mainShader.shader, "stippleAlpha" );
		mainShader.blendUniform = GPU_GetUniformLocation( mainShader.shader, "blendMode" );
		mainShader.outlineColorUniform = GPU_GetUniformLocation( mainShader.shader, "outlineColor" );
		mainShader.outlineOffsetRadiusUniform = GPU_GetUniformLocation( mainShader.shader, "outlineOffsetRadius" );
        mainShader.alphaThreshUniform = GPU_GetUniformLocation( mainShader.shader, "alphaThresh" );
        printf( "Main shader compiled.\n" );
	} else {
        printf ( "Shader error: %s\nFRAGMENT SHADER SRC:\n\n", GPU_GetShaderMessage() );
        // add line numbers
        stringstream fs(fragShader);
        string token;
        int line = 0;
        while (std::getline(fs, token, '\n')) {
            printf( "%d\t%s\n", line++, token.c_str() );
        }
        
        //printf ( "Shader error: %s\nFRAGMENT SHADER SRC:\n%s\n\nVERTEX SHADER SRC:\n%s\n", GPU_GetShaderMessage(), fragShader, vertShader );
		exit(1);
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
		printf( "Failed to link, fragment shader: %s\n\n", GPU_GetShaderMessage() );
		return false;
	}
	outShaderBlock = GPU_LoadShaderBlock( outShader, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix" );
	return true;
}

