new (function (){

	// create scene
	var scene = new Scene( {
		name: "Sprites",
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
		"Aviko renders sprites using ^B^1RenderSprite^n^c class.\n\n" +
		"It can display images loaded from ^Bpng^n, and ^Bjpg^n files, as " +
		"well as dynamic textures, using ^B^1Image^n^c class.\n\n" +
		"Sprite sheets (texture atlases) in ^BJSON^b format can be used to " +
		"combine multiple small sprites into one texture to increase performance.\n\n" +
		"Various blending modes, colored outline, multiplicative and additive color tinting are supported, as well as opacity, and stippling.\n\n" +
		"Sprite texture can be flipped and tiled in horizontal or vertical direction.\n\n" +
		"To help create user interface elements define stretchable regions on texture by setting ^B.slice^n properties."

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

	// rotate checkbox
	spriteContainer.addChild( 'ui/checkbox', {
		text: "^BSpin",
		x: 230, y: 120, minWidth: 65,
		change: function () {
			if ( sprite.update ) { sprite.angle = 0; sprite.update = null; }
			else sprite.update = function ( dt ) {
				sprite.angle += 10 * dt;
			}
		}
	} );

	// properties
	var props = rightColumn.addChild( 'ui/property-list', {
		flex: 1,
		minWidth: 300,
		valueWidth: 130,
		showAll: false,
		showContextMenu: false,
		showMoreButton: false,
		target: sprite.render,
	} );

	// overrides
	props.properties = {
		'texture': { enum: [
			{ text: "smiley.png", value: "/textures/smiley.png" },
			{ text: "clown.png", value: "/textures/clown.png" },
			{ text: "poop.png", value: "/textures/poop.png" },
            { text: "sliced1", value: "/textures/player:Frame001" },
            { text: "sliced2", value: "/textures/player:fish-title" },
		], reloadOnChange: true },
		image: false,
		active: false,
	};

	backButton.focus();
	return scene;
})();