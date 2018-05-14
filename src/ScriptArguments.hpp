#ifndef ScriptArguments_hpp
#define ScriptArguments_hpp

#include "common.h"

/* MARK:	-				Script arguments
 
 To avoid having to deal with Javascript specific data structures in
 scriptable classes, script arguments wrappers are used.
 
 ArgValue is a representation of a contents of a Javascript variable or param:
 numbers, objects, strings, etc. to be converted in and out of scriptable
 class's callback functions.
 
 ScriptArguments is a class facilitating this conversion. When defining
 functions for your class via DefineFunction, the callback provided will
 receive ScriptArguments&, which will contain parameters provided by the
 script to your callback, as .args vector. Returning values is as simple as
 calling Return___(val) with desired type.
 
 -------------------------------------------------------------------- */

/// property types
typedef enum {
	TypeUndefined,
	
	TypeBool,
	TypeChar,
	TypeInt,
	TypeFloat,
	TypeDouble,
	TypeObject,
	TypeString,
	TypeArray,
	
	TypeValue,
	TypeFunction,
	TypeIndex
} ScriptType;

struct ArgValue;
typedef vector<ArgValue> ArgValueVector;

/// a single argument
struct ArgValue {
	
	/// combines all possible value types
	union ArgValueUnion {
		bool	boolValue;
		Uint8	charValue;
		int		intValue;
		float 	floatValue;
		double 	doubleValue;
		void* 	objectValue;
		ArgValueVector* arrayValue;
		string* stringValue;
	};
	
	// type of this value
	ScriptType type = TypeUndefined;
	
	// value holder
	ArgValueUnion value = { 0 };
	
	// if js array, holds pointer to its object ( used during serialization )
	void* arrayObject = NULL;
	
	// constructors
	ArgValue(){}
	ArgValue( bool val ){ type = TypeBool; value.boolValue = val; }
	ArgValue( Uint8 val ){ type = TypeChar; value.charValue = val; }
	ArgValue( int val ){ type = TypeInt; value.intValue = val; }
	ArgValue( float val ){ type = TypeFloat; value.floatValue = val; }
	ArgValue( double val ){ type = TypeDouble; value.doubleValue = val; }
	ArgValue( const char* val ){
		type = TypeString;
		value.stringValue = new string();
		if ( val ) value.stringValue->assign( val );
	}
	ArgValue( void* val ){ type = TypeObject; value.objectValue = val; }
	ArgValue( const ArgValue& copyFrom );
	ArgValue( jsval val );
	ArgValue& operator=( const ArgValue& copyFrom );
	ArgValue& operator=( const jsval& jv );
	
	// value getters
	bool toNumber( float& dest );
	bool toNumber( double& dest );
	bool toInt( int& dest );
	bool toInt8( Uint8& dest );
	bool toBool();
	bool isNull( bool strict=false );
	bool isTrue() { return (type == TypeBool && value.boolValue == true); }
	bool isFalse() { return (type == TypeBool && value.boolValue == false); }
	string toString();
	bool get( void* destination, ScriptType desiredType );
	jsval toValue();
		
	// destructor
	~ArgValue();
};

/// wrapper class for passing multiple arguments to functions and returning a value
class ScriptArguments {
private:
	
	/// reference to original callargs, when using from inside a JSNative
	CallArgs* callArgs = NULL;

	/// arguments constructed with Add___Argument for calling Javascript functions from code
	AutoValueVector *_funcArgs = NULL;

	/// getter with lazy init
	AutoValueVector* funcArgs();
	
public:
	
	/// arguments passed from script to ScriptFunctionCallback
	ArgValueVector args;
	
	// call these methods from to return a specific type of a value
	void ReturnUndefined();
	void ReturnNull();
	void ReturnBool( bool val );
	void ReturnInt( int val );
	void ReturnFloat( float val );
	void ReturnDouble( double val );
	void ReturnString( string val );
	void ReturnString( const char* val );
	void ReturnObject( void* val );
	void ReturnValue( ArgValue val );
	void ReturnArray( ArgValueVector &arr );
	
	// easily add / clear script params for function callbacks
	void ResizeArguments( int len );
	void AddIntArgument( int val );
	void AddBoolArgument( bool val );
	void AddFloatArgument( float val );
	void AddDoubleArgument( double val );
	void AddStringArgument( const char* val );
	void AddObjectArgument( void* val );
	void AddArgument( ArgValue val );
	
	// constructs and returns an array of jsvals for JS function calls
	jsval* GetFunctionArguments( int* argc );

	// read params into typed values (helper for function calls)
	bool ReadArguments( int minRequired,
					   ScriptType type0, void* value0,
					   ScriptType type1=TypeUndefined, void* value1=NULL,
					   ScriptType type2=TypeUndefined, void* value2=NULL,
					   ScriptType type3=TypeUndefined, void* value3=NULL,
					   ScriptType type4=TypeUndefined, void* value4=NULL,
					   ScriptType type5=TypeUndefined, void* value5=NULL,
					   ScriptType type6=TypeUndefined, void* value6=NULL);
	
	// read params into typed values (helper for function calls), starting with nth param
	bool ReadArgumentsFrom( int startFrom, int minRequired,
					   ScriptType type0, void* value0,
					   ScriptType type1=TypeUndefined, void* value1=NULL,
					   ScriptType type2=TypeUndefined, void* value2=NULL,
					   ScriptType type3=TypeUndefined, void* value3=NULL,
					   ScriptType type4=TypeUndefined, void* value4=NULL,
					   ScriptType type5=TypeUndefined, void* value5=NULL,
					   ScriptType type6=TypeUndefined, void* value6=NULL);
	
	/// converts array to jsval(JSObject)
	static jsval ArrayToVal( ArgValueVector &arr );

	/// returns "this" if available
	void* GetThis();
	
	/// constructor
	ScriptArguments();
	ScriptArguments( CallArgs* args );
	~ScriptArguments();
	
};

/// stores a reference to script function, used by event system
class ScriptFunctionObject {
public:
	
	void *funcObject = NULL;
	void *thisObject = NULL;
	
	/// used by event dispatcher to remove this function from its list after a single call
	bool callOnce = false;
	
	/// is set to true while dispatching
	bool executing = false;
	
	/// used when sorting by priority
	static size_t _index;
	size_t indexAdded = 0;
	int priority = 0;
	
	/// invoke this function with arguments
	ArgValue Invoke( ScriptArguments& args );
	
	// comparing to a pointer to a JSFunction, used when removing event listeners
	bool operator==( void* jsfunc ) { return funcObject == jsfunc; };
	
	// create / destroy
	ScriptFunctionObject(){ indexAdded = _index++; };
	ScriptFunctionObject( void* scriptFunc, bool once=false );
	~ScriptFunctionObject();
	
};

#endif /* ScriptArguments_hpp */
