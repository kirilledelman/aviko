#include "BodyBehavior.hpp"
#include "Scene.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// no 'new BodyBehavior()'
BodyBehavior::BodyBehavior( ScriptArguments* ) { script.ReportError( "BodyBehavior can't be created using 'new'." ); }

/// default constructor
BodyBehavior::BodyBehavior() {
	
	// register event functions
	AddEventCallback( EVENT_ATTACHED, (BehaviorEventCallback) &BodyBehavior::Attached );
	AddEventCallback( EVENT_DETACHED, (BehaviorEventCallback) &BodyBehavior::Detached );
	AddEventCallback( EVENT_ADDEDTOSCENE, (BehaviorEventCallback) &BodyBehavior::Attached );
	AddEventCallback( EVENT_REMOVEDFROMSCENE, (BehaviorEventCallback) &BodyBehavior::Detached );
	AddEventCallback( EVENT_ACTIVECHANGED, (BehaviorEventCallback) &BodyBehavior::ActiveChanged );
	
	// is body
	this->isBodyBehavior = true;
	
};

//
BodyBehavior::~BodyBehavior() {};


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void BodyBehavior::InitClass() {
	
	// register class
	script.RegisterClass<BodyBehavior>( "Behavior", true );
	
	// properties
	
	script.AddProperty<BodyBehavior>
	("category", //
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		ArgValue ret;
		BodyBehavior::BitsToValue( ((BodyBehavior*) p)->categoryBits, ret );
		return ret;
	 }),
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		((BodyBehavior*) p)->categoryBits = BodyBehavior::ValueToBits( val );
		return val;
	 }));
	
	script.AddProperty<BodyBehavior>
	("mask", //
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		ArgValue ret;
		BodyBehavior::BitsToValue( ~((BodyBehavior*) p)->maskBits, ret );
		return ret;
	}),
	 static_cast<ScriptValueCallback>([](void* p, ArgValue val ){
		((BodyBehavior*) p)->maskBits = ~BodyBehavior::ValueToBits( val );
		return val;
	}));
	
	// functions
	
}


/* MARK:	-				Attached / removed / active
 -------------------------------------------------------------------- */


/// attach/detach from a gameObject
bool BodyBehavior::SetGameObject( GameObject* go ) {
	
	if ( go && go != this->gameObject && dynamic_cast<Scene*>(go) ) {
		script.ReportError( "Scene can not have a physics body." );
		return false;
	}
	
	// removing from object
	if ( this->gameObject && this->gameObject != go ) {
		// stamp object local coords
		this->gameObject->DirtyTransform();
		this->gameObject->Transform();
	}
	
	if ( go ) { go->DirtyTransform(); go->Transform(); }
	
	// base
	bool r = Behavior::SetGameObject( go );
	if ( r && go ) go->DirtyTransform();
	return r;
}

// attached to scene callback
void BodyBehavior::Attached( BodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// add body to world
	behavior->AddBody( target->GetScene() );
	
}

/// detached from scene callback
void BodyBehavior::Detached( BodyBehavior *behavior, GameObject* target, Event* event ) {
	
	// remove from world
	behavior->RemoveBody();
	
}

/// overrides behavior active setter
bool BodyBehavior::active( bool a ) {
	
	this->_active = a;
	if ( this->gameObject ) this->EnableBody( _active && this->gameObject->active() );
	return a;
	
}

/// active changed callback
void BodyBehavior::ActiveChanged( BodyBehavior* behavior, GameObject* target, Event* event ) {
	
	// set its active status to combination of active + on scene
	behavior->EnableBody( behavior->_active && behavior->gameObject->active() );
	
}


/* MARK:	-				Masking helpers
 -------------------------------------------------------------------- */


// converts int, or array of ints to flags integer
void BodyBehavior::BitsToValue( uint32 v, ArgValue& ret ) {
	// zero is zero
	if ( v == 0 ) {
		ret.type = TypeInt;
		ret.value.intValue = 0;
		return;
	}
	
	// make array
	ret.type = TypeArray;
	ret.value.arrayValue = new ArgValueVector();
	
	// loop over bits, add set bit numbers to array
	for ( uint32 i=1, iter=1; iter <= 32; i <<= 1, iter++ ){
		if ( v & i ) ret.value.arrayValue->emplace_back( (int) iter );
	}
	
	// single, or no value?
	size_t numBits = ret.value.arrayValue->size();
	if ( numBits == 1 ) {
		int val = ( numBits == 1 ? ret.value.arrayValue->at( 0 ).value.intValue : 0 );
		delete ret.value.arrayValue;
		ret.type = TypeInt;
		ret.value.intValue = val;
	}
}

// converts flags or mask to value array or single int
uint32 BodyBehavior::ValueToBits( ArgValue& value ) {

	// check for invalid set, or 0
	int x = 0;
	if ( (value.type != TypeArray && !value.toInt( x )) || x <= 0 || x > 32 ) {
		return 0;
	}
	
	// a single number
	if ( value.type != TypeArray ) {
		return 1 << ( x - 1 );
	}
	
	// array
	uint32 ret = 0;
	for ( size_t i = 0, na = value.value.arrayValue->size(); i < na; i++ ){
		if ( value.toInt( x ) && x > 0 && x <= 32 ) {
			ret |= ( 1 << ( x - 1 ) );
		}
	}
	return ret;
	
}



