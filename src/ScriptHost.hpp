#ifndef ScriptHost_hpp
#define ScriptHost_hpp

#include "common.h"
#include "ScriptArguments.hpp"
#include "ScriptResource.hpp"

class ScriptableClass;

/* MARK:	-				Get/set callback types
 
 Different types of getter an setter callbacks, based on value types
 Each receives a pointer to ScriptableClass object who registered this
 callback, and a value. Getters receive currently stored value, setters
 receive value that the script is trying to set. Callback must return
 the final value to return as getter, or set as setter.
 -------------------------------------------------------------------- */


/// getter and setter callbacks with various types
typedef function<bool (void*, bool)> ScriptBoolCallback;
typedef function<int32_t (void*, int32_t)> ScriptIntCallback;
typedef function<float (void*, float)> ScriptFloatCallback;
typedef function<void* (void*, void*)> ScriptObjectCallback;
typedef function<string (void*, string&)> ScriptStringCallback;
typedef function<ArgValueVector* (void*, ArgValueVector*)> ScriptArrayCallback;
typedef function<ArgValue (void*, ArgValue)> ScriptValueCallback;
typedef function<ArgValue (void*, uint32_t, ArgValue)> ScriptIndexCallback; // for object[ int ] access

/// function callback with arguments
typedef function<bool (void*, ScriptArguments&)> ScriptFunctionCallback;

/// static function that returns enumerable properties of the class (used with Vector)
typedef ArgValueVector* EnumerateInstanceProperties( ScriptableClass* self );
/// static function that resolves instance property by defining it on object and returning true
typedef bool ResolveInstanceProperty( ScriptableClass* self, ArgValue prop );

/* MARK:	-				Script Class Name
 
 A template, used when registering various class properties and functions,
 via static calls like AddProperty<MyClass>( prop, getter, setter ), to
 convert or lookup MyClass to string "MyClass". To make your class scriptable,
 add SCRIPT_CLASS_NAME( MyClass, "ClassName" ); before class declaration. Also, class must
 inherit from ScriptableClass.
 -------------------------------------------------------------------- */


/// template for mapping CLASS -> "Class"
template <class T> struct ScriptClassDesc {
	static const string& name(){ static string _name = "?"; return _name; }
	static EnumerateInstanceProperties* enumerate(){ static EnumerateInstanceProperties* _enum = NULL; return _enum; }
	static ResolveInstanceProperty* resolve(){ static ResolveInstanceProperty* _resolve = NULL; return _resolve; }
};

/// for each class that needs to be scriptable, use this macro to define class name
#define SCRIPT_CLASS_NAME(CLASS,CLASSNAME) template <> struct ScriptClassDesc<CLASS> { \
	static const string& name(){ static string _name = CLASSNAME; return _name; } \
	static EnumerateInstanceProperties* enumerate(){ return NULL; } \
	static ResolveInstanceProperty* resolve(){ return NULL; } \
};

/// as above, extra params for enumerate and resolve funcs - used for array-like access
#define SCRIPT_CLASS_NAME_EXT(CLASS,CLASSNAME,ENUMFUNC,RESOLVEFUNC) template <> struct ScriptClassDesc<CLASS> { \
	static const string& name(){ static string _name = CLASSNAME; return _name; } \
	static EnumerateInstanceProperties* enumerate(){ static EnumerateInstanceProperties* _enum = ENUMFUNC; return _enum; } \
	static ResolveInstanceProperty* resolve(){ static ResolveInstanceProperty* _resolve = RESOLVEFUNC; return _resolve; } \
};

/* MARK:	-				Script Host
 
 Master class wrapping Spidermonkey Javascript engine.
 A descendent of ScriptableClass can make itself available inside script
 by using SCRIPT_CLASS_NAME template, and then calling
 RegisterClass(), AddProperty, DefineFunction, GetProperty, and SetProperty
 to define class characteristics.
 
 When instances are created from script, a constructor of your class with
 (ScriptArguments*) as a parameter will be called. You must call
 script.NewObject<MyClass>( this ) to attach a scriptObject to your instance.
 
 When Javascript garbage collector destroys script object representation of
 your class, it will also delete your object.
 
 -------------------------------------------------------------------- */


class ScriptHost {
private:
	
	/// stores a callback of a specific type
	struct ScriptCallback {
		ScriptValueCallback valueCallback;
		ScriptBoolCallback boolCallback;
		ScriptIntCallback intCallback;
		ScriptFloatCallback floatCallback;
		ScriptObjectCallback objectCallback;
		ScriptStringCallback stringCallback;
		ScriptArrayCallback arrayCallback;
		ScriptIndexCallback indexCallback;
	};

#define PROP_ENUMERABLE (1 << 0)
#define PROP_STATIC 	(1 << 1)
#define PROP_SERIALIZED (1 << 2)
#define PROP_READONLY 	(1 << 3)
#define PROP_NOSTORE 	(1 << 4)
#define PROP_EARLY	 	(1 << 5)
#define PROP_LATE	 	(1 << 6)
#define PROP_OVERRIDE 	(1 << 7)
	
	/// struct storing a getter and a setter for an class property
	struct GetterSetter {
		ScriptType type;
		ScriptCallback getterSetter[2];
		unsigned flags = 0;
	
		// constructor with type
		GetterSetter( ScriptType st, unsigned flags ) : type( st ), flags( flags ) {}
		GetterSetter(){};
		
		// initializers for each type
		void Init( ScriptValueCallback* g, ScriptValueCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].valueCallback = *g; if ( s ) getterSetter[1].valueCallback = *s;
		}
		void Init( ScriptBoolCallback* g, ScriptBoolCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].boolCallback = *g; if ( s ) getterSetter[1].boolCallback = *s;
		}
		void Init( ScriptIntCallback* g, ScriptIntCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].intCallback = *g; if ( s ) getterSetter[1].intCallback = *s;
		}
		void Init( ScriptFloatCallback* g, ScriptFloatCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].floatCallback = *g; if ( s ) getterSetter[1].floatCallback = *s;
		}
		void Init( ScriptObjectCallback* g, ScriptObjectCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].objectCallback = *g; if ( s ) getterSetter[1].objectCallback = *s;
		}
		void Init( ScriptStringCallback* g, ScriptStringCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].stringCallback = *g; if ( s ) getterSetter[1].stringCallback = *s;
		}
		void Init( ScriptArrayCallback* g, ScriptArrayCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].arrayCallback = *g; if ( s ) getterSetter[1].arrayCallback = *s;
		}
		void Init( ScriptIndexCallback* g, ScriptIndexCallback *s=NULL ) {
			flags |= ( s == NULL ? PROP_READONLY : 0 );
			getterSetter[0].indexCallback = *g; if ( s ) getterSetter[1].indexCallback = *s;
		}
		
	};
	
	/// property name -> getter + setter
	typedef unordered_map<string, GetterSetter> GetterSetterMap;
	typedef unordered_map<string, GetterSetter>::iterator GetterSetterMapIterator;
	
	/// function name -> callback
	typedef unordered_map<string, ScriptFunctionCallback> FuncMap;
	typedef unordered_map<string, ScriptFunctionCallback>::iterator FuncMapIterator;
	
	/// class definition structure
	struct ClassDef {
		
		/// class name visible in script
		string className = "[Uninitialized]";
		
		/// if true, this class can't be instantiated with 'new' in script
		bool singleton = false;
		
		/// Spidermonkey JSClass structure
		JSClass jsc = {
			NULL,
			JSCLASS_HAS_PRIVATE,
			JS_PropertyStub,
			JS_DeletePropertyStub,
			JS_PropertyStub,
			JS_StrictPropertyStub,
			JS_EnumerateStub,
			JS_ResolveStub,
			JS_ConvertStub
		};
		
		/// class prototype object
		JSObject* proto = NULL;
		
		/// parent class
		ClassDef* parent = NULL;
		
		/// property name -> getter + setter
		GetterSetterMap getterSetter;
		
		/// funcname -> ScriptFunctionCallback
		FuncMap funcs;

	};

	/// callback for Javascript errors
	static void ErrorReport ( JSContext *cx, const char *message, JSErrorReport *report );
	
	/// class name -> class definition struct
	typedef unordered_map<string, ClassDef> ClassMap;
	typedef unordered_map<string, ClassDef>::iterator ClassMapIterator;
	
	/// class definitions
	ClassMap classDefinitions;
	
	/// global Javascript class
	JSClass global_class;
	
	/// function callbacks for global class function overrides (i.e. functions added to built-in classes, like String)
	FuncMap classFuncCallbacks;
	
	/// global garbage collector compartment
	JSAutoCompartment *compartment = NULL;
	
	
/* MARK:	-				Property get/set
 -------------------------------------------------------------------- */
	
	
	/// helper to look up class definition by class name
	inline static ClassDef* CDEF( string className ) {
		ClassMapIterator p = script.classDefinitions.find( className );
		return p == script.classDefinitions.end() ? NULL : &p->second;
	}
	
	/// generic resolver + getter/setter
	bool PropGetterSetter( JSContext *cx, int getOrSet, void* self, ClassDef* cdef, string& propName, JS::MutableHandleValue vp, uint32_t index=0 ) {
		
		// get callback
		GetterSetterMapIterator gsi = cdef->getterSetter.find( propName );
		
		// if found
		if ( gsi != cdef->getterSetter.end() ) {
			GetterSetter *gs = &gsi->second;
			
			// if setting a read only prop, fail
			if ( getOrSet == 1 && ( gs->flags & PROP_READONLY ) ) return false;
			
			// based on property type, call callback
			if ( gs->type == TypeFloat ) {
				double dval = 0;
				ToNumber( cx, vp, &dval );
				vp.setDouble( (double) gs->getterSetter[getOrSet].floatCallback( self, (float) dval ) );
			} else if ( gs->type == TypeInt ){
				int32_t ival = 0;
				ToInt32( cx, vp, &ival );
				vp.setInt32( gs->getterSetter[getOrSet].intCallback( self, ival ) );
			} else if ( gs->type == TypeBool ) {
				bool bval = ToBoolean( vp );
				vp.setBoolean( gs->getterSetter[getOrSet].boolCallback( self, bval ) );
			} else if ( gs->type == TypeObject ) {
				JSObject* oval = vp.isObjectOrNull() ? vp.toObjectOrNull() : NULL;
				vp.setObjectOrNull( (JSObject*) gs->getterSetter[getOrSet].objectCallback( self, oval ) );
			} else if ( gs->type == TypeValue ) {
				vp.set( gs->getterSetter[getOrSet].valueCallback ( self, ArgValue( vp.get() ) ).toValue() );
			} else if ( gs->type == TypeString ){
				JSString* str = JS_ValueToString( cx, vp );
				char* sval = JS_EncodeString( cx, str );
				string stval = sval;
				JS_free( cx, sval );
				stval = gs->getterSetter[getOrSet].stringCallback( self, stval );
				RootedString str2( cx, JS_NewStringCopyZ( cx, stval.c_str() ) );
				vp.setString( str2 );
			} else if ( gs->type == TypeIndex ) {
				vp.set( gs->getterSetter[getOrSet].indexCallback ( self, index, ArgValue( vp.get() ) ).toValue() );
			} else if ( gs->type == TypeArray ){
				ArgValue av( vp.get() );
				ArgValueVector *in = av.value.arrayValue;
				ArgValueVector *out = gs->getterSetter[getOrSet].arrayCallback( self, in );
				if ( out ) {
					vp.set( ScriptArguments::ArrayToVal( *out ) );
					if ( out != in ) delete out;
				} else {
					vp.setNull();
				}				
			}
			
			// bail if exception in getter or setter
			return !JS_IsExceptionPending( script.js );
			
		} else if ( cdef->parent ) {
			
			// try in parent
			return script.PropGetterSetter( cx, getOrSet, self, cdef->parent, propName, vp );
			
		}
		
		// not found
		return true;
		
	}
	
	/// generic setter
	static bool PropSetter( JSContext *cx, HandleObject obj, HandleId id, bool strict, MutableHandleValue vp ){
		// scoped request
		//JSAutoRequest req( cx );
		//JSAutoCompartment( cx, obj );
		
		// find class
		JSClass* jc = JS_GetClass( obj );
		ClassDef *cdef = CDEF( string( jc->name ) );
		
		// if found, and [id] is string
		if ( cdef && JSID_IS_STRING( id ) ) {
			// get property name
			char *idString = JS_EncodeString( cx, JSID_TO_STRING( id ) );
			string propName(idString);
			JS_free( cx, idString );
			
			return script.PropGetterSetter( cx, 1, JS_GetPrivate( obj ), cdef, propName, vp );
		// id is integer
		} else if( JSID_IS_INT( id ) ) {
			string numProp("#");
			return script.PropGetterSetter( cx, 1, JS_GetPrivate( obj ), cdef, numProp, vp, JSID_TO_INT( id ) );
		}
		
		return true;
	}
	
	/// generic getter
	static bool PropGetter (JSContext *cx, HandleObject obj, HandleId id, MutableHandleValue vp ) {
		// scoped request
		//JSAutoRequest req( cx );
		//JSAutoCompartment( cx, obj );
		
		// find class
		void* self = JS_GetPrivate( obj );
		if ( !self ) return false;
		JSClass* jc = JS_GetClass( obj );
		ClassDef *cdef = CDEF( string( jc->name ) );
		
		// if found, and [id] is string
		if ( cdef && JSID_IS_STRING( id ) ) {
			// get property name
			char *idString = JS_EncodeString( cx, JSID_TO_STRING( id ) );
			string propName(idString);
			JS_free( cx, idString );
			
			return script.PropGetterSetter( cx, 0, self, cdef, propName, vp );
			
		// id is integer
		} else if( JSID_IS_INT( id ) ){
			string numProp("#");
			return script.PropGetterSetter( cx, 0, self, cdef, numProp, vp, JSID_TO_INT( id ) );
		}
		
		return true;
	}
	
	/// used to return enumerable element indexes using class callback
	template<class CLASS>
	static JSBool EnumerateProp (JSContext *cx, JS::Handle<JSObject*> obj, JSIterateOp enum_op,
								 JS::MutableHandle<JS::Value> statep, JS::MutableHandleId idp) {
		// scoped request
		//JSAutoRequest req( cx );
		//JSAutoCompartment( cx, obj ); // JSNewEnumerateOp
		
		// begin
		ArgValueVector* props = NULL;
		bool noMore = false;
		if ( enum_op == JSENUMERATE_INIT || enum_op == JSENUMERATE_INIT_ALL ) {
			// make sure we're on base class
			CLASS* self = (CLASS*) JS_GetPrivate( obj );
			jsval numProps;
			if ( self ) {
				// get property names
				props = (*ScriptClassDesc<CLASS>::enumerate())( self );
				numProps.setInt32( (int) props->size() );
			} else {
				numProps.setInt32( 0 );
			}
			statep.set( PRIVATE_TO_JSVAL( props ) );
			JS_ValueToId( cx, numProps, idp.address() );
			
		// next element
		} else if ( enum_op == JSENUMERATE_NEXT ) {
			props = (ArgValueVector*) JSVAL_TO_PRIVATE( statep.get() );
			// still elements in array
			if ( props && props->size() ){
				// return last one, pop
				ArgValue lastVal = props->back();
				props->pop_back();
				JS_ValueToId( cx, lastVal.toValue(), idp.address() );
			} else {
				noMore = true;
			}
			
		// end loop
		} else if ( enum_op == JSENUMERATE_DESTROY ) {
			props = (ArgValueVector*) JSVAL_TO_PRIVATE( statep.get() );
			noMore = true;
		}
		
		// cleanup
		if ( noMore ) {
			statep.set( JSVAL_NULL );
			if ( props ) delete props;
		}
		
		return true;
	}

	/// used to resolve a property using a class callback
	template<class CLASS>
	static JSBool ResolveProp (JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, unsigned flags,
								JS::MutableHandleObject objp) {
		// scoped request
		//JSAutoRequest req( cx );
		//JSAutoCompartment( cx, obj ); // JSNewEnumerateOp
		
		// find class
		ArgValue prop;
		if( JSID_IS_STRING( id ) ) {
			JSString* s = JSID_TO_STRING( id );
			prop = STRING_TO_JSVAL( s );
		} else if ( JSID_IS_INT( id ) ){
			prop = JSID_TO_INT( id );
		}
		// ask class to resolve prop - returns true if property is resolved
		CLASS* self = (CLASS*) JS_GetPrivate( obj );
		if ( self && (*ScriptClassDesc<CLASS>::resolve())( self, prop ) ) {
			objp.set( obj );
		} else {
			objp.set( NULL );
		}
		return true;
	}
	
	
/* MARK:	-				Function callback
 -------------------------------------------------------------------- */

	
	template <class CLASS>
	static bool FuncCallbackCall ( JSContext *cx, ClassDef* cdef, CLASS* self, string& funcName, ScriptArguments& args ) {
		// recursive search for func
		FuncMapIterator fi = cdef->funcs.find( funcName );
		if ( fi != cdef->funcs.end() ){
			ScriptFunctionCallback &callback = fi->second;
			bool ret = callback( self, args );
			ret = ret && !JS_IsExceptionPending( cx );
			return ret;
		} else if ( cdef->parent ){
			return FuncCallbackCall( cx, cdef->parent, self, funcName, args );
		}
		return false;
	}
	
	template <class CLASS>
	static bool FuncCallback ( JSContext *cx, unsigned argc, Value *vp ) {
		// scope
		//JSAutoRequest req( cx );
		
		// construct arguments
		CallArgs args = CallArgsFromVp( argc, vp );
		ScriptArguments sa( &args );
		JSObject* obj = &args.thisv().toObject();
		
		// look up
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		if ( !classDef ) return false;
		
		// get self
		CLASS* self = (CLASS*) JS_GetPrivate( obj );
		
		// func name
		jsval name;
		JS_GetProperty( cx, &args.callee(), "name", &name );
		
		// jsstring to string
		char *buf = JS_EncodeString( cx, JSVAL_TO_STRING( name ) );
		string funcName(buf);
		JS_free( cx, buf );
		
		// if self is null, but classDef
		
		// find and call func
		return FuncCallbackCall( cx, classDef, self, funcName, sa );
	}
	
	static bool ClassFuncCallback ( JSContext *cx, unsigned argc, Value *vp ) {
		// scope
		//JSAutoRequest req( cx );
		
		// construct arguments
		CallArgs args = CallArgsFromVp( argc, vp );
		ScriptArguments sa( &args );
		
		// determine whether function is static, or built-in class, or global
		void* obj = NULL;
		string prefix, key;
		HandleValue thisv = args.thisv();
		ArgValue thisArgValue;
		
		// object
		if ( thisv.isObjectOrNull() ) {
			obj = thisv.toObjectOrNull();
			// global object, or null
			if ( obj == script.global_object || !obj ) {
				// global function
				obj = script.global_object;
			// some object
			} else {
				
				// get classname
				JSClass* clp = JS_GetClass( (JSObject*) obj );
				
				// if it's a constructor function
				if ( strcmp( clp->name, "Function" ) == 0 && JS_IsConstructor( (JSFunction*) obj ) ){
					// use its name
					ArgValue funcName = script.GetProperty( "name", obj );
					if ( funcName.type == TypeString ) {
						prefix.append( funcName.value.stringValue->c_str() );
					}
				} else {
					// key into table is Class.FunctionName
					prefix.append( clp->name );
				}
				prefix.append( "." );
			}
		// built in type
		} else {
			
			thisArgValue = thisv;
			obj = &thisArgValue;
			if ( thisv.isString() ) prefix = "String.";
			else if ( thisv.isNumber() ) prefix = "Number";
			
		}
		
		// get function name
		jsval name;
		JS_GetProperty( cx, &args.callee(), "name", &name );
		char *buf = JS_EncodeString( cx, JSVAL_TO_STRING( name ) );
		string funcName = buf;
		JS_free( cx, buf );
		
		// make "Class.funcName" key
		key = prefix + funcName;
		
		// find function
		FuncMapIterator it = script.classFuncCallbacks.find( key );
		if ( it == script.classFuncCallbacks.end() ) return false;
		
		// call it
		ScriptFunctionCallback &callback = it->second;
		bool ret = callback( (void*) obj, sa );
		ret = ret && !JS_IsExceptionPending( cx );
		return ret;
	}
	
	static bool GlobalFuncCallback ( JSContext *cx, unsigned argc, Value *vp ) {
		// scope
		//JSAutoRequest req( cx );
		
		// construct arguments
		CallArgs args = CallArgsFromVp( argc, vp );
		ScriptArguments sa( &args );
		
		// find function name
		jsval name;
		JS_GetProperty( cx, &args.callee(), "name", &name );
		
		// jsstring to string
		char *buf = JS_EncodeString( cx, JSVAL_TO_STRING( name ) );
		string funcName = buf;
		JS_free( cx, buf );
		
		// find function
		FuncMapIterator it = script.classFuncCallbacks.find( string( funcName ) );
		if ( it == script.classFuncCallbacks.end() ) return false;
		
		// call it
		ScriptFunctionCallback &callback = it->second;
		bool ret = callback( NULL, sa );
		ret = ret && !JS_IsExceptionPending( cx );
		return ret;
	}

	
/* MARK:	-				Logging
 -------------------------------------------------------------------- */
	
	
	/// tracing function
	static bool Log( JSContext *cx, unsigned argc, Value *vp );
	
	
/* MARK:	-				Object lifecycle callbacks
 -------------------------------------------------------------------- */

	
	/// constructor callback
	template <class CLASS>
	static void Construct( JSContext *cx, unsigned argc, Value *vp ) {
		// JSAutoCompartment( cx, script.global_object );
		
		// params
		CallArgs args = CallArgsFromVp( argc, vp );
		const char *className = ScriptClassDesc<CLASS>::name().c_str();
		// make sure class can be constructed
		ClassDef* cdef = CDEF( className );
		if ( cdef->singleton ) {
			script.ReportError( "Class %s can't be constructed using 'new'\n", className );
			args.rval().setUndefined();
			return;
		}
		// construct object
		ScriptArguments sa( &args );
		CLASS* obj = new CLASS( &sa );
		obj->scriptClassName = cdef->jsc.name;
		
		// return its scriptObject
		args.rval().set( OBJECT_TO_JSVAL( (JSObject*) obj->scriptObject ) );
	}
	
	/// destructor for GameObject JS object
	template <class CLASS>
	static void Finalize ( JSFreeOp *fop, JSObject *obj )  {
		// delete underlying object
		CLASS* object = (CLASS*) JS_GetPrivate( obj );
		if ( object ) {
			delete object;
		}
	};
	
public:
	
	// Initializer
	ScriptHost() {
		
		// init JS context
		this->jsr = JS_NewRuntime(32L * 1024 * 1024, JS_USE_HELPER_THREADS );
		this->js = this->jsr ? JS_NewContext( this->jsr, 16384 ) : NULL;
		if ( !this->js || !this->jsr ) {
			printf( "Can't initialize Javascript runtime or context.\n" );
			exit( 1 );
		}
		
		// set context option
		JS_SetOptions( this->js, JS_GetOptions( this->js ) | JSOPTION_VAROBJFIX | JSOPTION_ASMJS );
		JS_SetParallelCompilationEnabled( this->js, true );
		JS_SetGlobalCompilerOption( this->js, JSCOMPILER_PJS_ENABLE, 1);
		
		// set error handler
		JS_SetErrorReporter( this->js, this->ErrorReport );
		
		// scoped request
		//JSAutoRequest req( this->js );
		
		// create global object
		this->global_class = { "Global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub };
		this->global_object = JS_NewGlobalObject( this->js, &this->global_class, NULL );
		
		// global GC compartment
		this->compartment = new JSAutoCompartment( this->js, this->global_object );
		
		// add standard classes to global object
		JS_InitStandardClasses( this->js, this->global_object );
		
		// add global functions
		JS_DefineFunction( this->js, this->global_object, "log", (JSNative) this->Log, 0, JSPROP_READONLY | JSPROP_ENUMERATE );
		
		// add global object as root for GC
		JS_AddObjectRoot( this->js, &this->global_object );
		AddGlobalNamedObject( "global", this->global_object );
		
		// printf ( "Spidermonkey initialized\n" );
	}
	
	// shutdown
	void Shutdown () {
		delete this->compartment;
		while ( JS_IsExceptionPending( this->js ) ) {
			JS_ClearPendingException( this->js );
		}
		JS_DestroyContext( this->js );
		JS_DestroyRuntime( this->jsr );
		JS_ShutDown();
		this->js = NULL;
		this->jsr = NULL;
	}
	
	// destructor
	~ScriptHost() {}
	
	/// Javascript runtime
	JSRuntime* jsr;
	
	/// Javascript context
	JSContext* js;

	/// global object
	JSObject *global_object;
	
	bool IsAboutToBeFinalized( void** obj ) {
	
		return JS_IsAboutToBeFinalized( (JSObject**) obj );
		
	}

/* MARK:	-				Error reporting
 -------------------------------------------------------------------- */

	
	/// print error and exit
	/// call to print out errors from callback functions
	void ReportError(const char *fmt, ... ) {
		
		va_list myargs;
		va_start(myargs, fmt);
		char buf[ 1024 ];
		vsprintf( buf, fmt, myargs );
		va_end(myargs);
		
		JS_ReportError( script.js, "%s", buf );
		
	}
	
	
/* MARK:	-				Class registration
 -------------------------------------------------------------------- */

	
	/// register class for scripting
	template <class CLASS>
	void RegisterClass( const char *parentClassName=NULL, bool singleton=false ){
		// scoped request
		//JSAutoRequest req( this->js );
		
		// insert
		pair<ClassMapIterator,bool> ins = classDefinitions.insert( make_pair( ScriptClassDesc<CLASS>::name(), ClassDef() ) );
		ClassDef *def = &ins.first->second;
		
		// init class
		def->className = ScriptClassDesc<CLASS>::name();
		def->singleton = singleton;
		def->jsc.name = def->className.c_str(),
		def->jsc.finalize = (JSFinalizeOp) ScriptHost::Finalize<CLASS>;
		def->jsc.trace = (JSTraceOp) ScriptHost::TraceScriptableClass<CLASS>;
		def->jsc.getProperty = (JSPropertyOp) ScriptHost::PropGetter;
		def->jsc.setProperty = (JSStrictPropertyOp) ScriptHost::PropSetter;
		if ( ScriptClassDesc<CLASS>::enumerate() != NULL ){
			def->jsc.enumerate = (JSEnumerateOp) ScriptHost::EnumerateProp<CLASS>;
			def->jsc.flags |= JSCLASS_NEW_ENUMERATE;
		}
		if ( ScriptClassDesc<CLASS>::resolve() != NULL ){
			def->jsc.resolve = (JSResolveOp) ScriptHost::ResolveProp<CLASS>;
			def->jsc.flags |= JSCLASS_NEW_RESOLVE;
		}
		def->parent = ( def->parent = CDEF( string( parentClassName ? parentClassName : "ScriptableObject" ) ) ) == def ? NULL : def->parent;
		
		// register JS class
		def->proto = JS_InitClass( this->js, this->global_object,
								 ( def->parent ? def->parent->proto : NULL ),
								 &def->jsc, (JSNative) &ScriptHost::Construct<CLASS>,
								 0, NULL, NULL, NULL, NULL );
		// make constructor func enumerable in global object
		JSBool found;
		JS_SetPropertyAttributes( this->js, this->global_object, def->className.c_str(), JSPROP_PERMANENT | JSPROP_ENUMERATE, &found );
	}

	
/* MARK:	-				Define function
 -------------------------------------------------------------------- */

	
	template <class CLASS>
	void DefineFunction( const char *funcName, ScriptFunctionCallback callback ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		JS_DefineFunction( this->js, classDef->proto, funcName, (JSNative) FuncCallback<CLASS>, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT );//JSPROP_PERMANENT
		classDef->funcs[ string(funcName) ] = callback;
	}
	
	
/* MARK:	-				Add property
 -------------------------------------------------------------------- */

	
	template <class CLASS>
	void AddIndexProperty( ScriptIndexCallback getter, ScriptIndexCallback setter, unsigned flags=PROP_ENUMERABLE | PROP_NOSTORE ){ //PROP_ENUMERABLE|PROP_SERIALIZED
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		GetterSetter gs( TypeIndex, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string("#") ] = gs;
	}
	
	template <class CLASS>
	void AddIndexProperty( ScriptIndexCallback getter, unsigned flags=PROP_NOSTORE ){ //PROP_ENUMERABLE|PROP_SERIALIZED
																					  //JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		GetterSetter gs( TypeIndex, flags ); gs.Init( &getter );
		classDef->getterSetter[ string("#") ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptValueCallback getter, ScriptValueCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeValue, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptValueCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY;
		JS_DefineProperty( this->js, ((flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeValue, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptIntCallback getter, ScriptIntCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_ZERO, NULL, NULL, jflags );
		GetterSetter gs( TypeInt, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptIntCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY ;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_ZERO, NULL, NULL, jflags);
		GetterSetter gs( TypeInt, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}

	template <class CLASS>
	void AddProperty( const char *propName, ScriptFloatCallback getter, ScriptFloatCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_ZERO, NULL, NULL, jflags );
		GetterSetter gs( TypeFloat, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptFloatCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY ;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_ZERO, NULL, NULL, jflags );
		GetterSetter gs( TypeFloat, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptBoolCallback getter, ScriptBoolCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_FALSE, NULL, NULL, jflags );
		GetterSetter gs( TypeBool, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptBoolCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_FALSE, NULL, NULL, jflags );
		GetterSetter gs( TypeBool, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptStringCallback getter, ScriptStringCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeString, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptStringCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeString, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptObjectCallback getter, ScriptObjectCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeObject, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptObjectCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeObject, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptArrayCallback getter, ScriptArrayCallback setter, unsigned flags=PROP_ENUMERABLE|PROP_SERIALIZED ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags );
		GetterSetter gs( TypeArray, flags ); gs.Init( &getter, &setter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
	template <class CLASS>
	void AddProperty( const char *propName, ScriptArrayCallback getter, unsigned flags=PROP_ENUMERABLE ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		unsigned jflags = ((flags & PROP_NOSTORE) ? JSPROP_SHARED : 0 ) | ((flags & PROP_ENUMERABLE) ? JSPROP_ENUMERATE : 0) | (flags & PROP_OVERRIDE ? 0 : JSPROP_PERMANENT) | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY;
		JS_DefineProperty( this->js, ( (flags & PROP_STATIC) ? JS_GetConstructor( this->js, classDef->proto) : classDef->proto ),
						  propName, JSVAL_NULL, NULL, NULL, jflags);
		GetterSetter gs( TypeArray, flags ); gs.Init( &getter );
		classDef->getterSetter[ string(propName) ] = gs;
	}
	
/* MARK:	-				Property set
 -------------------------------------------------------------------- */

	
	/// sets property on script object
	void SetProperty( const char *propName, ArgValue value, void* obj ) {
		//JSAutoRequest req( this->js );
		jsval val = value.toValue();
		JS_SetProperty( this->js, (JSObject*) obj, propName, &val );
	}
	
	/// sets property on script object (pointer version)
	void SetProperty( const char *propName, ArgValue *value, void* obj ) {
		//JSAutoRequest req( this->js );
		jsval val = value->toValue();
		JS_SetProperty( this->js, (JSObject*) obj, propName, &val );
	}
	
	/// sets property on script object (string version)
	void SetProperty( const char *propName, const char* value, void* obj ) {
		//JSAutoRequest req( this->js );
		JSString* str = JS_NewStringCopyZ( this->js, value );
		jsval val = STRING_TO_JSVAL( str );
		JS_SetProperty( this->js, (JSObject*) obj, propName, &val );
	}
	
	/// prevent adding more properties
	void FreezeObject( void *obj ) {
		
		JS_FreezeObject( this->js, (JSObject*) obj );
		
	}
	
/* MARK:	-				Property get
 -------------------------------------------------------------------- */

	
	/// returns property of script object
	ArgValue GetProperty( const char *propName, void* obj ) {
		//JSAutoRequest req( this->js );
		if ( !obj ) return ArgValue(); // undefined
		jsval val;
		JS_GetProperty( this->js, (JSObject*) obj, propName, &val );
		return ArgValue( val );
	}
	
	/// returns property of script object
	ArgValue GetElement( uint32_t index, void* obj ) {
		//JSAutoRequest req( this->js );
		jsval val;
		JS_GetElement( this->js, (JSObject*) obj, index, &val);
		return ArgValue( val );
	}
	
	/// returns property of a constructor object
	ArgValue GetClassProperty( const char* className, const char* propName ) {
		//JSAutoRequest req( this->js );
		JSObject *class_constructor = NULL;//, *class_prototype = NULL;
		jsval val;
		
		// get constructor from the global object.
		if (!JS_GetProperty( this->js, this->global_object, className, &val ) || JSVAL_IS_PRIMITIVE( val ) ) {
			printf( "GetClassProperty: class %s is not found\n", className );
			return false;
		}
		class_constructor = JSVAL_TO_OBJECT(val);
		
		// get property
		if ( !JS_GetProperty( this->js, class_constructor, propName, &val ) ) {
			printf( "GetClassProperty: %s.%s is not found.\n", className, propName );
			return ArgValue();
		}
		
		// return
		return ArgValue( val );
	}

/* MARK:	-				Helper
 -------------------------------------------------------------------- */
	
	
	void* GetTypePrototypeObject( ScriptType type ) {

		// only used with string and number right now, can add others if needed
		JSObject *ret = NULL;
		switch( type ) {
			case TypeBool:
				JS_GetClassPrototype( this->js, JSProtoKey::JSProto_Boolean, &ret );
				break;
			case TypeFloat:
			case TypeInt:
			case TypeDouble:
				JS_GetClassPrototype( this->js, JSProtoKey::JSProto_Number, &ret );
				break;
			case TypeArray:
				JS_GetClassPrototype( this->js, JSProtoKey::JSProto_Array, &ret );
				break;
			case TypeString:
				JS_GetClassPrototype( this->js, JSProtoKey::JSProto_String, &ret );
				break;
			default:
				break;
		}
		return (void*) ret;
	}
	
	
/* MARK:	-				Function call
 -------------------------------------------------------------------- */
	
	
	/// calls function on object with params
	ArgValue CallFunction( void* funcObject, void* thisObject, ScriptArguments &args ){
		jsval rval;
		int argc;
		jsval* params = args.GetFunctionArguments( &argc );
		if ( JS_IsExceptionPending( this->js ) ) {
			JS_ReportPendingException( this->js );
			return ArgValue();
		}
		if ( JS_CallFunction( script.js, thisObject ? (JSObject*) thisObject : script.global_object, (JSFunction*) funcObject, argc, params, &rval ) ) {
			return ArgValue( rval );
		} else return ArgValue();
	}
	
	/// calls global function with params
	ArgValue CallGlobalFunction( const char* funcName, ScriptArguments &args ){
		jsval rval;
		int argc;
		jsval* params = args.GetFunctionArguments( &argc );
		/*if ( JS_IsExceptionPending( this->js ) ){
			return ArgValue();
		}*/
		if ( JS_CallFunctionName( script.js, script.global_object, funcName, argc, params, &rval ) ) {
			return ArgValue( rval );
		} else return ArgValue();
	}
	
	
/* MARK:	-				Serialization
 -------------------------------------------------------------------- */

	
	/// returns init object, // force option skips serializeable check for top level object only
	ArgValue MakeInitObject( ArgValue& val, bool force=false, bool forCloning=false );
	
	/// populates properties of object. Params includeIntKeys - include numeric keys of array-like object, useSerializeMask - exclude props in serializeMask, includeFunctions - include non-native functions as source
	void GetProperties( void* obj, ArgValueVector* ret, bool useSerializeMask, bool includeReadOnly, bool includeFunctions );
	
	private:
	
	// recursively construct init object
	ArgValue _MakeInitObject( ArgValue val, unordered_map<unsigned long,JSObject*> &alreadySerialized, bool force=false, bool forCloning=false );
	
	// places enumerable properties of given object, including all non-readonly props defined for scriptable class into given set. Returns ClassDef, if found.
	// ClassDef* _GetPropertyNames( void* obj, unordered_set<string>& ret, ClassDef* cdef=NULL );
	
	/// populates properties of object. Params includeIntKeys - include numeric keys of array-like object, includeFunctions - include non-native functions as source
	ClassDef* _GetProperties( void* obj, void* thisObj, unordered_set<string>& ret, bool useSerializeMask, bool includeReadOnly, bool includeFunctions, ClassDef* cdef );
	
	// used to fill out stubs when unserializing
	struct _StubRef {
		JSObject* object = NULL;
		string propName;
		int index = -1;
		_StubRef( JSObject* o, string s, int i ) : object( o ), propName( s ), index( i ){};
	};
	
	public:
	
	/// unserialize obj using initObject
	void* InitObject( void* initObj, bool forCloning=false );
	
	private:
	void* _InitObject( void* obj, void* initObj,
					  unordered_map<string, void*> *alreadyInitialized,
					  unordered_map<string, vector<_StubRef>> *stubs,
					  vector<ScriptableClass*> *awakeList, bool forCloning=false );
	
	bool _IsInitObject( void *obj, string& className );
	
	bool _IsStub( void *obj, string& stubName );
	
	void _AddToAlreadyInitialized( void *obj, void* initObj, unordered_map<string, void*> *alreadyInitialized );
	
	public:
	
	
/* MARK:	-				Static properties
 -------------------------------------------------------------------- */

	
	/// set static property to a new value (property is set on object constructor)
	template <class CLASS>
	void SetStaticProperty( const char *propName, ArgValue value ){
		//JSAutoRequest req( this->js );
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		JSObject* target = JS_GetConstructor( this->js, classDef->proto );
		jsval val = value.toValue();
		JS_SetProperty( this->js, target, propName, &val );
		JSBool f; JS_SetPropertyAttributes( this->js, this->global_object, propName, JSPROP_PERMANENT | JSPROP_READONLY, &f );
	}
	
	
/* MARK:	-				Global constants, properties, and functions
 -------------------------------------------------------------------- */


	/// create a global const value
	void SetGlobalConstant( const char *propName, ArgValue value ){
		//JSAutoRequest req( this->js );
		jsval val = value.toValue();
		JS_SetProperty( this->js, this->global_object, propName, &val );
		JSBool f; JS_SetPropertyAttributes( this->js, this->global_object, propName, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE, &f );
	}
	
	/// adds a string that's protected from GC and automatically shared with other code that needs a string with the same value.
	void InternString( const char* s ) {
		JS_InternString( this->js, s );
	}
	
	/// adds global property propName referencing obj
	void AddGlobalNamedObject( const char* propName, void* obj ) { JS_DefineProperty( this->js, this->global_object, propName, OBJECT_TO_JSVAL( (JSObject*) obj ), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT ); }
	
	/// adds a function to a global class (i.e. JSON, or Date)
	bool DefineClassFunction( const char *className, const char *funcName, bool isStatic, ScriptFunctionCallback callback ){
		//JSAutoRequest req( this->js );
		JSObject *class_constructor = NULL, *class_prototype = NULL;
		jsval val;
		
		// get constructor from the global object.
		if (!JS_GetProperty( this->js, this->global_object, className, &val ) || JSVAL_IS_PRIMITIVE( val ) ) {
			printf( "DefineClassFunction: class %s is not found\n", className );
			return false;
		}
		class_constructor = JSVAL_TO_OBJECT(val);
		
		// get prototype
		if ( !isStatic ) {
			if ( !JS_GetProperty( this->js, class_constructor, "prototype", &val ) || JSVAL_IS_PRIMITIVE( val ) ) {
				printf( "DefineClassFunction: %s.prototype is not an object.\n", className );
				return false;
			}
			class_prototype = JSVAL_TO_OBJECT(val);
		}
		// make key
		string key( className );
		key.append( "." ); key.append( funcName );
		
		// add function
		JS_DefineFunction( this->js, isStatic ? class_constructor : class_prototype, funcName, (JSNative) ClassFuncCallback, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT );
		this->classFuncCallbacks[ key ] = callback;
		return true;
	}

	/// adds a globally visible function
	void DefineGlobalFunction( const char *funcName, ScriptFunctionCallback callback ){
		JS_DefineFunction( this->js, this->global_object, funcName, (JSNative) GlobalFuncCallback, 0, JSPROP_ENUMERATE | JSPROP_PERMANENT );
		this->classFuncCallbacks[ string( funcName ) ] = callback;
	}
	
	/// calls a built-in class static function
	ArgValue CallClassFunction( const char* className, const char *funcName, ScriptArguments& args ) {
		//JSAutoRequest req( this->js );
		JSObject *class_constructor = NULL;
		JS::Value val;
		
		// get constructor from the global object.
		if (!JS_GetProperty( this->js, this->global_object, className, &val ) || JSVAL_IS_PRIMITIVE( val ) ) {
			printf( "CallClassFunction: class %s is not found\n", className );
			return ArgValue();
		}
		class_constructor = JSVAL_TO_OBJECT(val);
		
		// call
		jsval rval;
		int argc;
		jsval* params = args.GetFunctionArguments( &argc );
		if ( !JS_CallFunctionName(script.js, class_constructor, funcName, argc, params, &rval ) ) {
			// printf( "CallClassFunction: %s.%s is not a function\n", className, funcName );
			return ArgValue();
		}
		return ArgValue( rval );
	}
	
	/// calls given function using a built-in class as this
	ArgValue CallClassFunction( const char* className, const void *func, ScriptArguments& args ) {
		//JSAutoRequest req( this->js );
		JSObject *class_constructor = NULL;
		JS::Value val;
		
		// get constructor from the global object.
		if (!JS_GetProperty( this->js, this->global_object, className, &val ) || JSVAL_IS_PRIMITIVE( val ) ) {
			printf( "CallClassFunction: class %s is not found\n", className );
			return ArgValue();
		}
		class_constructor = JSVAL_TO_OBJECT(val);
		
		// call
		jsval rval;
		int argc;
		jsval* params = args.GetFunctionArguments( &argc );
		if ( !JS_CallFunction(script.js, class_constructor, (JSFunction*) func, argc, params, &rval ) ) {
			printf( "CallClassFunction: %s call failed\n", className );
			return ArgValue();
		}
		return ArgValue( rval );
	}
	
	/// calls a member function
	ArgValue CallFunction( void* thisObj, const char *funcName, ScriptArguments& args ) {
		//JSAutoRequest req( this->js );
		
		// call
		jsval rval;
		int argc;
		jsval* params = args.GetFunctionArguments( &argc );
		if ( !JS_CallFunctionName(script.js, (JSObject*) thisObj, funcName, argc, params, &rval ) ) {
			printf( "CallClassFunction: %s is not a function\n", funcName );
			return ArgValue();
		}
		return ArgValue( rval );
	}
	
	// copies properties from src to dest, respecting property definition order (no recursion, or _InitObject like functionality)
	void CopyProperties( void* src, void* dest );
	

/* MARK:	-				Instance and class lookup
 -------------------------------------------------------------------- */

	
	template <class CLASS>
	/// returns instance of class
	CLASS* GetInstance( void* scriptObject ){
		JSObject* obj = (JSObject*) scriptObject;
		if ( !obj ) return NULL;
		JSClass* clp = JS_GetClass( obj );
		if ( !(clp->flags & JSCLASS_HAS_PRIVATE) ) return NULL;
		void* pdata = JS_GetPrivate( obj );
		if ( static_cast<CLASS*>( pdata ) == nullptr ) return NULL;
		// verify class
		string className = ScriptClassDesc<CLASS>::name();
		ClassDef* cdef = CDEF( clp->name );
		while ( cdef ) {
			if ( cdef->className.compare( className ) == 0 ) return (CLASS*) pdata;
			cdef = cdef->parent;
		}
		return NULL;
	}
	
	// returns a string matching classname of constructor function (used by GameObject.getBehavior)
	const char* GetClassNameByConstructor( void* constructorFunc ){
		JSObject* obj = (JSObject*) constructorFunc;
		jsval vp;
		JS_GetProperty( this->js, obj, "prototype", &vp);
		obj = vp.toObjectOrNull();
		if ( obj ) {
			JSClass* c = JS_GetClass( obj );
			return c->name;
		}
		return NULL;
	}
	
	bool IsObjectDescendentOf( void* scriptObject, const char *className ) {
		// null matches all
		// classname is "Object" - matches any object
		if ( !scriptObject || strcmp( className, "Object" ) == 0 ) return true;
		// get class
		JSClass* clp = JS_GetClass( (JSObject*) scriptObject );
		do {
			// class name matches - yes
			if ( strcmp( clp->name, className ) == 0 ) return true;
			
			// if ScriptableClass, go up parent chain
			ClassDef* cdef = CDEF( clp->name );
			if ( cdef ) {
				if ( cdef->parent ) {
					clp = &cdef->parent->jsc;
				} else {
					clp = NULL;
				}
			// regular object
			} else {
				clp = NULL;
				jsval vp;
				if ( JS_GetProperty( this->js, (JSObject*) scriptObject, "prototype", &vp ) && vp.isObjectOrNull() ) {
					scriptObject = vp.toObjectOrNull();
					if ( scriptObject ) {
						clp = JS_GetClass( (JSObject*) scriptObject );
					}
				}
				
			}
			
		} while( clp != NULL );
			
		return false;
	}
	
	
/* MARK:	-				New object
 -------------------------------------------------------------------- */

	
	/// constructs and attaches a new scripting object to class instance. This must be called from constructor with ScriptArguments* parameter
	template <class CLASS>
	bool NewScriptObject( CLASS* instance ) {
		//JSAutoRequest req( this->js );
		// find classDef
		ClassDef *classDef = CDEF( ScriptClassDesc<CLASS>::name() );
		instance->scriptClassName = classDef->jsc.name;
		// add
		instance->scriptObject = JS_NewObject( this->js, &classDef->jsc, classDef->proto, NULL );
		JS_SetPrivate( (JSObject*) instance->scriptObject, instance );
		return true;
	}
	
	/// new blank object
	void* NewObject( const char* className=NULL ) {
		
		// plain object
		if ( !className ) return JS_NewObject( this->js, NULL, NULL, NULL );
		
		// find constructor
		jsval cval;
		if ( JS_GetProperty( this->js, this->global_object, className, &cval ) && cval.isObject() ) {
			JSObject* cons = cval.toObjectOrNull();
			return JS_New( this->js, cons, 0, NULL );//1, &param );
			
		// failed to find constructor
		} else {
			printf( "Failed to find construstor for %s\n", className );
			return JS_NewObject( this->js, NULL, NULL, NULL );
		}
		
	}

	
/* MARK:	-				Garbage collection
 -------------------------------------------------------------------- */

	
	template<class CLASS>
	static void TraceScriptableClass( JSTracer *trc, JSObject *obj ) {
		// ask instance which objects should be protected from GC
		vector<void**> protectedObjects;
		CLASS* instance = script.GetInstance<CLASS>( obj );
		if ( !instance ) return;
		instance->TraceProtectedObjects( protectedObjects );
		
		// protect them
		for ( size_t i = 0, np = protectedObjects.size(); i < np; i++ ){
			JS_CallObjectTracer( trc, (JSObject**) protectedObjects.at( i ), "" );
		}
	}
	
	/// protect / release script object from garbage collecton
	void ProtectObject( void ** obj, bool protect ) {
		
		if ( !script.js ) return; // if called after shutdown, ignore

		// protect
		if ( protect ) {
			JS_AddObjectRoot( script.js, (JSObject**) obj );
		} else {
			JS_RemoveObjectRoot( script.js, (JSObject**) obj );
		}
	}
	
	/// call garbage collector
	void GC() {
		// call garbage collection in Spidermonkey
		JS_GC( this->jsr );
	}
	
	
/* MARK:	-				Script execution & JSON
 -------------------------------------------------------------------- */

	ArgValue Evaluate( const char *code, const char* filename=NULL, void* thisObj=NULL ) {
		//JSAutoRequest req( this->js );
		RootedValue rval( this->js );
		int lineno = 1;
		// execute
		if ( !JS_EvaluateScript( this->js, thisObj ? ((JSObject*) thisObj ) : this->global_object, code, (int) strlen(code), filename ? filename : "", lineno, rval.address() )) {
			// resulted in error?
			if ( JS_IsExceptionPending( this->js ) ) {
				// return error object
				RootedValue err( this->js );
				JS_GetPendingException( this->js, err.address() );
				JS_ClearPendingException( this->js );
				return ArgValue( err.get() );
			}
		}
		return ArgValue( rval.get() );
	}
	
	/// execute compiled script resource
	bool Execute( ScriptResource* scriptResource, void* thisObject=NULL, ArgValue *out=NULL ) {
		
		// make sure it's valid
		if ( !scriptResource || scriptResource->error ) {
			if ( scriptResource->error == ERROR_COMPILE ) {
				printf( "Failed to execute %s - script isn't compiled.\n", scriptResource->key.c_str() );
			} else if ( scriptResource->error == ERROR_NOT_FOUND ){
				printf( "Script %s not found.\n", scriptResource->key.c_str() );
			}
			return false;
		}
		
		// context
		JSObject* obj = thisObject ? (JSObject*) thisObject : this->global_object;
		
		// run
		RootedValue rval( this->js );
		RootedObject robj( this->js, obj );
		// JSAutoCompartment( this->js, this->global_object );
		
		// Uint32 t = SDL_GetTicks(); // time script execution
		// execute
		JSBool success = JS_ExecuteScript( this->js, robj, scriptResource->compiledScript, rval.address() );
		if ( success && out ) *out = ArgValue( *rval.address() );
		// printf( "%s took %d ms\n", scriptResource->key.c_str(), SDL_GetTicks() - t );
		
		return success;
	}
	
	/// returns object containing parsed JSON or NULL on failure
	void* ParseJSON( const char* jsonString ) {
		RootedValue val (this->js);
		JSString* tempString = JS_NewStringCopyZ( script.js, jsonString );
		size_t slen = 0;
		const jschar* jbuf = JS_GetStringCharsAndLength( script.js, tempString, &slen );
		if ( JS_ParseJSON( script.js, jbuf, (uint32_t) slen, &val ) ) {
			return val.get().toObjectOrNull();
		}
		// fail
		return NULL;
	}
	
};

#endif /* JavascriptHost_hpp */


