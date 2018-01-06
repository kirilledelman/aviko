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

var obj = app.scene.addChild();
obj.render = new RenderShape( Shape.Rectangle );
obj.name = 'Obj1';
obj.x = 10; obj.y = 10;
obj.ui = new UI();
input.mouseMove = function ( x, y ) {
	
	this.ui.width = x - this.x;
	this.ui.height = y - this.y;
	this.dispatch( 'layout' );
	
}.bind( obj );

obj.ui.on( 'layout', function ( x, y, w, h ) {
    this.gameObject.render.x = w;
    this.gameObject.render.y = h;
});

obj = obj.addChild();
obj.render = new RenderShape( Shape.Rectangle );
obj.name = 'Obj2';
obj.ui = new UI();
obj.ui.anchorLeft = 0;
obj.ui.anchorRight = 0.5;
obj.ui.anchorTop = 0;
obj.ui.anchorBottom = 0;
obj.ui.left = 25;
obj.ui.right = -5;
obj.ui.top = 25;
obj.ui.bottom = 25;
obj.ui.maxWidth = 100;
obj.ui.on( 'layout', function ( x, y, w, h ) {
    this.gameObject.render.x = w;
    this.gameObject.render.y = h;
});

obj = obj.parent.addChild();
obj.render = new RenderShape( Shape.Rectangle );
obj.name = 'Obj3';
obj.ui = new UI();
obj.ui.anchorLeft = 0.5;
obj.ui.anchorRight = -1;
obj.ui.anchorTop = 0;
obj.ui.anchorBottom = 0;
obj.ui.left = 5;
obj.ui.right = 25;
obj.ui.top = 25;
obj.ui.bottom = 25;
obj.ui.width = 25; obj.ui.height = 25;
obj.ui.maxHeight = 100;
obj.ui.on( 'layout', function ( x, y, w, h ) {
    this.gameObject.render.x = w;
    this.gameObject.render.y = h;
		  });

