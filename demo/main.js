log( "Привет из main.js!" );

App.setWindowSize( 800, 480, 2 );
App.windowResizable = true;

Input.controllerAdded = function ( kb ) {
	kb.bindAxis( '-horizontal', Key.Left );
	kb.bindAxis( '+horizontal', Key.Right );
	kb.bindAxis( '-vertical', Key.Up );
	kb.bindAxis( '+vertical', Key.Down );
	kb.bind( 'accept', Key.Enter );
	kb.bind( 'cancel', Key.Escape );
	kb.save();
};

App.scene = new Scene();

/*var panel = App.scene.addChild ( 'ui/scrollable', {
	layoutType: Layout.Vertical,
	x: 10, y: 10,
	width: 150, height: 100,
	layoutAlign: LayoutAlign.Stretch
});

Input.mouseMove = function ( x, y ) {

	if ( Input.get( Key.MouseButton ) && Input.get( Key.LeftShift ) ) {
		var ww = Math.round( x ) - this.x;
		var hh = Math.round( y ) - this.y;
		this.width = ww;
		this.height = hh;
	}

}.bind( panel );


var grp = [];
grp.push( panel.addChild( 'ui/checkbox', {
	text: 'Option 1',
	group: grp,
	change: function ( v ) {
		if ( v ) log( "option 1" );
	}
} ) );

grp.push( panel.addChild( 'ui/checkbox', {
	text: 'Option 2',
	group: grp,
	change: function ( v ) {
		if ( v ) log( "option 2" );
	}
} ) );

panel.addChild( 'ui/button', {
	text: 'Tuba',
	icon: 'tuba',
	click: function () {
		log( "hello" );
	}
} );


panel.addChild( 'ui/text', {
	// text: "^BTitle\n^bSubtitle\nSub-Subtitle",
	text: "Label",
	align: TextAlign.Left
} );

panel.addChild( 'ui/textfield', {
	text: "Input 1",
	width: 90,
	flex: 1
} );

panel.addChild( 'ui/text', {
	text: "^BTitle\n^bSubtitle\nSub-Subtitle",
	align: TextAlign.Left
} );

panel.addChild( 'ui/image', {
	texture: 'tuba',
} );

panel.addChild( 'ui/textfield', {
	value: 0,
	numeric: true,
	min: 0,
	max: 1,
	step: 0.1,
	change: function ( v ) { log( v ); },
	width: 60
} );
*/

var t = App.scene.addChild();
t.x = 100; t.y = 150;
var r = new RenderShape();
r.filled = false;
r.shape = Shape.Chain;
r.lineThickness = 8;
r.color = 0xFFFF00;
r.points = [ 0,0, 150,0, 150,100, 0,100 ]; // sigma
t.render = r;

Input.mouseMove = function ( x, y ) {

	if ( Input.get( Key.LeftShift ) ) {
		var ww = Math.round( x ) - this.x;
		var hh = Math.round( y ) - this.y;

		r.points[ 2 ] = ww;
		r.points[ 3 ] = hh;
	}

}.bind( t );

Input.keyDown = function ( k ) {
	
	if ( k == Key.Space ) {
		r.shape = (r.shape == Shape.Chain ? Shape.Polygon : Shape.Chain);
		log( r.shape );
	}
}





