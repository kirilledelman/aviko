new (function (){

	var scene = new Scene( {
		name: "Main Menu",
		backgroundColor: Color.Background
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutAlign: LayoutAlign.Stretch,
		pad: [ 10, 20, 10, 20 ],
		width: App.windowWidth,
		height: App.windowHeight
	} );

	// title
	scene.addChild( 'ui/text', {
		size: 40,
		color: Color.Title,
		align: TextAlign.Center,
		marginBottom: 20,
		text: "Aviko Demo"
	} );

	// description
	scene.addChild( 'ui/text', {
		size: 15,
		color: Color.Text,
		align: TextAlign.Center,
		wrap: true,
		text: "Aviko is a simple 2D game engine, geared towards learning and rapid prototyping. This demo " +
		"shows some of Aviko's capabilities, and demo's source code can be used to learn how to accomplish common tasks."
	} );

	// buttons container
	var buttons = scene.addChild( 'ui/panel', {
		background: null,
		layoutType: Layout.Grid,
		layoutAlign: LayoutAlign.Center,
		spacing: 4,
		flex: 1
	} );

	// button click handler
	var buttonClick = function () {
		var sub = transitionScene( include( this.src ), scene, -1 );
		App.pushScene( sub );
	};

	buttons.addChild( 'ui/button', {
		text: "Sprites",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Shapes",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Text",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Input",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Transforms",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Physics",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Transforms",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "UI",
		src: 'sprites',
		click: buttonClick
	} );

	buttons.addChild( 'ui/button', {
		text: "Serialization",
		src: 'sprites',
		click: buttonClick
	} );

	// put
	scene.addChild( 'ui/button', {
		text: "Exit",
		click: function () {
			quit();
		}
	} );

	// focus on first button
	buttons.getChild( 0 ).focus();
	return scene;
})();