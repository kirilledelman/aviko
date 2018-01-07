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
input.mouseMove = function ( x, y ) {
	
	if ( input.get( Key.MouseButton ) ) {
		this.x = x * 0.1;
		this.y = y * 0.1;
		this.ui.width = x - this.x;
		this.ui.height = y - this.y;
		this.dispatch( 'layout' );
	}
	
}.bind( obj );
obj.ui.on( 'layout', function ( x, y, w, h ) {
	this.gameObject.x = x;
	this.gameObject.y = y;
    this.gameObject.render.width = w;
    this.gameObject.render.height = h;
});

// anchored child 1
var objA = obj.addChild();
objA.render = new RenderShape( Shape.Rectangle );
objA.name = 'objA';
objA.ui = new UI();
objA.ui.anchorLeft = 0;
objA.ui.anchorRight = 0.5;
objA.ui.anchorTop = 0;
objA.ui.anchorBottom = 0;
objA.ui.left = 25;
objA.ui.right = -5;
objA.ui.top = 25;
objA.ui.bottom = 25;
objA.ui.on( 'layout', function ( x, y, w, h ) {
	this.gameObject.x = x;
	this.gameObject.y = y;
	this.gameObject.render.width = w;
	this.gameObject.render.height = h;
		   // log( this.gameObject.name, x, y, w, h );
});

objA.ui.layoutType = Layout.Vertical;
objA.ui.pad = 4;

// subchilds
for ( var i = 0; i < 10; i++ ) {
	var objC = objA.addChild();
	objC.render = new RenderShape( Shape.Rectangle );
	objC.render.filled = true;
	objC.render.color = Math.floor( Math.random() * 0xFFFFFF );
	objC.name = 'objC' + i;
	objC.ui = new UI();
	objC.ui.margin = 2;
	objC.ui.width = 10 + Math.random() * 5;
	objC.ui.height = 10 + Math.random() * 5;
	objC.ui.on( 'layout', function ( x, y, w, h ) {
			   this.gameObject.x = x;
			   this.gameObject.y = y;
			   this.gameObject.render.width = w;
			   this.gameObject.render.height = h;
			   });
}


// anchored child 2
var objB = obj.addChild();
objB.render = new RenderShape( Shape.Rectangle );
objB.name = 'objB';
objB.ui = new UI();
objB.ui.anchorLeft = 0.5;
objB.ui.anchorRight = 0;
objB.ui.anchorTop = 0;
objB.ui.anchorBottom = 0;
objB.ui.left = 5;
objB.ui.right = 25;
objB.ui.top = 25;
objB.ui.bottom = 25;
objB.ui.padding = 4;
objB.ui.on( 'layout', function ( x, y, w, h ) {
	this.gameObject.x = x;
	this.gameObject.y = y;
	this.gameObject.render.width = w;
	this.gameObject.render.height = h;
		   // log( this.gameObject.name, x, y, w, h );
});

objB.ui.layoutType = Layout.Grid;

// subchilds
for ( var i = 0; i < 20; i++ ) {
	var objC = objB.addChild();
	objC.render = new RenderShape( Shape.Rectangle );
	objC.render.filled = true;
	objC.render.color = Math.floor( Math.random() * 0xFFFFFF );
	objC.name = 'objC' + i;
	objC.ui = new UI();
	objC.ui.margin = 4;
	objC.ui.width = 20 + Math.random() * 10;
	objC.ui.height = 20 + Math.random() * 10;
	objC.ui.on( 'layout', function ( x, y, w, h ) {
			   this.gameObject.x = x;
			   this.gameObject.y = y;
			   this.gameObject.render.width = w;
			   this.gameObject.render.height = h;
			   });
}
 

