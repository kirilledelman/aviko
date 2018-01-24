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
	
	// reset layout props
	this->layoutX = FLT_MAX;
	this->layoutY = FLT_MAX;
	
}

// destructor
UIBehavior::~UIBehavior() {
	printf( "~UIBehavior\n" );
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
	
	// props
	
	script.AddProperty<UIBehavior>
	( "over", // 
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->mouseOver; }));

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
	( "anchorLeft", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->anchorLeft; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->anchorLeft != val ) {
			ui->anchorLeft = val;
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
		}
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "layoutType",
	 static_cast<ScriptIntCallback>([](void *b, int val ){ return (int) ((UIBehavior*) b)->layoutType; }),
	 static_cast<ScriptIntCallback>([](void *b, int val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->layoutType != val ) {
			ui->layoutType = (LayoutType) val;
			ui->RequestLayout();
		}
		return ui->layoutType;
	}) );
	
	script.AddProperty<UIBehavior>
	( "expandChildren",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->layoutExpandCrossAxis; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->layoutExpandCrossAxis != val ) {
			ui->layoutExpandCrossAxis = val;
			ui->RequestLayout();
		}
		return ui->layoutExpandCrossAxis;
	 }));
	
	script.AddProperty<UIBehavior>
	( "fitChildren",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((UIBehavior*) b)->fitChildren; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->fitChildren != val ) {
			ui->fitChildren = val;
			ui->RequestLayout();
		}
		return ui->fitChildren;
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
		ui->RequestLayout();
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<UIBehavior>
	( "padTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->padTop; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->padTop != val ) {
			ui->padTop = val;
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
		ui->RequestLayout();
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE  );
	
	script.AddProperty<UIBehavior>
	( "marginTop", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->marginTop; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->marginTop != val ) {
			ui->marginTop = val;
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
			ui->RequestLayout();
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
		ui->RequestLayout();
		return val;
	}), PROP_ENUMERABLE | PROP_NOSTORE  );
	
	script.AddProperty<UIBehavior>
	( "spacingX", //
	 static_cast<ScriptFloatCallback>([](void *b, float val ){ return ((UIBehavior*) b)->spacingX; }),
	 static_cast<ScriptFloatCallback>([](void *b, float val ){
		UIBehavior* ui = (UIBehavior*) b;
		if ( ui->spacingX != val ) {
			ui->spacingX = val;
			ui->RequestLayout();
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
			ui->RequestLayout();
		}
		return val;
	}));
	
	// functions
	
	/*script.DefineFunction<UIBehavior>
	("triggerLayout",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		Event e(EVENT_LAYOUT);
		self->
		return self->Focus();
	} ));*/
	
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
		
		// get 0, 0 of this control in global coords
		float localX, localY;
		this->gameObject->ConvertPoint( 0, 0, localX, localY, true );
		
		// set of control/distance from this control
		unordered_map<UIBehavior*,float> candidates;
		
		// for each UI in scene
		vector<UIBehavior*> uiElements;
		this->scene->GetBehaviors( true, uiElements );		
		for ( size_t i = 0, nc = uiElements.size(); i < nc; i++ ) {
			other = uiElements[ i ];
			if ( other == this || !other->focusable || !other->Behavior::active() ) continue;
			
			// check if it's in the correct direction
			float otherX, otherY;
			other->gameObject->ConvertPoint( 0, 0, otherX, otherY, true );
			
			// skip if in wrong dir
			if ( ( dir == 0 && localY <= otherY ) ||
				( dir == 1 && localX >= otherX ) ||
				( dir == 2 && localY >= otherY ) ||
				( dir == 3 && localX <= otherX ) ) continue;
			
			// add to candidates
			float xx = localX - otherX, yy = localY - otherY;
			candidates[ other ] = sqrt( xx * xx + yy * yy );
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
/// * call this behavior's script layout event handler with x, y, w, h
/// * compute children's desired positions and sizes
/// * Behavior Layout event will propagate to each child after function ends
void UIBehavior::Layout( UIBehavior *behavior, void *p, Event *event ){

	if ( !behavior->gameObject ) return;
	
	// if this behavior doesn't have a parentUI / x, y are undefined
	if ( behavior->layoutX == FLT_MAX || !behavior->gameObject->parent || !behavior->gameObject->parent->ui ) {
		// copy from gameObject
		behavior->layoutX = behavior->gameObject->GetX();
		behavior->layoutY = behavior->gameObject->GetY();
	}
	
	// constrain size
	behavior->layoutWidth = fmax( behavior->minWidth, fmin( behavior->maxWidth, behavior->layoutWidth ) );
	behavior->layoutHeight = fmax( behavior->minHeight, fmin( behavior->maxHeight, behavior->layoutHeight ) );
	
	// collect children with UI component
	vector<UIBehavior*> childUIs;
	for ( size_t i = 0, nc = behavior->gameObject->children.size(); i < nc; i++ ){
		UIBehavior* ui = behavior->gameObject->children[ i ]->ui;
		if ( ui != NULL ) childUIs.push_back( ui );
	}
	
	// for each child ui
	float curX = behavior->padLeft;
	float curY = behavior->padTop;
	float maxSize = 0;
	float innerWidth = behavior->layoutWidth - ( behavior->padLeft + behavior->padRight );
	float innerHeight = behavior->layoutHeight - ( behavior->padTop + behavior->padBottom );
	float maxX = 0, maxY = 0;
	for ( size_t i = 0, nc = childUIs.size(); i < nc; i++ ){
		
		UIBehavior* childUI = childUIs[ i ];
		float x = 0, y = 0, w = 0, h = 0;
		
		if ( !childUI->Behavior::active() || !childUI->gameObject->active() ) continue;
		
		// anchor - based layout
		if ( behavior->layoutType == LayoutType::Anchors ) {
			
			childUI->GetAnchoredPosition( behavior, x, y, w, h );
			
		} else if ( behavior->layoutType == LayoutType::Vertical ) {
			
			x = behavior->padLeft + childUI->marginLeft;
			y = curY + childUI->marginTop;
			h = childUI->layoutHeight;
			w = behavior->layoutExpandCrossAxis ? (innerWidth - childUI->marginLeft - childUI->marginRight) : childUI->layoutWidth;
			curY += h + childUI->marginTop + childUI->marginBottom + behavior->spacingY;
			
		} else if ( behavior->layoutType == LayoutType::Horizontal ) {
			
			x = curX + childUI->marginLeft;
			y = behavior->padTop + childUI->marginTop;
			w = childUI->layoutWidth;
			h = behavior->layoutExpandCrossAxis ? (innerHeight - childUI->marginTop - childUI->marginBottom) : childUI->layoutHeight;
			curX += w + childUI->marginLeft + childUI->marginRight + behavior->spacingX;
			
		} else if ( behavior->layoutType == LayoutType::Grid ) {
			
			w = childUI->layoutWidth;
			h = childUI->layoutHeight;
			
			// new row?
			if ( curX + w + childUI->marginLeft + childUI->marginRight + behavior->spacingX >= innerWidth ) {
				
				curX = behavior->padLeft;
				curY += maxSize + behavior->spacingY;
				maxSize = h + childUI->marginTop + childUI->marginBottom;
				
			} else maxSize = fmax( maxSize, h + childUI->marginTop + childUI->marginBottom );
			
			x = curX + childUI->marginLeft;
			y = curY + childUI->marginTop;
			curX += w + childUI->marginLeft + childUI->marginRight + behavior->spacingX;
			
		} else if ( behavior->layoutType == LayoutType::None ){
		
			x = childUI->gameObject->GetX();
			y = childUI->gameObject->GetY();
			w = childUI->layoutWidth;
			h = childUI->layoutHeight;
			
		}
		
		// measure max
		maxX = fmax( maxX, x + w + childUI->marginRight );
		maxY = fmax( maxY, y + h + childUI->marginBottom );
		
		// store,
		childUI->layoutX = x;
		childUI->layoutY = y;
		childUI->layoutWidth = w;
		childUI->layoutHeight = h;
		
	}
	
	// if fit children is set
	if ( behavior->fitChildren ) {
		if ( behavior->layoutType == LayoutType::Grid || behavior->layoutType == LayoutType::Vertical ) {
			behavior->layoutHeight = curY + behavior->padBottom;
		} else if ( behavior->layoutType == LayoutType::Horizontal ) {
			behavior->layoutWidth = curX + behavior->padRight;
		} else if ( behavior->layoutType == LayoutType::None ) {
			behavior->layoutWidth = maxX;
			behavior->layoutHeight = maxY;
		}
	}
	
	// enforce min
	behavior->layoutWidth = fmax( behavior->layoutWidth, behavior->minWidth );
	behavior->layoutHeight = fmax( behavior->layoutHeight, behavior->minHeight );
	
	// dispatch layout event on this component
	Event e ( EVENT_LAYOUT );
	e.scriptParams.AddFloatArgument( behavior->layoutX );
	e.scriptParams.AddFloatArgument( behavior->layoutY );
	e.scriptParams.AddFloatArgument( behavior->layoutWidth );
	e.scriptParams.AddFloatArgument( behavior->layoutHeight );
	behavior->CallEvent( e );
	
}

void UIBehavior::GetAnchoredPosition( UIBehavior* parentUI, float& x, float& y, float& w, float& h ) {
	
	// use previous / default
	x = layoutX;
	y = layoutY;
	w = fmax( layoutWidth, minWidth );
	h = fmax( layoutHeight, minHeight );
	
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
	
}


void UIBehavior::RequestLayout() {

	Scene* scene = this->gameObject ? this->gameObject->GetScene() : NULL;
	if ( scene ) app.AddLateEvent( scene, EVENT_LAYOUT, true );
	
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
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOVER );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		behavior->CallEvent( event );
	// exited bounds
	} else if ( !inBounds && behavior->mouseOver ) {
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOUT );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		event.scriptParams.AddFloatArgument( x );
		event.scriptParams.AddFloatArgument( y );
		behavior->CallEvent( event );
	}
	behavior->mouseOver = inBounds;
	
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
			
			// if it's a directional event, and focus didn't change
			if ( behavior->scene->focusedUI == behavior && ( x != 0 || y != 0 ) ) {
			
				// determine new focus
				behavior->Navigate( x, y );
				
			// if it's accept
			} /* else if ( axisName.compare( app.input.navigationAccept ) == 0 ) {
			
				// generate 'click' event
				Event event( EVENT_CLICK );
				event.scriptParams.AddIntArgument( 0 );
				event.scriptParams.AddFloatArgument( 0 );
				event.scriptParams.AddFloatArgument( 0 );
				behavior->CallEvent( event );
				
			} */
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
			topObject = topObject->parent ? topObject->parent : topObject;
			app.AddLateEvent( topObject, EVENT_LAYOUT );
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
		Event event( EVENT_LAYOUT );
		behavior->gameObject->DispatchEvent( event, true );
	}
	
}

void UIBehavior::Detached( UIBehavior* behavior, GameObject* go, Event* e ){
	
	// clear scene
	behavior->scene = NULL;
	
}

