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
	
	// printf( "~GameObject %p\n", this );
	
	// release resource
	if ( this->scriptResource ) this->scriptResource->AdjustUseCount( -1 );

}


/* MARK:	-				Javascript
 -------------------------------------------------------------------- */


// init script classes
void GameObject::InitClass() {
	
	// register GameObject script class
	script.RegisterClass<GameObject>( "ScriptableObject" );
	
	// properties
	
	script.AddProperty<GameObject>
	( "name",
	 static_cast<ScriptStringCallback>([](void* go, string ) { return ((GameObject*) go)->name; }),
	 static_cast<ScriptStringCallback>([](void* go, string val ) { return ((GameObject*) go)->name = val; }));
	
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
	 static_cast<ScriptIntCallback>([](void* go, int) { return ((GameObject*) go)->children.size(); }));
	
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
		return ArgValue();
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
			printf( ".script path \"%s\" was not found.\n", key );
		
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
		const char* error = "usage: addChild( [ GameObject obj | String scriptPath [,Int desiredPosition | Object initProperties ] ] )";
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
		int pos = -1; //(int) other->children.size();
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
			sprintf( buf, "[GameObject (%s) %p]", self->scriptResource->key.c_str(), self );
		} else if ( self->name.size() ) {
			sprintf( buf, "[GameObject \"%s\" %p]", self->name.c_str(), self );
		} else sprintf( buf, "[GameObject %p]", self );
		
		sa.ReturnString( buf );
		return true;
	}));

}


/* MARK:	-				Behaviors / messaging
 -------------------------------------------------------------------- */


/// calls event handlers on each behavior, then script event listeners on gameObject. Recurses to active children.
void GameObject::DispatchEvent( Event& event, bool callOnSelf, GameObjectCallback *forEachGameObject ) {
	
	// if a lambda callback is provided, call it
	if ( forEachGameObject != NULL ) (*forEachGameObject)( this );
	
	// call on self first
	if ( !event.bubbles && callOnSelf ) {
		this->CallEvent( event );
		if ( event.stopped ) return;
	}
	
	// for each child
	for( int i = (int) this->children.size() - 1; i >= 0; i-- ) {
		GameObject* obj = (GameObject*) this->children[ i ];
		// recurse, if event doesn't need to skip this object
		if ( obj->active() && obj != event.skipObject ) {
			obj->DispatchEvent( event, true, forEachGameObject );
			if ( event.stopped ) return;
		}
	}
	
	// if bubbling, call on self after children
	if ( event.bubbles && callOnSelf ) {
		this->CallEvent( event );
	}
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
	if ( !event.stopped ) ScriptableClass::CallEvent( event );
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
			
			// find this object in parent's list of children
			GameObjectVector *parentList = &oldParent->children;
			GameObjectIterator listEnd = parentList->end();
			GameObjectIterator it = find( parentList->begin(), listEnd, this );
			
			// remove from list
			if ( it != listEnd ) parentList->erase( it );
			
			// clear
			this->parent = NULL;
			
			// call event on this object only
			Event event( this->scriptObject );
			event.name = EVENT_REMOVED;
			event.behaviorParam = oldParent;
			event.scriptParams.AddObjectArgument( oldParent->scriptObject );
			this->CallEvent( event );
			
			// call child removed
			event.stopped = false;
			event.scriptParams.ResizeArguments( 0 );
			event.name = EVENT_CHILDREMOVED;
			event.behaviorParam = this;
			event.scriptParams.AddObjectArgument( this->scriptObject );
			oldParent->CallEvent( event );
		}
		
		// set parent
		this->parent = newParent;
		
		// callback for orphaning
		GameObjectCallback makeOrphan = [](GameObject *obj) { obj->orphan = true; };
		
		// add to new parent
		if ( newParent ) {
			
			// TODO - detect circular linking
			
			// insert into children based on desired position
			int numChildren = (int) newParent->children.size();
			// convert negative pos
			if ( desiredPosition < 0 ) desiredPosition = numChildren + 1 + desiredPosition;
			// insert at location
			if ( desiredPosition <= numChildren ) {
				newParent->children.insert( newParent->children.begin() + max( 0, desiredPosition ), this );
			} else {
				newParent->children.push_back( this );
			}
			
			// call event on this object only
			Event event( this->scriptObject );
			event.name = EVENT_ADDED;
			event.behaviorParam = newParent;
			event.scriptParams.AddObjectArgument( newParent->scriptObject );
			this->CallEvent( event );
			
			// if orphan status is about to change
			if ( this->orphan != newParent->orphan ) {
				
				// orphan change via new parent
				Event event( this->scriptObject );
				event.behaviorParam = newParent;
				
				// new parent is not on scene
				if ( newParent->orphan ) {
					
					// means this object has been removed from scene, dispatch event and make it orphan
					event.name = EVENT_REMOVED_FROM_SCENE;
					this->DispatchEvent( event, true, &makeOrphan );
					
				} else {
					
					// means this object has been added to scene, dispatch event and set orphan values on descendents
					GameObjectCallback moveToScene = [](GameObject *obj) { obj->orphan = false; };
					event.name = EVENT_ADDED_TO_SCENE;
					this->DispatchEvent( event, true, &moveToScene );
					
				}
			}
			
			// call child added
			event.stopped = false;
			event.scriptParams.ResizeArguments( 0 );
			event.name = EVENT_CHILDADDED;
			event.behaviorParam = this;
			event.scriptParams.AddObjectArgument( this->scriptObject );
			newParent->CallEvent( event );
			
		// no new parent - means we've definitely been removed from scene
		} else {
			
			// means this object has been removed from scene, dispatch event and make it orphan
			Event event( this->scriptObject );
			event.name = EVENT_REMOVED_FROM_SCENE;
			event.behaviorParam = this;
			this->DispatchEvent( event, true, &makeOrphan );
			
		}
	}
}

/// finds scene this object is attached to
Scene* GameObject::GetScene() {
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

/// overwrites children
ArgValueVector* GameObject::SetChildrenVector( ArgValueVector* in ) {
	// remove all children first
	while( this->children.size() ) this->children[ 0 ]->SetParent( NULL );
	// add children
	size_t nc = in->size();
	for ( size_t i = 0; i < nc; i++ ){
		ArgValue &val = (*in)[ i ];
		if ( val.type == TypeObject ) {
			GameObject* go = script.GetInstance<GameObject>( val.value.objectValue );
			if ( go ) go->SetParent( this );
		}
	}
	return in;
}


/* MARK:	-				Transform
 -------------------------------------------------------------------- */

bool GameObject::HasBody(){ return ( this->body && this->body->live ); }

// returns current transformation matrix, also updates local coords, if body+dirty
float* GameObject::Transform() {
	
	assert( !isnan(this->_position.x) && !isnan(this->_position.y));

	// body + local coords arent up to date, or parent's transform
	if ( this->HasBody() && ( this->_localCoordsAreDirty || ( this->parent && this->parent->_inverseWorldDirty ) ) ) {
		
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
	
	assert( !isnan(this->_position.x) && !isnan(this->_position.y));
	
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
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetPosition( float x, float y ) {
	//this->Transform();
	this->_position.Set( x, y );
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetPositionAndAngle( float x, float y, float angle ) {
	//this->Transform();
	this->_position.Set( x, y );
	this->_angle = angle;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetX( float x ) {
	//this->Transform();
	this->_position.x = x;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetY( float y ) {
	//this->Transform();
	this->_position.y = y;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetZ( float z ) {
	//this->Transform();
	this->_z = z;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
}

void GameObject::SetAngle( float a ) {
	//this->Transform();
	this->_angle = a;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetSkewX( float sx ) {
	//this->Transform();
	this->_skew.x = sx;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetSkewY( float sy ) {
	//this->Transform();
	this->_skew.y = sy;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetScale( float sx, float sy ) {
	//this->Transform();
	this->_scale.Set( sx, sy );
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetScaleX( float sx ) {
	//this->Transform();
	this->_scale.x = sx;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

void GameObject::SetScaleY( float sy ) {
	//this->Transform();
	this->_scale.y = sy;
	this->_transformDirty = this->_inverseWorldDirty = this->_worldTransformDirty = true;
	if ( this->HasBody() ) this->body->SyncBodyToObject();
}

float GameObject::GetX() {
	this->Transform();
	return this->_position.x;
}

float GameObject::GetY() {
	this->Transform();
	return this->_position.y;
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
	if ( this->HasBody() ) {
		// set body transform
		this->_scale.Set( scaleX, scaleY );
		this->body->SetBodyTransform( b2Vec2( x, y ) , angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else {
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
	
	if ( this->HasBody() ) {
		this->body->SetBodyPosition( b2Vec2( x, y ) );
		this->body->SyncObjectToBody();
	} else {
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
	
	if ( this->HasBody() ) {
		this->body->SetBodyTransform( b2Vec2( x, y ), angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->body->GetBodyTransform( wpos, wangle );
		this->body->SetBodyPosition( b2Vec2( x, wpos.y ) );
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->body->GetBodyTransform( wpos, wangle );
		this->body->SetBodyPosition( b2Vec2( wpos.x, y ) );
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->body->SetBodyAngle( angle * DEG_TO_RAD );
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->_scale.Set( sx, sy );
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->_scale.x = sx;
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->_scale.y = sy;
		this->body->SyncObjectToBody();
	} else {
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
	if ( this->HasBody() ) {
		this->body->GetBodyTransform( wpos, wangle );
	}
	return wpos;
}

float GameObject::GetWorldX() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->HasBody() ) {
		this->body->GetBodyTransform( wpos, wangle );
	}
	return wpos.x;
}

float GameObject::GetWorldY() {
	b2Vec2 wpos, wscale; float wangle;
	this->DecomposeTransform( this->WorldTransform(), wpos, wangle, wscale );
	if ( this->HasBody() ) {
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
	if ( this->HasBody() ) {
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
		Event event( EVENT_ACTIVE_CHANGED, this->scriptObject );
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


// force recalculate matrices on this object + all descendents
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

// Renders this GameObject using renderer behavior, goes recursive
void GameObject::Render( Event& event ) {
	
	// push view matrix
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PushMatrix();
	
	// if ignoring camera, load identity
	if ( this->ignoreCamera ) GPU_MatrixIdentity( GPU_GetCurrentMatrix() );
	
	// push parent transform matrix
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PushMatrix();
	GPU_FlushBlitBuffer(); // without this, child transform affects parent
	
	// update combined opacity
	this->combinedOpacity = ( this->parent ? this->parent->combinedOpacity : 1 ) * this->opacity;
	
	// transforming using body
	if ( this->HasBody() ) {
		
		GPU_MatrixCopy( GPU_GetCurrentMatrix(), this->_worldTransform );
		
	} else {
		
		// multiply
		GPU_MultMatrix( this->Transform() );
		
		// update world matrix
		GPU_MatrixCopy( this->_worldTransform, GPU_GetCurrentMatrix() );
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
	if ( this->ui && app.debugUI ) ui->DebugDraw( (GPU_Target*) event.behaviorParam );
	
	// descend into children
	int numChildren = (int) this->children.size();
	for( int i = 0; i < numChildren; i++ ) {
		GameObject* obj = this->children[ i ];
		// recurse if render behavior didn't ask to skip it
		if ( obj->active() && obj != event.skipObject ) obj->Render( event );
	}	
	
	// render after children?
	if ( doRender && this->renderAfterChildren ) {
		// find function
		BehaviorEventCallback func = this->render->GetCallbackForEvent( event.name );
		if ( func != NULL ) (*func)( this->render, event.behaviorParam, &event );
	}
	
	// pop matrices
	GPU_MatrixMode( GPU_MODELVIEW );
	GPU_PopMatrix();
	GPU_MatrixMode( GPU_PROJECTION );
	GPU_PopMatrix();
}


