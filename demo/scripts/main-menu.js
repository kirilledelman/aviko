new (function (){

	var scene = new Scene( {
		name: "Main Menu",
		backgroundColor: Color.Background,
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		pad: 20,
		fitChildren: false,
		width: App.windowWidth,
		height: App.windowHeight
	} );

	// title
	scene.addChild( 'ui/text', {
		size: 40,
		color: Color.Title,
		align: TextAlign.Center,
		marginBottom: 20,
		text: "Aviko Demo",
		bold: true,
		name: "Title",
	} );

	// description
	scene.addChild( 'ui/text', {
		size: 16,
		color: Color.Text,
		align: TextAlign.Center,
		wrap: true,
		marginBottom: 20,
		text: "Aviko is a simple 2D game engine, geared towards learning and rapid prototyping. This demo " +
		"shows some of Aviko's capabilities, and demo's source code can be used to learn how to accomplish common tasks.",
		name: "Intro",
	} );

	// buttons container
	var buttons = scene.addChild( 'ui/panel', {
		name: "Buttons",
		backgroundColor: new Color( 'F0F0F066' ),
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		wrapEnabled: true,
		spacing: 8,
		flex: 1,
		minHeight: 40,
	} );

	// button click handler
	var buttonClick = function () {
		// block all buttons while scene is loading
		buttons.eventMask = [ 'mouseDown', 'click', 'mouseUp', 'keyDown', 'keyUp', 'keyPress', 'navigate' ];
		// load scene and animate transition
		sceneForward( include( this.src ) );
		// reenable buttons
		this.async( function () { buttons.eventMask = []; }, 0.25 );
	};

	buttons.addChild( 'ui/button', {
		text: "Sprites",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Shapes",
		src: 'shapes',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Text",
		src: 'text',
		disabled: false,
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Input",
		src: 'input',
		disabled: false,
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Transform",
		src: 'transform',
		disabled: false,
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Physics",
		src: 'physics',
		disabled: true,
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "UI",
		src: 'ui',
		disabled: false,
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Serialization",
		src: 'serialization',
		disabled: true,
		click: buttonClick
	} );

	scene.addChild( 'ui/button', {
		text: "Exit",
		click: function () {
			quit();
		}
	} );
	
	// focus on first button
	buttons.getChild( 0 ).focus(); //*/
	return scene;
})();
