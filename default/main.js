log( "Привет из main.js!" );

app.setWindowSize( 800, 480, 2 );

app.scene = new Scene();

var go = app.scene.addChild( new GameObject() );
go.render = new RenderSprite( "btn/btn" );
go.render.sliceLeft = go.render.sliceRight = 0.25;
go.render.width = 100;
go.ui = new UI();
go.x = 100; go.y = 100;

go.ui.mouseOver = function(){ go.render.addColor.set( 0x333333 ); };

go.ui.mouseOut = function(){ go.render.addColor.set( 0x0 ); };

go.ui.click = function(){
	ui.focus();
	log( "focused on ", go, this );
};

go.ui.keyPress = function () {
	log( go, "keyPress:", Array.prototype.slice.apply( arguments ) );
};


