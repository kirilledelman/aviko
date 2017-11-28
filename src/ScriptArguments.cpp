#include "ScriptArguments.hpp"
#include "ScriptHost.hpp"

/* MARK:	-				ScriptFunction
 -------------------------------------------------------------------- */

/// constructor
ScriptFunctionObject::ScriptFunctionObject( void* scriptFunc, bool once ) : callOnce( once ) {
	this->funcObject = (JSObject*) scriptFunc;
	JS_AddObjectRoot( script.js, &this->funcObject );
}

/// destructor
ScriptFunctionObject::~ScriptFunctionObject() {
	if ( script.js ) {
		JS_RemoveObjectRoot( script.js, &this->funcObject );
	}
}

/// invoke function with arguments
void ScriptFunctionObject::Invoke( ScriptArguments &args, void* thisObject ){
	jsval rval;
	int argc;
	jsval* params = args.GetFunctionArguments( &argc );
	JS_CallFunction( script.js, thisObject ? (JSObject*) thisObject : script.global_object, (JSFunction*) this->funcObject, argc, params, &rval );
}


/* MARK:	-				Arguments
 -------------------------------------------------------------------- */


// easy value return from a function registered with ScriptHost::AddFunction
void ScriptArguments::ReturnUndefined() { this->callArgs->rval().setUndefined(); }
void ScriptArguments::ReturnNull() { this->callArgs->rval().setNull(); }
void ScriptArguments::ReturnBool( bool val ) { this->callArgs->rval().setBoolean( val ); }
void ScriptArguments::ReturnInt( int val ) { this->callArgs->rval().setInt32( val ); }
void ScriptArguments::ReturnFloat( float val ) { this->callArgs->rval().setDouble( val ); }
void ScriptArguments::ReturnString( string val ) { RootedString rs( script.js, JS_NewStringCopyZ( script.js, val.c_str() ) ); this->callArgs->rval().setString( rs ); }
void ScriptArguments::ReturnString( const char* val ) { RootedString rs( script.js, JS_NewStringCopyZ( script.js, val ) ); this->callArgs->rval().setString( rs ); }
void ScriptArguments::ReturnObject( void* val ) { this->callArgs->rval().setObjectOrNull( (JSObject*) val ); }
void ScriptArguments::ReturnValue( ArgValue val ) { this->callArgs->rval().set( val.toValue() ); }
void ScriptArguments::ReturnArray( ArgValueVector &arr ){ this->callArgs->rval().set( ArrayToVal( arr ) ); }

// easy add arguments for script function call e.g. dispatching events on script objects
void ScriptArguments::ResizeArguments( int len ) {
	this->args.resize( len );
	this->funcArgs()->resize( len );
}
void ScriptArguments::AddIntArgument( int val ){ this->args.emplace_back( val ); jsval jv; jv.setInt32( val ); this->funcArgs()->append( jv ); }
void ScriptArguments::AddBoolArgument( bool val ){ this->args.emplace_back( val ); jsval jv; jv.setBoolean( val ); this->funcArgs()->append( jv ); }
void ScriptArguments::AddFloatArgument( float val ){ this->args.emplace_back( val ); jsval jv; jv.setDouble( val ); this->funcArgs()->append( jv ); }
void ScriptArguments::AddObjectArgument( void* val ){ this->args.emplace_back( val ); jsval jv; jv.setObjectOrNull( (JSObject*) val ); this->funcArgs()->append( jv ); }
void ScriptArguments::AddStringArgument( const char* val ){ this->args.emplace_back( val ); jsval jv; jv.setString( JS_NewStringCopyZ( script.js, val ) ); this->funcArgs()->append( jv ); }
void ScriptArguments::AddArgument( ArgValue val ){ this->args.emplace_back( val ); this->funcArgs()->append( val.toValue() ); }

AutoValueVector* ScriptArguments::funcArgs() {
	if ( !this->_funcArgs ) this->_funcArgs = new AutoValueVector( script.js );
	return this->_funcArgs;
}

// recursive array to jsval convert
jsval ScriptArguments::ArrayToVal( vector<ArgValue> &arr ) {
	int len = (int) arr.size();
	// populate
	jsval* values = (jsval*) malloc( sizeof(jsval) * len );
	memset( values, 0, sizeof( sizeof(jsval) * len ) );
	for ( size_t i = 0; i < len; i++ ) {
		ArgValue& v = arr[ i ];
		if ( v.type == TypeBool ) {
			values[ i ].setBoolean( v.value.boolValue );
		} else if ( v.type == TypeFloat ) {
			values[ i ].setDouble( v.value.floatValue );
		} else if ( v.type == TypeInt ) {
			values[ i ].setInt32( v.value.intValue );
		} else if ( v.type == TypeObject ) {
			values[ i ].setObjectOrNull( (JSObject*) v.value.objectValue );
		} else if ( v.type == TypeString ){
			JSString* s = JS_NewStringCopyZ( script.js, v.value.stringValue->c_str() );
			values[ i ].setString( s );
		} else if ( v.type == TypeArray ){
			values[ i ] = ArrayToVal( *v.value.arrayValue );
		} else {
			values[ i ].setUndefined();
		}
	}
	
	// new array
	JSObject* obj = JS_NewArrayObject( script.js, len, values );
	free( values );
	jsval ret;
	ret.setObjectOrNull( obj );
	return ret;
}

/// constructor
ScriptArguments::ScriptArguments(){
	// init vector holding script func arguments
	
}

/// destructor
ScriptArguments::~ScriptArguments(){
	// delete vector holding script func arguments
	if ( this->_funcArgs ) delete this->_funcArgs;
}

/// creates new arguments wrapper from CallArgs
ScriptArguments::ScriptArguments( CallArgs* ca ) : ScriptArguments() {
	// if passed callargs
	callArgs = ca;
	if ( callArgs ) {
		// copy and convert arguments
		unsigned len = ca->length();
		for ( unsigned i = 0; i < len; i++ ) {
			*this->args.emplace( this->args.end(), ca->get( i ) );
		}
	}
	
}

/// constructs and returns an array of jsvals, suitable for script function call
jsval* ScriptArguments::GetFunctionArguments( int* argc ) {
	// return size
	*argc = (int) this->funcArgs()->length();
	// return first elem
	return this->_funcArgs->begin();
}

bool ScriptArguments::ReadArguments( int minRequired,
									ScriptType type0, void* value0,
									ScriptType type1, void* value1,
									ScriptType type2, void* value2,
									ScriptType type3, void* value3,
									ScriptType type4, void* value4,
									ScriptType type5, void* value5,
									ScriptType type6, void* value6 ) {
	// fail if not enough args
	if ( args.size() < minRequired ) return false;
	
	// read args
	if ( !args[ 0 ].get( value0, type0 ) ) return false;
	if ( args.size() >= 2 && value1 != NULL && !args[ 1 ].get( value1, type1 ) ) return false;
	if ( args.size() >= 3 && value2 != NULL && !args[ 2 ].get( value2, type2 ) ) return false;
	if ( args.size() >= 4 && value3 != NULL && !args[ 3 ].get( value3, type3 ) ) return false;
	if ( args.size() >= 5 && value4 != NULL && !args[ 4 ].get( value4, type4 ) ) return false;
	if ( args.size() >= 6 && value5 != NULL && !args[ 5 ].get( value5, type5 ) ) return false;
	if ( args.size() >= 7 && value6 != NULL && !args[ 6 ].get( value6, type6 ) ) return false;

	return true;
}

void* ScriptArguments::GetThis() {
	
	if ( !this->callArgs ) return NULL;
	jsval thisVal = this->callArgs->computeThis( script.js );
	if ( thisVal.isObjectOrNull() ) return thisVal.toObjectOrNull();
	return NULL;
}

/* MARK:	-				ArgValue
 -------------------------------------------------------------------- */


// copy constructor
ArgValue::ArgValue( const ArgValue& copyFrom ) {
	this->type = copyFrom.type;
	this->value = copyFrom.value;
	if ( type == TypeString ) {
		this->value.stringValue = new string();
		*this->value.stringValue = *copyFrom.value.stringValue;
	} else if ( type == TypeArray ) {
		this->value.arrayValue = new ArgValueVector();
		*this->value.arrayValue = *copyFrom.value.arrayValue;
		this->arrayObject = copyFrom.arrayObject;
	}
}

// convert constructor
ArgValue::ArgValue( jsval val ) {
	type = TypeUndefined; value.intValue = 0;
	if ( val.isBoolean() ) {
		type = TypeBool;
		value.boolValue = val.toBoolean();
	} else if ( val.isInt32() ) {
		type = TypeInt;
		value.intValue = val.toInt32();
	} else if ( val.isDouble() ) {
		type = TypeFloat;
		value.floatValue = (float) val.toDouble();
	} else if ( val.isString() ) {
		type = TypeString;
		char *buf = JS_EncodeString( script.js, val.toString() );
		value.stringValue = new string( buf );
		JS_free( script.js, (void*) buf );
	} else if ( val.isObjectOrNull() ) {
		if ( val.isObject() ){
			JSObject* objVal = val.toObjectOrNull();
			if ( JS_IsArrayObject( script.js, objVal ) ) {
				type = TypeArray;
				arrayObject = objVal;
				value.arrayValue = new ArgValueVector();
				uint32_t len = 0;
				JS_GetArrayLength( script.js, objVal, &len );
				value.arrayValue->reserve( len );
				for ( uint32_t i = 0; i < len; i++ ){
					jsval elem;
					JS_GetElement( script.js, objVal, i, &elem );
					value.arrayValue->emplace_back( elem );
				}
			} else if ( JS_ObjectIsCallable( script.js, objVal ) ) {
				type = TypeFunction;
				value.objectValue = JS_ValueToFunction( script.js, val );
			} else {
				type = TypeObject;
				value.objectValue = objVal;
			}
		} else {
			type = TypeObject;
			value.objectValue = NULL;
		}
	}
}

/// copy assignment
ArgValue& ArgValue::operator=( const ArgValue& copyFrom ){
	
	// check
	if ( &copyFrom == this ) return *this;
	
	// clean up first
	if ( type == TypeString && this->value.stringValue ) {
		delete this->value.stringValue;
	} else if ( type == TypeArray && this->value.arrayValue ) {
		delete this->value.arrayValue;
	}
	
	// copy
	this->type = copyFrom.type;
	this->value = copyFrom.value;
	if ( type == TypeString ) {
		this->value.stringValue = new string();
		*this->value.stringValue = *copyFrom.value.stringValue;
	} else if ( type == TypeArray ) {
		this->value.arrayValue = new ArgValueVector();
		*this->value.arrayValue = *copyFrom.value.arrayValue;
		this->arrayObject = copyFrom.arrayObject;
	}
	// done
	return *this;
}

// destructor for argument
ArgValue::~ArgValue() {
	if ( type == TypeString && this->value.stringValue ) {
		delete this->value.stringValue;
		this->value.stringValue = NULL;
	} else if ( type == TypeArray && this->value.arrayValue ) {
		delete this->value.arrayValue;
		this->value.arrayValue = NULL;
	}
}

/// interprets pointer as destination of same type as this argument, extracts value
bool ArgValue::get( void *destination, ScriptType desiredType ) {
	if ( this->type == TypeBool ){
		if ( desiredType == TypeInt ) { *((int*)destination) = this->value.boolValue ? 1 : 0; }
		else if ( desiredType == TypeBool ) { *((bool*)destination) = this->value.boolValue; }
		else return false;
	} else if ( this->type == TypeInt ){
		if ( desiredType == TypeInt ) { *((int*)destination) = this->value.intValue; }
		else if ( desiredType == TypeBool ) { *((bool*)destination) = this->value.intValue != 0; }
		else if ( desiredType == TypeFloat ) { *((float*)destination) = this->value.intValue; }
		else return false;
	} else if ( this->type == TypeFloat ){
		if ( desiredType == TypeInt ) { *((int*)destination) = this->value.floatValue; }
		else if ( desiredType == TypeBool ) { *((bool*)destination) = this->value.floatValue != 0.0; }
		else if ( desiredType == TypeFloat ) { *((float*)destination) = this->value.floatValue; }
		else return false;
	} else if ( this->type == TypeObject && desiredType == TypeObject ){
		*((void**)destination) = this->value.objectValue;
	} else if ( this->type == TypeString && desiredType == TypeString ){
		*((string*)destination) = *this->value.stringValue;
	} else if ( this->type == TypeArray && desiredType == TypeArray){
		*((ArgValueVector*)destination) = *this->value.arrayValue;
	} else if ( this->type == TypeFunction && desiredType == TypeFunction ){
		*((void**)destination) = this->value.objectValue;
	} else {
		return false;
	}
	
	return true;
}

/// convert to jsval
jsval ArgValue::toValue() {
	jsval val;
	if ( this->type == TypeBool ){
		val.setBoolean( this->value.boolValue );
	} else if ( this->type == TypeInt ){
		val.setInt32( this->value.intValue );
	} else if ( this->type == TypeFloat ){
		val.setDouble( this->value.floatValue );
	} else if ( this->type == TypeObject ){
		val.setObjectOrNull( (JSObject*) this->value.objectValue );
	} else if ( this->type == TypeString ){
		val.setString( JS_NewStringCopyZ( script.js, this->value.stringValue->c_str() ) );
	} else if ( this->type == TypeArray ){
		val = ScriptArguments::ArrayToVal( *this->value.arrayValue );
	} else {
		val.setUndefined();
	}
	return val;
}

// easy convert bool, int, float to float, or return FALSE
bool ArgValue::toNumber( float &dest ) {
	if ( this->type == TypeBool ) {
		dest = this->value.boolValue ? 1.f : 0.f;
	} else if ( this->type == TypeFloat ) {
		dest = this->value.floatValue;
	} else if ( this->type == TypeInt ) {
		dest = (float) this->value.intValue;
	} else {
		return false;
	}
	
	return true;
}

bool ArgValue::toInt( int& dest ) {
	if ( this->type == TypeBool ) {
		dest = this->value.boolValue ? 1 : 0;
	} else if ( this->type == TypeFloat ) {
		dest = this->value.floatValue;
	} else if ( this->type == TypeInt ) {
		dest = (float) this->value.intValue;
	} else {
		return false;
	}
	return true;
}

bool ArgValue::toInt8( Uint8& dest ) {
	if ( this->type == TypeFloat ) {
		dest = this->value.floatValue;
	} else if ( this->type == TypeInt ) {
		dest = (float) this->value.intValue;
	} else {
		return false;
	}
	return true;
}

/// easy convert any value to bool
bool ArgValue::toBool() {
	if ( this->type == TypeBool ) {
		return this->value.boolValue;
	} else if ( this->type == TypeFloat ) {
		return this->value.floatValue != 0.f;
	} else if ( this->type == TypeInt ) {
		return this->value.intValue != 0;
	} else if ( this->type == TypeString ) {
		return (this->value.stringValue->length() > 0);
	} else if ( this->type == TypeObject ) {
		return this->value.objectValue != NULL;
	}
	return false;
}

///
bool ArgValue::isNull( bool strict ) {
	bool strictNull = ( this->type == TypeObject && this->value.objectValue == NULL );
	if ( strict ) {
		return strictNull;
	} else {
		return strictNull || ( this->type == TypeString && !this->value.stringValue->size() );
	}
}
