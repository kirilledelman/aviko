new (function (){

	// create scene
	var scene = new Scene( {
		name: "Sprites",
		backgroundColor: Color.Background
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		pad: 20,
		wrapEnabled: true,
		fitChildren: false,
		spacing: 10,
		width: App.windowWidth,
		height: App.windowHeight,
		//layout: function ( w, h ) { log( "scene.layout", w, h ); }
	} );

	// title
	scene.addChild( 'ui/text', {
		name: "Title",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		minWidth: 290,
		text: "Sprites"
	} );

	// scrollable description
	var description = scene.addChild( 'ui/textfield', {
		size: 14,
		disabled: true,
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
	} );

	// page selector scrollbar
	var scrollbar = scene.addChild( 'ui/scrollbar', {
		orientation: 'horizontal',
		totalSize: 4,
		position: 0,
		handleSize: 1,
		anchorTop: 1,
		top: -15,
		bottom: -45,
		left: -4,
		right: -4,
		discrete: true,
		acceptToScroll: true,
		scroll: changePage,
	});

	// back to main menu button
	scene.addChild( 'ui/button', {
		text: "Back to main menu",
		selfAlign: LayoutAlign.Stretch,
		forceWrap: true, // new column after this
		click: function () {
			App.popScene();
			transitionScene( App.scene, scene, 1 );
			scene = null;
		}
	} );

	// example holder
	var example = scene.addChild( 'ui/panel', {
		flex: 1,
		pad: 0,
		minWidth: 300,
		spacingY: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	// sample sprite container
	var spriteContainer = example.addChild( 'ui/scrollable', {
		minHeight: 140,
		layoutType: Layout.None,
		scrollbars: false,
		layout: function () {
			this.scrollWidth = this.width;
			this.scrollHeight = this.height;
		}
	} );

	// background for sprite
	spriteContainer.addChild( 'ui/image', {
		width: 300,
		height: 140,
		mode: 'fit',
		texture: 'checker.png',
	} );

	// sprite
	var sprite = spriteContainer.addChild( {
		render: new RenderSprite( 'smiley.png' ),
		x: 150, y: 70,
	} );
	sprite.render.pivotX = sprite.render.pivotY = 0.5;

	// properties
	var props = example.addChild( 'ui/property-list', {
		flex: 1,
		valueWidth: 150,
		showAll: false,
		target: sprite.render
	} );

	// multiple subsections of slide
	function changePage( p ) {
		scrollbar.handle.text = 'Page ' + (p + 1);
		switch ( p ){
			case 0:
				description.text =
				"Aviko renders sprites using ^B^1RenderSprite^n^c class.\n\n" +
				"It can display images loaded from ^Bpng^n, and ^Bjpg^n files, as " +
				"well as dynamic textures, using ^B^1Image^n^c class.\n\n" +
				"Sprite sheets (texture atlases) in JSON format can be used to " +
				"combine multiple small sprites into one texture to increase performance.";

				props.properties = {
					'texture': { enum: [
						{ text: "smiley.png", value: "/textures/smiley.png" },
						{ text: "clown.png", value: "/textures/clown.png" },
						{ text: "poop.png", value: "/textures/poop.png" },
						{ text: "normal", value: "/textures/test:test1" },
						{ text: "rotated", value: "/textures/test:test" },
					] },
					'originalWidth': { disabled: true },
					'originalHeight': { disabled: true },
					'width': { min: 0, max: Infinity, step: 1 },
					'height': { min: 0, max: Infinity, step: 1 },
				};
				props.groups = [
					{ name: 'Texture', properties: [ 'texture', 'width', 'height', 'originalWidth', 'originalHeight' ] },
				];

				break;
			case 1:
				description.text =
				"Sprite texture can be tiled, or flipped in horizontal or vertical direction.\n\n" +
				"Auto-tiling will keep the size of texture constant."

				props.properties = {
					'flipX': true,
					'flipY': true,
					'tileX': true,
					'tileY': true,
					'autoTileX': true,
					'autoTileY': true
				};
				props.groups = [
					{ name: "Flip", properties: [ 'flipX', 'flipY' ] },
					{ name: "Tile texture", properties: [ 'tileX', 'tileY', 'autoTileX', 'autoTileY' ] }
				];

				break;
			case 2:
				description.text = "Multiplicative and additive color tinting is supported, as well as opacity, and stippling.";
				props.properties = {
					'color': true,
					'addColor': { properties: { 'a': false } },
					'opacity': { min: 0, max: 1, step: 0.1, target: sprite },
					'stipple': { min: 0, max: 1, step: 0.1 }
				};
				props.groups = [
					{ name: "Tint", properties: [ 'color', 'addColor' ] },
					{ name: "Opacity", properties: [ 'opacity', 'stipple' ] },]
				break;
			case 3:
				description.text =
				"To help create user interface elements define stretchable regions on " +
				"texture by setting ^Bslice^n property.";
				props.properties = {
					'sliceLeft': { min: 0, step: 1 },
					'sliceTop': { min: 0, step: 1 },
					'sliceBottom': { min: 0, step: 1 },
					'sliceRight': { min: 0, step: 1 },
				};
				props.groups = [ { name: "Slicing", properties: [ 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' ]} ];
				sprite.update = null;
				sprite.angle = 0;

				break;

		}

	}

	// show first page
	changePage( 0 );
	scrollbar.focus();
	return scene;
})();