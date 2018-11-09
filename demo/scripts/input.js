new (function (){

	// create scene
	var scene = new Scene( {
		name: "Input",
		backgroundColor: Color.Background,
		cameraX: App.windowWidth
	} );

	scene.ui = new UI( {
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Stretch,
		pad: 20,
		wrapEnabled: true,
		fitChildren: false,
		spacing: 10,
		width: App.windowWidth,
		height: App.windowHeight,
	} );

	var leftColumn = scene.addChild( 'ui/panel', {
		flex: 1,
		pad: 0,
		minWidth: 150,
		spacing: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	// title
	var title = leftColumn.addChild( 'ui/text', {
		name: "Title",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		text: "Input",
	} );

	// scrollable description
	var description = leftColumn.addChild( 'ui/textfield', {
		size: 14,
		disabled: true,
		bold: false,
		states:{
			off: { background: false, },
			scroll: { background: 0xF0F0F0 },
			focus: { background: false },
			disabled: { background: false },
		},
		cornerRadius: 2,
		cancelToBlur: false,
		pad: 5,
		color: Color.Text,
		wrap: true,
		flex: 1,
		multiLine: true,
		focusRect: true,
		canScrollUnfocused: true,
		formatting: true,
		text:
		"^B^1Input^b^c is a global singleton class that provides properties and methods for reading and reacting to input events from " +
		"keyboard, mouse, and joysticks / gamepads.\n\n" +
		"Keys and buttons can be read directly, or ^B^1Controller^b^c class can be used to invoke event handlers when a change is detected. " +
		"Controllers also let you save and load axis / buttons mappings to a JSON file. Aviko includes ^Bcontroller-configurator.js^b helper script " +
		"for prompting player to map controller buttons to game actions.\n\n"
	} );

	// back to main menu button
	var backButton = leftColumn.addChild( 'ui/button', {
		text: "Back to main menu",
		click: function () {
			sceneBack();
		}
	} );

	var rightColumn = scene.addChild( 'ui/panel', {
		pad: 0,
		minWidth: 300,
		selfAlign: LayoutAlign.Stretch,
		spacingY: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	// sample sprite container
	var textContainer = rightColumn.addChild( 'ui/scrollable', {
		flex: 1,
		minWidth: 300,
		layoutType: Layout.Vertical,
	} );
	
	//
	textContainer.addChild( 'ui/text', {
		align: TextAlign.Left,
		text: "Input state:",
		size: 14, bold: true,
		color: Color.Title,
		autoSize: true,
	} );
	
	var inputState = textContainer.addChild( 'ui/text', {
		align: TextAlign.Left,
		text: "Hello",
		size: 12,
		multiline: true,
		color: Color.Text,
		wrap: false,
		autoSize: true,
	} );

	scene.currentKeys = [];
	
	// update text
	scene.update = function () {
		
		var txt =
			"^BMouse X, Y:^b\t\t\t" + Input.mouseX + ', ' + Input.mouseY + "\n" +
			"^BMouse Left:^b\t\t\t" + Input.mouseLeft.toString() + "\n" +
			"^BMouse Middle:^b\t\t" + Input.mouseMiddle.toString() + "\n" +
			"^BMouse Right:^b\t\t" + Input.mouseRight.toString() + "\n\n" +
			"^BKeys Down:^b\t[ " + scene.currentKeys.join( ', ' ) + " ]\n\n" +
			"^BNum Joysticks:^b\t" + Input.numJoysticks + "\n\n";
		
		for ( var i = 0; i <= Input.numJoysticks; i++ ) {
			var cont = Input.controllers[ i ];
			if ( cont.name == 'Keyboard' ) continue;
			txt += "^BController Name:^b\t\t" + cont.name + "\n";
			txt += "^BButtons, Axis, Hats:^b\t" + cont.numButtons + " / " + cont.numAxis + " / " + cont.numHats + "\n";
			var btns = [];
			for ( var j = 0; j < cont.numButtons; j++ ) {
				if ( Input.get( Key.JoyButton, j, cont.id ) ) btns.push( "Button " + j );
			}
			txt += "^BButtons Down:^b\t[ " + btns.join( ', ' ) + " ]\n";
			for ( var j = 0; j < cont.numAxis; j++ ) {
				txt += "\t^BAxis " + j + ":\t" + Input.get( Key.JoyAxis, j, cont.id ).toFixed( 4 ) + '\n';
			}
		}
			
		inputState.text = txt;
		
	}
	
	// handlers
	scene.keyDown = function ( k, s, a, c, m, repeat ) {
		if ( !repeat ) scene.currentKeys.push( Input.keyName( k ) );
	}
	
	scene.keyUp = function ( k ) {
		var name = Input.keyName( k );
		var index = scene.currentKeys.indexOf( name );
		if ( index >= 0 ) scene.currentKeys.splice( index, 1 );
	}

	
	Input.on( 'keyDown', scene.keyDown );
	Input.on( 'keyUp', scene.keyUp );
	
	scene.removed = function () {
		Input.off( 'keyDown', scene.keyDown );
		Input.off( 'keyUp', scene.keyUp );
	}
	
	backButton.focus();
	return scene;
})();