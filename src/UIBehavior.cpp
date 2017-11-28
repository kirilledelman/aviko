#include "UIBehavior.hpp"
#include "GameObject.hpp"
#include "Application.hpp"
#include "Scene.hpp"

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
	AddEventCallback( EVENT_ATTACHED, (BehaviorEventCallback) &UIBehavior::Attached );
	AddEventCallback( EVENT_DETACHED, (BehaviorEventCallback) &UIBehavior::Detached );
	AddEventCallback( EVENT_ADDED_TO_SCENE, (BehaviorEventCallback) &UIBehavior::AddedToScene );
	AddEventCallback( EVENT_REMOVED_FROM_SCENE, (BehaviorEventCallback) &UIBehavior::RemovedFromScene );
	AddEventCallback( EVENT_ACTIVE_CHANGED, (BehaviorEventCallback) &UIBehavior::ActiveChanged );
	
	// flag
	this->isUIBehavior = true;
	this->dispatchEventsToPropertyFunctions = true;
}

// destructor
UIBehavior::~UIBehavior() {}



/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// Behavior -> script class "Behavior"
SCRIPT_CLASS_NAME( UIBehavior, "UI" );

// init script classes
void UIBehavior::InitClass() {
	
	// register class
	script.RegisterClass<UIBehavior>( "Behavior" );
	
	// props
	
	script.AddProperty<UIBehavior>
	( "isMouseOver", // has to be that to prevent overlap w .mouseOver = func()
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
	
	// functions
	
	script.DefineFunction<UIBehavior>
	("focus",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		self->Focus();
		return true;
	} ));

	script.DefineFunction<UIBehavior>
	("blur",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		UIBehavior* self = (UIBehavior*) p;
		self->Blur();
		return true;
	} ));

}


/* MARK:	-				Focus
 -------------------------------------------------------------------- */

void UIBehavior::Focus() {
	
	// if can't be focused, return
	if ( !this->focusable || !this->scene || !this->gameObject || this->gameObject->orphan ) return;
	
	// get current focus
	UIBehavior* current = this->scene->focusedUI;
	
	// already focused?
	if ( current == this ) return;
	
	// blur previous
	if ( current ) current->Blur();
	
	// set focus
	this->scene->focusedUI = this;
	
	// dispatch event
	Event event( EVENT_FOCUS );
	event.scriptParams.AddObjectArgument( this->scriptObject );
	this->CallEvent( event );
	
}

void UIBehavior::Blur() {
	
	// if can't be focused, return
	if ( !this->focusable || !this->scene || !this->gameObject ) return;
	
	// get current focus
	UIBehavior* current = this->scene->focusedUI;
	
	// not focused?
	if ( current != this ) return;
	
	// clear focus
	this->scene->focusedUI = NULL;
	
	// dispatch event
	Event event( EVENT_BLUR );
	event.scriptParams.AddObjectArgument( this->scriptObject );
	this->CallEvent( event );
	
}


/* MARK:	-				Control
 -------------------------------------------------------------------- */


bool UIBehavior::IsScreenPointInBounds( float x, float y, float* localX, float* localY ) {
	// ask gameObject's RenderBehavior if screen point is inside object
	if ( this->gameObject != NULL && this->gameObject->render != NULL ) {
		return this->gameObject->render->IsScreenPointInside( x, y, localX, localY );
	}
	return false;
}


/* MARK:	-				Events
 -------------------------------------------------------------------- */

void UIBehavior::MouseMove( UIBehavior* behavior, Event* e){
	float x = e->scriptParams.args[ 0 ].value.floatValue;
	float y = e->scriptParams.args[ 1 ].value.floatValue;
	float localX, localY;
	bool inBounds = behavior->IsScreenPointInBounds( x, y, &localX, &localY );
	
	// entered bounds
	if ( inBounds && !behavior->mouseOver ) {
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOVER );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		behavior->CallEvent( event );
		
		// exited bounds
	} else if ( !inBounds && behavior->mouseOver ) {
		
		// dispatch a rollover event
		Event event( EVENT_MOUSEOUT );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		behavior->CallEvent( event );
		
	}
	behavior->mouseOver = inBounds;
}

void UIBehavior::MouseButton( UIBehavior* behavior, Event* e){
	float x = e->scriptParams.args[ 1 ].value.floatValue;
	float y = e->scriptParams.args[ 2 ].value.floatValue;
	int btn = e->scriptParams.args[ 0 ].value.intValue;
	float localX, localY;
	bool inBounds = behavior->IsScreenPointInBounds( x, y, &localX, &localY );
	bool down = ( strcmp( e->name, EVENT_MOUSEDOWN ) == 0 );
	
	if ( inBounds ) {
		// dispatch mousedown or mouseup
		Event event( e->name );
		event.scriptParams.AddIntArgument( btn );
		event.scriptParams.AddFloatArgument( localX );
		event.scriptParams.AddFloatArgument( localY );
		behavior->CallEvent( event );
		
		// mouse button was released
		if ( behavior->mouseDown[ btn ] && !down ){
			event.SetName( EVENT_CLICK );
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
			behavior->CallEvent( event );
		}
		
		// clear down state
		behavior->mouseDown[ btn ] = down;
	}
	
}

void UIBehavior::MouseWheel( UIBehavior* behavior, Event* e){

	// forward event if mouse is over
	if ( behavior->mouseOver ) {
		behavior->CallEvent( *e );
	}

}

void UIBehavior::Navigation( UIBehavior* behavior, Event* e){}

void UIBehavior::Key( UIBehavior* behavior, Event* e){
	// only forward event if this object has focus
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		behavior->CallEvent( *e );
	}
}

void UIBehavior::KeyPress( UIBehavior* behavior, Event* e){
	// only forward event if this object has focus
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		behavior->CallEvent( *e );
	}
}

void UIBehavior::ActiveChanged( UIBehavior* behavior, GameObject* topObject ){
	
	// todo - reset state, and blur, if deactivated
	
	// focused?
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		
		
	}
	
}

void UIBehavior::AddedToScene( UIBehavior* behavior, GameObject* topObject ){
	
	// add behavior to list of UIs on scene
	behavior->scene = topObject->GetScene();
	if ( behavior->scene ) behavior->scene->UIAdded( behavior );
	
}

void UIBehavior::RemovedFromScene( UIBehavior* behavior, GameObject* topObject ){

	// remove behavior from list of UIs on scene
	if ( behavior->scene ) behavior->scene->UIRemoved( behavior );
	
}

void UIBehavior::Attached( UIBehavior* behavior, GameObject* go ){
	
	// add behavior to list of UIs on scene
	if ( !go->orphan ) {
		behavior->scene = go->GetScene();
		if ( behavior->scene ) behavior->scene->UIAdded( behavior );
	}
	
}

void UIBehavior::Detached( UIBehavior* behavior, GameObject* go ){
	
	// remove behavior from list of UIs on scene
	if ( behavior->scene ) behavior->scene->UIRemoved( behavior );
	
}





