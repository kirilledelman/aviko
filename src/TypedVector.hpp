#ifndef TypedVector_hpp
#define TypedVector_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "ScriptArguments.hpp"

/// overrides SetElement behavior
typedef void (*TypedVectorSetCallback)(void*,int,ArgValue&);

class TypedVector;
typedef function<void (TypedVector*)> TypedVectorCallback;


/*

	Typed vector is used to create Array like objects in script,
	that only accept a specific type of values. The advantage is
	that on the engine side, these values are stored contiguously
	and can be accessed as STL vector<*>, and engine can be 
	notified when values change.
 
	In script it's created with
	new Vector( type ), where type is:
	a constructor of a class whose name to match when pushing items
	or a string of classname, or 'Byte', 'Boolean', 'Integer', 'Float', 'Number', 'String'
	
	
 
*/
class TypedVector : public ScriptableClass {
public:
	
	/// container vector<*>
	void* container = NULL;
	
	/// type of elements
	ScriptType type = ScriptType::TypeUndefined;
	string typeName; // holds string specifier of type - 'Number', or 'GameObject', etc
	
	// ignore push, splice, etc..
	bool lockedSize = false;
	
	// can't change type after creation
	bool lockedType = false;
	
// notifications
	
	/// overrides replace value behavior (used by RenderText w color array)
	TypedVectorSetCallback setCallback = NULL;
	
	/// set to callback func to call on change (used by BodyShape etc)
	TypedVectorCallback callback = NULL;
	
	void Notify();
	
// array operations
	
	/// initializes with type as parameter
	ArgValue InitWithType( ArgValue& typeVal );
	
	/// removes all elements
	void Clear();
	
	/// returns length
	int GetLength();
	
	/// resizes array / fills with default value
	void SetLength( int newLen );
	
	/// set entire array
	bool Set( ArgValue& arr );

	/// set one element, bool on success
	bool SetElement( ArgValue& val, int index );
	
	/// get one element
	ArgValue GetElement( int index );
	
	/// push to end
	bool PushElement( ArgValue& val );
	
	/// pop last
	ArgValue PopElement();
	
	/// like Array.slice
	ArgValueVector* Slice( int begin, int end );
	
	/// like Array.splice
	ArgValueVector* Splice( int start, int deleteCount, ArgValueVector::iterator insertElementIterator, int insertCount );
	
	/// true if value is valid for this array
	bool CheckValue( ArgValue& val );
	
// access
	
	// by type
	vector<bool>* ToBoolVector();
	vector<Uint8>* ToCharVector();
	vector<int>* ToIntVector();
	vector<float>* ToFloatVector();
	vector<double>* ToDoubleVector();
	vector<string>* ToStringVector();
	vector<void*>* ToObjectVector();
	
	/// converts to b2Vec2 points for BodyShape
	void ToVec2Vector( vector<b2Vec2>& points, float multiplier=WORLD_TO_BOX2D_SCALE );
	
	/// returns new array of values
	ArgValueVector* ToArray();
	
// scripting
	
	static void InitClass();
	
	void TraceProtectedObjects( vector<void**> &protectedObjects );
	
	static ArgValueVector* EnumerateVectorProperties( ScriptableClass* self );
	static bool ResolveVectorProperty( ScriptableClass* self, ArgValue prop );
	
	// init/destroy
	TypedVector( ScriptArguments* );
	TypedVector() {};
	~TypedVector();
	
};

SCRIPT_CLASS_NAME_EXT( TypedVector, "Vector", &TypedVector::EnumerateVectorProperties, &TypedVector::ResolveVectorProperty );

#endif /* Vector_hpp */
