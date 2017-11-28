log( "Привет из main.js!" );

app.setWindowSize( 800, 480, 2 );

app.scene = new Scene();

var go = new GameObject();
go.render = new RenderSprite( "mom" );
go.x = 100; go.y = 100;

var ui = go.ui = new UI();

app.scene.addChild( go );

ui.mouseOver = function(){
	  go.render.addColor.set( 0x333333 );
};

ui.mouseOut = function(){
		 go.render.addColor.set( 0x0 );
};

ui.click = function(){
	log( "CLICK" );
	ui.focus();
};

ui.mouseUpOutside = function(){
	   log( "up outside" );
	ui.blur();
};

ui.mouseWheel = function ( y, x ){
	go.y += y;
	go.x += x;
};

ui.keyDown = ui.keyUp = ui.keyPress = function () {
	log( Array.prototype.slice.apply( arguments ) );
};


