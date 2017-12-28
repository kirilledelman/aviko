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



function m0(){  this.gameObject.z = 10; this.gameObject.render.color.r = 0; }
function m1(){  this.gameObject.z = 0; this.gameObject.render.color.r = 1; }
function m2( wy, wx ){
	
	if ( input.get( KEY_LSHIFT ) ){
		
		this.gameObject.angle += wy;
		
	} else {
		this.gameObject.render.width += wx;
		this.gameObject.render.height += wy;
		if ( this.gameObject.body ){
			this.gameObject.body.shape.width = this.gameObject.render.width;
			this.gameObject.body.shape.height = this.gameObject.render.height;
		}
	}
	
}

function m3( btn ) {
	
	if ( this.gameObject.body ) {
		
		if ( input.get( KEY_LALT ) ) {
			
			this.gameObject.body.angularVelocity = 20;
			
		} else if ( input.get( KEY_LSHIFT ) || input.get( KEY_RSHIFT ) ) {
			
			this.gameObject.body.velocityX = 100;
			
			
		} else {
			var c = toInit( this.gameObject );
			c.y -= 100;
			app.scene.addChild( init( c ) );
		}
		
	} else {
		var b = this.gameObject.body = new Body();
		var s = b.shape;
		s.type = SHAPE_RECTANGLE;
		s.width = this.gameObject.render.width;
		s.height = this.gameObject.render.height;
		
		b.touch = function ( a, b, x, y, nx, ny, s ) {
			
			log( "touch", a.body.gameObject.name, b.body.gameObject.name );
			
			var tmp = app.scene.addChild();
			tmp.render = new RenderShape( SHAPE_LINE );
			tmp.render.x = nx * 10;
			tmp.render.y = ny * 10;
			tmp.render.color.set( 1, 0, 0 );
			tmp.x = x;
			tmp.y = y;
			tmp.z = -1;
			var t = new Tween( tmp, 'opacity', 1, 0, 2 );
			t.finished = function() { tmp.parent = null; }
			
			t = new Tween( this.gameObject.render.addColor, 'r', 1, 0 );
			
		}
		
		b.untouch = function ( a, b ) {
			
			log( "untouch", a.body.gameObject.name, b.body.gameObject.name );
			
		}
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
	// o.angle = 90 * Math.random();
	o.ui = new UI();
	o.ui.mouseOver = m0;
	o.ui.mouseOut = m1;
	o.ui.mouseWheel = m2;
	o.ui.click = m3;
	
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

a = app.scene.addChild();
a.body = new Body();
a.body.type = BODY_STATIC;
a.body.shape.sensor = true;
a.body.shape.type = SHAPE_RECTANGLE;
a.body.shape.width = app.windowWidth / 2;
a.body.shape.height = 20;
a.x = 0; a.y = app.windowHeight / 2 - a.body.shape.height - 100;
a.name = "Sensor";

input.keyDown = function () {
}
