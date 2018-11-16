new (function (){

	// create scene
	var scene = new Scene( {
		name: "Animation",
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
		text: "Animation",
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
		"^B^1Tween^b^c class helps interpolate one or more property of any object over time.\n\n" +
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
	var spriteContainer = rightColumn.addChild( 'ui/scrollable', {
		minHeight: 150,
		layoutType: Layout.None,
		scrollbars: false,
		layout: function () {
			this.scrollWidth = this.width;
			this.scrollHeight = this.height;
		},
	} );

	// background for sprite
	spriteContainer.addChild( 'ui/image', {
		width: 300,
		height: 150,
		mode: 'fit',
		texture: 'checker.png',
	} );

	// sprite
	var sprite = spriteContainer.addChild( {
		render: new RenderSprite( 'smiley.png' ),
		x: 150, y: 70,
	} );
	sprite.render.pivot = 0.5;
	
	backButton.focus();
	return scene;
})();