#include "UIBehavior.hpp"
#include "GameObject.hpp"
#include "Application.hpp"
#include "RenderSpriteBehavior.hpp"
#include "Image.hpp"
#include "Scene.hpp"

/*
 
 Layout notes:
 
	set layout type, anchors, offsets, paddings/margins etc
	add ui.layout event listener, which receives ( x, y, w, h ), and should update gameObject + render
	ui without parent (gameObject.parent.ui is null), uses gameObject's x, y, and ui.width, height as start point
 
 */



/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
UIBehavior::UIBehavior( ScriptArguments* args ) : UIBehavior() {
	
	// add scriptObject
	script.NewScriptObject<UIBehavior>( this );

	// obj argument - init object
	void *initObj = NULL;
	if ( args && args->ReadArguments( 1, TypeObject, &initObj ) ) {
		script.CopyProperties( initObj, this->scriptObject );
	}
	
}

// init
UIBehavior::UIBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_MOUSEMOVE, (BehaviorEventCallback) &UIBehavior::MouseMove );
	AddEventCallback( EVENT_MOUSEDOWN, (BehaviorEventCallback) &UIBehavior::MouseButton );
	AddEventCallback( EVENT_MOUSEUP, (BehaviorEventCallback) &UIBehavior::MouseButton );
	AddEventCallback( EVENT_MOUSEWHEEL, (BehaviorEventCallback) &UIBehavior::MouseWheel );
	AddEventCallback( EVENT_KEYDOWN, (BehaviorEventCallback) &UIBehavior::Key);
	AddEventCallback( EVENT_KEYUP, (BehaviorEventCallback) &UIBehavior::Key);
	AddEventCallback( EVENT_KEYPRESS, (BehaviorEventCallback) &UIBehavior::KeyPress);
	AddEventCallback( EVENT_NAVIGATION, (BehaviorEventCallback) &UIBehavior::Navigation);
	AddEventCallback( EVENT_ADDED_TO_SCENE, (BehaviorEventCallback) &UIBehavior::Attached );
	AddEventCallback( EVENT_REMOVED_FROM_SCENE, (BehaviorEventCallback) &UIBehavior::Detached );
	AddEventCallback( EVENT_ATTACHED, (BehaviorEventCallback) &UIBehavior::Attached );
	AddEventCallback( EVENT_DETACHED, (BehaviorEventCallback) &UIBehavior::Detached );
	AddEventCallback( EVENT_ADDED, (BehaviorEventCallback) &UIBehavior::Attached );
	AddEventCallback( EVENT_REMOVED, (BehaviorEventCallback) &UIBehavior::Detached );
	AddEventCallback( EVENT_ACTIVE_CHANGED, (BehaviorEventCallback) &UIBehavior::ActiveChanged );
	AddEventCallback( EVENT_LAYOUT, (BehaviorEventCallback) &UIBehavior::Layout );
	
	// flag
	this->isUIBehavior = true;
	
}

// destructor
UIBehavior::~UIBehavior() {
	

}



/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void UIBehavior::InitClass() {
	
	// register class
	script.RegisterClass<UIBehavior>( "Behavior" );
	
	// constants
	
	void* constants = script.NewObject();
	script.AddGlobalNamedObject( "Layout", constants );
	script.SetProperty( "None", ArgValue( (int) LayoutType::None ), constants );
	script.SetProperty( "Anchors", ArgValue( (int) LayoutType::Anchors ), constants );
	script.SetProperty( "Vertical", ArgValue( (int) LayoutType::Vertical ), constants );
	script.SetProperty( "Horizontal", ArgValue( (int) LayoutType::Horizontal ), constants );
	script.FreezeObject( constants );
	
	constants = script.NewObject();
	script.AddGlobalNamedObject( "LayoutAlign", constants );
	script.SetProperty( "Default", ArgValue( (int) LayoutAlign::Default ), constants );
	script.SetProperty( "Start", ArgValue( (int) LayoutAlign::Start ), constants );
	script.SetProperty( "Center", ArgValue( (int) LayoutAlign::Center ), constants );
	script.SetProperty( "End", ArgValue( (int) LayoutAlign::End ), constants );
	script.SetProperty( "Stretch", ArgValue( (int) LayoutAlign::Stretch ), constants );
	script.FreezeObject( constants );
	
	// props
	
	script.AddProperty<UIBehavior>
	( "over", // 
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->mouseOver; }));

	script.AddProperty<UIBehavior>
	( "down", //
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		return ui->mouseDown[ 0 ] || ui->mouseDown[ 1 ] || ui->mouseDown[ 2 ] || ui->mouseDown[ 3 ];
	}));
	
	script.AddProperty<UIBehavior>
	( "focusable",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->focusable; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return (((UIBehavior*) b)->focusable = val); }));
	
	script.AddProperty<UIBehavior>
	( "focused",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( !ui->scene ) return false;
		return ( ui->scene->focusedUI == ui );
	}),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( !ui->scene ) return false;
		if ( val && ui->focusable && ui != ui->scene->focusedUI ) ui->Focus();
		else if ( !val && ui == ui->scene->focusedUI ) ui->Blur();
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "autoMoveFocus",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->autoNavigate; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return (((UIBehavior*) b)->autoNavigate = val); }));
	
	script.AddProperty<UIBehavior>
	( "focusLeft",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationLeft ? ui->navigationLeft : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			GameObject *go = script.GetInstance<GameObject>( val );
			if ( go ) other = go->ui;
		}
		if ( !other && val ) {
			script.ReportError( ".focusLeft can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationLeft = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "focusRight",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationRight ? ui->navigationRight : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			GameObject *go = script.GetInstance<GameObject>( val );
			if ( go ) other = go->ui;
		}
		if ( !other && val ) {
			script.ReportError( ".focusRight can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationRight = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "focusUp",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationUp ? ui->navigationUp : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			GameObject *go = script.GetInstance<GameObject>( val );
			if ( go ) other = go->ui;
		}
		if ( !other && val ) {
			script.ReportError( ".focusUp can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationUp = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "focusDown",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationDown ? ui->navigationDown : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			GameObject *go = script.GetInstance<GameObject>( val );
			if ( go ) other = go->ui;
		}
		if ( !other && val ) {
			script.ReportError( ".focusDown can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationDown = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "focusGroup", //
	 static_cast<ScriptStringCallback>([](void *b, string val ){ return ((UIBehavior*) b)->navigationGroup; }),
	 static_cast<ScriptStringCallback>([](void *b, string val ){
		UIBehavior* ui = (UIBehavior*) b;
		ui->navigationGroup = val;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "anchorLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->anchorLeft; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->anchorLeft != val ) {
			ui->anchorLeft = val;
			ui->RequestLayout( ArgValue( "anchorLeft" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "anchorRight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->anchorRight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->anchorRight != val ) {
			ui->anchorRight = val;
			ui->RequestLayout( ArgValue( "anchorRight" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "anchorTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->anchorTop; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->anchorTop != val ) {
			ui->anchorTop = val;
			ui->RequestLayout( ArgValue( "anchorTop" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "anchorBottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->anchorBottom; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->anchorBottom != val ) {
			ui->anchorBottom = val;
			ui->RequestLayout( ArgValue( "anchorBottom" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "left", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->left; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->left != val ) {
			ui->left = val;
			ui->RequestLayout( ArgValue( "left" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "right", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->right; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->right != val ) {
			ui->right = val;
			ui->RequestLayout( ArgValue( "right" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "top", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->top; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->top != val ) {
			ui->top = val;
			ui->RequestLayout( ArgValue( "top" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "bottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->bottom; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->bottom != val ) {
			ui->bottom = val;
			ui->RequestLayout( ArgValue( "bottom" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "width", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->layoutWidth; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		val = fmin( ( ui->maxWidth > 0 ? ui->maxWidth : 9999999 ), fmax( val, ui->minWidth ) );
		if ( ui->layoutWidth != val ) {
			ui->layoutWidth = val;
			ui->RequestLayout( ArgValue( "width" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "height", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->layoutHeight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		val = fmin( ( ui->maxHeight > 0 ? ui->maxHeight : 9999999 ), fmax( val, ui->minHeight ) );
		if ( ui->layoutHeight != val ) {
			ui->layoutHeight = val;
			ui->RequestLayout( ArgValue( "height" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "minWidth", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->minWidth; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->minWidth != val ) {
			val = ui->minWidth = fmax( 0, val );
			ui->layoutWidth = fmax( val, ui->layoutWidth );
			ui->RequestLayout( ArgValue( "minWidth" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "maxWidth", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->maxWidth; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->maxWidth != val ) {
			val = ui->maxWidth = fmax( 0, val );
			if ( val > 0 ) ui->layoutWidth = fmin( val, ui->layoutWidth );
			ui->RequestLayout( ArgValue( "maxWidth" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "minHeight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->minHeight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->minHeight != val ) {
			val = ui->minHeight = fmax( 0, val );
			ui->layoutHeight = fmax( val, ui->layoutHeight );
			ui->RequestLayout( ArgValue( "minHeight" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "maxHeight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->maxHeight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->maxHeight != val ) {
			val = ui->maxHeight = fmax( 0, val );
			if ( val > 0 ) ui->layoutHeight = fmin( val, ui->layoutHeight );
			ui->RequestLayout( ArgValue( "maxHeight" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "layoutType",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->layoutType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		LayoutType v = (LayoutType) val;
		if ( ui->layoutType != v ) {
			ui->layoutType = v;
			ui->RequestLayout( ArgValue( "layoutType" ) );
		}
		return (int) ui->layoutType;
	}) );
	
	script.AddProperty<UIBehavior>
	( "layoutAlignX",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->axisAlignX; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		LayoutAlign v = (LayoutAlign) val;
		if ( ui->axisAlignX != v ) {
			ui->axisAlignX = v;
			ui->RequestLayout( ArgValue( "layoutAlignX" ) );
		}
		return (int) ui->axisAlignX;
	 }));
	
	script.AddProperty<UIBehavior>
	( "layoutAlignY",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->axisAlignY; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		LayoutAlign v = (LayoutAlign) val;
		if ( ui->axisAlignY != v ) {
			ui->axisAlignY = v;
			ui->RequestLayout( ArgValue( "layoutAlignY" ) );
		}
		return (int) ui->axisAlignY;
	}));
	
	script.AddProperty<UIBehavior>
	( "selfAlign",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->selfAlign; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		LayoutAlign v = (LayoutAlign) val;
		if ( ui->selfAlign != v ) {
			ui->selfAlign = v;
			ui->RequestLayout( ArgValue( "selfAlign" ) );
		}
		return (int) ui->selfAlign;
	}));
	
	script.AddProperty<UIBehavior>
	( "flex", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->flex; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->flex != val ) {
			val = ui->flex = fmax( 0, val );
			ui->RequestLayout( ArgValue( "flex" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "wrapEnabled",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->wrapEnabled; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->wrapEnabled != val ) {
			ui->wrapEnabled = val;
			ui->RequestLayout( ArgValue( "wrapEnabled" ) );
		}
		return ui->wrapEnabled;
	}));
	
	script.AddProperty<UIBehavior>
	( "wrapAfter",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->wrapAfter; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->wrapAfter != val ) {
			ui->wrapAfter = val;
			ui->RequestLayout( ArgValue( "wrapAfter" ) );
		}
		return (int) ui->wrapAfter;
	}) );
	
	script.AddProperty<UIBehavior>
	( "forceWrap",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->forceWrap; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->forceWrap != val ) {
			ui->forceWrap = val;
			ui->RequestLayout( ArgValue( "forceWrap" ) );
		}
		return ui->forceWrap;
	}));
	
	script.AddProperty<UIBehavior>
	( "fitChildren",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->fitChildren; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->fitChildren != val ) {
			ui->fitChildren = val;
			ui->RequestLayout( ArgValue( "fitChildren" ) );
		}
		return ui->fitChildren;
	}));

	script.AddProperty<UIBehavior>
	( "fixedPosition",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->fixedPosition; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->fixedPosition != val ) {
			ui->fixedPosition = val;
			ui->RequestLayout( ArgValue( "fixedPosition" ) );
		}
		return ui->fixedPosition;
	}));	
	
	script.AddProperty<UIBehavior>
	( "pad",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		ArgValue v;
		v.type = TypeArray;
		v.value.arrayValue = new ArgValueVector();
		v.value.arrayValue->emplace_back( ArgValue( ui->padTop ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->padRight ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->padBottom ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->padLeft ) );
		return v;
	 }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		float sameVal = 0;
		float padLeft = ui->padLeft, padRight = ui->padRight,
			  padTop = ui->padTop, padBottom = ui->padBottom;
		
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( ui->padTop ) ) {
				if ( val.value.arrayValue->size() >= 2 && val.value.arrayValue->at( 1 ).toNumber( ui->padRight ) ) {
					if ( val.value.arrayValue->size() >= 3 && val.value.arrayValue->at( 2 ).toNumber( ui->padBottom ) ) {
						if ( val.value.arrayValue->size() >= 4 ) val.value.arrayValue->at( 3 ).toNumber( ui->padLeft );
					}
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			ui->padTop = ui->padRight = ui->padBottom = ui->padLeft = sameVal;
		}
		
		// check for change
		if ( padLeft != ui->padLeft || padRight != ui->padRight ||
			padTop != ui->padTop || padBottom != ui->padBottom ) ui->RequestLayout( ArgValue( "pad" ) );
		
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<UIBehavior>
	( "padTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->padTop; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->padTop != val ) {
			ui->padTop = val;
			ui->RequestLayout( ArgValue( "padTop" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "padRight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->padRight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->padRight != val ) {
			ui->padRight = val;
			ui->RequestLayout( ArgValue( "padRight" ) );
		}
		return val;
	}));

	script.AddProperty<UIBehavior>
	( "padBottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->padBottom; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->padBottom != val ) {
			ui->padBottom = val;
			ui->RequestLayout( ArgValue( "padBottom" ) );
		}
		return val;
	}));

	script.AddProperty<UIBehavior>
	( "padLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->padLeft; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->padLeft != val ) {
			ui->padLeft = val;
			ui->RequestLayout( ArgValue( "padLeft" ) );
		}
		return val;
	}));

	script.AddProperty<UIBehavior>
	( "margin",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		ArgValue v;
		v.type = TypeArray;
		v.value.arrayValue = new ArgValueVector();
		v.value.arrayValue->emplace_back( ArgValue( ui->marginTop ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->marginRight ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->marginBottom ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->marginLeft ) );
		return v;
	}),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		float sameVal = 0;
		float marginLeft = ui->marginLeft, marginRight = ui->marginRight,
			  marginTop = ui->marginTop, marginBottom = ui->marginBottom;
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( ui->marginTop ) ) {
				if ( val.value.arrayValue->size() >= 2 && val.value.arrayValue->at( 1 ).toNumber( ui->marginRight ) ) {
					if ( val.value.arrayValue->size() >= 3 && val.value.arrayValue->at( 2 ).toNumber( ui->marginBottom ) ) {
						if ( val.value.arrayValue->size() >= 4 ) val.value.arrayValue->at( 3 ).toNumber( ui->marginLeft );
					}
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			ui->marginTop = ui->marginRight = ui->marginBottom = ui->marginLeft = sameVal;
		}
		// change?
		if ( marginLeft != ui->marginLeft || marginRight != ui->marginRight ||
			marginTop != ui->marginTop || marginBottom != ui->marginBottom ) ui->RequestLayout( ArgValue( "margin" ) );
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE  );
	
	script.AddProperty<UIBehavior>
	( "marginTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->marginTop; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->marginTop != val ) {
			ui->marginTop = val;
			ui->RequestLayout( ArgValue( "marginTop" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "marginRight", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->marginRight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->marginRight != val ) {
			ui->marginRight = val;
			ui->RequestLayout( ArgValue( "marginRight" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "marginBottom", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->marginBottom; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->marginBottom != val ) {
			ui->marginBottom = val;
			ui->RequestLayout( ArgValue( "marginBottom" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "marginLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->marginLeft; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->marginLeft != val ) {
			ui->marginLeft = val;
			ui->RequestLayout( ArgValue( "marginLeft" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "spacing",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		ArgValue v;
		v.type = TypeArray;
		v.value.arrayValue = new ArgValueVector();
		v.value.arrayValue->emplace_back( ArgValue( ui->spacingX ) );
		v.value.arrayValue->emplace_back( ArgValue( ui->spacingY ) );
		return v;
	}),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		UIBehavior* ui = (UIBehavior*) b;
		float sameVal = 0;
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( ui->spacingX ) ) {
				if ( val.value.arrayValue->size() >= 2 ) {
					val.value.arrayValue->at( 1 ).toNumber( ui->spacingY );
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			ui->spacingX = ui->spacingY = sameVal;
		}
		ui->RequestLayout( ArgValue( "spacing" ) );
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE  );
	
	script.AddProperty<UIBehavior>
	( "spacingX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->spacingX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->spacingX != val ) {
			ui->spacingX = val;
			ui->RequestLayout( ArgValue( "spacingX" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "spacingY", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->spacingY; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->spacingY != val ) {
			ui->spacingY = val;
			ui->RequestLayout( ArgValue( "spacingY" ) );
		}
		return val;
	}));
	
	// functions
	
	script.DefineFunction<UIBehavior>
	("focus",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		return self->Focus();
	} ));

	script.DefineFunction<UIBehavior>
	("blur",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		self->Blur();
		return true;
	} ));
	
	script.DefineFunction<UIBehavior>
	("moveFocus",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		int dirX = 0, dirY = 0;
		if ( !sa.ReadArguments( 1, TypeInt, &dirX, TypeInt, &dirY ) ) {
			script.ReportError( "usage: moveFocus( Int directionX, [ Int directionY ] )" );
			return false;
		}
		// two args?
		if ( sa.args.size() > 1 ) {
			
			sa.ReturnBool( self->Navigate( dirX, dirY ) );
			
		// one? try both direction
		} else {
		
			sa.ReturnBool( self->Navigate( dirX, 0 ) || self->Navigate( 0, dirX ) );
		}
		return true;
	} ));
	
	script.DefineFunction<UIBehavior>
	("getFocusable",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		int dirX = 0, dirY = 0;
		if ( !sa.ReadArguments( 2, TypeInt, &dirX, TypeInt, &dirY ) ) {
			script.ReportError( "usage: getFocusable( Int directionX, Int directionY )" );
			return false;
		}
		
		// find
		UIBehavior* ui = self->FindFocusable( dirX, dirY );
		sa.ReturnObject( ui ? ui->scriptObject : NULL );
		return true;
	} ));
	
	script.DefineFunction<UIBehavior>
	( "resize", // setSize( Number width, Number height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		UIBehavior* self = (UIBehavior*) obj;
		float lw = self->layoutWidth, lh = self->layoutHeight;
		if ( !sa.ReadArguments( 2, TypeFloat, &lw, TypeFloat, &lh ) ) {
			script.ReportError( "usage: resize( Number width, Number height )" );
			return false;
		}
		if ( lw != self->layoutWidth || lh != self->layoutHeight ) {
			self->layoutWidth = lw; self->layoutHeight = lh;
			self->RequestLayout( ArgValue( "resize" ) );
		}
		return true;
	}));
	
	script.DefineFunction<UIBehavior>
	("requestLayout",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		// if passed argument, do request layout (called on top level parent)
		if ( sa.args.size() )
			self->RequestLayout( sa.args[ 0 ] );
		else
			self->RequestLayout( ArgValue() );
		return true;
	}));

}


/* MARK:	-				Focus and navigation
 -------------------------------------------------------------------- */


bool UIBehavior::Focus() {
	
	// if can't be focused, return
	if ( !this->focusable || !this->scene || !this->gameObject || this->gameObject->orphan ) return false;
	
	// get current focus
	UIBehavior* current = this->scene->focusedUI;
	
	// already focused?
	if ( current == this ) return true;
	
	// blur previous
	if ( current ) current->Blur( false );
	
	// set focus
	this->scene->focusedUI = this;
	
	// if focus changed during a navigation event, cancel it
	vector<Event*>::iterator e = Event::eventStack.begin();
	string navEvent(EVENT_NAVIGATION);
	while( e != Event::eventStack.end() ) {
		if ( navEvent.compare( (*e)->name ) == 0 ) {
			(*e)->stopped = true;
			break;
		}
		e++;
	}
	
	// dispatch event on previous and new focus
	Event event( EVENT_FOCUSCHANGED );
	event.scriptParams.AddObjectArgument( this->scriptObject );
	this->CallEvent( event );
	if ( current ) current->CallEvent( event );

	
	return true;
}

void UIBehavior::Blur( bool sendEvent ) {
	
	// if can't be focused, return
	if ( !this->focusable || !this->scene || !this->gameObject ) return;
	
	// get current focus
	UIBehavior* current = this->scene->focusedUI;
	
	// not focused?
	if ( current != this ) return;
	
	// clear focus
	this->scene->focusedUI = NULL;
	
	// dispatch event
	if ( sendEvent ) {
		Event event( EVENT_FOCUSCHANGED );
		event.scriptParams.AddObjectArgument( NULL );
		this->CallEvent( event );
	}
	
}


bool UIBehavior::IsScreenPointInBounds( float x, float y, float* localX, float* localY ) {
	// ask gameObject's RenderBehavior if screen point is inside object
	if ( this->gameObject != NULL ) {
		if ( this->gameObject->render != NULL && ( this->layoutHeight == 0 || this->layoutWidth == 0 ) ) {
			return this->gameObject->render->IsScreenPointInside( x, y, localX, localY );
		} else {
			this->gameObject->ConvertPoint( x, y, *localX, *localY, false );
			return (*localX >= 0 && *localX < this->layoutWidth ) && (*localY >= 0 && *localY < this->layoutHeight );
		}
	}
	return false;
}


/// change focus to focusable UI that's in the direction x, y from this control
bool UIBehavior::Navigate( float x, float y ) {
		
	UIBehavior* other = this->FindFocusable( x, y );
	
	if ( other ) {
		other->Focus(); return true;
	}
	
	// nothing found
	return false;
	
}

UIBehavior* UIBehavior::FindFocusable( float x, float y ) {

	// normalize direction
	Uint8 dir = ( y < 0 ? 0 :
				 ( y > 0 ? 2 :
				  ( x > 0 ? 1 : 3 ) ) );
	
	// check overrides
	UIBehavior* other = NULL;
	if ( dir == 0 && this->navigationUp ) other = this->navigationUp;
	else if ( dir == 1 && this->navigationRight ) other = this->navigationRight;
	else if ( dir == 2 && this->navigationDown ) other = this->navigationDown;
	else if ( dir == 3 && this->navigationLeft ) other = this->navigationLeft;
	
	// found!
	if ( other ) return other;
	
	// other is self, return null
	if ( other == this ) return NULL;
	
	// get min,max of this control in global coords
	float minX, minY, maxX, maxY;
	this->gameObject->ConvertPoint( 0, 0, minX, minY, true );
	this->gameObject->ConvertPoint( layoutWidth, layoutHeight, maxX, maxY, true );
	
	// set of control/distance from this control
	unordered_map<UIBehavior*,float> candidates;
	
	// for each UI in scene
	vector<UIBehavior*> uiElements;
	this->scene->GetBehaviors( true, uiElements );
	if ( !uiElements.size() || ( uiElements.size() == 1 && uiElements[ 0 ] == this ) ) return NULL;
	
	// try until find a candidate or margin of search is too big
	float maxMargin = fmin( maxX - minX, maxY - minY );
	float extraMargin = fmin( maxX - minX, maxY - minY ) * 0.5;
	while ( !candidates.size() && extraMargin < maxMargin ) {
		for ( size_t i = 0, nc = uiElements.size(); i < nc; i++ ) {
			other = uiElements[ i ];
			if ( other == this || !other->focusable || other->navigationGroup.compare( navigationGroup ) != 0 || !other->Behavior::active() || !other->gameObject->activeRecursive() ) continue;
			
			// get min/max of other control
			float otherMinX, otherMinY, otherMaxX, otherMaxY;
			other->gameObject->ConvertPoint( 0, 0, otherMinX, otherMinY, true );
			other->gameObject->ConvertPoint( other->layoutWidth, other->layoutHeight, otherMaxX, otherMaxY, true );
			
			switch( dir ) {
				case 0: // up
					otherMinX -= extraMargin;
					otherMaxX += extraMargin;
					if ( otherMaxY > minY || // below
						otherMaxX < minX || otherMinX > maxX ) // no overlap
						continue;
					break;
				case 2: // down
					otherMinX -= extraMargin;
					otherMaxX += extraMargin;
					if ( otherMinY < maxY || // above
						otherMaxX < minX || otherMinX > maxX ) // no overlap
						continue;
					break;
				case 3: // left
					otherMinY -= extraMargin;
					otherMaxY += extraMargin;
					if ( otherMaxX > minX || // right
						otherMaxY < minY || otherMinY > maxY ) // no overlap
						continue;
					break;
				case 1: // right
					otherMinY -= extraMargin;
					otherMaxY += extraMargin;
					if ( otherMinX < maxX || // left
						otherMaxY < minY || otherMinY > maxY ) // no overlap
						continue;
					break;
			}
			
			// calc distance from center of this control to midpoint on each of four sides of candidate
			float midX = (minX + (maxX - minX) * 0.5),
			midY = (minY + (maxY - minY) * 0.5),
			otherMidX = (otherMinX + (otherMaxX - otherMinX) * 0.5),
			otherMidY = (otherMinY + (otherMaxY - otherMinY) * 0.5),
			xx, yy, dist[ 4 ];
			
			// top side
			yy = midY - otherMinY;
			xx = midX - otherMidX;
			dist[ 0 ] = sqrt( xx * xx + yy * yy );
			// right
			yy = midY - otherMidY;
			xx = midX - otherMaxX;
			dist[ 1 ] = sqrt( xx * xx + yy * yy );
			// bottom
			yy = midY - otherMaxY;
			xx = midX - otherMidX;
			dist[ 2 ] = sqrt( xx * xx + yy * yy );
			// left
			yy = midY - otherMidY;
			xx = midX - otherMinX;
			dist[ 3 ] = sqrt( xx * xx + yy * yy );
			
			// add to candidates
			candidates[ other ] = fmin( fmin( fmin( dist[ 0 ], dist[ 1 ] ), dist[ 2 ] ), dist[ 3 ] );
		}
		// increase search margin
		extraMargin += maxMargin * 0.25;
	}
	
	// pick candidate with shortest dist
	other = NULL;
	float minDist = 0;
	unordered_map<UIBehavior*,float>::iterator it = candidates.begin(), end = candidates.end();
	while( it != end ) {
		if ( !other || ( other && it->second < minDist ) ) {
			other = it->first;
			minDist = it->second;
		}
		it++;
	}
		
	return other;
	
}

/* MARK:	-				Layout
 -------------------------------------------------------------------- */


void UIBehavior::Layout( UIBehavior *behavior, void *p, Event *event ){
	
	if ( !behavior->gameObject ) return;
	
	// constrain size
	behavior->layoutWidth = fmax( behavior->minWidth, fmin( ( behavior->maxWidth > 0 ? behavior->maxWidth : 9999999 ), behavior->layoutWidth ) );
	behavior->layoutHeight = fmax( behavior->minHeight, fmin( ( behavior->maxHeight > 0 ? behavior->maxHeight : 9999999 ), behavior->layoutHeight ) );
	
	// remember current, to retrigger layout later if changed
	float
	oldWidth = behavior->layoutWidth,
	oldHeight = behavior->layoutHeight;
	
	// collect children with active UI component
	vector<UIBehavior*> childUIs;
	for ( size_t i = 0, nc = behavior->gameObject->children.size(); i < nc; i++ ){
		UIBehavior* ui = behavior->gameObject->children[ i ]->ui;
		if ( ui != NULL && ui->Behavior::active() && ui->gameObject->active() && !ui->fixedPosition ) childUIs.push_back( ui );
	}
	
	// call appropriate layout function
	switch ( behavior->layoutType ) {
		case LayoutType::None:
			behavior->LayoutNone( childUIs );
			break;
		case LayoutType::Anchors:
			behavior->LayoutAnchors( childUIs );
			break;
		case LayoutType::Horizontal:
			behavior->LayoutHorizontal( childUIs );
			break;
		case LayoutType::Vertical:
			behavior->LayoutVertical( childUIs );
			break;
	}
	
	// if there's render component
	if ( behavior->gameObject->render ) {
		// determine if there's no layout handler
		EventListenersMap::iterator hit = behavior->eventListeners.find( string( EVENT_LAYOUT ) );
		bool hasHandler = ( hit != behavior->eventListeners.end() && hit->second.size() > 0 );
		if ( !hasHandler ) {
			ArgValue hval = script.GetProperty( EVENT_LAYOUT, behavior->scriptObject );
			if ( hval.type == TypeUndefined ) {
				// no layout handler - call Resize on render component
				behavior->gameObject->render->Resize( behavior->layoutWidth, behavior->layoutHeight );
			}
		}
	}
	
	// dispatch layout event listeners on this component 
	Event e ( EVENT_LAYOUT );
	e.scriptParams.AddFloatArgument( behavior->layoutWidth );
	e.scriptParams.AddFloatArgument( behavior->layoutHeight );
	// if dispatched as event
	if ( event )  {
		// add params in reverse order
		for ( size_t i = event->scriptParams.args.size(); i > 0; i-- ) e.scriptParams.AddArgument( event->scriptParams.args[ i - 1 ] );
	}
	behavior->CallEvent( e );
	
	// if layout process changed width/height/min/max
	if ( behavior->layoutWidth != oldWidth || behavior->layoutHeight != oldHeight ) {
		// layout again
		if ( behavior->gameObject->parent &&
			behavior->gameObject->parent->ui &&
			behavior->gameObject->parent->ui->_active ) {
			
			// call directly on parent
			UIBehavior::Layout( behavior->gameObject->parent->ui, p, event );
			//behavior->gameObject->parent->ui->RequestLayout( ArgValue() );
			
		} else {
			
			// behavior->RequestLayout( ArgValue( "childSizeChanged" ) );
			behavior->RequestLayout( ArgValue() );
			
		}
	}
	
}

bool UIBehavior::IsStretchyX(){
	UIBehavior* parentUI = this->gameObject->parent ? this->gameObject->parent->ui : NULL;
	if ( !parentUI ) return false;
	if ( parentUI->layoutType == LayoutType::Vertical ) {
		return ( parentUI->axisAlignX == LayoutAlign::Stretch || this->selfAlign == LayoutAlign::Stretch );
	} else if ( parentUI->layoutType == LayoutType::Horizontal ) {
		return (flex != 0);
	}
	return false;
}

bool UIBehavior::IsStretchyY(){
	UIBehavior* parentUI = this->gameObject->parent ? this->gameObject->parent->ui : NULL;
	if ( !parentUI ) return false;
	if ( parentUI->layoutType == LayoutType::Horizontal ) {
		return ( parentUI->axisAlignY == LayoutAlign::Stretch || this->selfAlign == LayoutAlign::Stretch );
	} else if ( parentUI->layoutType == LayoutType::Vertical ) {
		return (flex != 0);
	}
	return false;
}

void UIBehavior::LayoutHorizontal( vector<UIBehavior *> &childUIs ) {
	
	float innerWidth = fmax( 0, this->layoutWidth - ( this->padLeft + this->padRight ) );
	float innerHeight = fmax( 0, this->layoutHeight - ( this->padTop + this->padBottom ) );
	float totalMinSize = 0;
	float minChildSize = 0;
	float minSpaceUsed = 0;
	float curX = 0;
	float curY = 0;

	struct LayoutRow {
		vector<UIBehavior*> items;
		float maxSize = 0;
		float minSize = 0;
		float spaceUsed = 0;
		float totalFlex = 0;
		vector<UIBehavior*> flexed;
	};
	
	vector<LayoutRow> rows;
	rows.emplace_back();
	LayoutRow* currentRow = &rows.back();
	
	// put all elements in rows first
	for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
		UIBehavior* childUI = childUIs[ i ];
		float
		w = childUI->minWidth + ( childUI->marginLeft + childUI->marginRight ),
		h = childUI->minHeight + ( childUI->marginTop + childUI->marginBottom );
		
		// if child won't fit in current row
		if ( wrapEnabled && currentRow->items.size() > 0 && curX + w + this->spacingX >= innerWidth ) {
			
			// start new row
			curX = 0;
			totalMinSize += currentRow->minSize;
			rows.emplace_back();
			currentRow = &rows.back();
			
		}
		
		// add to current row
		currentRow->items.push_back( childUI );
		currentRow->maxSize = fmax( h, currentRow->maxSize );
		currentRow->minSize = fmax( childUI->minHeight, currentRow->minSize );
		currentRow->spaceUsed = curX + w;
		curX += w + spacingX;
		minChildSize = fmax( minChildSize, w );
		minSpaceUsed = fmax( minSpaceUsed, currentRow->spaceUsed );
		if ( childUI->flex ) {
			currentRow->flexed.push_back( childUI );
			currentRow->totalFlex += childUI->flex;
		}
		
		// check if new row is needed for next item
		if ( wrapEnabled && ( childUI->forceWrap || ( wrapAfter > 0 && currentRow->items.size() >= wrapAfter ) ) ) {
		
			// force wrap next row
			curX = innerWidth;
		
		}
		
	}
	
	// min/max size required for all rows
	int numSpacers = (int) rows.size() - 1;
	totalMinSize += currentRow->minSize + numSpacers * spacingY;
	
	// if fit children is set or size is 0
	if ( this->fitChildren ) {
		innerHeight = fmin( ( maxHeight > 0 ? maxHeight : 9999999 ) - ( this->padTop + this->padBottom ), fmax( totalMinSize, innerHeight ) );
		innerWidth = fmin( ( maxWidth > 0 ? maxWidth : 9999999 ) - ( this->padLeft + this->padRight ), fmax( minSpaceUsed, innerWidth ) );
	}

	// if stretching rows across, and there's more space than needed
	if ( innerHeight > totalMinSize && ( this->axisAlignY == LayoutAlign::Stretch || this->axisAlignY == LayoutAlign::Center ) ) {
		float spaceLeft = innerHeight;
		float averageRowHeight = floor( ( spaceLeft - numSpacers * spacingY ) / (float) rows.size() );
		for ( size_t r = 0, nr = rows.size(); r < nr; r++ ){
			LayoutRow* currentRow = &rows[ r ];
			currentRow->maxSize = fmax( currentRow->minSize, averageRowHeight );
			spaceLeft -= currentRow->maxSize;
		}
	}
	
	// prepare
	float maxX = 0, maxY = 0, spacing = 0, startOffset = 0;
	curY = this->padTop;
	
	// process each row
	for ( size_t r = 0, nr = rows.size(); r < nr; r++ ){
		LayoutRow* currentRow = &rows[ r ];
		// distribute flex space to fill row, if there are flex items
		if ( currentRow->totalFlex > 0 ) {
			float extraSpace = innerWidth - currentRow->spaceUsed;
			float spaceLeft = extraSpace, spaceShare = 0;
			for ( size_t i = 0, nf = currentRow->flexed.size(); i < nf; i++ ) {
				if ( spaceLeft <= 0 ) break;
				UIBehavior* f = currentRow->flexed[ i ];
				spaceShare = fmin( spaceLeft, extraSpace * ( f->flex / currentRow->totalFlex ) );
				if ( f->maxWidth > 0 ) spaceShare = fmin( spaceShare, f->maxWidth - ( f->marginLeft + f->marginRight ) );
				f->layoutWidth = f->minWidth + spaceShare;
				spaceLeft -= spaceShare;
			}
			spacing = spacingX;
			currentRow->spaceUsed = innerWidth;
			startOffset = 0;
		// otherwise, if stretch main axis align, add space
		} else if ( axisAlignX == LayoutAlign::Stretch ){
			spacing = ( innerWidth - ( currentRow->spaceUsed - spacingX * ( currentRow->items.size() - 1 ) ) ) / (float) currentRow->items.size();
			startOffset = spacing * 0.5;
			currentRow->spaceUsed = innerWidth;
		} else {
			// normal
			spacing = spacingX;
			startOffset = 0;
		}
		
		// start row
		
		// reset X
		if ( this->axisAlignX == LayoutAlign::End ) {
			curX = padLeft + innerWidth - currentRow->spaceUsed;
		} else if ( this->axisAlignX == LayoutAlign::Center ) {
			curX = padLeft + ( innerWidth - currentRow->spaceUsed ) * 0.5;
		} else {
			curX = padLeft;
		}
		curX += startOffset;
		
		// lay out each child in row
		for ( size_t i = 0, nc = currentRow->items.size(); i < nc; i++ ){
			
			UIBehavior* childUI = currentRow->items[ i ];
			float x = 0, y = 0, w = 0, h = 0;
			
			// measure
			x = curX + childUI->marginLeft;
			w = (childUI->flex ? childUI->layoutWidth : childUI->minWidth);
			h = childUI->minHeight;
			LayoutAlign align = this->axisAlignY;
			if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
			switch ( align ) {
				case LayoutAlign::Stretch:
					y = curY + childUI->marginTop;
					h = currentRow->maxSize - ( childUI->marginTop + childUI->marginBottom );
					break;
				case LayoutAlign::Default:
				case LayoutAlign::Start:
					y = curY + childUI->marginTop;
					break;
				case LayoutAlign::End:
					y = curY + (currentRow->maxSize - ( h + childUI->marginBottom ) );
					break;
				case LayoutAlign::Center:
					y = curY + (currentRow->maxSize - h) * 0.5;
					break;
					
			}
			
			// max dims
			maxX = fmax( maxX, x + w + childUI->marginRight );
			maxY = fmax( maxY, y + h + childUI->marginBottom );

			// step forward
			curX += w + childUI->marginLeft + childUI->marginRight + spacing;

			// set/store
			childUI->gameObject->SetPosition( x, y );
			childUI->layoutWidth = w;
			childUI->layoutHeight = h;
			
		}
		
		// next row
		curY += currentRow->maxSize + spacingY;
		
	}
	
	// if fit children is set, ensure children are contained, and min sizes set
	if ( this->fitChildren ) {
		if ( this->axisAlignY != LayoutAlign::Stretch && !this->IsStretchyY() ) this->layoutHeight = maxY + this->padBottom;
		if ( !wrapEnabled && !this->IsStretchyX() ) {
			this->layoutWidth = maxX + this->padRight;
		}
		if ( wrapEnabled ) {
			this->minWidth = minChildSize + this->padLeft + this->padRight;
		} else if ( this->axisAlignX != LayoutAlign::Stretch ){
			this->minWidth = maxX + this->padRight;
		}
		if ( this->axisAlignY == LayoutAlign::Stretch ) {
			this->minHeight = this->padTop + totalMinSize + this->padBottom;
		} else {
			this->minHeight = maxY + this->padBottom;
		}
	}
	
	// enforce min/max again
	this->layoutWidth = ( fmax( this->minWidth, fmin( ( this->maxWidth > 0 ? this->maxWidth : 9999999 ), this->layoutWidth ) ) );
	this->layoutHeight = ( fmax( this->minHeight, fmin( ( this->maxHeight > 0 ? this->maxHeight : 9999999 ), this->layoutHeight ) ) );
	
}

void UIBehavior::LayoutVertical( vector<UIBehavior *> &childUIs ) {
	
	float innerWidth = fmax( 0, this->layoutWidth - ( this->padLeft + this->padRight ) );
	float innerHeight = fmax( 0, this->layoutHeight - ( this->padTop + this->padBottom ) );
	float totalMinSize = 0;
	float minChildSize = 0;
	float minSpaceUsed = 0;
	float curX = 0;
	float curY = 0;
	
	struct LayoutRow {
		vector<UIBehavior*> items;
		float maxSize = 0;
		float minSize = 0;
		float spaceUsed = 0;
		float totalFlex = 0;
		vector<UIBehavior*> flexed;
	};
	
	vector<LayoutRow> rows;
	rows.emplace_back();
	LayoutRow* currentRow = &rows.back();
	
	// put all elements in rows first
	for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
		UIBehavior* childUI = childUIs[ i ];
		float
		w = childUI->minWidth + ( childUI->marginLeft + childUI->marginRight ),
		h = childUI->minHeight + ( childUI->marginTop + childUI->marginBottom );
		
		// if child won't fit in current row
		if ( wrapEnabled && currentRow->items.size() > 0 && curY + h + this->spacingY >= innerHeight ) {
			
			// start new row
			curY = 0;
			totalMinSize += currentRow->minSize;
			rows.emplace_back();
			currentRow = &rows.back();
			
		}
		
		// add to current row
		currentRow->items.push_back( childUI );
		currentRow->maxSize = fmax( w, currentRow->maxSize );
		currentRow->minSize = fmax( childUI->minWidth, currentRow->minSize );
		currentRow->spaceUsed = curY + h;
		curY += h + spacingY;
		minChildSize = fmax( minChildSize, h );
		minSpaceUsed = fmax( minSpaceUsed, currentRow->spaceUsed );
		if ( childUI->flex ) {
			currentRow->flexed.push_back( childUI );
			currentRow->totalFlex += childUI->flex;
		}
		
		// check if new row is needed for next item
		if ( wrapEnabled && ( childUI->forceWrap || ( wrapAfter > 0 && currentRow->items.size() >= wrapAfter ) ) ) {
			
			// force wrap next row
			curY = innerHeight;
			
		}
		
	}
	
	// min/max size required for all rows
	int numSpacers = (int) rows.size() - 1;
	totalMinSize += currentRow->minSize + numSpacers * spacingX;
	
	// if fit children is set or size is 0
	if ( this->fitChildren ) {
		innerHeight = fmin( ( maxHeight > 0 ? maxHeight : 9999999 ) - ( this->padTop + this->padBottom ), fmax( minSpaceUsed, innerHeight ) );
		innerWidth = fmin( ( maxWidth > 0 ? maxWidth : 9999999 ) - ( this->padLeft + this->padRight ), fmax( totalMinSize, innerWidth ) );
	}
	
	// distribute extra space to rows maxSize
	if ( innerWidth > totalMinSize && ( this->axisAlignX == LayoutAlign::Stretch || this->axisAlignX == LayoutAlign::Center ) ) {
		float spaceLeft = innerWidth;
		float averageRowWidth = floor( ( spaceLeft - numSpacers * spacingX ) / (float) rows.size() );
		for ( size_t r = 0, nr = rows.size(); r < nr; r++ ){
			LayoutRow* currentRow = &rows[ r ];
			currentRow->maxSize = fmax( currentRow->minSize, averageRowWidth );
			spaceLeft -= currentRow->maxSize;
		}
	}
	
	// prepare
	float maxX = 0, maxY = 0, spacing = 0, startOffset = 0;
	curX = this->padLeft;
	
	// process each row
	for ( size_t r = 0, nr = rows.size(); r < nr; r++ ){
		LayoutRow* currentRow = &rows[ r ];
		// distribute flex space to fill row, if there are flex items
		if ( currentRow->totalFlex > 0 ) {
			float extraSpace = innerHeight - currentRow->spaceUsed;
			float spaceLeft = extraSpace, spaceShare = 0;
			for ( size_t i = 0, nf = currentRow->flexed.size(); i < nf; i++ ) {
				if ( spaceLeft <= 0 ) break;
				UIBehavior* f = currentRow->flexed[ i ];
				spaceShare = fmin( spaceLeft, extraSpace * ( f->flex / currentRow->totalFlex ) );
				if ( f->maxHeight > 0 ) spaceShare = fmin( spaceShare, f->maxHeight - ( f->marginTop + f->marginBottom ) );
				f->layoutHeight = f->minHeight + spaceShare;
				spaceLeft -= spaceShare;
			}
			spacing = spacingY;
			currentRow->spaceUsed = innerHeight;
			startOffset = 0;
		// otherwise, if stretch main axis align, add space
		} else if ( axisAlignY == LayoutAlign::Stretch ){
			spacing = ( innerHeight - ( currentRow->spaceUsed - spacingY * ( currentRow->items.size() - 1 ) ) ) / (float) currentRow->items.size();
			startOffset = spacing * 0.5;
			currentRow->spaceUsed = innerHeight;
		} else {
			// normal
			spacing = spacingY;
			startOffset = 0;
		}
		
		// start row
		
		// reset Y
		if ( this->axisAlignY == LayoutAlign::End ) {
			curY = padTop + innerHeight - currentRow->spaceUsed;
		} else if ( this->axisAlignY == LayoutAlign::Center ) {
			curY = padTop + ( innerHeight - currentRow->spaceUsed ) * 0.5;
		} else {
			curY = padTop;
		}
		curY += startOffset;
		
		// lay out each child in row
		for ( size_t i = 0, nc = currentRow->items.size(); i < nc; i++ ){
			
			UIBehavior* childUI = currentRow->items[ i ];
			float x = 0, y = 0, w = 0, h = 0;
			
			// measure
			y = curY + childUI->marginTop;
			w = childUI->minWidth;
			h = (childUI->flex ? childUI->layoutHeight : childUI->minHeight);
			LayoutAlign align = this->axisAlignX;
			if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
			switch ( align ) {
				case LayoutAlign::Stretch:
					x = curX + childUI->marginLeft;
					w = currentRow->maxSize - ( childUI->marginLeft + childUI->marginRight );
					break;
				case LayoutAlign::Default:
				case LayoutAlign::Start:
					x = curX + childUI->marginLeft;
					break;
				case LayoutAlign::End:
					x = curX + (currentRow->maxSize - ( w + childUI->marginRight ) );
					break;
				case LayoutAlign::Center:
					x = curX + (currentRow->maxSize - w) * 0.5;
					break;
					
			}
			
			// max dims
			maxX = fmax( maxX, x + w + childUI->marginRight );
			maxY = fmax( maxY, y + h + childUI->marginBottom );
			
			// step forward
			curY += h + childUI->marginTop + childUI->marginBottom + spacing;
			
			// set/store
			childUI->gameObject->SetPosition( x, y );
			childUI->layoutWidth = w;
			childUI->layoutHeight = h;
			
		}
		
		// next row
		curX += currentRow->maxSize + spacingX;
		
	}
	
	// if fit children is set, ensure children are contained, and min sizes set
	if ( this->fitChildren ) {
		if ( this->axisAlignX != LayoutAlign::Stretch && !this->IsStretchyX() ) this->layoutWidth = maxX + this->padRight;
		if ( !wrapEnabled && !this->IsStretchyY() ) {
			this->layoutHeight = maxY + this->padBottom;
		}
		if ( wrapEnabled ) {
			this->minHeight = minChildSize + this->padTop + this->padBottom;
		} else if ( this->axisAlignY != LayoutAlign::Stretch ){
			 this->minHeight = maxY + this->padBottom;
		}
		if ( this->axisAlignX == LayoutAlign::Stretch ) {
			this->minWidth = this->padLeft + totalMinSize + this->padRight;
		} else {
			this->minWidth = maxX + this->padRight;
		}
	}
	
	// enforce min/max again
	this->layoutWidth = ( fmax( this->minWidth, fmin( ( this->maxWidth > 0 ? this->maxWidth : 9999999 ), this->layoutWidth ) ) );
	this->layoutHeight = ( fmax( this->minHeight, fmin( ( this->maxHeight > 0 ? this->maxHeight : 9999999 ), this->layoutHeight ) ) );

}

void UIBehavior::LayoutAnchors( vector<UIBehavior *> &childUIs ) {
	for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
		UIBehavior* childUI = childUIs[ i ];
		float x = 0, y = 0, w = 0, h = 0;
		
		childUI->GetAnchoredPosition( this, x, y, w, h );
		
		childUI->gameObject->SetPosition( x, y );
		childUI->layoutWidth = w;
		childUI->layoutHeight = h;
	}
}

void UIBehavior::LayoutNone( vector<UIBehavior *> &childUIs ) {
	// no action
}

/// resolves anchored position/size
void UIBehavior::GetAnchoredPosition( UIBehavior* parentUI, float& x, float& y, float& w, float& h ) {
	
	// use previous / default
	x = gameObject->GetX();
	y = gameObject->GetY();
	w = fmin( ( maxWidth > 0 ? maxWidth : 9999999 ), fmax( layoutWidth, minWidth ) );
	h = fmin( ( maxHeight > 0 ? maxHeight : 9999999 ), fmax( layoutHeight, minHeight ) );
	
	if ( parentUI ) {
	
		float parentWidth = parentUI->layoutWidth - ( parentUI->padLeft + parentUI->padRight );
		float parentHeight = parentUI->layoutHeight - ( parentUI->padTop + parentUI->padBottom );
		
		// anchors
		
		float thisSide = 0;
		float otherSide = 0;
		
		// left is measured from start edge
		if ( anchorLeft == 0 ) {
			thisSide = left;
		// left is measured from opposite edge
		} else if ( anchorLeft == 1 ) {
			thisSide = parentWidth - left;
		// left is measured from middle
		} else if ( anchorLeft > 0 ){
			thisSide = parentWidth * anchorLeft + left;
		}
		// right is measured from start edge
		if ( anchorRight == 0 ) {
			otherSide = parentWidth - right;
		// right is measured from opposite edge
		} else if ( anchorRight == 1 ) {
			otherSide = right;
		// right is measured from middle
		} else if ( anchorRight > 0 ){
			otherSide = parentWidth * ( 1 - anchorRight ) + right;
		}
		
		// apply
		if ( anchorLeft >= 0 || anchorRight >= 0 ) {
			// start anchor is disabled
			if ( anchorLeft < 0 ) {
				x = otherSide - w;
			// end anchor is disabled
			} else if ( anchorRight < 0 ) {
				x = thisSide;
			} else {
				x = thisSide;
				w = otherSide - thisSide;
			}
		}
		
		// top is measured from start edge
		if ( anchorTop == 0 ) {
			thisSide = top;
		// top is measured from opposite edge
		} else if ( anchorTop == 1 ) {
			thisSide = parentHeight - top;
		// top is measured from middle
		} else if ( anchorTop > 0 ){
			thisSide = parentHeight * anchorTop + top;
		}
		// bottom is measured from start edge
		if ( anchorBottom == 0 ) {
			otherSide = parentHeight - bottom;
		// right is measured from opposite edge
		} else if ( anchorBottom == 1 ) {
			otherSide = bottom;
			// right is measured from middle
		} else if ( anchorBottom > 0 ){
			otherSide = parentHeight * ( 1 - anchorBottom ) + bottom;
		}
		
		if ( anchorTop >= 0 || anchorBottom >= 0 ) {
			// start anchor is disabled
			if ( anchorTop < 0 ) {
				y = otherSide - h;
			// end anchor is disabled
			} else if ( anchorBottom < 0 ) {
				y = thisSide;
			} else {
				y = thisSide;
				h = otherSide - thisSide;
			}
		}
		
		// add parents' padding
		x += parentUI->padLeft;
		y += parentUI->padTop;
		
		// add margins
		x += this->marginLeft;
		w -= this->marginLeft + this->marginRight;
		y += this->marginTop;
		h -= this->marginTop + this->marginBottom;
		
	}
	
	w = fmin( ( maxWidth > 0 ? maxWidth : 9999999 ), fmax( w, minWidth ) );
	h = fmin( ( maxHeight > 0 ? maxHeight : 9999999 ), fmax( h, minHeight ) );
	
}

/// debounces multiple layout requests
void UIBehavior::RequestLayout( ArgValue trigger ) {
	
	if ( !this->gameObject ) return;
	
	// dispatch down
	GameObject* top = NULL;
	// fixed position - same object
	if ( this->fixedPosition ) {
		
		top = this->gameObject;
		
	// no trigger - use parent
	} else if ( trigger.type == TypeUndefined ) {
		
		top = ( this->gameObject->parent ? this->gameObject->parent : this->gameObject );
	
	// top level - scene
	} else {
		
		top = ( this->gameObject ? this->gameObject->GetScene() : NULL );
		
	}
	if ( top ) {
		ArgValueVector* params = app.AddLateEvent( top, EVENT_LAYOUT, true, false );
		if ( trigger.type != TypeUndefined ) {
			params->push_back( trigger );
			params->push_back( ArgValue( this->scriptObject ) );
		}
	}
}


/* MARK:	-				Debug draw
 -------------------------------------------------------------------- */


/// draws ui bounds
void UIBehavior::DebugDraw( GPU_Target* targ ) {
	
	static SDL_Color
		bounds = { 255, 64, 64, 64 },
		boundsMinMax = { 255, 128, 128, 64 },
		padding = { 255, 255, 128, 64 },
		margins = { 128, 128, 200, 64 };
	GPU_ActivateShaderProgram( 0, NULL );
	GPU_SetShapeBlendMode( GPU_BLEND_NORMAL );
	GPU_SetLineThickness( 1 );
	GPU_SetDepthTest( targ, false );
	GPU_SetDepthWrite( targ, false );
	
	// bounds
	GPU_Rectangle( targ, 0, 0, layoutWidth, layoutHeight, bounds );
	
	// padding
	if ( padLeft != 0 || padRight != 0 || padTop != 0 || padBottom != 0 ) {
		GPU_Rectangle( targ, padLeft, padTop, layoutWidth - padRight, layoutHeight - padBottom, padding );
	}
	
	GPU_SetLineThickness( 2 );
	
	// min, max
	if ( minWidth > 0 || minHeight > 0 ) {
		GPU_Line( targ, minWidth, minHeight, minWidth, minHeight - 5, boundsMinMax );
		GPU_Line( targ, minWidth, minHeight, minWidth - 5, minHeight, boundsMinMax );
	}
	if ( maxWidth > 0 || maxHeight > 0 ) {
		GPU_Line( targ, maxWidth, maxHeight, maxWidth, maxHeight - 10, boundsMinMax );
		GPU_Line( targ, maxWidth, maxHeight, maxWidth - 10, maxHeight, boundsMinMax );
	}
	
	// margins
	if ( marginLeft != 0 || marginRight != 0 || marginTop != 0 || marginBottom != 0 ) {
		GPU_Line( targ, 0, -marginTop, layoutWidth, -marginTop, margins );
		GPU_Line( targ, layoutWidth + marginRight, 0, layoutWidth + marginRight, layoutHeight, margins );
		GPU_Line( targ, 0, layoutHeight + marginBottom, layoutWidth, layoutHeight + marginBottom, margins );
		GPU_Line( targ, -marginLeft, 0, -marginLeft, layoutHeight, margins );
	}
	
	GPU_SetDepthTest( targ, true );
	GPU_SetDepthWrite( targ, true );
	
}


/* MARK:	-				Clipping
 -------------------------------------------------------------------- */


// finds closest RenderSpriteBehavior with image+autoDraw = parent
void UIBehavior::CheckClipping() {

	// reset
	this->clippedBy = NULL;
	if ( !this->gameObject ) return;
	
	// go up the tree
	GameObject* p = this->gameObject->parent;
	GameObject* c = this->gameObject;
	while ( p ) {
		RenderSpriteBehavior* rs = ClassInstance<RenderSpriteBehavior>(p->render);
		if ( rs ) {
			if ( rs->imageInstance && rs->imageInstance->autoDraw == c ) {
				this->clippedBy = rs;
				return;
			}
		}
		// keep climbing
		c = p;
		p = p->parent;
	}
	
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */


void UIBehavior::MouseMove( UIBehavior* behavior, void* param, Event* e ){
	float x = e->scriptParams.args[ 0 ].value.floatValue;
	float y = e->scriptParams.args[ 1 ].value.floatValue;
	float localX = 0, localY = 0;
	bool inBounds = behavior->IsScreenPointInBounds( x, y, &localX, &localY );
	
	// if clipped by RenderSprite/image+autoDraw, check if still in bounds
	if ( inBounds && behavior->clippedBy ) {
		float clippedLocalX, clippedLocalY;
		inBounds = behavior->clippedBy->IsScreenPointInside( x, y, &clippedLocalX, &clippedLocalY );
	}
	
	// entered bounds
	if ( inBounds && !behavior->mouseOver ) {
		
		behavior->mouseOver = true;
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOVER );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		behavior->CallEvent( event );
	// exited bounds
	} else if ( !inBounds && behavior->mouseOver ) {
		
		behavior->mouseOver = false;
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOUT );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		behavior->CallEvent( event );
	}
	
}

void UIBehavior::MouseButton( UIBehavior* behavior, void* param, Event* e){
	float x = e->scriptParams.args[ 1 ].value.floatValue;
	float y = e->scriptParams.args[ 2 ].value.floatValue;
	int btn = e->scriptParams.args[ 0 ].value.intValue;
	float localX = 0, localY = 0;
	bool inBounds = behavior->IsScreenPointInBounds( x, y, &localX, &localY );
	bool down = ( strcmp( e->name, EVENT_MOUSEDOWN ) == 0 );
	
	// if clipped by RenderSprite/image+autoDraw, check if still in bounds
	if ( inBounds && behavior->clippedBy ) {
		float clippedLocalX, clippedLocalY;
		inBounds = behavior->clippedBy->IsScreenPointInside( x, y, &clippedLocalX, &clippedLocalY );
	}
	
	//printf( "MouseButton inBounds=%d,down=%d, %s.ui\n", inBounds, down, behavior->gameObject->name.c_str() );
	
	if ( inBounds ) {
		// dispatch mousedown or mouseup
		Event event( e->name );
		event.scriptParams.AddIntArgument( btn );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		behavior->CallEvent( event );
		
		// mouse button was released
		if ( behavior->mouseDown[ btn ] && !down ){
			event.name = EVENT_CLICK;
			behavior->CallEvent( event );
		}
		
		// store mouse down
		behavior->mouseDown[ btn ] = down;
		
	// released button out of bounds
	} else if( !down ) {
		
		// was down
		if ( behavior->mouseDown[ btn ] ) {
			// generate release outside event
			Event event( EVENT_MOUSEUPOUTSIDE );
			event.scriptParams.AddIntArgument( btn );
			event.scriptParams.AddFloatArgument( localX );
			event.scriptParams.AddFloatArgument( localY );
			event.scriptParams.AddFloatArgument( x );
			event.scriptParams.AddFloatArgument( y );
			behavior->CallEvent( event );
		}
		
		// clear down state
		behavior->mouseDown[ btn ] = down;
	}
	
}

void UIBehavior::MouseWheel( UIBehavior* behavior, void* param, Event* e){

	// forward event if mouse is over
	if ( behavior->mouseOver ) {
		behavior->CallEvent( *e );
	}

}

void UIBehavior::Navigation( UIBehavior* behavior, void* param, Event* e ){
	// if focused
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		
		// get name and direction
		string axisName = *e->scriptParams.args[ 0 ].value.stringValue;
		float direction = 0;
		e->scriptParams.args[ 1 ].toNumber( direction );
		float x = axisName.compare( app.input.navigationXAxis ) == 0 ? direction : 0;
		float y = axisName.compare( app.input.navigationYAxis ) == 0 ? direction : 0;
		
		// if this is "up" event, ignore
		if ( direction == 0 ) return;
		
		// dispatch event to this behavior
		behavior->CallEvent( *e );
		
		// behavior wants system to handle navigation
		if ( behavior->autoNavigate ) {
			
			if ( !behavior->scene ) behavior->scene = behavior->gameObject->GetScene();
			
			// if it's a directional event, and focus didn't change
			if ( behavior->scene && behavior->scene->focusedUI == behavior && ( x != 0 || y != 0 ) ) {
			
				// determine new focus
				behavior->Navigate( x, y );
				
			}
		}
	}
}

void UIBehavior::Key( UIBehavior* behavior, void* param, Event* e){
	// only forward event if this object has focus
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		behavior->CallEvent( *e );
	}
}

void UIBehavior::KeyPress( UIBehavior* behavior, void* param, Event* e){
	// only forward event if this object has focus
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		behavior->CallEvent( *e );
	}
}

void UIBehavior::ActiveChanged( UIBehavior* behavior, GameObject* topObject, Event* e ){
	if ( topObject ) {
		// activated
		if ( topObject->active() ) {
			// request layout
			behavior->RequestLayout( ArgValue( e->name ) );
		// if now belong to inactivated object hierarchy
		} else {
			// blur
			behavior->Blur();
			// clear mouse flags
			behavior->mouseOver = false;
			behavior->mouseDown[ 0 ] = behavior->mouseDown[ 1 ] = behavior->mouseDown[ 2 ] = behavior->mouseDown[ 3 ] = false;
		}
	}
}

bool UIBehavior::active( bool a ) {
	// clear focus and flags
	if ( !a ) {
		this->Blur();
		this->mouseOver = false;
		this->mouseDown[ 0 ] = this->mouseDown[ 1 ] = this->mouseDown[ 2 ] = this->mouseDown[ 3 ] = false;
	}
	// return new active
	return this->_active = a;
};

void UIBehavior::Attached( UIBehavior* behavior, GameObject* go, Event* e ){
	
	// remember scene
	behavior->scene = go->GetScene();
	behavior->CheckClipping();
	
	// do layout, if on scene
	if ( behavior->scene ) {
		behavior->RequestLayout( ArgValue( e->name ) );
	}
	
}

void UIBehavior::Detached( UIBehavior* behavior, GameObject* go, Event* e ){
	
	// clear scene
	behavior->scene = NULL;
	
}

