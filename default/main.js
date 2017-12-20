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

var b = new GameObject();
b.render = new RenderShape( SHAPE_RECTANGLE );
b.render.x = 100;
b.render.y = 80;
b.render.filled = false;
b.x = b.y = 120;
app.scene.addChild ( b );


var sel = null;

function m0(){  this.gameObject.render.color.r = 0; }
function m1(){  this.gameObject.render.color.r = 1; }
function m2( wy ){  this.gameObject.angle += wy; }
function m3( btn ) {
	
	sel = this.gameObject;
	
}


for ( var i = 0; i < 10; i++ ){

	var o = new GameObject();
	o.render = new RenderSprite( '123/4' );
	o.ox = o.x = 10 + i * 30;
	o.oy = o.y = 50;
	o.ui = new UI();
	o.ui.mouseOver = m0;
	o.ui.mouseOut = m1;
	o.ui.mouseWheel = m2;
	o.ui.click = m3;
	
	app.scene.addChild( o );

	sel = o;
}

sel.moveTo( 0, 0 );


input.mouseUp = function( btn, x, y ){

	if ( sel ) sel.moveTo( x, y );
	
}
