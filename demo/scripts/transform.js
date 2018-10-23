new (function (){

	// create scene
	var scene = new Scene( {
		name: "Transform",
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
		text: "Sprites",
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
		formatting: true,
		canScrollUnfocused: true,
		text:
		"Aviko organizes ^B^1GameObject^n^c class has severa.\n\n"
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

	// head
	var sprite = spriteContainer.addChild( {
		render: new RenderSprite( 'smiley.png', { pivot: 0.5 } ),
		x: 150, y: 70,
		name: "Smiley",
		__propertyListConfig: { properties: { parent: false } }, // hide parent field in inspector
		children: [
			new GameObject( {
				render: new RenderSprite( 'poop.png', { pivot: 0.5 } ),
				x: -25, y: -33, scale: 0.25,
				name: "Left Eye",
				__propertyListConfig: { properties: { parent: { disabled: true }, children: false, } }, // disable parent field in inspector
			}),
			new GameObject( {
				render: new RenderSprite( 'clown.png', { pivot: 0.5 } ),
				x: 18, y: -23, scale: 0.25,
				name: "Right Eye",
				__propertyListConfig: { properties: { parent: { disabled: true }, children: false, } }, // disable parent field in inspector
			})
		]
	} );

	// properties
	var props = rightColumn.addChild( 'ui/property-list', {
		flex: 1,
		minWidth: 300,
		valueWidth: 130,
		showAll: false,
		// showBackButton: false,
		showContextMenu: false,
		showMoreButton: false,
		target: sprite,
	} );

	// overrides
	props.properties = {
		scene: false,
		eventMask: false,
		script: false,
		ui: false,
		body: false,
		ignoreCamera: false,
		opacity: false,
		render: false,
		renderAfterChildren: false,
	};
	props.groups = [
		{ name: '', properties: 'active,name' },
		{ name: 'Hierarchy', properties: 'parent,children' },
		{ name: 'Transform', properties: 'x,y,z,scaleX,scaleY,angle,skewX,skewY' },
	]

	backButton.focus();
	return scene;
})();