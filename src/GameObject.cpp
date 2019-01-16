#include "GameObject.hpp"
#include "Scene.hpp"
#include "Application.hpp"
#include "Tween.hpp"


/* MARK:	-				Init / destroy
 -------------------------------------------------------------------- */


/// constructor for when a GameObject is created from script
GameObject::GameObject( ScriptArguments* args ) : GameObject() {
	
	// add scriptObject
	script.NewScriptObject<GameObject>( this );
    RootedObject robj( script.js, (JSObject*) this->scriptObject );

	// event mask
	this->eventMask = new TypedVector( NULL );
	ArgValue dv( "String" );
	this->eventMask->InitWithType( dv );
	
	// string argument - script name, obj argument - init object
	if ( args && args->args.size() >= 1) {
		// string - script name
		string scriptName;
		void *obj = NULL;
		if ( args->ReadArguments( 1, TypeString, &scriptName, TypeObject, &obj ) ) {
			script.SetProperty( "script", ArgValue( scriptName.c_str() ), this->scriptObject );
		} else {
			args->ReadArguments( 1, TypeObject, &obj );
		}
		// if have object param, copy properties from it
		if ( obj ) script.CopyProperties( obj, this->scriptObject );
	}
    
}

/// base constructor
GameObject::GameObject() {
}

/// destructor
GameObject::~GameObject() {
	
	// release resource
	if ( this->scriptResource ) this->scriptResource->AdjustUseCount( -1 );
    
}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void GameObject::InitClass() {
	
	// register GameObject script class
	script.RegisterClass<GameObject>( NULL );
	
	// properties
	
	script.AddProperty<GameObject>
	( "name",
	 static_cast<ScriptStringCallback>([](void* go, string ) { return ((GameObject*) go)->name; }),
	 static_cast<ScriptStringCallback>([](void* go, string val ) {
		GameObject* self = (GameObject*) go;
		Event e( EVENT_NAMECHANGED );
		e.scriptParams.AddStringArgument( val.c_str() );
		e.scriptParams.AddStringArgument( self->name.c_str() );
		self->name = val;
		self->CallEvent( e );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "active",
	 static_cast<ScriptBoolCallback>([](void* go, bool a ) { return ((GameObject*) go)->active(); }),
	 static_cast<ScriptBoolCallback>([](void* go, bool a ) {
		GameObject* self = (GameObject*) go;
		if ( self->_active != a ) {
			a = self->active( a );
			if ( self->ui != NULL ) self->ui->RequestLayout( ArgValue( "active" ) );
		}
		return a;
	}));
	
	script.AddProperty<GameObject>
	( "ignoreCamera",
	 static_cast<ScriptBoolCallback>([](void* go, bool a ) { return ((GameObject*) go)->ignoreCamera; }),
	 static_cast<ScriptBoolCallback>([](void* go, bool a ) {
		return (((GameObject*) go)->ignoreCamera = a );
	}));
	
	script.AddProperty<GameObject>
	( "numChildren",
	 static_cast<ScriptIntCallback>([](void* go, int) { return ((GameObject*) go)->children.size(); }),
	 static_cast<ScriptIntCallback>([](void* go, int n ) {
		GameObject *g = (GameObject*) go;
		int curSize = (int) g->children.size();
		n = max( 0, n );
		// removing children
		if ( curSize > n ) {
			while( curSize > 0 && curSize > n ) {
				curSize--;
				GameObject* c = g->children[ curSize ];
				c->SetParent( NULL );
			}
		// adding children
		} else if ( curSize < n ) {
			while( curSize < n ) {
				GameObject* c = new GameObject( NULL );
				c->SetParent( g );
				curSize++;
			}
		}
		return curSize;
		
	 }),
	 PROP_ENUMERABLE);
	
	script.AddProperty<GameObject>
	( "parent",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		return self->parent ? self->parent->scriptObject : NULL;
	}),
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		GameObject* other = script.GetInstance<GameObject>( p );
		self->SetParent( other );
		return self->parent ? self->parent->scriptObject : NULL;
	}), PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<GameObject>
	( "scene",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		Scene* scene = self->GetScene();
		return scene ? scene->scriptObject : NULL;
	}), PROP_ENUMERABLE | PROP_NOSTORE );
	
	script.AddProperty<GameObject>
	( "body",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) { GameObject* self = (GameObject*) go; return self->body ? self->body->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		Behavior* beh = script.GetInstance<Behavior>( p );
		// check if incorrect type
		if ( beh && !beh->isBodyBehavior ) {
			printf( ".body can only be assigned a physics behavior, e.g. RigidBody\n" );
		} else {
			// clear old one
			if ( self->body && self->body->scriptObject != p ) {
				self->body->SetGameObject( NULL );
			}
			// set new one
			if ( beh ) beh->SetGameObject( self );
		}
		return self->body ? self->body->scriptObject : NULL;
	}));
	
	script.AddProperty<GameObject>
	( "render",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) { GameObject* self = (GameObject*) go; return self->render ? self->render->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		Behavior* beh = script.GetInstance<Behavior>( p );
		// check if incorrect type
		if ( beh && !beh->isRenderBehavior ) {
			printf( ".render can only be assigned a rendering behavior, e.g. RenderShape\n" );
		} else {
			// clear old one
			if ( self->render && self->render->scriptObject != p ) {
				self->render->SetGameObject( NULL );
			}
			// set new one
			if ( beh ) beh->SetGameObject( self );
		}
		return self->render ? self->render->scriptObject : NULL;
	 }));
	
	script.AddProperty<GameObject>
	( "ui",
	 static_cast<ScriptObjectCallback>([](void* go, void* p) { GameObject* self = (GameObject*) go; return self->ui ? self->ui->scriptObject : NULL; }),
	 static_cast<ScriptObjectCallback>([](void* go, void* p) {
		GameObject* self = (GameObject*) go;
		Behavior* beh = script.GetInstance<Behavior>( p );
		// check if incorrect type
		if ( beh && !beh->isUIBehavior ) {
			printf( ".ui can only be assigned an instance of UIBehavior.\n" );
		} else {
			// clear old one
			if ( self->ui && self->ui->scriptObject != p ) {
				self->ui->SetGameObject( NULL );
			}
			// set new one
			if ( beh ) beh->SetGameObject( self );
		}
		return self->ui ? self->ui->scriptObject : NULL;
	}));
	
	script.AddProperty<GameObject>
	( "opacity",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->opacity; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->opacity = max( 0.0f, min( 1.0f, val ) );
		return go->opacity;
	}));

	script.AddProperty<GameObject>
	( "renderAfterChildren",
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ((GameObject*) b)->renderAfterChildren; }),
	 static_cast<ScriptBoolCallback>([](void *b, bool val ){ return ( ((GameObject*) b)->renderAfterChildren = val ); }) );
	
	script.AddProperty<GameObject>
	( "eventMask",
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){ return ArgValue(((GameObject*) b)->eventMask->scriptObject); }),
	 static_cast<ScriptValueCallback>([](void *b, ArgValue val ){
		GameObject* rb = (GameObject*) b;
		rb->eventMask->Set( val );
		return ArgValue( rb->eventMask->scriptObject );
	}),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE );
	
	script.AddProperty<GameObject>
	( "x",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetX( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "y",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetY(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetY( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "z",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->_z; }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetZ( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "scale",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetScaleX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetScale( val, val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "scaleX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetScaleX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetScaleX( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "scaleY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetScaleY(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetScaleY( val );
		return val;
	}));
		
	script.AddProperty<GameObject>
	( "skewX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetSkewX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetSkewX( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "skewY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetSkewY(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetSkewY( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "angle",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetAngle(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetAngle( val );
		return val;
	}));
	
	script.AddProperty<GameObject>
	( "worldX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldX( val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "worldY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldY(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldY( val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "worldScale",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldScaleX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldScale( val, val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "worldScaleX",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldScaleX(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldScaleX( val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "worldScaleY",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldScaleY(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldScaleY( val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "worldAngle",
	 static_cast<ScriptFloatCallback>([](void* o, float) { return ((GameObject*) o)->GetWorldAngle(); }),
	 static_cast<ScriptFloatCallback>([](void* o, float val ) {
		GameObject* go = ((GameObject*) o);
		go->SetWorldAngle( val );
		return val;
	}), PROP_ENUMERABLE );
	
	script.AddProperty<GameObject>
	( "children",
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ) { return ((GameObject*) go)->GetChildrenVector(); }),
	 static_cast<ScriptArrayCallback>([](void *go, ArgValueVector* in ){ return ((GameObject*) go)->SetChildrenVector( in ); }),
	 PROP_ENUMERABLE | PROP_SERIALIZED | PROP_NOSTORE
	 );
	
	script.AddProperty<GameObject>
	( "script",
	 static_cast<ScriptValueCallback>([](void *go, ArgValue ) {
		GameObject* self = ((GameObject*) go);
		if ( self->scriptResource ) return ArgValue( self->scriptResource->key.c_str() );
		return ArgValue( "" );
	 }),
	 static_cast<ScriptValueCallback>([](void *go, ArgValue in ){
		GameObject* self = ((GameObject*) go);
		// setting to null or empty
		if ( in.type != TypeString || !in.value.stringValue->size() ) {
			if ( self->scriptResource ) self->scriptResource->AdjustUseCount( -1 );
			self->scriptResource = NULL;
			return in;
		}
		// find
		const char *key = in.value.stringValue->c_str();
		ScriptResource* s = app.scriptManager.Get( key );
		// script found
		if ( s->error == ERROR_NONE ) {
			// is different from previous
			if ( self->scriptResource != s ) {
				// adjust use counts
				if ( self->scriptResource ) self->scriptResource->AdjustUseCount( -1 );
				s->AdjustUseCount( 1 );
				// set new
				self->scriptResource = s;
				// execute it, if error
				if ( !script.Execute( s, self->scriptObject ) ) {
					// print error, and fail
					if ( JS_IsExceptionPending( script.js ) )
						JS_ReportPendingException( script.js );
					return ArgValue();
				} else {
					// if not unserializing, call awake
					if ( !app.isUnserializing ) {
						Event event( EVENT_AWAKE );
						self->CallEvent( event );
					}
					return in;
				}
			}
		} else if ( s->error == ERROR_NOT_FOUND ) {
			JSScript* scr = NULL;
			unsigned int line;
			JS_DescribeScriptedCaller( script.js, &scr, &line );
			printf( ".script path \"%s\" for [GameObject (%s)%p] was not found at %s:%d.\nPath resolved to %s\n", key, self->name.c_str(), self, JS_GetScriptFilename( script.js, scr ), line, s->path.c_str() );
			//script.ReportError( ".script path \"%s\" was not found.\n", key );
		}
		return ArgValue();
		
	}), PROP_ENUMERABLE | PROP_SERIALIZED | PROP_EARLY );
	
	// functions
	
	script.DefineFunction<GameObject>
	( "setTransform", // ( x, y, angle, sx, sy )
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: setTransform( float x, float y[, float angleInDegrees[, float sx[, float sy ] ] ] )";
		GameObject* self = (GameObject*) go;
		float x = 0, y = 0,
			sx = self->_scale.x, sy = self->_scale.y, angle = self->_angle;

		// if not a valid call report error
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeFloat, &angle, TypeFloat, &sx, TypeFloat, &sy ) ) {
			script.ReportError( error );
			return false;
		}
		
		// if only sx is specified, make sy=sx
		if ( sa.args.size() == 4 ) sy = sx;
		
		// use full
		self->SetTransform(x, y, angle, sx, sy);
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "setWorldTransform", // ( x, y, angle, sx, sy )
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: setWorldTransform( float x, float y[, float angleInDegrees[, float sx[, float sy ] ] ] )";
		GameObject* self = (GameObject*) go;
		b2Vec2 scale = self->GetWorldScale();
		float x = 0, y = 0, angle = self->GetWorldAngle();
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeFloat, &angle, TypeFloat, &scale.x, TypeFloat, &scale.y ) ) {
			script.ReportError( error );
			return false;
		}
		
		// if only sx is specified, make sy=sx
		if ( sa.args.size() == 4 ) scale.y = scale.x;
		
		// use full
		self->SetWorldTransform(x, y, angle, scale.x, scale.y);
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "localToGlobal", //
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: localToGlobal( Float x, Float y, [ Boolean screenSpace ] )";
		float x, y;
		float xx, yy;
		bool screenSpace = false;
		GameObject* self = (GameObject*) go;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeBool, &screenSpace ) ) {
			script.ReportError( error );
			return false;
		}
		// convert
		self->ConvertPoint( x, y, xx, yy, true, screenSpace );
		
		// return object
		void *robj = script.NewObject();
		script.SetProperty( "x", ArgValue( xx ), robj );
		script.SetProperty( "y", ArgValue( yy ), robj );
		sa.ReturnObject( robj );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "globalToLocal", //
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: globalToLocal( Float x, Float y, [ Boolean screenSpace ] )";
		float x, y;
		float xx, yy;
		bool screenSpace = false;
		GameObject* self = (GameObject*) go;
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeBool, &screenSpace ) ) {
			script.ReportError( error );
			return false;
		}
		// convert
		self->ConvertPoint( x, y, xx, yy, false, screenSpace );
		
		// return object
		void *robj = script.NewObject();
		script.SetProperty( "x", ArgValue( xx ), robj );
		script.SetProperty( "y", ArgValue( yy ), robj );
		sa.ReturnObject( robj );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "isDescendantOf", //
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: isDescendantOf( GameObject grandDaddy )";
		GameObject* self = (GameObject*) go;
		GameObject* other = NULL;
		void* otherObj;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeObject, &otherObj ) || ( other = script.GetInstance<GameObject>( otherObj ) ) == NULL ) {
			script.ReportError( error );
			return false;
		}
		// return result
		sa.ReturnBool( self->IsDescendantOf( other ) );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "getChild", // ( int position ) -> GameObject child | NULL if out of range
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: getChild( Int position )";
		int pos = -1;
		GameObject* self = (GameObject*) go;
		GameObject* other = NULL;

		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeInt, &pos ) ) {
			script.ReportError( error );
			return false;
		}
		
		// evaluate
		int numChildren = (int) self->children.size();
		if ( pos < 0 ) pos = numChildren + pos;
		if ( pos >= 0 && pos < numChildren ) other = self->children[ pos ];
		
		// return object
		sa.ReturnObject( other ? other->scriptObject : NULL );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	("addChild", // ( GameObject child [, int position ] ) -> GameObject child
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		// validate params
		const char* error = "usage: addChild( [ GameObject obj | String scriptPath [,Integer desiredPosition | (Object initProperties, Integer desiredPosition) ] ] )";
		void* obj = NULL;
		void *initObj = NULL;
		string scriptName;
		GameObject* other = NULL;
		
		// read args
		if ( sa.ReadArguments( 0, TypeObject, &obj ) ) {
			// no object?
			if ( !obj ) {
				// make new game object
				other = new GameObject( NULL );
			} else {
				other = script.GetInstance<GameObject>( obj );
				if ( !other && sa.args.size() == 1 ) {
					initObj = obj;
					other = new GameObject( NULL );
				}
			}
		} else if ( sa.ReadArguments( 1, TypeString, &scriptName ) ){
			// make new game object
			other = new GameObject( NULL );
			script.SetProperty( "script", ArgValue( scriptName.c_str() ), other->scriptObject );
		}
		
		// validate
		if ( !other ){
			script.ReportError( error );
			return false;
		}
		
		// either position or initObj
		int pos = -1;
		if ( !sa.ReadArgumentsFrom( (initObj ? 0 : 1), 1, TypeObject, &initObj, TypeInt, &pos ) ){
			sa.ReadArgumentsFrom( (initObj ? 0 : 1), 1, TypeInt, &pos );
		}
		
		// all good
		GameObject* self = (GameObject*) go;
		other->SetParent( self, pos );
		
		// if second param was object
		if ( initObj ) {
			// copy properties from it
			script.CopyProperties( initObj, other->scriptObject );
		}
		
		// schedule layout
		if ( self->ui ) self->ui->RequestLayout( ArgValue( "addChild" ) );
		
		// return added object
		sa.ReturnObject( other->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "removeChild", // ( GameObject child | int position ) -> GameObject child
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: removeChild( GameObject obj | Int removePosition )";
		GameObject* self = (GameObject*) go;
		GameObject* other = NULL;
		void* obj = NULL;
		int pos = -1;
		
		// argument is object?
		if ( sa.ReadArguments( 1, TypeObject, &obj ) ) {
			other = script.GetInstance<GameObject>( obj );
			if ( !other ) error = "removeChild - parameter object is null";
		// argument is int?
		} else if ( sa.ReadArguments( 1, TypeInt, &pos ) ) {
			int numChildren = (int) self->children.size();
			if ( pos < 0 ) pos = numChildren + pos;
			if ( pos < 0 || pos >= numChildren ) error = "removeChild - index out of range";
			else other = self->children[ pos ];
		}
		
		// if not a valid call report error
		if ( !other ) {
			script.ReportError( error );
			return false;
		}
		
		// return the object
		sa.ReturnObject( other->scriptObject );
		
		// unparent
		other->SetParent( NULL );
		
		// schedule layout
		if ( self->ui ) self->ui->RequestLayout( ArgValue( "removeChild" ) );
		
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "removeAllChildren",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		GameObject* self = (GameObject*) go;
		
		// return the object
		ArgValueVector* prevChildren = self->GetChildrenVector();
		sa.ReturnArray( *prevChildren );
		delete prevChildren;
		
		// unparent all
		while ( self->children.size() ) {
			self->children.back()->SetParent( NULL );
		}
		
		// schedule layout
		if ( self->ui ) self->ui->RequestLayout( ArgValue( "removeChild" ) );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "moveTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: moveTo( Number x, Number y[, Float duration[, Int easeType[, Int easeFunc ] )";
		float x, y, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "x" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 2 );
		t->properties[ 0 ] = "x";
		t->properties[ 1 ] = "y";
		t->startValues.resize( 2 );
		t->startValues[ 0 ] = self->GetX();
		t->startValues[ 1 ] = self->GetY();
		t->endValues.resize( 2 );
		t->endValues[ 0 ] = x;
		t->endValues[ 1 ] = y;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "moveBy",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: moveBy( Number deltaX, Number deltaY[, Float duration[, Int easeType[, Int easeFunc ] )";
		float x, y, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 2, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "x" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 2 );
		t->properties[ 0 ] = "x";
		t->properties[ 1 ] = "y";
		t->startValues.resize( 2 );
		t->startValues[ 0 ] = self->GetX();
		t->startValues[ 1 ] = self->GetY();
		t->endValues.resize( 2 );
		t->endValues[ 0 ] = self->GetX() + x;
		t->endValues[ 1 ] = self->GetY() + y;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "rotateTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: rotateTo( Number angle[, Float duration[, Int easeType[, Int easeFunc ] )";
		float a, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeFloat, &a, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "angle" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 1 );
		t->properties[ 0 ] = "angle";
		t->startValues.resize( 1 );
		t->startValues[ 0 ] = self->GetAngle();
		t->endValues.resize( 1 );
		t->endValues[ 0 ] = a;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "rotateBy",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: rotateBy( Number deltaAngle[, Float duration[, Int easeType[, Int easeFunc ] )";
		float a, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeFloat, &a, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "angle" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 1 );
		t->properties[ 0 ] = "angle";
		t->startValues.resize( 1 );
		t->startValues[ 0 ] = self->GetAngle();
		t->endValues.resize( 1 );
		t->endValues[ 0 ] = self->GetAngle() + a;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "scaleTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: scaleTo( Number scale[, Float duration[, Int easeType[, Int easeFunc ] )";
		float s, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeFloat, &s, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "scale" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 1 );
		t->properties[ 0 ] = "scale";
		t->startValues.resize( 1 );
		t->startValues[ 0 ] = self->GetScaleX();
		t->endValues.resize( 1 );
		t->endValues[ 0 ] = s;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "scaleBy",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: scaleBy( Number deltaScale[, Float duration[, Int easeType[, Int easeFunc ] )";
		float s, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeFloat, &s, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop tweens
		Tween::StopTweens( self->scriptObject, "scale" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 1 );
		t->properties[ 0 ] = "scale";
		t->startValues.resize( 1 );
		t->startValues[ 0 ] = self->GetScaleX();
		t->endValues.resize( 1 );
		t->endValues[ 0 ] = self->GetScaleX() + s;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "fadeTo",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		
		// validate params
		const char* error = "usage: fadeTo( Number opacity[, Float duration[, Int easeType[, Int easeFunc ] )";
		float a, dur = 1;
		int etype = (int) Tween::EaseInOut, efunc = (int) Tween::EaseSine;
		GameObject* self = (GameObject*) go;
		
		// if not a valid call report error
		if ( !sa.ReadArguments( 1, TypeFloat, &a, TypeFloat, &dur, TypeInt, &etype, TypeInt, &efunc ) ) {
			script.ReportError( error );
			return false;
		}
		
		// stop opacity tweens
		Tween::StopTweens( self->scriptObject, "opacity" );
		
		// make tween
		Tween* t = new Tween( NULL );
		t->target = self->scriptObject;
		t->properties.resize( 1 );
		t->properties[ 0 ] = "opacity";
		t->startValues.resize( 1 );
		t->startValues[ 0 ] = self->opacity;
		t->endValues.resize( 1 );
		t->endValues[ 0 ] = a;
		t->duration = max( 0.0f, dur );
		t->easeType = (Tween::EasingType) etype;
		t->easeFunc = (Tween::EasingFunc) efunc;
		t->active( true );
		sa.ReturnObject( t->scriptObject );
		return true;
	}));

	script.DefineFunction<GameObject>
	( "stopMotion",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		GameObject* self = (GameObject*) go;
		Tween::StopTweens( self->scriptObject );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "dispatch", // like fire, but for hierarchy
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		const char* error = "usage: dispatch( String eventName [, arguments... ] )";
		string eventName;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeString, &eventName ) || !eventName.length() ) {
			script.ReportError( error );
			return false;
		}
		
		// make event with additional arguments
		Event event( eventName.c_str() );
		for ( size_t i = 1, np = sa.args.size(); i < np; i++ ) {
			event.scriptParams.AddArgument( sa.args[ i ] );
		}
		// dispatch
		GameObject* self = (GameObject*) go;
		self->DispatchEvent( event, true );
		sa.ReturnBool( !event.stopped );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "dispatchLate",
	 static_cast<ScriptFunctionCallback>([]( void* go, ScriptArguments& sa ) {
		const char* error = "usage: dispatchLate( String eventName [, arguments... ] )";
		string eventName;
		
		// validate
		if ( !sa.ReadArguments( 1, TypeString, &eventName ) || !eventName.length() ) {
			script.ReportError( error );
			return false;
		}
		
		// add to late events
		GameObject* self = (GameObject*) go;
		ArgValueVector *lateEvent = app.AddLateEvent( self, eventName.c_str(), true );
		if ( lateEvent ) {
			for ( size_t i = 1, np = sa.args.size(); i < np; i++ ) {
				lateEvent->push_back( sa.args[ i ] );
			}
		}
		return true;
	}));
	
	script.DefineFunction<GameObject>
	( "toString",
	 static_cast<ScriptFunctionCallback>([]( void* o, ScriptArguments& sa ) {
		static char buf[512];
		GameObject* self = (GameObject*) o;
		
		if ( !self ) {
			sprintf( buf, "[GameObject prototype]" );
		} else if ( self->scriptResource ) {
			if ( self->name.size() )
				sprintf( buf, "[GameObject %s <%s> %p]", self->name.c_str(), self->scriptResource->key.c_str(), self );
			else
				sprintf( buf, "[GameObject <%s> %p]", self->scriptResource->key.c_str(), self );
		} else if ( self->name.size() ) {
			sprintf( buf, "[GameObject %s %p]", self->name.c_str(), self );
		} else sprintf( buf, "[GameObject %p]", self );
		
		sa.ReturnString( buf );
		return true;
	}));
	
	script.DefineFunction<GameObject>
	("rayCast",
	 static_cast<ScriptFunctionCallback>([]( void* p, ScriptArguments& sa ){
		GameObject* g = (GameObject*) p;
		// arguments
		float x = 0, y = 0, dx = 0, dy = 0;
		void* ignoreBody = NULL;
		bool allObjects = false;
		int maxResults = 0;
		const char *error = "usage: rayCast( Number worldX, Number worldY, Number directionX, Number directionY, [ Body ignoreBody | Boolean allObjects, [ Int maxResults ] ] )";
		if ( !sa.ReadArguments( 4, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dx, TypeFloat, &dy ) ) {
			script.ReportError( error );
			return false;
		}
		
		// ignoreBody / allObjects
		if ( sa.args.size() >= 4 ) {
			if ( !sa.ReadArgumentsFrom( 4, 1, TypeObject, &ignoreBody, TypeInt, &maxResults ) ) {
				if ( !sa.ReadArgumentsFrom( 4, 1, TypeBool, &allObjects, TypeInt, &maxResults ) ) {
					script.ReportError( error );
					return false;
				}
			}
		}
		
		// call
		ArgValueVector* res = NULL;
		Scene* scene = g->GetScene();
		if ( allObjects ) {
			res = g->RayCastAll( x, y, dx, dy, maxResults );
		} else if ( scene ) {
			res = scene->RayCast( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE, dx * WORLD_TO_BOX2D_SCALE, dy * WORLD_TO_BOX2D_SCALE, maxResults, ignoreBody, g );
		} else {
			sa.ReturnNull();
			return true;
		}
		sa.ReturnArray( *res );
		delete res;
		return true;
	}));
	
	script.DefineFunction<GameObject>
	("query",
	 static_cast<ScriptFunctionCallback>([]( void* p, ScriptArguments& sa ){
		GameObject* g = (GameObject*) p;
		// arguments
		float x = 0, y = 0, dx = 0, dy = 0;
		void* ignoreBody = NULL;
		bool allObjects = false;
		int maxResults = 0;
		const char *error = "usage: query( Number worldX, Number worldY, Number width, Number height, [ Body ignoreBody | Boolean allObjects, [ Int maxResults ] ] )";
		if ( !sa.ReadArguments( 4, TypeFloat, &x, TypeFloat, &y, TypeFloat, &dx, TypeFloat, &dy ) ) {
			script.ReportError( error );
			return false;
		}
		
		// ignoreBody / allObjects
		if ( sa.args.size() >= 4 ) {
			if ( !sa.ReadArgumentsFrom( 4, 1, TypeObject, &ignoreBody, TypeInt, &maxResults ) ) {
				if ( !sa.ReadArgumentsFrom( 4, 1, TypeBool, &allObjects, TypeInt, &maxResults ) ) {
					script.ReportError( error );
					return false;
				}
			}
		}
		
		// call
		ArgValueVector* res = NULL;
		Scene* scene = g->GetScene();
		if ( allObjects ) {
			res = g->QueryAll( x, y, dx, dy, maxResults );
		} else if ( scene ) {
			res = scene->Query( x * WORLD_TO_BOX2D_SCALE, y * WORLD_TO_BOX2D_SCALE, dx * WORLD_TO_BOX2D_SCALE, dy * WORLD_TO_BOX2D_SCALE, maxResults, ignoreBody, g );
		} else {
			sa.ReturnNull();
			return true;
		}
		sa.ReturnArray( *res );
		delete res;
		return true;
	}));

	script.DefineFunction<GameObject>
	("requestLayout",
	 static_cast<ScriptFunctionCallback>([](void* p, ScriptArguments &sa) {
		GameObject* self = (GameObject*) p;
		if ( !self->ui ) return true;
		if ( sa.args.size() )
			self->ui->RequestLayout( sa.args[ 0 ] );
		else
			self->ui->RequestLayout( ArgValue() );
		return true;
	}));
	
}


/* MARK:	-				Behaviors / messaging
 -------------------------------------------------------------------- */


/// calls event handlers on each behavior, then script event listeners on gameObject. Recurses to active children.
void GameObject::DispatchEvent( Event& event, bool callOnSelf, GameObjectCallback *forEachGameObject ) {
	
	// event mask
	if ( this->eventMask ) {
		vector<string>* emask = this->eventMask->ToStringVector();
		size_t numEventMask = emask->size();
		if ( numEventMask > 0 ) {
			for ( size_t i = 0; i < numEventMask; i++ ) {
				// masked, ignore
				if ( (*emask)[ i ].compare( event.name ) == 0 ) return;
			}
		}
	}
	
	// clippable UI events
	bool clearClipped = false;
	if ( event.name == EVENT_MOUSEDOWN || event.name == EVENT_MOUSEMOVE || event.name == EVENT_MOUSEUP ) {
	    // parent clips this object
		if ( this->parent && this->parent->render != NULL && this->parent->render->ClipsMouseEventsFor( this ) ) {
			clearClipped = true;
			event.clippedBy = this->parent->render;
		}
	}
	
	// if it's a UI event considered in bounds by this UI or one of children, and this UI is blocking, auto-stop it
	if ( this->ui && this->ui->blocking && event.isBlockableUIEvent ) {
		event.willBlockUIEvent = true;
	}
	
	// if a lambda callback is provided, call it
	if ( forEachGameObject != NULL ) if ( !(*forEachGameObject)( this ) ) return;
	
	// call on self first
	if ( !event.bubbles && callOnSelf ) {
		this->CallEvent( event );
		if ( event.stopped ) return;
	}
	
	// behavior said skip children
	if ( event.skipChildren ) {
		// reset
		event.skipChildren = false;
	} else {
		// for each child
		GameObjectVector _children = this->children;
		for( int i = (int) _children.size() - 1; i >= 0; i-- ) {
			GameObject* obj = (GameObject*) _children[ i ];
			// recurse, if event doesn't need to skip this object
			if ( obj->scriptObject && obj->parent && obj->active() && obj != event.skipObject && obj != event.skipObject2 ) {
				obj->DispatchEvent( event, true, forEachGameObject );
				if ( event.stopped ) return;
			}
		}
	}
	
	// if bubbling, call on self after children
	if ( event.bubbles && callOnSelf ) {
		this->CallEvent( event );
	}
	
	// if it's a UI event considered in bounds by this UI or one of children, and this UI is blocking, auto-stop it
	if ( this->ui && this->ui->blocking && event.isBlockableUIEvent ) {
		// stop
		if ( event.isUIEventInBounds ) event.stopped = true;
		// reset
		event.isUIEventInBounds = event.willBlockUIEvent = false;
	}
	
	// clear clippedby when leaving recursion
	if ( clearClipped ) event.clippedBy = NULL;
	
}

// calls handler for event on each behavior
void GameObject::CallEvent( Event &event ) {
	
	if ( event.stopped ) return;
	
	// for each behavior
	for( BehaviorList::iterator i = this->behaviors.begin(), e = this->behaviors.end(); i != e; i++ ){
		
		// get callback
		Behavior* behavior = *i;
		
		// ensure it's active
		if ( !behavior->active() ) continue;
		
		// process event
		behavior->CallEventCallback( event );
	}
	
	// dispatches script events on this object
	if ( !event.stopped && !event.behaviorsOnly ) ScriptableClass::CallEvent( event );
	
}

/// returns ArgValueVector with each behavior's scriptObject
ArgValueVector* GameObject::GetBehaviorsVector() {
	ArgValueVector* vec = new ArgValueVector();
	for( BehaviorList::iterator i = this->behaviors.begin(), e = this->behaviors.end(); i != e; i++ ){
		Behavior *b = *i;
		vec->emplace_back( b->scriptObject );
	}
	return vec;
}

/// overwrites behaviors
ArgValueVector* GameObject::SetBehaviorsVector( ArgValueVector* in ) {
	// remove all behaviors first
	while( this->behaviors.begin() != this->behaviors.end() ) this->behaviors.front()->SetGameObject( NULL );
	// add behaviors
	size_t nc = in->size();
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*in)[ i ];
		Behavior* beh = script.GetInstance<Behavior>( val.value.objectValue );
		if ( beh ) beh->SetGameObject( this );
	}
	return in;
}

///
bool GameObject::Traverse( GameObjectCallback *forEachGameObject ) {
	
	// if a lambda callback is provided, call it
	if ( forEachGameObject != NULL ) if ( !(*forEachGameObject)( this ) ) return false;
	
	// for each child
	for( int i = (int) this->children.size() - 1; i >= 0; i-- ) {
		GameObject* obj = (GameObject*) this->children[ i ];
		if ( obj->active() ) {
			if ( !obj->Traverse( forEachGameObject ) ) return false;
		}
	}
	
	return true;
}



/* MARK:	-				Garbage collection
 -------------------------------------------------------------------- */


void GameObject::TraceProtectedObjects( vector<void **> &protectedObjects ) {
	// children
	for ( size_t i = 0, nc = this->children.size(); i < nc; i++ ) {
		protectedObjects.push_back( &(this->children[ i ]->scriptObject) );
	}
	// behaviors
	for( BehaviorList::iterator i = this->behaviors.begin(), e = this->behaviors.end(); i != e; i++ ){
		Behavior *b = *i;
		protectedObjects.push_back( &(b->scriptObject) );
	}
	// parent
	if ( this->parent ) protectedObjects.push_back( &parent->scriptObject );
	
	// event mask
	protectedObjects.push_back( &this->eventMask->scriptObject );
	
	// call super
	ScriptableClass::TraceProtectedObjects( protectedObjects );
}


/* MARK:	-				Hierarchy
 -------------------------------------------------------------------- */


// set a new parent for object
void GameObject::SetParent( GameObject* newParent, int desiredPosition ) {
	
	// if parent is different
	if ( newParent != this->parent ) {
	
		// if had parent
		GameObject* oldParent = this->parent;
		if ( oldParent ) {
			
			// parent's z-order should be updated
			oldParent->zSortedChildren.clear();
			
			// find this object in parent's list of children
			GameObjectVector *parentList = &oldParent->children;
			GameObjectVector::iterator listEnd = parentList->end();
			GameObjectVector::iterator it = find( parentList->begin(), listEnd, this );
			
			// remove from list
			int removedAt = -1;
			if ( it != listEnd ) {
				removedAt = (int) (it - parentList->begin());
				parentList->erase( it );
			}
			
			// clear
			this->parent = NULL;
			
			// call event on this object only
			Event event( this->scriptObject );
			event.name = EVENT_REMOVED;
			event.behaviorParam = oldParent;
			event.scriptParams.AddObjectArgument( oldParent->scriptObject );
			event.scriptParams.AddIntArgument( removedAt );
			this->CallEvent( event );
			
			// call child removed
			event.stopped = false;
			event.scriptParams.ResizeArguments( 0 );
			event.name = EVENT_CHILDREMOVED;
			event.behaviorParam = this;
			event.scriptParams.AddObjectArgument( this->scriptObject );
			event.scriptParams.AddIntArgument( removedAt );
			oldParent->CallEvent( event );
		}
		
		// set parent
		this->parent = newParent;
		
		// callback for orphaning
		GameObjectCallback makeOrphan = [](GameObject *obj) { obj->orphan = true; return true; };
		
		// add to new parent
		if ( newParent ) {
			
			// detect circular parenting
			if ( newParent->IsDescendantOf( this ) ){
				// unparent new parent first
				newParent->SetParent( NULL );
			}
			
			// insert into children based on desired position
			int numChildren = (int) newParent->children.size();
			// convert negative pos
			if ( desiredPosition < 0 ) desiredPosition = numChildren + 1 + desiredPosition;
			// insert at location
			if ( desiredPosition <= numChildren ) {
				desiredPosition = max( 0, desiredPosition );
				newParent->children.insert( newParent->children.begin() + desiredPosition, this );
			} else {
				newParent->children.push_back( this );
				desiredPosition = (int) newParent->children.size();
			}
			
			// parent's z-order should be updated
			newParent->zSortedChildren.clear();
			
			// call event on this object only
			Event event( this->scriptObject );
			event.name = EVENT_ADDED;
			event.behaviorParam = newParent;
			event.scriptParams.AddObjectArgument( newParent->scriptObject );
			event.scriptParams.AddIntArgument( desiredPosition );
			this->CallEvent( event );
			
			// if orphan status is about to change
			if ( this->orphan != newParent->orphan ) {
				
				// orphan change via new parent
				Event event( this->scriptObject );
				event.behaviorParam = newParent;
				
				// new parent is not on scene
				if ( newParent->orphan ) {
					
					// means this object has been removed from scene, dispatch event and make it orphan
					event.name = EVENT_REMOVEDFROMSCENE;
					this->DispatchEvent( event, true, &makeOrphan );
					
				} else {
					
					// means this object has been added to scene, dispatch event and set orphan values on descendants
					GameObjectCallback moveToScene = [](GameObject *obj) { obj->orphan = false; return true; };
					event.name = EVENT_ADDEDTOSCENE;
					this->DispatchEvent( event, true, &moveToScene );
					
				}
			}
			
			// call child added
			event.stopped = false;
			event.scriptParams.ResizeArguments( 0 );
			event.name = EVENT_CHILDADDED;
			event.behaviorParam = this;
			event.scriptParams.AddObjectArgument( this->scriptObject );
			event.scriptParams.AddIntArgument( desiredPosition );
			newParent->CallEvent( event );
			
		// no new parent - means we've definitely been removed from scene
		} else {
			
			// means this object has been removed from scene, dispatch event and make it orphan
			Event event( this->scriptObject );
			event.name = EVENT_REMOVEDFROMSCENE;
			event.behaviorParam = this;
			this->DispatchEvent( event, true, &makeOrphan );
			
		}
	}
	
}

// 
bool GameObject::IsDescendantOf( GameObject* grandDaddy ) {
	if ( this->parent == grandDaddy ) return true;
	else if ( this->parent ) return this->parent->IsDescendantOf( grandDaddy );
	return false;
}

/// finds scene this object is attached to
Scene* GameObject::GetScene() {
	if ( this == app.overlay && app.sceneStack.size() ) return app.sceneStack.back();
	if ( this->orphan || !this->parent ) return NULL;
	return this->parent->GetScene();
}

/// returns ArgValueVector with each child's scriptObject
ArgValueVector* GameObject::GetChildrenVector() {
	ArgValueVector* vec = new ArgValueVector();
	size_t nc = this->children.size();
	vec->resize( nc );
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*vec)[ i ];
		val.type = TypeObject;
		val.value.objectValue = this->children[ i ]->scriptObject;
	}
	return vec;
}

/// overwrites children, if array element is null, keeps existing child in that spot
ArgValueVector* GameObject::SetChildrenVector( ArgValueVector* in ) {
	// go over passed array
	int nc = (int) in->size();
	size_t curSize;
	size_t i = 0;
	for (; i < nc; i++ ){
		// each element
		ArgValue &val = (*in)[ i ];
		GameObject* go = NULL;
		curSize = this->children.size();
		if ( val.type == TypeObject && val.value.objectValue != NULL ) {
			go = script.GetInstance<GameObject>( val.value.objectValue );
		}
		// if a valid gameobject, and not same as child at this spot
		if ( go && ( i >= curSize || go != this->children[ i ] ) ) {
			// if there's a child here
			if ( i < curSize ) {
				// remove it
				this->children[ i ]->SetParent( NULL );
			}
			// insert in this spot
			go->SetParent( this, (int) i );
			
		// otherwise
		} else {
			// skip over this spot
		}
	}
	// remove remaining
	if ( in->size() < this->children.size() ) {
		for ( int j = (int) this->children.size() - 1; j >= nc; j-- ){ // >= ?
			this->children[ j ]->SetParent( NULL );
		}
	}
	return in;
}


/* MARK:	-				Non-Physics query
 -------------------------------------------------------------------- */


ArgValueVector* GameObject::QueryAll( float x, float y, float w, float h, int maxResults ) {
	// populate and return results
	ArgValueVector* ret = new ArgValueVector();
	float ox = x + w, oy = y + h;
	GameObjectCallback callback = static_cast<GameObjectCallback>([x,y,ox,oy,maxResults,ret]( GameObject* go ){
		// check if any of the corners of go are inside rectangle xywh first
		GPU_Rect bounds = go->GetBounds();
		float gx, gy;
		bool inside = false;
		go->ConvertPoint( bounds.x, bounds.y, gx, gy, true );
		inside = ( gx >= x && gx < ox && gy >= y && gy < oy );
		if ( !inside ) {
			go->ConvertPoint( bounds.x + bounds.w, bounds.y, gx, gy, true );
			inside = ( gx >= x && gx < ox && gy >= y && gy < oy );
			if ( !inside ) {
				go->ConvertPoint( bounds.x + bounds.w, bounds.y + bounds.h, gx, gy, true );
				inside = ( gx >= x && gx < ox && gy >= y && gy < oy );
				if ( !inside ) {
					go->ConvertPoint( bounds.x, bounds.y, gx, gy, true );
					inside = ( gx >= x && gx < ox && gy >= y && gy < oy );
				}
			}
		}
		// check the opposite - if any of the corners of outer rect are inside go
		if ( !inside ) {
			float bx = bounds.x + bounds.w, by = bounds.y + bounds.h;
			go->ConvertPoint( x, y, gx, gy, false );
			inside = ( gx >= bounds.x && gx < bx && gy >= bounds.y && gy < by );
			if ( !inside ) {
				go->ConvertPoint( ox, y, gx, gy, false );
				inside = ( gx >= bounds.x && gx < bx && gy >= bounds.y && gy < by );
				if ( !inside ) {
					go->ConvertPoint( ox, oy, gx, gy, false );
					inside = ( gx >= bounds.x && gx < bx && gy >= bounds.y && gy < by );
					if ( !inside ) {
						go->ConvertPoint( x, oy, gx, gy, false );
						inside = ( gx >= bounds.x && gx < bx && gy >= bounds.y && gy < by );
					}
				}
			}
		}
		
		// add
		if ( inside ) {
			ret->push_back( ArgValue( go->scriptObject ) );
		}
		return ( !maxResults || ret->size() < maxResults );
	});
	this->Traverse( &callback );
	return ret;
}

ArgValueVector* GameObject::RayCastAll( float x, float y, float dx, float dy, int maxResults ) {
	// populate and return results
	//https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
	ArgValueVector* ret = new ArgValueVector();
	return ret;
}


/* MARK:	-				Transform
 -------------------------------------------------------------------- */


bool GameObject::UseBodyTransform(){
    if ( this->body ) return this->body->UseBodyTransform();
    return false;
}

// returns current transformation matrix, also updates local coords, if body+dirty
float* GameObject::Transform() {
	
	if( isnan(this->_position.x) || isnan(this->_position.y) ) {
		_position.Set(0, 0);
	}

	// body + local coords arent up to date, or parent's transform
	if ( this->UseBodyTransform() && ( this->_localCoordsAreDirty || ( this->parent && this->parent->_inverseWorldDirty ) ) ) {
		
		// get global coords from body
		b2Vec2 pos; float angle;
		this->body->GetBodyTransform( pos, angle );
		GPU_MatrixIdentity( this->_worldTransform );
		GPU_MatrixTranslate( this->_worldTransform, floor( pos.x ), floor( pos.y ), _z );
		if ( angle != 0 ) GPU_MatrixRotate( this->_worldTransform, angle * RAD_TO_DEG, 0, 0, 1 );
		if ( this->_scale.x != 1 || this->_scale.y != 1 ) GPU_MatrixScale( this->_worldTransform, this->_scale.x, this->_scale.y, 1 );
		this->_worldTransformDirty = false;
		
		// multiply by parent's inverse world to get local
		if ( this->parent ) GPU_MatrixMultiply( this->_transform, this->parent->InverseWorld(), this->_worldTransform );
		
		// decompose local into components
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		
		// add skew
		if ( this->_skew.x != 0 || this->_skew.y != 0 ) MatrixSkew( this->_transform, _skew.x, _skew.y );
		
		this->_localCoordsAreDirty = false;
		
	} else if ( this->_transformDirty ) {
		
		// apply local transform
		GPU_MatrixIdentity( this->_transform );
		GPU_MatrixTranslate( this->_transform, floor( this->_position.x ), floor( this->_position.y ), _z );
		if ( this->_angle != 0 ) GPU_MatrixRotate( this->_transform, this->_angle, 0, 0, 1 );
		if ( this->_scale.x != 1 || this->_scale.y != 1 ) GPU_MatrixScale( this->_transform, this->_scale.x, this->_scale.y, 1 );
		if ( this->_skew.x != 0 || this->_skew.y != 0 ) MatrixSkew( this->_transform, _skew.x, _skew.y );
	}
	
	// clear flag
	this->_transformDirty = false;
	if( isnan(this->_position.x) || isnan(this->_position.y) ){
		printf( "GameObject::Transform position NaN\n" );
	}

	// return matrix
	return this->_transform;
}

// sets local transform
void GameObject::SetTransform( float x, float y, float angle, float scaleX, float scaleY ) {
	// set local transform vars
	this->_position.Set( x, y );
	this->_scale.Set( scaleX, scaleY );
	this->_angle = angle;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	this->_localCoordsAreDirty = false;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetPosition( float x, float y ) {
	//this->Transform();
	this->_position.Set( x, y );
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetPositionAndAngle( float x, float y, float angle ) {
	//this->Transform();
	this->_position.Set( x, y );
	this->_angle = angle;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetX( float x ) {
	//this->Transform();
	this->_position.x = x;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetY( float y ) {
	//this->Transform();
	this->_position.y = y;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetZ( float z ) {
	this->_z = z;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->parent ) this->parent->zSortedChildren.clear();
}

void GameObject::SetAngle( float a ) {
	//this->Transform();
	this->_angle = a;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetSkewX( float sx ) {
	//this->Transform();
	this->_skew.x = sx;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetSkewY( float sy ) {
	//this->Transform();
	this->_skew.y = sy;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetScale( float sx, float sy ) {
	//this->Transform();
	this->_scale.Set( sx, sy );
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetScaleX( float sx ) {
	//this->Transform();
	this->_scale.x = sx;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

void GameObject::SetScaleY( float sy ) {
	//this->Transform();
	this->_scale.y = sy;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->UseBodyTransform() ) this->body->SyncBodyToObject();
}

float GameObject::GetX() {
	this->Transform();
	return this->_position.x;
}

float GameObject::GetY() {
	this->Transform();
	return this->_position.y;
}

float GameObject::GetZ() {
	return this->_z;
}

float GameObject::GetScaleX() {
	this->Transform();
	return this->_scale.x;
}

float GameObject::GetScaleY() {
	this->Transform();
	return this->_scale.y;
}

float GameObject::GetSkewX() {
	this->Transform();
	return this->_skew.x;
}

float GameObject::GetSkewY() {
	this->Transform();
	return this->_skew.y;
}

float GameObject::GetAngle() {
	this->Transform();
	return this->_angle;
}

// sets world transform
void GameObject::SetWorldTransform( float x, float y, float angle, float scaleX, float scaleY ) {
	// resets skew
	this->_skew.SetZero();
	
	// have body
	if ( this->UseBodyTransform() ) {
		// set body transform
		this->_scale.Set( scaleX, scaleY );
		this->body->SetBodyTransform( b2Vec2( x, y ) , angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, x, y, angle, scaleX, scaleY ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldPosition( float x, float y ) {
	// resets skew
	this->_skew.SetZero();
	
	if ( this->UseBodyTransform() ) {
		this->body->SetBodyPosition( b2Vec2( x, y ) );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// get world space coords first
		b2Vec2 wpos, wscale; float wangle;
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		// local transform = parent's inverse world * this world matrix
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, x, y, wangle, wscale.x, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldPositionAndAngle( float x, float y, float angle ) {
	// resets skew
	this->_skew.SetZero();
	
	if ( this->UseBodyTransform() ) {
		this->body->SetBodyTransform( b2Vec2( x, y ), angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// get world space coords first
		b2Vec2 wpos, wscale; float wangle;
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		// local transform = parent's inverse world * this world matrix
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, x, y, angle, wscale.x, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldX( float x ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
		this->body->SetBodyPosition( b2Vec2( x, wpos.y ) );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, x, wpos.y, wangle, wscale.x, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldY( float y ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
		this->body->SetBodyPosition( b2Vec2( wpos.x, y ) );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, wpos.x, y, wangle, wscale.x, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldAngle( float angle ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->body->SetBodyAngle( angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, wpos.x, wpos.y, angle, wscale.x, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldScale( float sx, float sy ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->_scale.Set( sx, sy );
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, wpos.x, wpos.y, wangle, sx, sy ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldScaleX( float sx ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->_scale.x = sx;
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, wpos.x, wpos.y, wangle, sx, wscale.y ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

void GameObject::SetWorldScaleY( float sy ) {
	// resets skew
	this->_skew.SetZero();
	
	// get world space coords first
	b2Vec2 wpos, wscale; float wangle;
	if ( this->UseBodyTransform() ) {
		this->_scale.y = sy;
		this->body->SyncObjectToBody();
	} else if ( this->parent ) {
		// local transform = parent's inverse world * this world matrix
		this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
		GPU_MatrixMultiply( this->_transform,
						this->parent->InverseWorld(),
						MatrixCompose( this->_worldTransform, wpos.x, wpos.y, wangle, wscale.x, sy ) );
		// extract pos, rot, scale back into variables
		this->DecomposeTransform( this->_transform, this->_position, this->_angle, this->_scale );
		this->_transformDirty = false; this->_inverseWorldDirty = true;
	}
}

b2Vec2 GameObject::GetWorldPosition() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
	}
	return wpos;
}

float GameObject::GetWorldX() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
	}
	return wpos.x;
}

float GameObject::GetWorldY() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
	}
	return wpos.y;
}

float GameObject::GetWorldScaleX() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	return wscale.x;
}

float GameObject::GetWorldScaleY() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	return wscale.y;
}

b2Vec2 GameObject::GetWorldScale() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	return wscale;
}

float GameObject::GetWorldAngle() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->UseBodyTransform() ) {
		this->body->GetBodyTransform( wpos, wangle );
		wangle *= RAD_TO_DEG;
	}
	return wangle;
}

/// returns world transform of this object
float* GameObject::WorldTransform() {
	
	if ( this->_worldTransformDirty ) {
		
		// update transform
		this->Transform();
		
		// have parent
		if ( this->parent != NULL ) {
			
			// concat world transform
			GPU_MatrixMultiply( this->_worldTransform, this->parent->WorldTransform(), this->_transform );
			
		// no parent
		} else {
			
			//local transform is world
			GPU_MatrixCopy( this->_worldTransform, this->_transform );
			
		}
		// dirty flags
		this->_worldTransformDirty = false;
		this->_inverseWorldDirty = true;
		
	}
	
	return this->_worldTransform;
}

/// returns inverse of local transform of this object
float* GameObject::InverseWorld() {
	
	if ( this->_inverseWorldDirty ) {
		
		// invert world matrix
		GameObject::MatrixInverse( this->WorldTransform(), this->_inverseWorldTransform );
		
		// dirty flags
		this->_inverseWorldDirty = false;
		
	}
	
	return this->_inverseWorldTransform;
}

/// extracts local x, y, angle, and scale from local transform matrix
void GameObject::DecomposeTransform( float *te, b2Vec2& pos, float& angle, b2Vec2& scale ) {
	
	float sx = b2Vec3( te[ 0 ], te[ 1 ], te[ 2 ] ).Length();
	float sy = b2Vec3( te[ 4 ], te[ 5 ], te[ 6 ] ).Length();
	
	float n11 = te[ 0 ], n12 = te[ 4 ], n13 = te[ 8 ], n14 = te[ 12 ];
	float n21 = te[ 1 ], n22 = te[ 5 ], n23 = te[ 9 ], n24 = te[ 13 ];
	float n31 = te[ 2 ], n32 = te[ 6 ], n33 = te[ 10 ], n34 = te[ 14 ];
	float n41 = te[ 3 ], n42 = te[ 7 ], n43 = te[ 11 ], n44 = te[ 15 ];
	
	float det = ( n41 * (
						 + n14 * n23 * n32
						 - n13 * n24 * n32
						 - n14 * n22 * n33
						 + n12 * n24 * n33
						 + n13 * n22 * n34
						 - n12 * n23 * n34
						 ) + n42 * (
									+ n11 * n23 * n34
									- n11 * n24 * n33
									+ n14 * n21 * n33
									- n13 * n21 * n34
									+ n13 * n24 * n31
									- n14 * n23 * n31
									) + n43 * (
											   + n11 * n24 * n32
											   - n11 * n22 * n34
											   - n14 * n21 * n32
											   + n12 * n21 * n34
											   + n14 * n22 * n31
											   - n12 * n24 * n31
											   ) + n44 * (
														  - n13 * n22 * n31
														  - n11 * n23 * n32
														  + n11 * n22 * n33
														  + n13 * n21 * n32
														  - n12 * n21 * n33
														  + n12 * n23 * n31
														  ) );
	
	if ( det < 0 ) sx = - sx;
	
	// extract position
	pos.x = te[ 12 ];
	pos.y = te[ 13 ];
	
	// extract rotation
	angle = -RAD_TO_DEG * atan2( te[ 4 ] / sy, te[ 0 ] / sx );
	
	// scale
	scale.x = sx;
	scale.y = sy;
	
}

// setter for active
bool GameObject::active( bool a ) {
	
	// value changed
	if ( a != this->_active ){
		
		// apply
		this->_active = a;
		
		// dispatch to self and children
		Event event( EVENT_ACTIVECHANGED, this->scriptObject );
		this->DispatchEvent( event, true );
		
	}
	
	return a;
}

/// inverts source matrix, writes to dest, returns true on success
bool GameObject::MatrixInverse( float *m, float *inv ) {
	
	inv[0] = m[5]  * m[10] * m[15] -
	m[5]  * m[11] * m[14] -
	m[9]  * m[6]  * m[15] +
	m[9]  * m[7]  * m[14] +
	m[13] * m[6]  * m[11] -
	m[13] * m[7]  * m[10];
	
	inv[4] = -m[4]  * m[10] * m[15] +
	m[4]  * m[11] * m[14] +
	m[8]  * m[6]  * m[15] -
	m[8]  * m[7]  * m[14] -
	m[12] * m[6]  * m[11] +
	m[12] * m[7]  * m[10];
	
	inv[8] = m[4]  * m[9] * m[15] -
	m[4]  * m[11] * m[13] -
	m[8]  * m[5] * m[15] +
	m[8]  * m[7] * m[13] +
	m[12] * m[5] * m[11] -
	m[12] * m[7] * m[9];
	
	inv[12] = -m[4]  * m[9] * m[14] +
	m[4]  * m[10] * m[13] +
	m[8]  * m[5] * m[14] -
	m[8]  * m[6] * m[13] -
	m[12] * m[5] * m[10] +
	m[12] * m[6] * m[9];
	
	inv[1] = -m[1]  * m[10] * m[15] +
	m[1]  * m[11] * m[14] +
	m[9]  * m[2] * m[15] -
	m[9]  * m[3] * m[14] -
	m[13] * m[2] * m[11] +
	m[13] * m[3] * m[10];
	
	inv[5] = m[0]  * m[10] * m[15] -
	m[0]  * m[11] * m[14] -
	m[8]  * m[2] * m[15] +
	m[8]  * m[3] * m[14] +
	m[12] * m[2] * m[11] -
	m[12] * m[3] * m[10];
	
	inv[9] = -m[0]  * m[9] * m[15] +
	m[0]  * m[11] * m[13] +
	m[8]  * m[1] * m[15] -
	m[8]  * m[3] * m[13] -
	m[12] * m[1] * m[11] +
	m[12] * m[3] * m[9];
	
	inv[13] = m[0]  * m[9] * m[14] -
	m[0]  * m[10] * m[13] -
	m[8]  * m[1] * m[14] +
	m[8]  * m[2] * m[13] +
	m[12] * m[1] * m[10] -
	m[12] * m[2] * m[9];
	
	inv[2] = m[1]  * m[6] * m[15] -
	m[1]  * m[7] * m[14] -
	m[5]  * m[2] * m[15] +
	m[5]  * m[3] * m[14] +
	m[13] * m[2] * m[7] -
	m[13] * m[3] * m[6];
	
	inv[6] = -m[0]  * m[6] * m[15] +
	m[0]  * m[7] * m[14] +
	m[4]  * m[2] * m[15] -
	m[4]  * m[3] * m[14] -
	m[12] * m[2] * m[7] +
	m[12] * m[3] * m[6];
	
	inv[10] = m[0]  * m[5] * m[15] -
	m[0]  * m[7] * m[13] -
	m[4]  * m[1] * m[15] +
	m[4]  * m[3] * m[13] +
	m[12] * m[1] * m[7] -
	m[12] * m[3] * m[5];
	
	inv[14] = -m[0]  * m[5] * m[14] +
	m[0]  * m[6] * m[13] +
	m[4]  * m[1] * m[14] -
	m[4]  * m[2] * m[13] -
	m[12] * m[1] * m[6] +
	m[12] * m[2] * m[5];
	
	inv[3] = -m[1] * m[6] * m[11] +
	m[1] * m[7] * m[10] +
	m[5] * m[2] * m[11] -
	m[5] * m[3] * m[10] -
	m[9] * m[2] * m[7] +
	m[9] * m[3] * m[6];
	
	inv[7] = m[0] * m[6] * m[11] -
	m[0] * m[7] * m[10] -
	m[4] * m[2] * m[11] +
	m[4] * m[3] * m[10] +
	m[8] * m[2] * m[7] -
	m[8] * m[3] * m[6];
	
	inv[11] = -m[0] * m[5] * m[11] +
	m[0] * m[7] * m[9] +
	m[4] * m[1] * m[11] -
	m[4] * m[3] * m[9] -
	m[8] * m[1] * m[7] +
	m[8] * m[3] * m[5];
	
	inv[15] = m[0] * m[5] * m[10] -
	m[0] * m[6] * m[9] -
	m[4] * m[1] * m[10] +
	m[4] * m[2] * m[9] +
	m[8] * m[1] * m[6] -
	m[8] * m[2] * m[5];
	
	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	
	if (det == 0) return false;
	
	det = 1.0 / det;
	inv[ 0 ] *= det; inv[ 1 ] *= det; inv[ 2 ] *= det; inv[ 3 ] *= det;
	inv[ 4 ] *= det; inv[ 5 ] *= det; inv[ 6 ] *= det; inv[ 7 ] *= det;
	inv[ 8 ] *= det; inv[ 9 ] *= det; inv[ 10 ] *= det; inv[ 11 ] *= det;
	inv[ 12 ] *= det; inv[ 13 ] *= det; inv[ 14 ] *= det; inv[ 15 ] *= det;
	
	return true;
}

/// recursive ignore cam check
bool GameObject::IsCameraIgnored() {
	if ( ignoreCamera ) return true;
	if ( parent ) return parent->IsCameraIgnored();
	return false;
}

// converts between coord systems
void GameObject::ConvertPoint( float x, float y, float &outX, float &outY, bool localToGlobal, bool screenSpace ) {
	
	// matrix with x, y
	float mat[ 16 ];
	float res[ 16 ];
	GPU_MatrixIdentity( mat );
	GPU_MatrixTranslate( mat, x, y, 0 );
	
	// multiply by inverse world
	Scene* scene = orphan ? NULL : GetScene();
	if ( localToGlobal ) {
		this->_worldTransformDirty = true;
		GPU_MatrixMultiply( res, this->WorldTransform(), mat );
		// apply camera transform
		if ( screenSpace && scene && !this->IsCameraIgnored() ) {
			GPU_MatrixCopy( mat, res );
			GPU_MatrixMultiply( res, scene->CameraTransform(), mat );
		}
	// multiply by world
	} else {
		// apply inverse camera
		if ( screenSpace && scene && !this->IsCameraIgnored() ) {
			GPU_MatrixMultiply( res, scene->InverseCameraTransform(), mat );
			GPU_MatrixCopy( mat, res );
		}
		GPU_MatrixMultiply( res, this->InverseWorld(), mat );

	}
	outX = isnan(res[ 12 ]) ? 0 : res[ 12 ];
	outY = isnan(res[ 13 ]) ? 0 : res[ 13 ];
}

void GameObject::ConvertDirection( float x, float y, float &outX, float &outY, bool localToGlobal ) {
    float* m;
    float len = sqrtf( x * x + y * y );
    if ( len == 0.0 ) {
        outX = outY = 0;
        return;
    }
    
    // multiply by inverse world
    if ( localToGlobal ) {
        this->_worldTransformDirty = true;
        m = this->WorldTransform();
    // multiply by world
    } else {
       m = this->InverseWorld();
    }

    outX = m[ 0 ] * x + m[ 4 ] * y;
    outY = m[ 1 ] * x + m[ 5 ] * y;
}

GPU_Rect GameObject::GetBounds(){
	if ( this->ui ) return { 0, 0, this->ui->layoutWidth, this->ui->layoutHeight };
	if ( this->render ) return this->render->GetBounds(); // TODO - shape bounds arent implemented
	GPU_Rect r = { 0, 0, 0, 0 };
	return r;
}

// force recalculate matrices on this object + all descendants
void GameObject::DirtyTransform() {
	
	// set dirty
	this->_transformDirty = this->_inverseWorldDirty = this->_localCoordsAreDirty = this->_worldTransformDirty = true;
	// recurse
	for ( size_t i = 0, nc = this->children.size(); i < nc; i++ ){
		this->children[ i ]->DirtyTransform();
	}
	
}

/* MARK:	-				Render
 -------------------------------------------------------------------- */

bool _zSortChildrenCompare( GameObject* a, GameObject* b ) {
	return a->GetZ() < b->GetZ();
}

// Renders this GameObject using renderer behavior, goes recursive
void GameObject::Render( Event& event ) {
	
	// clipped by image
	bool clearClipped = false;
	if ( this->parent && this->parent->render != NULL && this->parent->render->ClipsMouseEventsFor( this ) ) {
		clearClipped = true;
		event.clippedBy = this->parent->render;
	}
	
	// if ignoring camera, load identity
    if ( this->ignoreCamera ) {
        // push view matrix
        GPU_MatrixMode( GPU_PROJECTION );
        GPU_PushMatrix();
        GPU_Target* rt = (GPU_Target*) event.behaviorParam;
        float *p = GPU_GetProjection();
        GPU_MatrixIdentity( p );
        GPU_MatrixOrtho( p, 0, rt->w, 0, rt->h, -1024, 1024 );
    }
	
	// push parent transform matrix
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PushMatrix();
<<<<<<< HEAD
    float* mv = GPU_GetCurrentMatrix();
=======
	GPU_FlushBlitBuffer(); // without this, child transform affects parent
>>>>>>> parent of 5e63b91... wip optimisation
	
	// update combined opacity
	this->combinedOpacity = ( this->parent ? this->parent->combinedOpacity : 1 ) * this->opacity;
	
	// transforming using body
	if ( this->UseBodyTransform() ) {
		
		// if rendering to image / clipped
		if ( event.clippedBy ) {
			// container's world transform
			GPU_MatrixCopy( mv, event.clippedBy->gameObject->InverseWorld() );
			GPU_MultMatrix( this->_worldTransform );
		} else {
			GPU_MatrixCopy( mv, this->_worldTransform );
		}
		
	} else {
		
		// multiply
		GPU_MultMatrix( this->Transform() );
		
		// update world matrix
		GPU_MatrixCopy( this->_worldTransform, mv );
		this->_worldTransformDirty = false;
		this->_inverseWorldDirty = true;
		
	}
	
	// render before children?
	bool doRender = (this->render != NULL && this->render->active());
	if ( doRender && !this->renderAfterChildren ) {
		// find function
		BehaviorEventCallback func = this->render->GetCallbackForEvent( event.name );
		if ( func != NULL ) (*func)( this->render, event.behaviorParam, &event );
	}
	
	// debug ui
	if ( this->ui && app.debugDraw ) ui->DebugDraw( (GPU_Target*) event.behaviorParam );
	
	// sort children
	int numChildren = (int) this->children.size();
	if ( zSortedChildren.size() != numChildren ) {
		zSortedChildren = children;
		if ( numChildren > 1 ) sort( zSortedChildren.begin(), zSortedChildren.end(), &_zSortChildrenCompare );
	}
	
	// descend into children
	for( int i = 0; i < numChildren; i++ ) {
		GameObject* obj = zSortedChildren[ i ];
		// recurse if render behavior didn't ask to skip it
		if ( obj->active() && obj != event.skipObject && obj != event.skipObject2 ) obj->Render( event );
	}
	
	// clear skip
	event.skipObject = event.skipObject2 = NULL;
	
	// render after children?
	if ( doRender && this->renderAfterChildren ) {
		// find function
		BehaviorEventCallback func = this->render->GetCallbackForEvent( event.name );
		if ( func != NULL ) (*func)( this->render, event.behaviorParam, &event );
	}
	
	// pop matrices
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PopMatrix();
    if ( this->ignoreCamera ) {
        GPU_MatrixMode( GPU_PROJECTION );
        GPU_PopMatrix();
    }
	
	// clear clippedby when leaving recursion
	if ( clearClipped ) event.clippedBy = NULL;

}
