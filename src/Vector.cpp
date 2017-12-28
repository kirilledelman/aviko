#include "Vector.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


FloatVector::FloatVector( ScriptArguments* args ) {
	
	// add scriptObject
	script.NewScriptObject<FloatVector>( this );
	
	// if passed an array as argument, assign to it
	if ( args && args->args.size() && args->args[ 0 ].type == TypeArray ) {
		script.SetProperty( "array", args->args[ 0 ], this->scriptObject );
	}
	
}


/* MARK:	-				Ops
 -------------------------------------------------------------------- */


/// converts array to an array of b2Vec2 points
void FloatVector::ToVec2Vector( vector<b2Vec2>& points, float multiplier ) {
	points.resize( ceil( this->vec.GetLength() / 2 ) );
	for ( size_t i = 0, np = this->vec.GetLength(); i < np; i++ ){
		if ( i % 2 ) {
			points[ i / 2 ].y = multiplier * this->vec.GetElement( (int) i );
		} else {
			points[ i / 2 ].x = multiplier * this->vec.GetElement( (int) i );
		}
	}
}

/// notifies callback of change
void FloatVector::Notify() {

	if ( notify ) this->callback( this );
	
}

/// updates .points from ArgValueVector
bool FloatVector::Set( ArgValue& in ) {
	
	// from array
	if ( in.type == TypeArray ){
		for ( size_t i = 0, np = in.value.arrayValue->size(); i < np; i++ ){
			float fval = 0;
			(*in.value.arrayValue)[ i ].toNumber( fval );
			this->vec.Push( fval );
		}
		return true;
	// from another FloatVector
	} else if ( in.type == TypeObject && !in.isNull( true ) ) {
		FloatVector* fv = script.GetInstance<FloatVector>( in.value.objectValue );
		if ( fv ) this->vec.Copy( fv->vec );
		return true;
	}
	
	// ignore other types
	return false;
	
}

/* MARK:	-				Script
 -------------------------------------------------------------------- */


void FloatVector::InitClass() {
	
	// create class
	script.RegisterClass<FloatVector>( "ScriptableObject" );
	
	// properties

	script.AddIndexProperty<FloatVector>// [] operator
	(static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		FloatVector* self = (FloatVector*) p;
		if ( index >= self->vec.GetLength() ) return ArgValue();
		float v = ((FloatVector*) p)->vec.GetElement( index );
		return ArgValue( v );
	}),
	static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		FloatVector* self = (FloatVector*) p;
		float v = 0;
		val.toNumber( v );
		self->vec.SetElement( index, v );
		self->Notify();
		return val;
	}));
	
	script.AddProperty<FloatVector>
	( "length",
	 static_cast<ScriptIntCallback>([](void *p, int val ){ return ((FloatVector*) p)->vec.GetLength(); }),
	static_cast<ScriptIntCallback>([](void *p, int val){
		FloatVector* self = (FloatVector*) p;
		((FloatVector*) p)->vec.SetLength( val );
		self->Notify();
		return val;
	 }),
	 PROP_ENUMERABLE | PROP_NOSTORE );
	
	// get or set vector as array
	script.AddProperty<FloatVector>
	( "array",
	 static_cast<ScriptArrayCallback>([]( void* p, ArgValueVector* val ){ return ((FloatVector*) p)->vec.toArray(); }),
	 static_cast<ScriptArrayCallback>([]( void* p, ArgValueVector* val ){
		FloatVector* self = (FloatVector*) p;
		size_t np = val->size();
		self->vec.SetLength( (int) np );
		for ( size_t i = 0; i < np; i++ ) {
			float f = 0;
			val->at( i ).toNumber( f );
			self->vec.SetElement( (int) i, f );
		}
		self->Notify();
		return val;
	 }));
	
	// TODO - slice, splice, concat
	
}
