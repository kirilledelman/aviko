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

var scrollView = app.scene.addChild( new GameObject() );
scrollView.render = new RenderSprite();
scrollView.render.image = new Image( 100, 150 );
scrollView.x = 100; scrollView.y = 25;
scrollView.ui = new UI();
scrollView.ui.focusable = false;
var container = scrollView.addChild( new GameObject() );
container.x = 25;
scrollView.render.image.autoDraw = container;


var go;
for ( var yy = 20; yy < 71; yy += 50 ) {
	
	//if ( Math.random() > 0.8 ) continue;
	
	go = container.addChild( new GameObject() );
	go.name = '(' + yy + ')';
	go.render = new RenderSprite( "btn/btn" );
	go.render.sliceLeft = go.render.sliceRight = 0.25;
	go.render.width = 80;
	go.render.height = 40;
	go.ui = new UI();
	go.x = 0; go.y = yy;

	go.ui.mouseOver = function(){ this.gameObject.render.addColor.set( 0x333333 ); };
	go.ui.mouseOut = function(){ this.gameObject.render.addColor.set( 0x0 ); };
	go.ui.click = function(){
		log( "CLICK on ", this.gameObject );
		scrollView.x = 20 + 300 * Math.random();
	};
	go.ui.mouseDown = function(){
		log( "mouseDown" );
		this.focus();
	};
	go.ui.keyPress = function () {
		log( this.gameObject, "keyPress:", Array.prototype.slice.apply( arguments ) );
	};
	go.ui.focusChanged = function ( f ) {
		if ( f == this ) {
			log( "new focus = ", f );
			this.gameObject.render.color.set( 0xFFFF66 );
		} else {
			this.gameObject.render.color.set( 0xFFFFFF );
		}
	}
	go.ui.navigation = function () {
		log( this.gameObject, "navigation:", Array.prototype.slice.apply( arguments ) );
	}
	
	go.ui.mouseWheel = function( w, x ){
		container.y += w;
		this.gameObject.x += x;
	}
	

	
}




