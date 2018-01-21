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

var panel = app.scene.addChild( 'ui/scrollable' );
panel.x = 10; panel.y = 10;
panel.width = 200; panel.height = 200;
panel.layoutType = Layout.Vertical;


/*input.mouseMove = function ( x, y ) {

	if ( !app.scene.currentFocus && input.get( Key.MouseButton ) ) {
		var ww = Math.round(x) - this.x;
		var hh = Math.round(y) - this.y;
		this.width = ww;
		this.height = hh;
	}

}.bind( panel );*/


var label;
for ( var i = 0; i < 16; i++ ) {

	label = panel.addChild( 'ui/input' );
	label.name = label.text = label.lineHeight + " : " + i;
	label.acceptToEdit = true;
	label.selectAllOnFocus = true;
	if ( i == 2 ) { label.disabled = true; }
	label.accept = function ( txt ) {
		var a = parseFloat( txt );
		this.size = a ? Math.max( 10, a ) : this.size;
	}

}



