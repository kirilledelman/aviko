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

var scrollable = app.scene.addChild( 'ui/scrollable', {
	x: 50, y: 20,
	width: 150, height: 200
} );

input.mouseMove = function ( x, y ) {

	if ( input.get( Key.MouseButton ) && input.get( Key.LeftShift ) ) {
		var ww = Math.round( x ) - this.x;
		var hh = Math.round( y ) - this.y;
		this.width = ww;
		this.height = hh;
	}

}.bind( scrollable );

var panel = scrollable.addChild ( 'ui/panel', {
	layoutType: Layout.Vertical,// background: null,
	pad: 8, width: 150
});

panel.addChild( new GameObject( 'ui/text', { text: "^BTitle\n^bSubtitle", align: TextAlign.Center } ) );

for ( var i = 0; i < 1; i++ ) {

	var f = new GameObject( 'ui/textfield', {
		text: "Input " + i
	} );
	panel.addChild( f );

}

panel.addChild( new GameObject( 'ui/image', {
	texture: 'mom'
} ) );


