#ifndef Vector_hpp
#define Vector_hpp

#include "common.h"
#include "ScriptableClass.hpp"

/// base class for vector container
template <class T>
class TVector {
public:

	vector<T> vec;
	T defaultValue;
	
	T* Data(){ return vec.data(); };
	
	int GetLength() { return (int) vec.size(); }
	int SetLength( int len ) { len = max( 0, len ); vec.resize( len ); return len; }
	ArgValueVector* toArray(){
		ArgValueVector* out = new ArgValueVector();
		size_t np = vec.size();
		out->resize( np );
		for ( size_t i = 0; i < np; i++ ) {
			out->at( i ) = ArgValue( (T) this->vec[ i ] );
		}
		return out;
	}
	T GetElement( size_t pos ) {
		if ( pos < vec.size() ) return vec[ pos ];
		return defaultValue;
	}
	void SetElement( int pos, T val ) {
		if ( pos >= vec.size() ) SetLength( pos + 1 );
		vec[ pos ] = val;
	}
	
	void Push( T val ){ vec.push_back( val ); }
	T Pop(){ return vec.size() ? vec.pop_back() : defaultValue; }
	//void Splice( int pos, int numRemove, ScriptArguments& inserts, ArgValue& returned );
	//void Slice( int pos, int length, ArgValue& returned );
	//void Concat( ScriptArguments& additions, ArgValue& returned );

	void Copy( TVector<T>& other ) {
		vec = other.vec;
	}
	
	// init/destroy
	TVector() { };
	~TVector() { };
	
};

class FloatVector;
typedef function<void (FloatVector*)> FloatVectorCallback;

/// base class for vector container
class FloatVector : public ScriptableClass {
public:
	
	// container
	TVector<float> vec;
	
	bool notify = false;
	FloatVectorCallback callback = NULL;
	
	// ops
	
	void Notify();
	
	void ToVec2Vector( vector<b2Vec2>& points, float multiplier=WORLD_TO_BOX2D_SCALE );
	
	bool Set( ArgValue& val );
	
	// scripting
	
	static void InitClass();
	
	// init/destroy
	FloatVector( ScriptArguments* );
	FloatVector() { vec.defaultValue = 0; };
	~FloatVector() { };
	
};

SCRIPT_CLASS_NAME( FloatVector, "FloatVector" );



#endif /* Vector_hpp */
