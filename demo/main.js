log( "Привет из main.js!" );

App.setWindowSize( 400, 240, 2 );
App.windowResizable = true;
App.scene.name = "Main scene";

var configurator = include( 'ui/controller-configurator' );
configurator.axis = [
	{
		id: 'horizontal',
		minus: 'Left',
		plus:  'Right',
		description: 'Player movement'
	}, {
		id: 'vertical',
		minus: 'Up',
		plus:  'Down',
	}
];
configurator.buttons = [
	{
		id: 'accept',
		description: 'Jump / Select in menus'
	}, {
		id: 'cancel',
		description: 'Fire / Cancel in menus'
	},
];
configurator.buttonsFirst = 1;
configurator.controllerAdded = function ( controller ) {
	log( controller.name, "axis:", controller.numAxis, "buttons:", controller.numButtons, "hats:", controller.numHats );
	// return controller.name != 'Keyboard';
}
configurator.ready = function ( controller ) {
	var p = function ( a, b ) {
		log ( this, " #", a, ":", b );
	}
	controller.on( 'horizontal', p );
	controller.on( 'vertical', p );
	controller.on( 'accept', p );
	controller.on( 'cancel', p );

}


/* Input.controllerAdded = function ( kb ) {
	if ( kb.name == 'Keyboard' ) {
		kb.bindAxis( '-horizontal', Key.Left );
		kb.bindAxis( '+horizontal', Key.Right );
		kb.bindAxis( '-vertical', Key.Up );
		kb.bindAxis( '+vertical', Key.Down );
		kb.bind( 'accept', Key.Enter );
		kb.bind( 'cancel', Key.Escape );

		kb.save();

	}
};*/

var panel = App.scene.addChild ( 'ui/scrollable', {
	layoutType: Layout.Vertical,
	name: "Main UI",
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

panel.addChild( 'ui/dropdown', {
	items: [
		{ value: "First", text: "Item 1", icon: "tuba" },
		{ value: "Second", text: "Item 2", icon: "tuba" },
		{ value: "Third", text: "Item 3", icon: "tuba" },
		{ value: "Fourth", text: "Item 4", icon: "tuba" },
		{ value: "Fifth", text: "Item 5", icon: "tuba", disabled: true },
	],
	value: "Fourth",
	change: function ( v ) {
		log( "dropdown", v );
	}
} );

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


Input.mouseDown = function ( btn ) {
	if ( btn == 3 ) {

	}
}
