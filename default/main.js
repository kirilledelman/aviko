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

function m0(){  this.gameObject.render.color.r = 0; }
function m1(){  this.gameObject.render.color.r = 1; }
function m2( wy ){  this.gameObject.angle += wy; }
function m3() {
	var t = new Tween
	( this.gameObject,
	 ['x','y'],
	 null,
	 [ this.gameObject.x + (50 * Math.random() - 25),
	  this.gameObject.y + (50 * Math.random() - 25) ] );
	
	t.finished = function () {
		log( "Finished!");
		app.async( function(){ gc(); } );
	}
}


for ( var i = 0; i < 10; i++ ){

	var o = new GameObject();
	o.render = new RenderSprite( '123/4' );
	o.x = 10 + i * 30;
	o.y = 50;
	o.ui = new UI();
	o.ui.mouseOver = m0;
	o.ui.mouseOut = m1;
	o.ui.mouseWheel = m2;
	o.ui.click = m3;
	
	app.scene.addChild( o );

}


