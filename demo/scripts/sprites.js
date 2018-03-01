new (function (){

	var scene = new Scene( {
		name: "Sprites",
		backgroundColor: Color.Background
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutAlign: LayoutAlign.Stretch,
		pad: [ 20, 40, 20, 40 ],
		width: App.windowWidth,
		height: App.windowHeight
	} );

	// title
	scene.addChild( 'ui/text', {
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		text: "Sprites"
	} );

	// description
	scene.addChild( 'ui/text', {
		size: 15,
		color: Color.Text,
		align: TextAlign.Left,
		wrap: true,
		text: "Aviko renders sprites using ^B^1RenderSprite^n^c class. Sprites"
	} );

	// container


	// buttons
	var buttons = scene.addChild( 'ui/panel', {
		background: null,
		layoutType: Layout.Horizontal,
		layoutAlign: LayoutAlign.Center,
		spacing: 4
	} );

	buttons.addChild( 'ui/button', {
		text: "?",
		click: function () {

		}
	} );

	scene.addChild( 'ui/button', {
		text: "Back to main menu",
		click: function () {
			App.popScene();
			transitionScene( App.scene, scene, 1 );
			scene = null;
		}
	} );

	buttons.getChild( 0 ).focus();
	return scene;
})();