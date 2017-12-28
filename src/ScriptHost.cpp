#include "ScriptHost.hpp"
#include "ScriptableClass.hpp"


/* MARK:	-				Serialization
 -------------------------------------------------------------------- */


/// returns init object
ArgValue ScriptHost::MakeInitObject( ArgValue& val ) {
	// this will track of all objects already added
	unordered_map<unsigned long, JSObject*> alreadySerialized;
	return _MakeInitObject( val, alreadySerialized );
};

/// populates property names
void ScriptHost::GetPropertyNames( void* obj, unordered_set<string>& ret ) {
	this->_GetPropertyNames( obj, ret );
}

// recursively construct init object
ArgValue ScriptHost::_MakeInitObject( ArgValue val, unordered_map<unsigned long,JSObject*> &alreadySerialized ) {
	
	// if value is an array
	if ( val.type == TypeArray ) {
		// replace each value with processed value
		for ( size_t i = 0, ne = val.value.arrayValue->size(); i < ne; i++ ) {
			(*val.value.arrayValue)[ i ] = _MakeInitObject( (*val.value.arrayValue)[ i ], alreadySerialized );
		}
		// return it
		return val;
		
	// otherwise, if value is object
	} else if ( val.type == TypeObject && val.value.objectValue != NULL ) {
		
		// check .serialized property, skip if === false
		ArgValue serialized = GetProperty( "serialized", val.value.objectValue );
		if ( serialized.type == TypeBool && serialized.value.boolValue == false ) return ArgValue();
		
		// object
		JSObject* obj = (JSObject*) val.value.objectValue;
		
		// if object is already serialized, return an object stub
		unordered_map<unsigned long,JSObject*>::iterator it = alreadySerialized.find( (unsigned long) obj );
		if ( it != alreadySerialized.end() ) {
			// create stub object
			JSObject* stub = JS_NewObject( this->js, NULL, NULL, NULL );
			static char buf[16];
			sprintf( buf, "%p", obj );
			ArgValue objId( buf );
			SetProperty( "__stub__", &objId, stub );
			// make sure original object has id property
			SetProperty( "__id__", &objId, it->second );
			return stub;
		}
		
		// get property names
		unordered_set<string> properties;
		ClassDef* cdef = this->_GetPropertyNames( obj, properties );
		
		// if ScriptableObject
		if ( cdef ) {
			// bail if reference to singleton class
			if ( cdef->singleton ) return ArgValue( (void*) NULL );
		}
		
		// create blank object
		JSObject* init = JS_NewObject( this->js, NULL, NULL, NULL );
		
		// add it to alreadySerialized
		alreadySerialized[ (unsigned long) obj ] = init;
		
		// get classname
		const char* className = NULL;
		JSClass* clp = NULL;
		
		// if it's ScriptableObject
		if ( cdef ) {
			className = cdef->className.c_str();
		// built-in object
		} else if ( ( clp = JS_GetClass( obj ) ) && strcmp( clp->name, "Object" ) != 0 ) {
			className = clp->name;
			
			// TODO - if need to save built in objects like "Date", or "RegExp", can add props here to re-initialize them upon load
		
		}
		
		// add "__class__" property
		if ( className ) {
			SetProperty( "__class__", ArgValue( className ), init );
		}
		
		// add all values
		unordered_set<string>::iterator p = properties.begin(), end = properties.end();
		ArgValue prop;
		while ( p != end ) {
			prop = this->GetProperty( p->c_str(), obj );
			SetProperty( p->c_str(), _MakeInitObject( prop, alreadySerialized ), init );
			p++;
		}
		
		// return this new object
		return ArgValue( (void*) init );
		
	}
	
	// otherwise return original value
	return val;
	
};

// places enumerable properties of given object, including all non-readonly props defined for scriptable class. Returns ClassDef, if found.
ScriptHost::ClassDef* ScriptHost::_GetPropertyNames( void* obj, unordered_set<string>& ret, ClassDef* cdef ) {
	// add object's own properties first
	JSObject* iterator = JS_NewPropertyIterator( this->js, (JSObject*) obj );
	jsval propVal;
	jsid propId;
	while( JS_NextProperty( this->js, iterator, &propId ) && propId != JSID_VOID ) {
		// convert each property to string
		if ( JS_IdToValue( this->js, propId, &propVal ) ) {
			JSString* str = JS_ValueToString( this->js, propVal );
			char *buf = JS_EncodeString( this->js, str );
			unsigned attrs = 0;
			JSBool found = false;
			// if property isn't read-only
			if ( JS_GetPropertyAttributes( this->js, (JSObject*) obj, buf, &attrs, &found) && found && !( attrs & JSPROP_READONLY ) ) {
				// add it to end of list
				ret.emplace( buf );
			}
			JS_free( this->js, buf );
		}
	}
	
	// if class def wasn't passed in
	if ( !cdef ) {
		// grab object's own class def
		JSClass* clp = JS_GetClass( (JSObject*)obj );
		cdef = CDEF( string(clp->name) );
	}
	
	// if class def is found
	if ( cdef ) {
		// add properties listed in class def
		GetterSetterMapIterator iter = cdef->getterSetter.begin(), iterEnd = cdef->getterSetter.end();
		while( iter != iterEnd ) {
			GetterSetter& gs = iter->second;
			// if property is serialized
			if ( gs.flags & PROP_SERIALIZED & ~PROP_READONLY) {
				ret.emplace( iter->first.c_str() );
			}
			iter++;
		}
		
		// if there's parent class, add that too and return result
		if ( cdef->parent ) {
			this->_GetPropertyNames( obj, ret, cdef->parent );
			return cdef;
		}
		
	}
	
	return cdef;
}

/// unserialize obj using initObject
void* ScriptHost::InitObject( void* initObj ) {
	JSAutoRequest r( this->js );
	
	// populate this object
	unordered_map<string, void*> map;
	unordered_map<string, vector<_StubRef>> stubs;
	// this is a list of all objects who will receive 'awake' event after init is complete
	vector<ScriptableClass*> awakeList;
	void* obj = _InitObject( NULL, initObj, &map, &stubs, &awakeList );
	
	// process stubs
	unordered_map<string, vector<_StubRef>>::iterator it = stubs.begin();
	while( it != stubs.end() ) {
		// see if value is now available
		unordered_map<string, void*>::iterator vit = map.find( it->first );
		
		// found
		if ( vit != map.end() ) {
			jsval objVal;
			objVal.setObjectOrNull( (JSObject*) vit->second );
			vector<_StubRef> &stubList = it->second;
			for ( size_t i = 0, ns = stubList.size(); i < ns; i++ ) {
				_StubRef &stub = stubList[ i ];
				if ( stub.index == -1 ) {
					JS_SetProperty( this->js, stub.object, stub.propName.c_str(), &objVal );
				} else {
					JS_SetElement( this->js, stub.object, stub.index, &objVal );
				}
			}
		} else {
			printf("%s not found during deserialization\n", it->first.c_str() );
		}
		it++;
	}
	
	// call awake in reverse order
	Event event( EVENT_AWAKE );
	for ( size_t i = awakeList.size(); i > 0; i-- ){
		ScriptableClass* instance = awakeList[ i - 1 ];
		event.scriptParams.ResizeArguments( 0 );
		event.scriptParams.AddObjectArgument( instance->scriptObject );
		instance->CallEvent( event );
		if ( event.stopped ) break;
	}
	
	return obj;
	
}

void* ScriptHost::_InitObject( void* obj, void* initObj,
				  unordered_map<string, void*> *alreadyInitialized,
				  unordered_map<string, vector<_StubRef>> *stubs,
				  vector<ScriptableClass*> *awakeList ){
	
	string stubName, className;
	
	// if obj is null, this function was called from init(), and obj needs to be created first as initObj[__class__]
	if ( !obj ) {
		if ( _IsInitObject( initObj, className ) ) {
			obj = NewObject( className.c_str() );
		} else {
			obj = NewObject();
		}
	}

	// get ScriptableClass definition
	JSClass* clp = JS_GetClass( (JSObject*) obj );
	ClassDef* cdef = CDEF( string( clp->name ) );
	
	// add this object to alreadyInitialized ( if it has __id__ property )
	_AddToAlreadyInitialized( obj, initObj, alreadyInitialized );
	
	// check if object is array or regular object
	bool isArray = JS_IsArrayObject( this->js, (JSObject*) obj );
	uint32_t length = 0;
	JSObject* iterator = NULL;
	
	// if object is array
	if ( isArray ) {
		// get array length
		JS_GetArrayLength( this->js, (JSObject*) initObj, &length );
	// object
	} else {
		// start iterating over its properties
		iterator = JS_NewPropertyIterator( this->js, (JSObject*) initObj );
		// if object is ScriptableClass
		if ( cdef ) {
			// add to awake list
			ScriptableClass* instance = GetInstance<ScriptableClass>( obj );
			if ( instance ) awakeList->push_back( instance );
		}
	}
	
	// set up
	jsid propId;
	jsval propVal;
	uint32_t propIndex = 0;
	char *propName = NULL;
	struct _SetVal {
		string propName;
		int index = -1;
		jsval value;
		_SetVal( const char *prop, int i, jsval& val ) : propName(prop), index(i), value(val) {};
	};
	list<_SetVal> setValues;
	list<_SetVal> setLateValues;
	
	// read properties
	while( true ) {
		
		// array
		if ( isArray ) {
			
			// check end condition
			if ( propIndex + 1 > length ) break;
			
		// object
		} else {
			
			// check end condition
			if ( !JS_NextProperty( this->js, iterator, &propId ) || propId == JSID_VOID ) break;
			
			// convert property to string
			if ( !JS_IdToValue( this->js, propId, &propVal ) ) continue;
			
			// convert property name to string
			JSString* str = JS_ValueToString( this->js, propVal );
			propName = JS_EncodeString( this->js, str );
			size_t propLen = strlen( propName );
			propIndex = 0;
			// skip __special_properties__
			if ( propLen >= 4 && propName[ 0 ] == '_' && propName[ 1 ] == '_' && propName[ propLen - 1 ] == '_' && propName[ propLen - 2 ] == '_' ) continue;
		}
		
		// get value
		ArgValue val = isArray ? GetElement( propIndex, initObj ) : GetProperty( propName, initObj );
		
		// check for early property
		bool isEarlyProp = false;
		bool isLateProp = false;
		if ( !isArray && cdef ) {
			GetterSetterMapIterator it = cdef->getterSetter.find( propName );
			if ( it != cdef->getterSetter.end() ) {
				isEarlyProp = (it->second.flags & PROP_EARLY);
				isLateProp = (it->second.flags & PROP_LATE);
			}
		}
		
		// value is an object
		if ( val.type == TypeObject ) {
			
			// null value
			if ( !val.value.objectValue ) {
				
				propVal.setNull();
				
			// check if it's a stub first
			} else if ( _IsStub( val.value.objectValue, stubName ) ){
				
				// add to stubs list
				vector<_StubRef> &stubList = (*stubs)[ stubName ];
				if ( propName ) {
					stubList.emplace_back( (JSObject*) obj, string( propName ), -1 );
				} else {
					stubList.emplace_back( (JSObject*) obj, string(""), propIndex );
				}
				// set to null
				propVal.setNull();
				
			// otherwise, if it's an init object
			} else if ( _IsInitObject( val.value.objectValue, className ) ) {
				
				JSObject* newObject = (JSObject*) NewObject( className.c_str() );
				if ( newObject ) _InitObject( newObject, val.value.objectValue, alreadyInitialized, stubs, awakeList );
				propVal.setObjectOrNull( newObject );
				
			// regular object
			} else {
				
				// construct a blank object
				JSObject* newObject = JS_NewObject( this->js, NULL, NULL, NULL);
				// populate it, recursively
				_InitObject( newObject, val.value.objectValue, alreadyInitialized, stubs, awakeList );
				// will set this new object
				propVal.setObjectOrNull( newObject );
				
			}
			
			// value is an array
		} else if ( val.type == TypeArray ) {
			
			// create new array object
			JSObject* newObject = JS_NewArrayObject( this->js, 0, &propVal );
			// populate it, recursively
			_InitObject( newObject, val.arrayObject, alreadyInitialized, stubs, awakeList );
			uint32_t len = 0;
			JS_GetArrayLength( this->js, newObject, &len );
			// will set this new object
			propVal.setObjectOrNull( newObject );
			
		// simple, just copy prop
		} else {
			propVal = val.toValue();
		}
		
		// propVal contains our value
		
		// array
		if ( isArray ) {
			if ( isEarlyProp ) {
				setValues.emplace_front( "", propIndex, propVal );
			} else if ( isLateProp ) {
				setLateValues.emplace_back( "", propIndex, propVal );
			} else {
				setValues.emplace_back( "", propIndex, propVal );
			}
			propIndex++;
		// early property?
		} else if ( isEarlyProp ){
			setValues.emplace_front( propName, -1, propVal );
		// late property?
		} else if ( isLateProp ){
			setLateValues.emplace_front( propName, -1, propVal );
		// normal
		} else if( propName ){
			setValues.emplace_back( propName, -1, propVal );
		}
		
		// release propName
		if ( !isArray && propName ) JS_free( this->js, propName );
		
	}
	
	// ready to set properties
	list<_SetVal>::iterator it = setValues.begin(), end = setValues.end();
	while( it != end ) {
		// array index
		if ( it->index >= 0 ) {
			JS_SetElement( this->js, (JSObject*) obj, it->index, &it->value );
		// normal prop
		} else {
			JS_SetProperty( this->js, (JSObject*)obj, it->propName.c_str(), &it->value );
		}
		it++;
		// end of setValues? go onto late values
		if ( it == setValues.end() ) {
			it = setLateValues.begin();
			end = setLateValues.end();
		}
	}
	
	return obj;
}

bool ScriptHost::_IsInitObject( void *obj, string& className ) {
	jsval hasClass;
	JS_GetProperty( this->js, (JSObject*) obj, "__class__", &hasClass );
	if ( JSVAL_IS_STRING( hasClass ) ) {
		char* buf = JS_EncodeString( this->js, JSVAL_TO_STRING( hasClass ) );
		className = buf;
		JS_free( this->js, (void*) buf );
		return true;
	}
	return false;
}

bool ScriptHost::_IsStub( void *obj, string& stubName ) {
	jsval stubVal;
	JS_GetProperty( this->js, (JSObject*) obj, "__stub__", &stubVal );
	if ( JSVAL_IS_STRING( stubVal ) ) {
		char* buf = JS_EncodeString( this->js, JSVAL_TO_STRING( stubVal ) );
		stubName = buf;
		JS_free( this->js, (void*) buf );
		return true;
	}
	return false;
}

void ScriptHost::_AddToAlreadyInitialized( void *obj, void* initObj, unordered_map<string, void*> *alreadyInitialized ) {
	jsval idVal;
	if ( JS_IsArrayObject( this->js, (JSObject*) obj ) ) return;
	JS_GetProperty( this->js, (JSObject*) initObj, "__id__", &idVal );
	if ( JSVAL_IS_STRING( idVal ) ) {
		char* buf = JS_EncodeString( this->js, JSVAL_TO_STRING( idVal ) );
		string idName = buf;
		JS_free( this->js, (void*) buf );
		(*alreadyInitialized)[ idName ] = obj;
		// printf( "%s added to map.\n", idName.c_str() );
	}
}
