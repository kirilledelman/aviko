new (function (){

	// create scene
	var scene = new Scene( {
		name: "Text",
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
		text: "RenderText",
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
		text:
		"Aviko renders text using ^B^1RenderText^n^c class.\n\n" +
		""

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
	var textContainer = rightColumn.addChild( 'ui/panel', {
		minHeight: 150,
		background: 0xA0A0A0,
		layoutType: Layout.None,
	} );

	// background for sprite
	var text = textContainer.addChild( 'ui/text', {
		width: 280,
		maxWidth: 280,
		x: 10,
		autoSize: true,
		multiLine: true,
		text: "Sample",
	} );
	
	// properties
	var props = rightColumn.addChild( 'ui/property-list', {
		flex: 1,
		minWidth: 300,
		valueWidth: 130,
		showAll: true,
		showBackButton: false,
		showContextMenu: false,
		showMoreButton: false,
		target: text.renderText,
	} );

	// overrides
	/*props.properties = {
		'texture': { enum: [
			{ text: "smiley.png", value: "/textures/smiley.png" },
			{ text: "clown.png", value: "/textures/clown.png" },
			{ text: "poop.png", value: "/textures/poop.png" },
		]},
		image: false,
		active: false,
	};*/

	backButton.focus();
	return scene;
})();