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


/* MARK:	-				Script
 -------------------------------------------------------------------- */


void FloatVector::InitClass() {
	
	// create class
	script.RegisterClass<FloatVector>( "ScriptableObject" );
	
	// properties

	script.AddIndexProperty<FloatVector>
	(static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		FloatVector* self = (FloatVector*) p;
		if ( index >= self->vec.GetLength() ) return ArgValue();
		float v = ((FloatVector*) p)->vec.GetElement( index );
		return ArgValue( v );
	}),
	static_cast<ScriptIndexCallback>([]( void* p, uint32_t index, ArgValue val ){
		float v = 0;
		val.toNumber( v );
		((FloatVector*) p)->vec.SetElement( index, v );
		return val;
	}));
	
	script.AddProperty<FloatVector>
	( "length",
	 static_cast<ScriptIntCallback>([](void *p, int val ){ return ((FloatVector*) p)->vec.GetLength(); }),
	static_cast<ScriptIntCallback>([](void *p, int val){ return ((FloatVector*) p)->vec.SetLength( val ); }),
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
		return val;
	 }));
	
	// TODO - slice, splice, concat
	
}
