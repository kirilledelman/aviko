#include "ScriptHost.hpp"
#include "ScriptableClass.hpp"
#include "Application.hpp"


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
		
			// regular expression
			if ( strcmp( className, "RegExp" ) == 0 ) {
				properties.emplace( "source" );
				properties.emplace( "sticky" );
				properties.emplace( "multiline" );
				properties.emplace( "global" );
				properties.emplace( "ignoreCase" );
			}
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
			// skip functions
			if ( prop.type != TypeFunction ) {
				SetProperty( p->c_str(), _MakeInitObject( prop, alreadySerialized ), init );
			}
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
	ArgValue serializeMaskVal;
	// first, see if there's serializeMask prop
	if ( JS_GetProperty( this->js, (JSObject*) obj, "serializeMask", &propVal ) ) {
		serializeMaskVal = propVal;
	}
	while( JS_NextProperty( this->js, iterator, &propId ) && propId != JSID_VOID ) {
		// convert each property to string
		if ( JS_IdToValue( this->js, propId, &propVal ) ) {
			JSString* str = JS_ValueToString( this->js, propVal );
			char *buf = JS_EncodeString( this->js, str );
			unsigned attrs = 0;
			JSBool found = false;
			// check if it's in serializeMask
			if ( serializeMaskVal.type != TypeUndefined ) {
				bool masked = false;
				if ( serializeMaskVal.type == TypeArray ) {
					for ( size_t i = 0, n = serializeMaskVal.value.arrayValue->size(); i < n; i++ ) {
						ArgValue& check = (*serializeMaskVal.value.arrayValue)[ i ];
						if ( check.type == TypeString && check.value.stringValue->compare( buf ) == 0 ) {
							masked = true;
							break;
						}
					}
				} else if ( serializeMaskVal.type == TypeObject ) {
					ArgValue check = this->GetProperty( buf, serializeMaskVal.value.objectValue );
					masked = check.toBool();
				}
				// found prop in serializeMask
				if ( masked ) {
					// skip
					JS_free( this->js, buf );
					continue;
				}
			}
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
				// check if it's in serializeMask
				if ( serializeMaskVal.type != TypeUndefined ) {
					bool masked = false;
					if ( serializeMaskVal.type == TypeArray ) {
						for ( size_t i = 0, n = serializeMaskVal.value.arrayValue->size(); i < n; i++ ) {
							ArgValue& check = (*serializeMaskVal.value.arrayValue)[ i ];
							if ( check.type == TypeString && check.value.stringValue->compare( iter->first ) == 0 ) {
								masked = true;
								break;
							}
						}
					} else if ( serializeMaskVal.type == TypeObject ) {
						ArgValue check = this->GetProperty( iter->first.c_str(), serializeMaskVal.value.objectValue );
						masked = check.toBool();
					}
					// found prop in serializeMask
					if ( masked ) {
						// skip
						iter++;
						continue;
					}
				}
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
	app.isUnserializing = true;
	
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
	}
	app.isUnserializing = false;
	
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
	className = clp->name;
	
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
		// special cases for built in objects
		} else if ( className.compare( "RegExp" ) == 0 ) {
			ScriptArguments args;
			ArgValue v = script.GetProperty( "source", initObj );
			if ( v.type == TypeString ) args.AddStringArgument( v.value.stringValue->c_str() );
			string flags;
			v = script.GetProperty( "global", initObj );
			if ( v.toBool() ) flags.append( "g" );
			v = script.GetProperty( "ignoreCase", initObj );
			if ( v.toBool() ) flags.append( "i" );
			v = script.GetProperty( "sticky", initObj );
			if ( v.toBool() ) flags.append( "y" );
			v = script.GetProperty( "multiline", initObj );
			if ( v.toBool() ) flags.append( "m" );
			args.AddStringArgument( flags.c_str() );
			script.CallFunction( obj, "compile", args );
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
			if ( !iterator || !JS_NextProperty( this->js, iterator, &propId ) || propId == JSID_VOID ) break;
			
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
		
		// skip functions
		if ( val.type == TypeFunction ) continue;
		
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
		} else if ( it->propName.length() ){
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

//
void ScriptHost::CopyProperties( void* src, void* dest ) {
	
	string stubName, className;
	
	// get ScriptableClass definition
	JSClass* clp = JS_GetClass( (JSObject*) dest );
	ClassDef* cdef = CDEF( string( clp->name ) );
	
	// start iterating over its properties
	JSObject* iterator = JS_NewPropertyIterator( this->js, (JSObject*) src );
	
	// set up
	jsid propId;
	jsval propVal;
	uint32_t propIndex = 0;
	char *propName = NULL;
	struct _SetVal {
		string propName;
		jsval value;
		_SetVal( const char *prop, jsval& val ) : propName(prop), value(val) {};
	};
	list<_SetVal> setValues;
	list<_SetVal> setLateValues;
	
	// read properties
	while( true ) {

			
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
		
		// get value
		JS_GetProperty( this->js, (JSObject*) src, propName, &propVal );
		
		// check for early property
		bool isEarlyProp = false;
		bool isLateProp = false;
		if ( cdef ) {
			GetterSetterMapIterator it = cdef->getterSetter.find( propName );
			if ( it != cdef->getterSetter.end() ) {
				isEarlyProp = (it->second.flags & PROP_EARLY);
				isLateProp = (it->second.flags & PROP_LATE);
			}
		}
		
		// propVal contains our value
		
		// array
		if ( isEarlyProp ){
			setValues.emplace_front( propName, propVal );
			// late property?
		} else if ( isLateProp ){
			setLateValues.emplace_front( propName, propVal );
			// normal
		} else if( propName ){
			setValues.emplace_back( propName, propVal );
		}
		
		// release propName
		if ( propName ) JS_free( this->js, propName );
		
	}
	
	// ready to set properties
	list<_SetVal>::iterator it = setValues.begin(), end = setValues.end();
	while( it != end ) {
		JS_SetProperty( this->js, (JSObject*) dest, it->propName.c_str(), &it->value );
		it++;
		// end of setValues? go onto late values
		if ( it == setValues.end() ) {
			it = setLateValues.begin();
			end = setLateValues.end();
		}
	}
	
}

/* MARK:	-				Logging
 -------------------------------------------------------------------- */

/// tracing function
bool ScriptHost::Log( JSContext *cx, unsigned argc, Value *vp ) {
	// scope
	JSAutoRequest req( cx );
	
	// determine if there's log handler registered
	ScriptableClass::EventListenersMap::iterator hit = app.eventListeners.find( string( EVENT_LOG ) );
	bool hasHandler = ( hit != app.eventListeners.end() && hit->second.size() > 0 );
	if ( !hasHandler ) {
		ArgValue hval = script.GetProperty( EVENT_LOG, app.scriptObject );
		hasHandler = ( hval.type == TypeFunction );
	}
	
	// print each argument as string, followed by newline
	CallArgs args = CallArgsFromVp( argc, vp );
	string combinedString;
	for ( int i = 0; i < argc; i++ ) {
		jsval val = args.get( i );
		RootedString str( cx, JS_ValueToString( cx, val ) );
		if ( val.isObject() && JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT( val ) ) ) {
			static char cbuf[ 256 ];
			sprintf( cbuf, "[Function %p]%s", JSVAL_TO_OBJECT( val ), (i == argc - 1 ? "" : " ") );
			combinedString.append( cbuf );
		} else {
			char* buf = JS_EncodeString( cx, str );
			bool isArray = val.isObject() ? JS_IsArrayObject( script.js, val.toObjectOrNull() ) : false;
			if ( isArray ) {
				combinedString.append( "[" );
				combinedString.append( buf );
				combinedString.append( "]" );
			} else {
				combinedString.append( buf );
			}
			JS_free( cx, buf );
		}
		if (i < argc) combinedString.append( " " );
	}
	
	// if there's a handler, use event
	if ( hasHandler ) {
		Event e ( EVENT_LOG );
		e.scriptParams.AddStringArgument( combinedString.c_str() );
		app.CallEvent( e );
		if ( !e.stopped ) printf( "%s\n", combinedString.c_str() );
	} else {
		// otherwise print to console
		printf( "%s\n", combinedString.c_str() );
	}
	return true;
}


/* MARK:	-				Error reporting
 -------------------------------------------------------------------- */


/// callback for Javascript errors
void ScriptHost::ErrorReport ( JSContext *cx, const char *message, JSErrorReport *report ){
	// scope
	JSAutoRequest req( cx );
		
	// this error is handled
	JS_ClearPendingException( cx );
	
	// if already quitting, ignore
	if ( !app.run ) return;
	
	// error preview
	string preview;
	ScriptResource* sr = NULL;
	if ( report->linebuf ) {
		preview = report->linebuf;
	} else if ( (sr = app.scriptManager.FindByPath( report->filename ) ) != NULL ){
		istringstream iss( sr->source );
		int i = 0;
		while ( i++ <= report->lineno ) getline( iss, preview );
	}
	
	// determine if there's error handler registered
	ScriptableClass::EventListenersMap::iterator hit = app.eventListeners.find( string( EVENT_ERROR ) );
	bool hasHandler = ( hit != app.eventListeners.end() && hit->second.size() > 0 );
	if ( !hasHandler ) {
		ArgValue hval = script.GetProperty( EVENT_ERROR, app.scriptObject );
		if ( hval.type == TypeUndefined ) {
			preview.append( "\n" );
			if ( preview.length() ) {
				// add ~~~~^ for error column
				for ( size_t i = 0; i < report->column; i++ ){
					preview.append( preview[ i ] == '\t' ? "\t" : "~" );
				}
				preview.append( "^\n" );
			}
			
			// no handler - dump error and exit
			printf( "%s:%u\n%s%s\n",
				   report->filename ? report->filename : "[top]",
				   (unsigned int) report->lineno,
				   preview.c_str(),
				   message);
			app.run = false;
			return;
		}
	}
	
	// there's a handler let it deal with error
	Event e ( EVENT_ERROR );
	e.scriptParams.AddStringArgument( message );
	e.scriptParams.AddStringArgument( report->filename ? report->filename : "" );
	e.scriptParams.AddIntArgument( report->lineno );
	e.scriptParams.AddIntArgument( report->column );
	e.scriptParams.AddStringArgument( preview.c_str() );
	e.scriptParams.AddIntArgument( report->flags );
	app.CallEvent( e );
	
}

