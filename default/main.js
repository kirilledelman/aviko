log( "Привет из main.js!" );

app.setWindowSize( 800, 480, 2 );

input.controllerAdded = function ( kb ) {
	kb.bindAxis( '-horizontal', KEY_LEFT );
	kb.bindAxis( '+horizontal', KEY_RIGHT );
	kb.bindAxis( '-vertical', KEY_UP );
	kb.bindAxis( '+vertical', KEY_DOWN );
	kb.bind( 'accept', KEY_RETURN );
	kb.bind( 'cancel', KEY_ESCAPE );
};

app.scene = new Scene();
app.scene.debugDraw = true;
app.scene.gravityY = 10;


function f(f) { this.gameObject.render.addColor.set( f == this ? 0x333333 : 0x0 ); }
function m0(){  this.gameObject.z = 10; this.gameObject.render.color.r = 0; }
function m1(){  this.gameObject.z = 0; this.gameObject.render.color.r = 1; }
function m2( wy, wx ){
	
	this.gameObject.render.width += wx;
	this.gameObject.render.height += wy;
	if ( this.gameObject.body ){
		this.gameObject.body.shape.width = this.gameObject.render.width;
		this.gameObject.body.shape.height = this.gameObject.render.height;
	}
	
}

var mouseJoint = null;
function m3( btn, x, y, gx, gy ) {
	
	if ( input.get( KEY_LSHIFT ) && this.gameObject.body && app.scene.currentFocus != this ) {
		
		var rj = this.gameObject.body.addJoint( new Joint( JOINT_WELD, app.scene.currentFocus.gameObject.body ) );
		return;
	}
	
	this.focus();
	
	if ( this.gameObject.body ) {
		
		mouseJoint = new Joint( JOINT_MOUSE, gx, gy );
		mouseJoint.maxForce = 100 * this.gameObject.body.mass;
		mouseJoint.body = this.gameObject.body;
		
	} else {
		
		var b = this.gameObject.body = new Body();
		var s = b.shape;
		s.type = SHAPE_RECTANGLE;
		s.width = this.gameObject.render.width;
		s.height = this.gameObject.render.height;
		
	}
	
}

function m4() {
	if ( mouseJoint ) {
		mouseJoint.destroy();
		mouseJoint = null;		
	}
}




for ( var i = 0; i < 10; i++ ){

	var o = app.scene.addChild();
	o.name = "brick" + i;
	o.render = new RenderSprite( 'mom' );
	o.render.width = 20;
	o.render.height = 10;
	o.render.pivotX = o.render.pivotY = 0;
	o.x = 10 + i * 30;
	o.y = 50;

	o.ui = new UI();
	o.ui.focusChanged = f;
	o.ui.mouseOver = m0;
	o.ui.mouseOut = m1;
	o.ui.mouseWheel = m2;
	o.ui.mouseDown = m3;
	o.ui.mouseUp = o.ui.mouseUpOutside = m4;
	
}

var a = app.scene.addChild();
a.render = new RenderSprite( 'mom' );
a.body = new Body();
a.body.type = BODY_STATIC;
a.body.shape.type = SHAPE_RECTANGLE;
a.render.centered = false;
a.z = -1;
a.render.color.set( 0.5, 1, 1 );
a.filled = true;
a.render.width = a.body.shape.width = app.windowWidth / 2;
a.render.height = a.body.shape.height = 20;
a.x = 0; a.y = app.windowHeight / 2 - a.body.shape.height;
a.name = "Ground";
global.ground = a;

input.on( 'mouseMove', function (  x, y ) {
		 if ( mouseJoint ) {
			 mouseJoint.setMouseTarget( x, y );
		 }
});

input.keyDown = function ( k ) {

	if ( app.scene.currentFocus ) {
		var b = app.scene.currentFocus.gameObject.body;
		if ( b && b.joint && b.joint.type == JOINT_REVOLUTE ) {
			//b.joint.pinMotor = !b.joint.pinMotor;
			//log ( stringify( b.joint ) );
		}
	}
};
