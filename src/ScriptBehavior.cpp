#include "ScriptBehavior.hpp"
#include "GameObject.hpp"
#include "Application.hpp"

/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */

// creating from script
ScriptBehavior::ScriptBehavior( ScriptArguments* args ) : ScriptBehavior() {
	
	// add scriptObject
	script.NewScriptObject<ScriptBehavior>( this );
	
	// with arguments
	if ( args && args->args.size() >= 1) {
		// string - script name
		string scriptName;
		if ( args->ReadArguments( 1, TypeString, &scriptName ) ) {
			script.SetProperty( "script", ArgValue( scriptName.c_str() ), this->scriptObject );
		}
	}
}

// init
ScriptBehavior::ScriptBehavior() {

	// flag
	this->dispatchEventsToPropertyFunctions = true;
	
}

// destructor
ScriptBehavior::~ScriptBehavior() {

	// release resource
	if ( this->scriptResource ) this->scriptResource->AdjustUseCount( -1 );
	
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// Behavior -> script class "Behavior"
SCRIPT_CLASS_NAME( ScriptBehavior, "Script" );

// init script classes
void ScriptBehavior::InitClass() {
	
	// register class
	script.RegisterClass<ScriptBehavior>( "Behavior" );
	
	// props
	
	script.AddProperty<ScriptBehavior>
	( "script",
	 static_cast<ScriptValueCallback>([](void *go, ArgValue ) {
		ScriptBehavior* self = (ScriptBehavior*) go;
		if ( self->scriptResource ) return ArgValue( self->scriptResource->key.c_str() );
		return ArgValue();
	}),
	 static_cast<ScriptValueCallback>([](void *go, ArgValue in ){
		ScriptBehavior* self = (ScriptBehavior*) go;
		
		// setting to null or empty
		if ( in.type != TypeString || !in.value.stringValue->size() ) {
			if ( self->scriptResource ) self->scriptResource->AdjustUseCount( -1 );
			self->scriptResource = NULL;
			return in;
		}
		
		// find
		const char *key = in.value.stringValue->c_str();
		ScriptResource* s = app.scriptManager.Get( key );
		
		// script found
		if ( s->error == ERROR_NONE ) {
			// is different from previous
			if ( self->scriptResource != s ) {
				// adjust use counts
				if ( self->scriptResource ) self->scriptResource->AdjustUseCount( -1 );
				s->AdjustUseCount( 1 );
				// set new
				self->scriptResource = s;
				// execute it, if error
				if ( !script.Execute( s, self->scriptObject ) ) {
					// print error, and fail
					if ( JS_IsExceptionPending( script.js ) )
						JS_ReportPendingException( script.js );
					return ArgValue();
				} else {
					return in;
				}
			}
		} else if ( s->error == ERROR_NOT_FOUND ) {
			printf( ".script path \"%s\" was not found.\n", key );
		}
		return ArgValue();
		
	}), PROP_ENUMERABLE | PROP_SERIALIZED | PROP_LATE );

	
}






