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

var panel = app.scene.addChild( 'ui/panel' );
panel.x = 10; panel.y = 10;
panel.width = 200; panel.height = 100;
panel.layoutType = Layout.Vertical;
panel.fitChildren = true;
panel.pad = 8;

input.mouseMove = function ( x, y ) {
	
	if ( !app.scene.currentFocus && input.get( Key.MouseButton ) ) {
		this.width = x - this.x;
		this.height = y - this.y;
	}
	
}.bind( panel );


var label;
for ( var i = 0; i < 2; i++ ) {

	label = panel.addChild( 'ui/input' );
	label.text = "Input " + i;
	label.accept = function ( txt ) {
		this.size = parseFloat( txt );
		this.parent.dispatch( 'layout' );
	}

}





