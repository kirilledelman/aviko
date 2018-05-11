#ifndef GameObject_hpp
#define GameObject_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "Behavior.hpp"
#include "RenderBehavior.hpp"
#include "UIBehavior.hpp"
#include "RigidBodyBehavior.hpp"
#include "TypedVector.hpp"

typedef vector<GameObject*> GameObjectVector;
typedef vector<GameObject*>::iterator GameObjectIterator;

typedef list<Behavior*> BehaviorList;

SCRIPT_CLASS_NAME( GameObject, "GameObject" );

class GameObject : public ScriptableClass {
protected:
	
	// object unique id
	uint32 _instanceId = 0;
	
	/// object's active status
	bool _active = true;
	
	/// local transformation matrix
	float _transform[16];
	
	/// combined transformation matrix
	float _worldTransform[16];
	
	/// inverse world transformation matrix
	float _inverseWorldTransform[16];
	
	/// x, y coordinates
	b2Vec2 _position = { 0, 0 };
	
	/// x, y scale
	b2Vec2 _scale = { 1, 1 };
	
	// skew
	b2Vec2 _skew = { 0, 0 };
	
	/// z-index
	float _z = 0;
	
	/// rotation angle in degrees
	float _angle = 0;
	
	/// true when an object with body's world matrix hasnt been converted back to local coords yet (on demand)
	bool _localCoordsAreDirty = true;
	
	/// true when matrix needs to be recalculated
	bool _transformDirty = true;
	
	/// true when world matrix needs to be recalculated
	bool _worldTransformDirty = true;

	/// true when inverse world transform matrix needs to be recalculated
	bool _inverseWorldDirty = true;
	
public:
	
	// init, destroy
	GameObject( ScriptArguments* args );
	GameObject();
	~GameObject();

// scripting
		
	/// registers classes for scripting
	static void InitClass();
	
	/// script resource
	ScriptResource* scriptResource = NULL;
		
// identity
		
	/// object name
	string name;
	
// serialization
	
	void* MakeInitObject();
	
// transformation
	
	/// shortcut to rigid body behavior, if attached
	BodyBehavior* body = NULL;
	
	/// returns true if theres a physics body in the world
	bool HasBody();
	
	/// shortcut to rendering behavior
	RenderBehavior* render = NULL;
	
	/// shortcut to ui behavior
	UIBehavior* ui = NULL;
	
	/// used for HUD style drawing
	bool ignoreCamera = false;
	
	/// returns true if any parent has ignored camera
	bool IsCameraIgnored();
	
	/// sets local transform
	void SetTransform( float x, float y, float angle, float scaleX, float scaleY );
	void SetPosition( float x, float y );
	void SetPositionAndAngle( float x, float y, float angle );
	void SetScale( float sx, float sy );
	void SetAngle( float a );
	void SetX( float x );
	void SetY( float y );
	void SetZ( float z );
	void SetScaleX( float sx );
	void SetScaleY( float sy );
	void SetSkewX( float sx );
	void SetSkewY( float sy );
	
	// gets local transform
	float GetX();
	float GetY();
	float GetZ();
	float GetScaleX();
	float GetScaleY();
	float GetSkewX();
	float GetSkewY();
	float GetAngle();

	/// sets world transform
	void SetWorldTransform( float x, float y, float angle, float scaleX, float scaleY );
	void SetWorldPosition( float x, float y );
	void SetWorldPositionAndAngle( float x, float y, float angle );
	void SetWorldScale( float sx, float sy );
	void SetWorldAngle( float a );
	void SetWorldX( float x );
	void SetWorldY( float y );
	void SetWorldScaleX( float sx );
	void SetWorldScaleY( float sy );
	
	/// reset matrix on this object + descendents
	void DirtyTransform();
	
	// get world transform
	b2Vec2 GetWorldPosition();
	b2Vec2 GetWorldScale();
	float GetWorldX();
	float GetWorldY();
	float GetWorldScaleX();
	float GetWorldScaleY();
	float GetWorldAngle();
	
	// convert local to global and back
	void ConvertPoint( float x, float y, float &outX, float &outY, bool localToGlobal, bool screenSpace=true );

	/// returns local bounding box
	GPU_Rect GetBounds();
	
	/// returns updated local transformation matrix
	float* Transform();
	
	/// returns updated world transformation matrix
	float* WorldTransform();
	
	/// returns updated inverse world transformation matrix
	float* InverseWorld();
	
	/// extracts local x, y, angle, and scale from matrix
	void DecomposeTransform( float *te, b2Vec2& pos, float& angle, b2Vec2& scale );
	
	/// inverts source matrix, writes to dest, returns true on success
	static bool MatrixInverse( float *source, float *dest );
	
// hierarchy and lifecycle
	
	/// false, if object is the descendent of scene, true, if part of an orphaned tree
	bool orphan = true;
		
	/// reference to parent object
	GameObject* parent = NULL;
	
	/// children of this object
	GameObjectVector children;
	
	/// sets a new parent for object
	virtual void SetParent( GameObject* newParent, int desiredPosition=-1 );

	/// returns scene this object's in, or NULL if orphaned
	virtual Scene* GetScene();
	
	/// used for serialization
	ArgValueVector* GetChildrenVector();

	/// used for serialization
	ArgValueVector* SetChildrenVector( ArgValueVector* in );
	
	/// garbage collection callback
	void TraceProtectedObjects( vector<void **> &protectedObjects );
	
// render
	
	/// called to render this object, and all children
	virtual void Render( Event& event );
	
	/// object's combined opacity = inherited opacity * own opacity (updated in Render event)
	float combinedOpacity = 1;
	
	/// object's own opacity
	float opacity = 1;

	/// set to true to draw last
	bool renderAfterChildren = false;
		
// behavior
	
	// can access private fields
	friend class RigidBodyBehavior;
	
	/// inactive objects are skipped for rendering, and event dispatches
	bool active(){ return this->_active; } // getter
	
	/// recursively checks active
	bool activeRecursive(){ return this->_active && ( this->parent ? this->parent->activeRecursive() : true ); } // getter
	
    /// inactive objects are skipped for rendering, and event dispatches
	bool active( bool ); // setter
	
	/// contains game object's behaviors
	BehaviorList behaviors;

	/// used for serialization
	ArgValueVector* GetBehaviorsVector();
	
	/// used for serialization
	ArgValueVector* SetBehaviorsVector( ArgValueVector* in );

	/// returns first behavior of class
	template<class BEHAVIOR>
	BEHAVIOR* GetBehavior() {
		for( BehaviorList::iterator i = this->behaviors.begin(), e = this->behaviors.end(); i != e; i++ ){
			Behavior *b = *i;
			if ( script.GetInstance<BEHAVIOR>( b->scriptObject ) == b ) return b;
		}
		return NULL;
	}

	/// returns first behavior of class
	template<class BEHAVIOR>
	void GetBehaviors( bool recurse, vector<BEHAVIOR*> &ret ) {
		for( BehaviorList::iterator i = this->behaviors.begin(), e = this->behaviors.end(); i != e; i++ ){
			Behavior *b = *i;
			if ( script.GetInstance<BEHAVIOR>( b->scriptObject ) == b ) {
				ret.push_back( (BEHAVIOR*) b );
			}
		}
		// recursive
		if ( recurse ) {
			for ( size_t i = 0, nc = this->children.size(); i < nc; i++ ){
				this->children[ i ]->GetBehaviors( true, ret );
			}
		}
	}
	
// events
	
	/// events ignored by this object and descendents
	TypedVector* eventMask = NULL;
	
	typedef function<bool (GameObject*)> GameObjectCallback;
	
	/// calls event handlers on each behavior, then script event listeners on gameObject. Recurses to active children.
	virtual void DispatchEvent( Event& event, bool callOnSelf=false, GameObjectCallback *forEachGameObject=NULL);
	
	/// calls callback on each game object
	bool Traverse( GameObjectCallback *forEachGameObject );

	/// calls handler for event on each behavior, then dispatches script event listeners on this GameObject
	void CallEvent( Event& event );
	
};

#endif /* GameObject_hpp */
