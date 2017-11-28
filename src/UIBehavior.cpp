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
	AddEventCallback( EVENT_NAVIGATION, (BehaviorEventCallback) &UIBehavior::Navigation);
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
UIBehavior::~UIBehavior() {
	
	// make sure we're not in scene's ui list
	if ( this->scene  ) {
		this->scene->UIRemoved( this );
	}
}



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
	
	script.AddProperty<UIBehavior>
	( "navigationLeft",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationLeft ? ui->navigationLeft : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			script.ReportError( ".navigationLeft can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationLeft = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "navigationRight",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationRight ? ui->navigationRight : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			script.ReportError( ".navigationRight can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationRight = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "navigationUp",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationUp ? ui->navigationUp : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			script.ReportError( ".navigationUp can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationUp = other;
		return val;
	}));
	
	script.AddProperty<UIBehavior>
	( "navigationDown",
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){ UIBehavior* ui = (UIBehavior*) b; return ui->navigationDown ? ui->navigationDown : NULL; }),
	 static_cast<ScriptObjectCallback>([](void *b, void* val ){
		UIBehavior* ui = (UIBehavior*) b;
		UIBehavior* other = val ? script.GetInstance<UIBehavior>( val ) : NULL;
		if ( !other && val ) {
			script.ReportError( ".navigationDown can only be set to null or instance of UIBehavior" );
			return (void*) NULL;
		}
		ui->navigationDown = other;
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
	if ( this->gameObject != NULL && this->gameObject->render != NULL ) {
		return this->gameObject->render->IsScreenPointInside( x, y, localX, localY );
	}
	return false;
}


/// change focus to focusable UI that's in the direction x, y from this control
void UIBehavior::Navigate( float x, float y ) {
	
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
		
		// for each UI
		for ( size_t i = 0, nc = this->scene->uiElements.size(); i < nc; i++ ) {
			other = this->scene->uiElements[ i ];
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
	if ( other ) other->Focus();
	
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

void UIBehavior::Navigation( UIBehavior* behavior, Event* e ){
	// if focused
	if ( behavior->scene && behavior->scene->focusedUI == behavior ) {
		
		// event handled
		e->stopped = true;
		
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
		
		// if it's a directional event, and focus didn't change
		if ( behavior->scene->focusedUI == behavior && ( x != 0 || y != 0 ) ) {
		
			// determine new focus
			behavior->Navigate( x, y );
			
		// if it's accept
		} else if ( axisName.compare( app.input.navigationAccept ) ) {
		
			// generate 'click' event
			Event event( EVENT_CLICK );
			event.scriptParams.AddIntArgument( 0 );
			event.scriptParams.AddFloatArgument( 0 );
			event.scriptParams.AddFloatArgument( 0 );
			behavior->CallEvent( event );
			
		}
	}
}

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
	// if now belong to inactivated object hierarchy
	if ( topObject && !topObject->active() ) {
		// blur
		behavior->Blur();
		// clear mouse flags
		behavior->mouseOver = false;
		behavior->mouseDown[ 0 ] = behavior->mouseDown[ 1 ] = behavior->mouseDown[ 2 ] = behavior->mouseDown[ 3 ] = false;
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





