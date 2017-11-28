#ifndef RenderTextBehavior_hpp
#define RenderTextBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"

class FontResource;

class RenderTextBehavior: public RenderBehavior {
protected:
	
	/// needs repaint
	bool _dirty = false;
	
public:
	
	// init, destroy
	RenderTextBehavior( ScriptArguments* args );
	RenderTextBehavior();
	~RenderTextBehavior();
	
// appearance
	
	/// font for drawing
	FontResource* fontResource = NULL;
	
	GPU_Image* surface = NULL;
	GPU_Rect surfaceRect = { 0 };
	
	/// font size
	unsigned fontSize = 16;
	
	/// font name
	string fontName;
	
	/// text
	string text;
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
// methods
	
	/// assigns font
	bool SetFont( const char* face, int size );
	
	/// repaints current text, clears dirty flag
	void Repaint();
	
	/// render callback
	static void Render( RenderTextBehavior* behavior, GPU_Target* target );
	
	
};


#endif /* RenderTextBehavior_hpp */
