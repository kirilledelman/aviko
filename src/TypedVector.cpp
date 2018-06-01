#include "TypedVector.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


TypedVector::TypedVector( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<TypedVector>( this );
	
	// if args passed
	size_t numArgs = 0;
	if ( args && (numArgs = args->args.size()) > 0 ) {
		
		// if first argument is array
		if ( args->args[ 0 ].type == TypeArray ) {
			script.SetProperty( "array", args->args[ 0 ], this->scriptObject );
		} else {
			// first is type
			script.SetProperty( "type", args->args[ 0 ], this->scriptObject );
			// second arg is used to init
			if ( numArgs >= 2 && args->args[ 1 ].type == TypeArray ) {
				script.SetProperty( "array", args->args[ 1 ], this->scriptObject );
			}
		}
	} else {
		// default
		script.SetProperty( "type", ArgValue( "Number" ), this->scriptObject );
	}
	
}

TypedVector::~TypedVector() {
	
	// free container
	if ( container != NULL ) {
		if ( type == TypeInt ){
			delete (vector<int>*) container;
		} else if ( type == TypeChar ){
			delete (vector<Uint8>*) container;
		} else if ( type == TypeFloat ){
			delete (vector<float>*) container;
		} else if ( type == TypeDouble ){
			delete (vector<double>*) container;
		} else if ( type == TypeBool ){
			delete (vector<bool>*) container;
		} else if ( type == TypeString ){
			delete (vector<string>*) container;
		} else if ( type == TypeObject || type == TypeArray ){
			delete (vector<void*>*) container;
		}
	}
	
}


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void TypedVector::InitClass() {
	
	// create class
	script.RegisterClass<TypedVector>( NULL );
	
	// properties
	
	script.AddIndexProperty<TypedVector>// [] operator
	(static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		return self->GetElement( (int) index );
	}),
	 static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		if ( self->lockedSize && index >= self->GetLength() ) return ArgValue();
		self->SetElement( val, (int) index );
		self->Notify();
		return val;
	}));
	
	script.AddProperty<TypedVector>
	( "length",
	 static_cast<ScriptIntCallback>([](void *p, int val ){ return ((TypedVector*) p)->GetLength(); }),
	 static_cast<ScriptIntCallback>([](void *p, int val){
		TypedVector* self = (TypedVector*) p;
		if ( self->lockedSize ) return self->GetLength();
		self->SetLength( val );
		self->Notify();
		return val;
	}),
	 PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<TypedVector>
	( "type",
	 static_cast<ScriptValueCallback>([](void *p, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		if ( self->type != TypeUndefined ) {
			return ArgValue( self->typeName.c_str() );
		}
		return ArgValue();
	}),
	 static_cast<ScriptValueCallback>([](void *p, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		if ( self->lockedType && self->type != TypeUndefined ) return ArgValue( self->typeName.c_str() );
		return self->InitWithType( val );
	}), PROP_NOSTORE | PROP_ENUMERABLE | PROP_EARLY | PROP_SERIALIZED );
	
	// get or set vector as array
	script.AddProperty<TypedVector>
	( "array",
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		if ( !self->container ) return ArgValue();
		// return array
		ArgValue ret;
		ret.type = TypeArray;
		ret.value.arrayValue = self->ToArray();
		return ret;
	 }),
	 static_cast<ScriptValueCallback>([]( void* p, ArgValue val ){
		TypedVector* self = (TypedVector*) p;
		self->Set( val );
		self->Notify();
		return val;
	}), PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );
	
	// functions
	
	script.DefineFunction<TypedVector>
	( "isValid",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		if ( sa.args.size() == 0 ) {
			script.ReportError( "usage: isValid( value )" );
			return false;
		}
		sa.ReturnBool( self->CheckValue( sa.args[ 0 ] ) );
		return true;
	}));
	
	script.DefineFunction<TypedVector>
	( "push",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		if ( self->lockedSize ) return true;
		for ( size_t i = 0, np = sa.args.size(); i < np; i++ ){
			self->PushElement( sa.args[ i ] );
		}
		self->Notify();
		return true;
	}));
	
	script.DefineFunction<TypedVector>
	( "pop",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		if ( self->lockedSize ) return true;
		sa.ReturnValue( self->PopElement() );
		self->Notify();
		return true;
	}));
	
	script.DefineFunction<TypedVector>
	( "clear",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		if ( self->lockedSize ) return true;
		self->Clear();
		self->Notify();
		return true;
	}));
	
	script.DefineFunction<TypedVector>
	( "slice",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		int start = 0, end = self->GetLength();
		if ( !sa.ReadArguments( 1, TypeInt, &start, TypeInt, &end ) ) {
			script.ReportError( "usage: slice( Integer start, [ Integer end ] )" );
			return false;
		}
		// slice
		ArgValueVector* slice = self->Slice( start, end );
		if ( slice ) {
			sa.ReturnArray( *slice );
			delete slice;
		} else {
			sa.ReturnNull();
		}
		return true;
	}));
	
	script.AddProperty<TypedVector>
	( "lockedSize",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((TypedVector*) b)->lockedSize; } ) );
	
	script.AddProperty<TypedVector>
	( "lockedType",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((TypedVector*) b)->lockedType; } ) );
	
	script.DefineFunction<TypedVector>
	( "splice",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		TypedVector* self = (TypedVector*) go;
		int start = 0, numDelete = 0;
		if ( self->lockedSize ) return true;
		if ( !sa.ReadArguments( 1, TypeInt, &start, TypeInt, &numDelete ) ) {
			script.ReportError( "usage: splice( Integer start, [ Integer numDelete, [ insertElement1, insertElement2 ... ] ] )" );
			return false;
		}
		// splice
		ArgValueVector* splice = self->Splice( start, numDelete, sa.args.begin() + 2, (int) sa.args.size() - 2 );
		if ( splice ) {
			sa.ReturnArray( *splice );
			self->Notify();
			delete splice;
		} else {
			sa.ReturnNull();
		}
		return true;
	}));
	
	script.DefineFunction<TypedVector>
	( "toString",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		static char buf[512];
		TypedVector* self = (TypedVector*) o;
		
		if ( !self ) {
			sprintf( buf, "[Vector prototype]" );
		} else if ( self->typeName.size() ) {
			sprintf( buf, "[Vector (%s) %p]", self->typeName.c_str(), self );
		} else sprintf( buf, "[Vector %p]", self );
		
		sa.ReturnString( buf );
		return true;
	}));
}

void TypedVector::TraceProtectedObjects( vector<void**> &protectedObjects ) {
	// if objects vector, add to protectedObjects
	if ( type == TypeObject || type == TypeArray ){
		vector<void*>* cont = (vector<void*>*) container;
		for ( size_t i = 0, nc = cont->size(); i < nc; i++ ) {
			protectedObjects.push_back( &cont->at( i ) );
		}
	}
}


void TypedVector::Notify() {

	if ( callback ) callback( this );
	Event e( EVENT_CHANGE );
	e.scriptParams.AddObjectArgument( this->scriptObject );
	this->CallEvent( e );
	
}


/* MARK:	-				Property callbacks
 -------------------------------------------------------------------- */


/// return new ArgValueVector with enumerable property names
ArgValueVector* TypedVector::EnumerateVectorProperties( ScriptableClass* self ) {
	
	// printf( "(EnumerateVectorProperties %p)", self );
	ArgValueVector* ret = new ArgValueVector();
	ret->emplace_back( "array" );
	ret->emplace_back( "length" );
	ret->emplace_back( "type" );
	ret->emplace_back( "isValid" );
	ret->emplace_back( "push" );
	ret->emplace_back( "pop" );
	ret->emplace_back( "slice" );
	ret->emplace_back( "toString" );
	ret->emplace_back( "clear" );
	// add numeric keys
	TypedVector* vec = (TypedVector*) self;
	// static char str[ 16 ];
	for ( size_t i = 0, nk = vec->GetLength(); i < nk; i++ ) {
		ret->emplace_back( (int) i ); // SDL_itoa( (int) i, str, 10 ) );
	}
	return ret;
	
}

///
bool TypedVector::ResolveVectorProperty( ScriptableClass* self, ArgValue prop ) {
	// return true if string ( default behavior )
	if ( prop.type == TypeString ) return true;
	// integer?
	else {
		if ( prop.value.intValue < 0 ) return false;
		TypedVector* vec = (TypedVector*) self;
		return ( prop.value.intValue < vec->GetLength() );
	}
}

/* MARK:	-				Ops
 -------------------------------------------------------------------- */


ArgValue TypedVector::InitWithType( ArgValue& typeVal ) {

	// type from param
	TypedVector* copyFrom = NULL;	
	// string
	if ( typeVal.type == TypeString ) {
		// just copy type to objectTypeName
		typeName = *typeVal.value.stringValue;
	// another typed vector
	} else if ( typeVal.type == TypeObject && !typeVal.isNull() &&
			   ( copyFrom = script.GetInstance<TypedVector>( typeVal.value.objectValue ) ) != NULL ) {
		if ( !copyFrom->container ) {
			script.ReportError( "Invalid Vector type specified: given Vector is not initialized." );
			return ArgValue( false );
		}
		typeName = copyFrom->typeName;
	// constructor
	} else if ( typeVal.type == TypeFunction ) {
		// get class name
		typeName = script.GetClassNameByConstructor( typeVal.value.objectValue );
	} else if ( typeVal.type == TypeInt ) {
		typeName = "Integer";
	} else if ( typeVal.type == TypeBool ) {
		typeName = "Boolean";
	} else {
		typeName = "Number";
	}
	
	// free container
	if ( container != NULL ) {
		if ( type == TypeInt ){
			delete (vector<int>*) container;
		} else if ( type == TypeChar ){
			delete (vector<Uint8>*) container;
		} else if ( type == TypeFloat ){
			delete (vector<float>*) container;
		} else if ( type == TypeDouble ){
			delete (vector<double>*) container;
		} else if ( type == TypeBool ){
			delete (vector<bool>*) container;
		} else if ( type == TypeString ){
			delete (vector<string>*) container;
		} else if ( type == TypeObject || type == TypeArray ){
			delete (vector<void*>*) container;
		}
	}
	
	// create container based on type
	if ( typeName.compare( "Boolean" ) == 0 ) {
		type = TypeBool;
		container = new vector<bool>();
	} else if ( typeName.compare( "Byte" ) == 0 ) {
		type = TypeChar;
		container = new vector<Uint8>();
	} else if ( typeName.compare( "Integer" ) == 0 ) {
		type = TypeInt;
		container = new vector<int>();
	} else if ( typeName.compare( "Float" ) == 0 ) {
		type = TypeFloat;
		container = new vector<float>();
	} else if ( typeName.compare( "Number" ) == 0 || typeName.compare( "Double" ) == 0 ) {
		type = TypeDouble;
		container = new vector<double>();
	} else if ( typeName.compare( "String" ) == 0 ) {
		type = TypeString;
		container = new vector<string>();
	} else if ( typeName.compare( "Array" ) == 0 ) {
		type = TypeArray;
		container = new vector<void*>();
	} else {
		type = TypeObject;
		container = new vector<void*>();
	}
	
	// copy values
	if ( copyFrom ) this->Set( typeVal );
	
	return ArgValue( typeName.c_str() );
}

/// removes all elements
void TypedVector::Clear() {
	switch( type ) {
		case TypeBool:
			((vector<bool>*) container)->clear();
			break;
		case TypeChar:
			((vector<Uint8>*) container)->clear();
			break;
		case TypeInt:
			((vector<int>*) container)->clear();
			break;
		case TypeFloat:
			((vector<float>*) container)->clear();
			break;
		case TypeDouble:
			((vector<double>*) container)->clear();
			break;
		case TypeString:
			((vector<string>*) container)->clear();
			break;
		case TypeObject:
		case TypeArray:
			((vector<void*>*) container)->clear();
			break;
		default:
			break;
	}
}

/// returns length
int TypedVector::GetLength() {
	switch( type ) {
		case TypeBool:
			return (int) ((vector<bool>*) container)->size();
		case TypeChar:
			return (int) ((vector<Uint8>*) container)->size();
		case TypeInt:
			return (int) ((vector<int>*) container)->size();
		case TypeFloat:
			return (int) ((vector<float>*) container)->size();
		case TypeDouble:
			return (int) ((vector<double>*) container)->size();
		case TypeString:
			return (int) ((vector<string>*) container)->size();
		case TypeObject:
		case TypeArray:
			return (int) ((vector<void*>*) container)->size();
		default:
			break;
	}
	return 0;
}
	
/// resizes array / fills with default value
void TypedVector::SetLength( int newLen ) {
	switch( type ) {
		case TypeBool:
			((vector<bool>*) container)->resize( newLen, false );
			break;
		case TypeChar:
			((vector<Uint8>*) container)->resize( newLen, 0 );
			break;
		case TypeInt:
			((vector<int>*) container)->resize( newLen, 0 );
			break;
		case TypeFloat:
			((vector<float>*) container)->resize( newLen, 0 );
			break;
		case TypeDouble:
			((vector<double>*) container)->resize( newLen, 0 );
			break;
		case TypeString:
			((vector<string>*) container)->resize( newLen );
			break;
		case TypeObject:
		case TypeArray:
			((vector<void*>*) container)->resize( newLen, NULL );
			break;
		default:
			break;
	}
}
	
/// set entire array
bool TypedVector::Set( ArgValue& in ) {
	
	// from array
	TypedVector* other = NULL;
	if ( in.type == TypeArray ){
		int np = (int) in.value.arrayValue->size();
		if ( !container ) {
			ArgValue dummy( "Number" );
			if ( !np ) this->InitWithType( dummy );
			else if ( this->InitWithType( in.value.arrayValue->at( 0 ) ).isFalse() ){
				return false;
			}
		}
		if ( !lockedSize ) this->SetLength( np );
		for ( int i = 0, n = this->GetLength(); i < n; i++ ){
			this->SetElement( in.value.arrayValue->at( i ), i );
		}
		return true;
		
	// from another TypedVector
	} else if ( in.type == TypeObject && (other = script.GetInstance<TypedVector>( in.value.objectValue ) ) ) {
		int np = other->GetLength();
		if ( !container ) {
			ArgValue v( other->typeName.c_str() );
			this->InitWithType( v );
		}
		if ( !lockedSize ) this->SetLength( np );
		for ( int i = 0, n = this->GetLength(); i < n; i++ ){
			ArgValue val = other->GetElement( i );
			this->SetElement( val, i );
		}
		return true;
	}
	
	// ignore other types
	return false;

}
 
/// set one element, bool on success
bool TypedVector::SetElement( ArgValue& val, int index ) {
	if ( !container ) return false;
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	bool b;
	Uint8 c;
	int i;
	float f;
	double d;
	bool valIsNull = val.isNull( true );
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			if ( val.type == TypeBool ) {
				b = val.toBool();
			} else return false;
			if ( vb->size() <= index ) { if ( lockedSize ) return false; else vb->resize( index + 1 ); }
			(*vb)[ index ] = b;
			break;
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			if ( !val.toInt8( c ) ) { return false; }
			if ( vc->size() <= index ) { if ( lockedSize ) return false; else vc->resize( index + 1 ); }
			(*vc)[ index ] = c;
			break;
		case TypeInt:
			vi = ((vector<int>*) container);
			if ( !val.toInt( i ) ) { return false; }
			if ( vi->size() <= index ) { if ( lockedSize ) return false; else vi->resize( index + 1 ); }
			(*vi)[ index ] = i;
			break;
		case TypeFloat:
			vf = ((vector<float>*) container);
			if ( !val.toNumber( f ) ) { return false; }
			if ( vf->size() <= index ) { if ( lockedSize ) return false; else vf->resize( index + 1 ); }
			(*vf)[ index ] = f;
			break;
		case TypeDouble:
			vd = ((vector<double>*) container);
			if ( !val.toNumber( d ) ) { return false; }
			if ( vd->size() <= index ) { if ( lockedSize ) return false; else vd->resize( index + 1 ); }
			(*vd)[ index ] = d;
			break;
		case TypeString:
			vs = ((vector<string>*) container);
			if ( val.type != TypeString ) { return false; }
			if ( vs->size() <= index ) { if ( lockedSize ) return false; else vs->resize( index + 1 ); }
			(*vs)[ index ] = *val.value.stringValue;
			break;
		case TypeArray:
			vo = ((vector<void*>*) container);
			if ( vo->size() <= index ) { if ( lockedSize ) return false; else vo->resize( index + 1 ); }
			// check object type
			if ( val.type == TypeArray ) {
				(*vo)[ index ] = val.arrayObject;
			} else if ( valIsNull ) {
				(*vo)[ index ] = NULL;
			} else return false;
			break;
		case TypeObject:
			vo = ((vector<void*>*) container);
			if ( vo->size() <= index ) { if ( lockedSize ) return false; else vo->resize( index + 1 ); }
			// check object type
			if ( val.type == TypeObject && !valIsNull && script.IsObjectDescendentOf( val.value.objectValue, typeName.c_str() ) ) {
				(*vo)[ index ] = val.value.objectValue;
			} else if ( valIsNull ) {
				(*vo)[ index ] = NULL;
			} else return false;
			break;
		default:
			return false;
	}
	return true;
}

/// get one element
ArgValue TypedVector::GetElement( int index ) {
	if ( !container ) return ArgValue();
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			if ( vb->size() <= index ) return ArgValue();
			return ArgValue( (*vb)[ index ] );
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			if ( vc->size() <= index ) return ArgValue();
			return ArgValue( (*vc)[ index ] );
		case TypeInt:
			vi = ((vector<int>*) container);
			if ( vi->size() <= index ) return ArgValue();
			return ArgValue( (*vi)[ index ] );
		case TypeFloat:
			vf = ((vector<float>*) container);
			if ( vf->size() <= index ) return ArgValue();
			return ArgValue( (*vf)[ index ] );
		case TypeDouble:
			vd = ((vector<double>*) container);
			if ( vd->size() <= index ) return ArgValue();
			return ArgValue( (*vd)[ index ] );
		case TypeString:
			vs = ((vector<string>*) container);
			if ( vs->size() <= index ) return ArgValue();
			return ArgValue( ((*vs)[ index ]).c_str() );
		case TypeObject:
		case TypeArray:
			vo = ((vector<void*>*) container);
			if ( vo->size() <= index ) return ArgValue();
			return ArgValue( (*vo)[ index ] );
		default:
			break;
	}
	return ArgValue();
}

/// push to end
bool TypedVector::PushElement( ArgValue& val ) {
	if ( !container || lockedSize ) return false;
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	bool b;
	Uint8 c;
	int i;
	float f;
	double d;
	bool valIsNull = val.isNull( true );
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			if ( val.type == TypeBool ) {
				b = val.toBool();
			} else return false;
			vb->push_back( b );
			break;
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			if ( !val.toInt8( c ) ) { return false; }
			vc->push_back( c );
			break;
		case TypeInt:
			vi = ((vector<int>*) container);
			if ( !val.toInt( i ) ) { return false; }
			vi->push_back( i );
			break;
		case TypeFloat:
			vf = ((vector<float>*) container);
			if ( !val.toNumber( f ) ) { return false; }
			vf->push_back( f );
			break;
		case TypeDouble:
			vd = ((vector<double>*) container);
			if ( !val.toNumber( d ) ) { return false; }
			vd->push_back( d );
			break;
		case TypeString:
			vs = ((vector<string>*) container);
			if ( val.type != TypeString ) { return false; }
			vs->push_back( *val.value.stringValue );
			break;
		case TypeArray:
			vo = ((vector<void*>*) container);
			// check object type
			if ( val.type == TypeArray ) {
				vo->push_back( val.arrayObject );
			} else if ( valIsNull ) {
				vo->push_back( NULL );
			} else return false;
			break;
		case TypeObject:
			vo = ((vector<void*>*) container);
			// check object type
			if ( val.type == TypeObject && !valIsNull && script.IsObjectDescendentOf( val.value.objectValue, typeName.c_str() ) ) {
				vo->push_back( val.value.objectValue );
			} else if ( valIsNull ) {
				vo->push_back( NULL );
			} else return false;
			break;
		default:
			return false;
	}
	return true;
}

/// pop last
ArgValue TypedVector::PopElement() {
	if ( !container || lockedSize ) return ArgValue();
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	bool b;
	Uint8 c;
	int i;
	float f;
	double d;
	string s;
	void* o;
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			if ( !vb->size() ) return ArgValue();
			b = vb->back();
			vb->pop_back();
			return ArgValue( b );
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			if ( !vc->size() ) return ArgValue();
			c = vc->back();
			vc->pop_back();
			return ArgValue( c );
		case TypeInt:
			vi = ((vector<int>*) container);
			if ( !vi->size() ) return ArgValue();
			i = vi->back();
			vi->pop_back();
			return ArgValue( i );
		case TypeFloat:
			vf = ((vector<float>*) container);
			if ( !vf->size() ) return ArgValue();
			f = vf->back();
			vf->pop_back();
			return ArgValue( f );
		case TypeDouble:
			vd = ((vector<double>*) container);
			if ( !vd->size() ) return ArgValue();
			d = vd->back();
			vd->pop_back();
			return ArgValue( d );
		case TypeString:
			vs = ((vector<string>*) container);
			if ( !vs->size() ) return ArgValue();
			s = vs->back();
			vs->pop_back();
			return ArgValue( s.c_str() );
		case TypeObject:
		case TypeArray:
			vo = ((vector<void*>*) container);
			if ( !vo->size() ) return ArgValue();
			o = vo->back();
			vo->pop_back();
			return ArgValue( o ) ;
		default:
			return ArgValue();
	}
	return ArgValue();
}

/// like Array.slice
ArgValueVector* TypedVector::Slice( int start, int end ) {
	if ( !container ) return NULL;
	ArgValueVector* ret = new ArgValueVector();
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	int sz;
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			sz = (int) vb->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vb)[ i ] );
			}
			return ret;
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			sz = (int) vc->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vc)[ i ] );
			}
			return ret;
		case TypeInt:
			vi = ((vector<int>*) container);
			sz = (int) vi->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vi)[ i ] );
			}
			return ret;
		case TypeFloat:
			vf = ((vector<float>*) container);
			sz = (int) vf->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vf)[ i ] );
			}
			return ret;
		case TypeDouble:
			vd = ((vector<double>*) container);
			sz = (int) vd->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vd)[ i ] );
			}
			return ret;
		case TypeString:
			vs = ((vector<string>*) container);
			sz = (int) vs->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vs)[ i ].c_str() );
			}
			return ret;
		case TypeObject:
		case TypeArray:
			vo = ((vector<void*>*) container);
			sz = (int) vo->size();
			ret->reserve( sz );
			if ( start < 0 ) start = sz + start;
			if ( end < 0 ) end = sz + end;
			for ( size_t i = start; i < end && i < sz; i++ ) {
				ret->emplace_back( (*vo)[ i ] );
			}
			return ret;
		default:
			break;
	}
	return NULL;
}

/// like Array.splice
ArgValueVector* TypedVector::Splice( int start, int deleteCount, ArgValueVector::iterator insertElementIterator, int insertCount ) {
	if ( !container || lockedSize ) return NULL;
	vector<bool>* vb = NULL;
	vector<Uint8>* vc = NULL;
	vector<int>* vi = NULL;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	vector<string>* vs = NULL;
	vector<void*>* vo = NULL;
	int np;
	int sizeDiff;
	// will be returning removed elements
	ArgValueVector* removed = new ArgValueVector();
	removed->reserve( deleteCount );
	switch( type ) {
		case TypeBool:
			vb = ((vector<bool>*) container);
			np = (int) vb->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vb)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vb->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vb)[ i ] = (*vb)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vb)[ i + sizeDiff ] = (*vb)[ i ];
				}
				vb->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vb)[ start + i ] = (insertElementIterator + i)->toBool();
			}
			break;
		case TypeChar:
			vc = ((vector<Uint8>*) container);
			np = (int) vc->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vc)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vc->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vc)[ i ] = (*vc)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vc)[ i + sizeDiff ] = (*vc)[ i ];
				}
				vc->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vc)[ start + i ] = 0;
				(insertElementIterator + i)->toInt8( (*vc)[ start + i ] );
			}
			break;
		case TypeInt:
			vi = ((vector<int>*) container);
			np = (int) vi->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vi)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vi->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vi)[ i ] = (*vi)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vi)[ i + sizeDiff ] = (*vi)[ i ];
				}
				vi->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vi)[ start + i ] = 0;
				(insertElementIterator + i)->toInt( (*vi)[ start + i ] );
			}
			break;
		case TypeFloat:
			vf = ((vector<float>*) container);
			np = (int) vf->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vf)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vf->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vf)[ i ] = (*vf)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vf)[ i + sizeDiff ] = (*vf)[ i ];
				}
				vf->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vf)[ start + i ] = 0;
				(insertElementIterator + i)->toNumber( (*vf)[ start + i ] );
			}
			break;
		case TypeDouble:
			vd = ((vector<double>*) container);
			np = (int) vd->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vd)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vd->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vd)[ i ] = (*vd)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vd)[ i + sizeDiff ] = (*vd)[ i ];
				}
				vd->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vd)[ start + i ] = 0;
				(insertElementIterator + i)->toNumber( (*vd)[ start + i ] );
			}
			break;
		case TypeString:
			vs = ((vector<string>*) container);
			np = (int) vs->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vs)[ i ].c_str() );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vs->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vs)[ i ] = (*vs)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vs)[ i + sizeDiff ] = (*vs)[ i ];
				}
				vs->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vs)[ start + i ] = (insertElementIterator + i)->toString();
			}
			break;
		case TypeArray:
		case TypeObject:
			vo = ((vector<void*>*) container);
			np = (int) vo->size();
			if ( start < 0 ) start = np + start;
			// copy deleted elements
			for ( int i = start, end = start + deleteCount; i < np && i < end; i++ ) {
				removed->emplace_back( (*vo)[ i ] );
			}
			// determine if array will shrink or expand, and move elements after insertion point <- or ->
			sizeDiff = insertCount - (int) removed->size();
			if ( sizeDiff > 0 ) {
				vs->resize( np + sizeDiff );
				for ( int i = np + sizeDiff - 1, end = start + deleteCount; i >= end; i-- ) {
					(*vo)[ i ] = (*vo)[ i - sizeDiff ];
				}
			} else if ( sizeDiff < 0 ) {
				for ( int i = start + deleteCount; i < np; i++ ) {
					(*vo)[ i + sizeDiff ] = (*vo)[ i ];
				}
				vs->resize( np + sizeDiff );
			}
			// copy new elements in
			for ( int i = 0; i < insertCount; i++ ) {
				(*vo)[ start + i ] = (insertElementIterator + i)->value.objectValue;
			}
			break;
		default:
			break;
	}
	
	return removed;
}


/// true if value is valid for this array
bool TypedVector::CheckValue( ArgValue& val ) {
	switch( type ) {
		case TypeBool:
			return ( val.type == TypeBool );
		case TypeChar:
		case TypeInt:
		case TypeFloat:
		case TypeDouble:
			return (val.type == TypeBool || val.type == TypeChar || val.type == TypeInt || val.type == TypeFloat || val.type == TypeDouble );
		case TypeString:
			return ( val.type == TypeString );
		case TypeArray:
			return ( val.type == TypeArray || val.isNull( true ) );
		case TypeObject:
			return ( val.type == TypeObject );
		default:
			return false;
	}
	return true;
}

/// converts to b2Vec2 points for BodyShape (float and double only)
void TypedVector::ToVec2Vector( vector<b2Vec2>& points, float multiplier ) {
	// fill points with values
	points.clear();
	if ( !container ) return;
	vector<float>* vf = NULL;
	vector<double>* vd = NULL;
	size_t np;
	switch( type ) {
		case TypeFloat:
			vf = ((vector<float>*) container);
			np = vf->size();
			points.resize( ceil( np / 2 ) );
			for ( size_t i = 0; i < np; i++ ){
				if ( i % 2 ) {
					points[ i / 2 ].y = multiplier * (*vf)[ i ];
				} else {
					points[ i / 2 ].x = multiplier * (*vf)[ i ];
				}
			}
			break;
		case TypeDouble:
			vd = ((vector<double>*) container);
			np = vd->size();
			points.resize( ceil( np / 2 ) );
			for ( size_t i = 0; i < np; i++ ){
				if ( i % 2 ) {
					points[ i / 2 ].y = multiplier * (*vd)[ i ];
				} else {
					points[ i / 2 ].x = multiplier * (*vd)[ i ];
				}
			}
			break;
		default:
			break;
	}
}


/* MARK:	-				Access
 -------------------------------------------------------------------- */


/// returns array of values
ArgValueVector* TypedVector::ToArray() {
	// copy elements
	ArgValueVector* out = new ArgValueVector();
	size_t np = GetLength();
	out->resize( np );
	for ( size_t i = 0; i < np; i++ ) {
		switch( type ) {
			case TypeBool:
				(*out)[ i ] = ArgValue( (*((vector<bool>*) container))[ i ] );
				break;
			case TypeChar:
				((vector<Uint8>*) container)->clear();
				(*out)[ i ] = ArgValue( (*((vector<Uint8>*) container))[ i ] );
				break;
			case TypeInt:
				(*out)[ i ] = ArgValue( (*((vector<int>*) container))[ i ] );
				break;
			case TypeFloat:
				(*out)[ i ] = ArgValue( (*((vector<float>*) container))[ i ] );
				break;
			case TypeDouble:
				(*out)[ i ] = ArgValue( (*((vector<double>*) container))[ i ] );
				break;
			case TypeString:
				(*out)[ i ] = ArgValue( (*((vector<string>*) container))[ i ].c_str() );
				break;
			case TypeObject:
			case TypeArray:
				(*out)[ i ] = ArgValue( (*((vector<void*>*) container))[ i ] );
				break;
			default:
				break;
		}
	}
	
	return out;
}

vector<bool>* TypedVector::ToBoolVector() { return ((vector<bool>*) container); }

vector<Uint8>* TypedVector::ToCharVector() { return ((vector<Uint8>*) container); }

vector<int>* TypedVector::ToIntVector() { return ((vector<int>*) container); }

vector<float>* TypedVector::ToFloatVector() { return ((vector<float>*) container); }

vector<double>* TypedVector::ToDoubleVector() { return ((vector<double>*) container); }

vector<string>* TypedVector::ToStringVector() { return ((vector<string>*) container); }

vector<void*>* TypedVector::ToObjectVector() { return ((vector<void*>*) container); }







