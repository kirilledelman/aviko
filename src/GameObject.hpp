#ifndef GameObject_hpp
#define GameObject_hpp

#include "common.h"
#include "ScriptableClass.hpp"
#include "Behavior.hpp"
#include "RenderBehavior.hpp"
#include "UIBehavior.hpp"
#include "RigidBodyBehavior.hpp"

typedef vector<GameObject*> GameObjectVector;
typedef vector<GameObject*>::iterator GameObjectIterator;

typedef vector<Behavior*> BehaviorVector;
typedef vector<Behavior*>::iterator BehaviorIterator;

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
	RigidBodyBehavior* body = NULL;
	
	/// shortcut to rendering behavior
	RenderBehavior* render = NULL;
	
	/// shortcut to ui behavior
	UIBehavior* ui = NULL;
	
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
	
	// gets local transform
	b2Vec2 GetPosition();
	b2Vec2 GetScale();
	float GetX();
	float GetY();
	float GetZ();
	float GetScaleX();
	float GetScaleY();
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
	
	// get world transform
	b2Vec2 GetWorldPosition();
	b2Vec2 GetWorldScale();
	float GetWorldX();
	float GetWorldY();
	float GetWorldScaleX();
	float GetWorldScaleY();
	float GetWorldAngle();
	
	// convert local to global and back
	void ConvertPoint( float x, float y, float &outX, float &outY, bool localToGlobal );

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
	
// render
	
	/// called to render this object, and all children
	virtual void Render( Event& event );
	
	/// object's combined opacity = inherited opacity * own opacity (updated in Render event)
	float combinedOpacity = 1;
	
	/// object's own opacity
	float opacity = 1;
	
// behavior
	
	// can access private fields
	friend class RigidBodyBehavior;
	
	/// inactive objects are skipped for rendering, and event dispatches
	bool active(){ return this->_active; } // getter
	
    /// inactive objects are skipped for rendering, and event dispatches
	bool active( bool ); // setter
	
	/// contains game object's behaviors
	BehaviorVector behaviors;

	/// used for serialization
	ArgValueVector* GetBehaviorsVector();
	
	/// used for serialization
	ArgValueVector* SetBehaviorsVector( ArgValueVector* in );

	/// returns first behavior of class
	template<class BEHAVIOR>
	BEHAVIOR* GetBehavior() {
		for( BehaviorVector::size_type i = 0, nb = this->behaviors.size(); i < nb; i++ ){
			Behavior *b = this->behaviors[ i ];
			if ( static_cast<BEHAVIOR*>( b ) != nullptr ) return b;
		}
		return NULL;
	}

// events
	
	typedef function<void (GameObject*)> GameObjectCallback;
	
	/// calls event handlers on each behavior, then script event listeners on gameObject. Recurses to active children.
	void DispatchEvent( Event& event, bool callOnSelf=false, GameObjectCallback *forEachGameObject=NULL);
	
	/// calls handler for event on each behavior, then dispatches script event listeners on this GameObject
	void CallEvent( Event& event );
	
};

#endif /* GameObject_hpp */
