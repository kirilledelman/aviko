log( "Привет из main.js!" );

app.setWindowSize( 800, 480, 2 );
app.windowResizable = true;

input.controllerAdded = function ( kb ) {
	kb.bindAxis( '-horizontal', Key.Left );
	kb.bindAxis( '+horizontal', Key.Right );
	kb.bindAxis( '-vertical', Key.Up );
	kb.bindAxis( '+vertical', Key.Down );
	kb.bind( 'accept', Key.Enter );
	kb.bind( 'cancel', Key.Escape );
};

app.scene = new Scene();

// resized top container
var obj = app.scene.addChild();
obj.render = new RenderShape( Shape.Rectangle );
obj.name = 'Obj1';
obj.x = 10; obj.y = 10;
obj.ui = new UI();
obj.ui.width = 200;
obj.ui.height = 100;
obj.ui.layoutType = Layout.Vertical;
obj.ui.pad = 8;
input.mouseMove = function ( x, y ) {
	
	if ( !app.scene.currentFocus && input.get( Key.MouseButton ) ) {
		this.ui.width = x - this.x;
		this.ui.height = y - this.y;
	}
	
}.bind( obj );
obj.ui.on( 'layout', function ( x, y, w, h ) {
	this.gameObject.x = x;
	this.gameObject.y = y;
    this.gameObject.render.width = w;
    this.gameObject.render.height = h;
});


var label = obj.addChild( 'ui/input');

input.mouseDown = function ( btn, x, y ) {
	if ( btn == 3 ) {

		label = obj.addChild( clone( label ) );

	}
}







