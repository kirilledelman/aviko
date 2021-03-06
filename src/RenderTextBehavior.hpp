#ifndef RenderTextBehavior_hpp
#define RenderTextBehavior_hpp

#include "common.h"
#include "RenderBehavior.hpp"
#include "TypedVector.hpp"

class FontResource;

class RenderTextBehavior: public RenderBehavior {
protected:
	
	/// needs repaint
	bool _dirty = false;
	
	// a prerendered single glyph
	struct GlyphInfo {
		//SDL_Surface* surface = NULL;
		GPU_Image* surface = NULL;
		int minX = 0;
		int minY = 0;
		int maxX = 0;
		int maxY = 0;
		int advance = 0;
		GlyphInfo(){};
		~GlyphInfo(){ if ( this->surface ) { GPU_FreeTarget( this->surface->target ); GPU_FreeImage( this->surface ); } /* SDL_FreeSurface( this->surface ); */ };
	};
	
	// single character
	struct RenderTextCharacter {
		Uint16 value = 0;
		float x = 0;
		float width = 0;
		size_t pos = 0; // position in JSString
		SDL_Color color = { 255,255,255,255 };
		GlyphInfo* glyphInfo = NULL;
		bool isWhiteSpace = false;
		int pad = 0;
	};
	
	/// prerendered glyphs ( 0 - normal, others, mask of TTF_STYLE_BOLD and TTF_STYLE_ITALIC )
	unordered_map<Uint16, GlyphInfo> glyphs[ 4 ];
	
	/// returns prerendered glyph with style
	GlyphInfo* GetGlyph( Uint16 c, bool b, bool i );
	
	struct RenderTextLine {
		float width = 0;
        float x = 0, y = 0;
		int firstCharacterPos = 0;
		vector<RenderTextCharacter> characters;
	};
	
	vector<RenderTextLine> lines;
	
public:
	
	// init, destroy
	RenderTextBehavior( ScriptArguments* args );
	RenderTextBehavior();
	~RenderTextBehavior();
	
// appearance
	
	/// font for drawing
	FontResource* fontResource = NULL;
	FontResource* fontBoldResource = NULL;
	FontResource* fontItalicResource = NULL;
	FontResource* fontBoldItalicResource = NULL;
	
	/// finished surface
	GPU_Image* surface = NULL;
	GPU_Rect surfaceRect = { 0 };
	
	/// update notification
	ColorCallback colorUpdated;
	TypedVectorCallback colorsUpdated;
	static void ColorsSetItem( void*, int, ArgValue& );
	
	/// font size
	unsigned fontSize = 16;
	
	/// font name
	string fontName;
	string fontBoldName;
	string fontItalicName;
	string fontBoldItalicName;
	
	/// text
	string text;
	
	/// draw as outline
	int outlineWidth = 0;
	
	/// quality
	bool antialias = true;
		
	/// current setting
	bool bold = false;
	bool italic = false;
	
	/// text alignment ( 0 - left, 1 - center, 2 - right )
	int align = 0;
	
	/// base text color
	Color *textColor = NULL;
	
	/// selection color
	Color *selectionColor = NULL;
	
	/// selection color
	Color *selectionTextColor = NULL;
	
	/// inline colors
	// Color *colors[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
	TypedVector* colors = NULL;
	
	// bg
	Color *backgroundColor = NULL;
	
	/// surface width
	int width = 0;
	
	/// surface height
	int height = 0;
	
	/// automatically assume text size
	bool autoResize = true;
	
    /// break lines at width
    bool wrap = true;
	
	/// use ^codes
	bool formatting = true;
	
	/// allows newlines
	bool multiLine = false;
	
    // offset x by this value
    int scrollLeft = 0;

	// offset y by this value
	int scrollTop = 0;
	
	// content width
	int scrollWidth = 0;
	
	// constent height
	int scrollHeight = 0;
	
    // extra character spacing
	float characterSpacing = 0;
	
	// extra line spacing
	float lineSpacing = 0;
	
	/// num spaces
	int tabSpaces = 4;

	// number of characters to hide from beginning of line
	int revealStart = 0;
	
	// number of characters to hide from end of line
	int revealEnd = 0;
	
// selection / input
    
    //
    bool showCaret = false;
    
    /// caret position ( 0 = before first char, string length = after last )
    int caretPosition = 0;
	
	float caretX = 0;
	float caretY = 0;
	int caretLine = 0;
    
    /// selection drawing enabled
    bool showSelection = false;
    
    /// selection painting
    int selectionStart = 0, selectionEnd = 0;
    
	
// scripting
	
	/// registers class for scripting
	static void InitClass();
	
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
// methods
	
	/// assigns font
	bool SetFont( const char* face, int size, bool b, bool i );
	
	/// repaints current text, clears dirty flag
	void Repaint( bool justMeasure=false );
	
	/// destroys prerendered glyphs on font change
	void ClearGlyphs();
	
    ///
    int GetCaretPositionAt( float x, float y );
    
	/// overridden from RenderBehavior
	GPU_Rect GetBounds();
	
	/// UIs without layout handler will call this on gameObject's render component
	void Resize( float w, float h );
	
	/// render callback
	static void Render( RenderTextBehavior* behavior, GPU_Target* target, Event* event );

// shape from render
	
	RigidBodyShape* MakeShape();
	
};

SCRIPT_CLASS_NAME( RenderTextBehavior, "RenderText" );

#endif /* RenderTextBehavior_hpp */
