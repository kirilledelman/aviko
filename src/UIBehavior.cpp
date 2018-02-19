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
	// printf( "~UIBehavior\n" );
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
	script.SetProperty( "Grid", ArgValue( (int) LayoutType::Grid ), constants );
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
			script.ReportError( ".navigationLeft can only be set to null or instance of UIBehavior" );
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
			script.ReportError( ".navigationRight can only be set to null or instance of UIBehavior" );
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
			script.ReportError( ".navigationUp can only be set to null or instance of UIBehavior" );
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
			script.ReportError( ".navigationDown can only be set to null or instance of UIBehavior" );
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
		if ( ui->layoutWidth != val ) {
			val = ui->layoutWidth = fmax( 0, val );
			ui->RequestLayout( ArgValue( "width" ) );
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "height", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->layoutHeight; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->layoutHeight != val ) {
			val = ui->layoutHeight = fmax( 0, val );
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
	( "layoutAlign",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->crossAxisAlign; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		LayoutAlign v = (LayoutAlign) val;
		if ( ui->crossAxisAlign != v ) {
			ui->crossAxisAlign = v;
			ui->RequestLayout( ArgValue( "layoutAlign" ) );
		}
		return (int) ui->crossAxisAlign;
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
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( ui->padTop ) ) {
				if ( val.value.arrayValue->size() >= 2 && val.value.arrayValue->at( 1 ).toNumber( ui->padRight ) ) {
					if ( val.value.arrayValue->size() >= 3 && val.value.arrayValue->at( 2 ).toNumber( ui->padBottom ) ) {
						if ( val.value.arrayValue->size() >= 4 )
							val.value.arrayValue->at( 0 ).toNumber( ui->padLeft );
					}
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			ui->padTop = ui->padRight = ui->padBottom = ui->padLeft = sameVal;
		}
		ui->RequestLayout( ArgValue( "pad" ) );
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
		if ( val.type == TypeArray ) {
			if ( val.value.arrayValue->size() >= 1 && val.value.arrayValue->at( 0 ).toNumber( ui->marginTop ) ) {
				if ( val.value.arrayValue->size() >= 2 && val.value.arrayValue->at( 1 ).toNumber( ui->marginRight ) ) {
					if ( val.value.arrayValue->size() >= 3 && val.value.arrayValue->at( 2 ).toNumber( ui->marginBottom ) ) {
						if ( val.value.arrayValue->size() >= 4 )
							val.value.arrayValue->at( 0 ).toNumber( ui->marginLeft );
					}
				}
			}
		} else if ( val.toNumber( sameVal ) ) {
			ui->marginTop = ui->marginRight = ui->marginBottom = ui->marginLeft = sameVal;
		}
		ui->RequestLayout( ArgValue( "margin" ) );
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
	( "resize", // setSize( Number width, Number height )
	 static_cast<ScriptFunctionCallback>([]( void* obj, ScriptArguments& sa ) {
		UIBehavior* self = (UIBehavior*) obj;
		if ( !sa.ReadArguments( 2, TypeFloat, &self->layoutWidth, TypeFloat, &self->layoutWidth ) ) {
			script.ReportError( "usage: resize( Number width, Number height )" );
			return false;
		}
		self->RequestLayout( ArgValue( "resize" ) );
		return true;
	}));
	
	script.DefineFunction<UIBehavior>
	("requestLayout",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
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
		if ( this->gameObject->render != NULL ) {
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

	// not overridden
	if ( !other ) {
		
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
		float maxMargin = fmax( maxX - minX, maxY - minY );
		float extraMargin = fmin( maxX - minX, maxY - minY ) * 0.5;
		while ( !candidates.size() && extraMargin < maxMargin ) {
			for ( size_t i = 0, nc = uiElements.size(); i < nc; i++ ) {
				other = uiElements[ i ];
				if ( other == this || !other->focusable || other->navigationGroup.compare( navigationGroup ) != 0 || !other->Behavior::active() ) continue;
				
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
				
				// add to candidates
				float yy = (minY + (maxY - minY) * 0.5) - (otherMinY + (otherMaxY - otherMinY) * 0.5),
					  xx = (minX + (maxX - minX) * 0.5) - (otherMinX + (otherMaxX - otherMinX) * 0.5);
				candidates[ other ] = sqrt( xx * xx + yy * yy );
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
		
	}
	
	// if candidate is found, focus on it
	if ( other ) {
		other->Focus(); return true;
	}
	
	// nothing found
	return false;
	
}


/* MARK:	-				Layout
 -------------------------------------------------------------------- */

/// layout type defines how this UI lays out its children
/// this function is called after parentUI has already this UI's width and height
void UIBehavior::Layout( UIBehavior *behavior, void *p, Event *event ){

	if ( !behavior->gameObject ) return;
	
	// constrain size
	behavior->layoutWidth = fmax( behavior->minWidth, fmin( ( behavior->maxWidth > 0 ? behavior->maxWidth : 9999999 ), behavior->layoutWidth ) );
	behavior->layoutHeight = fmax( behavior->minHeight, fmin( ( behavior->maxHeight > 0 ? behavior->maxHeight : 9999999 ), behavior->layoutHeight ) );
	
	// remember current, to retrigger layout later if changed
	float	oldWidth = behavior->layoutWidth,
			oldHeight = behavior->layoutHeight,
			oldMinWidth = behavior->minWidth,
			oldMinHeight = behavior->minHeight;
	
	// collect children with UI component
	vector<UIBehavior*> childUIs;
	for ( size_t i = 0, nc = behavior->gameObject->children.size(); i < nc; i++ ){
		UIBehavior* ui = behavior->gameObject->children[ i ]->ui;
		if ( ui != NULL && ui->Behavior::active() && ui->gameObject->active() && !ui->fixedPosition ) childUIs.push_back( ui );
	}
	
	// for each child ui
	float curX = behavior->padLeft;
	float curY = behavior->padRight;
	float maxSize = 0;
	float innerWidth = fmax( 0, behavior->layoutWidth - ( behavior->padLeft + behavior->padRight ) );
	float innerHeight = fmax( 0, behavior->layoutHeight - ( behavior->padTop + behavior->padBottom ) );
	float maxX = 0, maxY = 0;
	
	// first layout step
	
	// measure space used by items
	float totalFlex = 0;
	vector<UIBehavior*> flexed;
	size_t gridRowStartElement = 0;
	for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
		UIBehavior* childUI = childUIs[ i ];
		float x = 0, y = 0, w = 0, h = 0;
		float fw = 0, fh = 0; // used to calculate maxX, maxY
		
		// anchor - based layout
		if ( behavior->layoutType == LayoutType::Anchors ) {
			
			childUI->GetAnchoredPosition( behavior, x, y, w, h );
			fw = w; fh = h;
			
		// vertical list
		} else if ( behavior->layoutType == LayoutType::Vertical ) {
			
			// measure
			x = curX; w = fw = childUI->layoutWidth;
			y = curY + childUI->marginTop;
			if ( childUI->flex ) {
				h = childUI->minHeight;
				flexed.push_back( childUI );
				totalFlex += childUI->flex;
			} else h = childUI->layoutHeight;
			curY += h + childUI->marginTop + childUI->marginBottom + behavior->spacingY;
			fh = h;
			
			// if child is stretchy, use its minWidth for maxX
			LayoutAlign align = behavior->crossAxisAlign;
			if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
			if ( childUI->flex || align == LayoutAlign::Stretch ) fw = childUI->minWidth;
			
		// horizontal list
		} else if ( behavior->layoutType == LayoutType::Horizontal ) {

			// measure
			y = curY; h = fh = childUI->layoutHeight;
			x = curX + childUI->marginLeft;
			if ( childUI->flex ) {
				w = childUI->minWidth;
				flexed.push_back( childUI );
				totalFlex += childUI->flex;
			} else w = childUI->layoutWidth;
			curX += w + childUI->marginLeft + childUI->marginRight + behavior->spacingX;
			fw = w;
			
			// if child is stretchy, use its minHeight for maxY
			LayoutAlign align = behavior->crossAxisAlign;
			if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
			if ( childUI->flex || align == LayoutAlign::Stretch ) fh = childUI->minHeight;
			
		// grid - left to right, row after row
		} else if ( behavior->layoutType == LayoutType::Grid ) {
			
			w = fw = childUI->layoutWidth;
			h = fh = childUI->layoutHeight;
			// new row?
			if ( curX + w + childUI->marginLeft + childUI->marginRight + behavior->spacingX >= innerWidth ) {
				
				// re-align current row vertically to maxSize height
				for ( size_t j = gridRowStartElement; j < i; j++ ) {
					UIBehavior* rowItem = childUIs[ j ];
					GameObject* rowGo = rowItem->gameObject;
					LayoutAlign align = behavior->crossAxisAlign;
					if ( rowItem->selfAlign != LayoutAlign::Default ) align = rowItem->selfAlign;
					switch ( align ) {
						case LayoutAlign::Stretch:
							rowItem->layoutHeight = maxSize - rowItem->marginBottom;
							break;
						case LayoutAlign::End:
							rowGo->SetY( rowGo->GetY() + ( maxSize - rowItem->marginBottom ) - rowItem->layoutHeight );
							break;
						case LayoutAlign::Center:
							rowGo->SetY( rowGo->GetY() + ( ( maxSize - rowItem->marginBottom ) - rowItem->layoutHeight ) * 0.5 );
							break;
						default: break;
					}
				}
				
				// set position to beginning of next row
				gridRowStartElement = i;
				curX = behavior->padLeft;
				curY += maxSize + behavior->spacingY;
				maxSize = h + childUI->marginTop + childUI->marginBottom;
			} else maxSize = fmax( maxSize, h + childUI->marginTop + childUI->marginBottom );
			
			x = curX + childUI->marginLeft;
			y = curY + childUI->marginTop;
			
			if ( childUI->flex ) curX = innerWidth; // flex in grid layout = new row after this child
			else curX += w + childUI->marginLeft + childUI->marginRight + behavior->spacingX;
			
		} else if ( behavior->layoutType == LayoutType::None ){
			
			x = childUI->gameObject->GetX();
			y = childUI->gameObject->GetY();
			w = fw = childUI->layoutWidth;
			h = fh = childUI->layoutHeight;
			
		}
		
		// max extents of children
		maxX = fmax( maxX, x + fw + childUI->marginRight );
		maxY = fmax( maxY, y + fh + childUI->marginBottom );
		
		// set/store
		childUI->gameObject->SetPosition( floorf( x ), floorf( y ) );
		childUI->layoutWidth = ( w );
		childUI->layoutHeight = ( h );

	}
	
	// if grid, finish aligning last row
	if ( behavior->layoutType == LayoutType::Grid ) {
		
		// re-align last row vertically to maxSize height
		for ( size_t j = gridRowStartElement, nc = childUIs.size(); j < nc; j++ ) {
			UIBehavior* rowItem = childUIs[ j ];
			GameObject* rowGo = rowItem->gameObject;
			LayoutAlign align = behavior->crossAxisAlign;
			if ( rowItem->selfAlign != LayoutAlign::Default ) align = rowItem->selfAlign;
			switch ( align ) {
				case LayoutAlign::Stretch:
					rowItem->layoutHeight = maxSize - rowItem->marginBottom;
					break;
				case LayoutAlign::End:
					rowGo->SetY( rowGo->GetY() + ( maxSize - rowItem->marginBottom ) - rowItem->layoutHeight );
					break;
				case LayoutAlign::Center:
					rowGo->SetY( rowGo->GetY() + ( ( maxSize - rowItem->marginBottom ) - rowItem->layoutHeight ) * 0.5 );
					break;
				default: break;
			}
		}
		
	// in vertical and horizontal layouts
	} else if ( behavior->layoutType == LayoutType::Vertical || behavior->layoutType == LayoutType::Horizontal ) {
	
		// distribute leftover extra space to flexed ui's
		float extraSpace = ( behavior->layoutType == LayoutType::Vertical ) ?
							( innerHeight - ( maxY - behavior->padTop ) ) : ( innerWidth - ( maxX - behavior->padLeft ) );
		float spaceLeft = extraSpace, spaceShare = 0;
		for ( size_t i = 0, nf = flexed.size(); i < nf; i++ ) {
			if ( spaceLeft <= 0 ) break;
			UIBehavior* f = flexed[ i ];
			spaceShare = fmin( spaceLeft, extraSpace * ( f->flex / totalFlex ) );
			if ( behavior->layoutType == LayoutType::Vertical ) {
				f->layoutHeight = f->minHeight + spaceShare;
			} else {
				f->layoutWidth = f->minWidth + spaceShare;
			}
			spaceLeft -= spaceShare;
		}
	}
	
	// if fit children is set
	if ( behavior->fitChildren && behavior->layoutType != LayoutType::None && behavior->layoutType != LayoutType::Anchors ) {
		if ( behavior->layoutType == LayoutType::Grid || behavior->layoutType == LayoutType::Vertical || behavior->layoutType == LayoutType::Horizontal ) {
			behavior->layoutHeight = fmax( behavior->layoutHeight, maxY + behavior->padBottom );
			behavior->layoutWidth = fmax( behavior->layoutWidth, maxX + behavior->padRight );
		} else if ( behavior->layoutType == LayoutType::None ) {
			behavior->layoutWidth = fmax( behavior->layoutWidth, maxX );
			behavior->layoutHeight = fmax( behavior->layoutHeight, maxY );
		}
		behavior->minWidth = maxX + behavior->padRight;
		behavior->minHeight = maxY + behavior->padBottom;
	}
	
	// enforce min/max again
	behavior->layoutWidth = fmax( behavior->minWidth, fmin( ( behavior->maxWidth > 0 ? behavior->maxWidth : 9999999 ), behavior->layoutWidth ) );
	behavior->layoutHeight = fmax( behavior->minHeight, fmin( ( behavior->maxHeight > 0 ? behavior->maxHeight : 9999999 ), behavior->layoutHeight ) );

	// horizontal and vertical layouts require second pass
	if ( behavior->layoutType == LayoutType::Vertical || behavior->layoutType == LayoutType::Horizontal ) {
	
		curX = behavior->padLeft;
		curY = behavior->padTop;
		innerWidth = behavior->layoutWidth - ( behavior->padLeft + behavior->padRight );
		innerHeight = behavior->layoutHeight - ( behavior->padTop + behavior->padBottom );
		for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
			UIBehavior* childUI = childUIs[ i ];
			float x = 0, y = 0, w = 0, h = 0;
			// vertical list
			if ( behavior->layoutType == LayoutType::Vertical ) {
				// measure
				y = curY + childUI->marginTop;
				w = childUI->layoutWidth;
				h = childUI->layoutHeight;
				LayoutAlign align = behavior->crossAxisAlign;
				if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
				switch ( align ) {
					case LayoutAlign::Stretch:
						x = behavior->padLeft + childUI->marginLeft;
						w = innerWidth - (childUI->marginLeft + childUI->marginRight);
						break;
					case LayoutAlign::Default:
					case LayoutAlign::Start:
						x = behavior->padLeft + childUI->marginLeft;
						break;
					case LayoutAlign::End:
						x = behavior->padLeft + (innerWidth - ( w + childUI->marginRight ) );
						break;
					case LayoutAlign::Center:
						x = behavior->padLeft + (innerWidth - w) * 0.5;
						break;
						
				}
				curY += h + childUI->marginTop + childUI->marginBottom + behavior->spacingY;
				
			// horizontal list
			} else if ( behavior->layoutType == LayoutType::Horizontal ) {
				
				// measure
				x = curX + childUI->marginLeft;
				w = childUI->layoutWidth;
				h = childUI->layoutHeight;
				LayoutAlign align = behavior->crossAxisAlign;
				if ( childUI->selfAlign != LayoutAlign::Default ) align = childUI->selfAlign;
				switch ( align ) {
					case LayoutAlign::Stretch:
						y = behavior->padTop + childUI->marginTop;
						h = innerHeight - ( childUI->marginTop + childUI->marginBottom );
						break;
					case LayoutAlign::Default:
					case LayoutAlign::Start:
						y = behavior->padTop + childUI->marginTop;
						break;
					case LayoutAlign::End:
						y = behavior->padTop + (innerHeight - ( h + childUI->marginBottom ) );
						break;
					case LayoutAlign::Center:
						y = behavior->padTop + (innerHeight - h) * 0.5;
						break;
						
				}
				curX += w + childUI->marginLeft + childUI->marginRight + behavior->spacingX;
				
			}
			
			// set/store
			childUI->gameObject->SetPosition( floorf( x ), floorf( y ) );
			childUI->layoutWidth = floorf( w );
			childUI->layoutHeight = floorf( h );
			
		}
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
	
	// dispatch layout event on this component
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
	if ( behavior->layoutWidth != oldWidth || behavior->layoutHeight != oldHeight ||
			behavior->minWidth != oldMinWidth || behavior->minHeight != oldMinHeight ) {
		// request layout again
		behavior->RequestLayout( ArgValue() );
	}
	
}

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

void UIBehavior::RequestLayout( ArgValue trigger ) {

	//
	// UIBehavior::Layout( this, NULL, NULL );
	
	// dispatch down
	GameObject* top = this->gameObject ? this->gameObject->GetScene() : NULL;
	if ( top ) {
		// printf( "--layout requested-- (%s)\n", ( trigger.type == TypeString ? trigger.value.stringValue->c_str() : NULL ) );
		ArgValueVector* params = app.AddLateEvent( top, EVENT_LAYOUT, true );
		if ( trigger.type != TypeUndefined ) {
			params->push_back( trigger );
			params->push_back( ArgValue( this->scriptObject ) );
		}
	}
	
	
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

