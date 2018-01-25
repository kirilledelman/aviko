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
	kb.save();
};


app.scene = new Scene();

var panel = app.scene.addChild( 'ui/scrollable', {
	x: 10, y: 10,
	width: 100, height: 100,
	layoutType: Layout.Vertical
} );

/*var p = panel.addChild( new GameObject( 'ui/panel' ) );
p.x = p.y = 0;
p.background = 'mom';
p.width = 200;
p.height = 40;
*/

input.mouseMove = function ( x, y ) {

	if ( input.get( Key.MouseButton ) && input.get( Key.LeftShift ) ) {
		var ww = Math.round( x ) - this.x;
		var hh = Math.round( y ) - this.y;
		this.width = ww;
		this.height = hh;
	}

}.bind( panel );


input.keyDown = function ( k ) {

	if ( k == Key.C ) {
		var copy = clone( panel );
		copy.x = input.mouseX;
		copy.y = input.mouseY;
		app.scene.addChild( copy );
	}

}.bind( panel );

panel.addChild( 'ui/text', {
	text: "Hello!\n^2This is a test.",
	align: TextAlign.Center,

} );

for ( var i = 0; i < 2; i++ ) {

	panel.addChild( 'ui/textfield', {
		text: "Приветик! " + i,
		acceptToEdit: true,
		selectAllOnFocus: true,
		numLines: (i == 1 ? 3 : 1),
	} );

}



