new (function (){

	// create scene
	var scene = new Scene( {
		name: "Sprites",
		backgroundColor: Color.Background,
		opacity: 0,
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
	} );

	// title
	var title = scene.addChild( 'ui/text', {
		name: "Title",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		minWidth: 290,
		text: "Sprites",
	} );

	// scrollable description
	var description = scene.addChild( 'ui/textfield', {
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
	sprite.render.pivotX = sprite.render.pivotY = 0.5;

	// rotate checkbox
	spriteContainer.addChild( 'ui/checkbox', {
		text: "^BSpin",
		x: 230, y: 125,
		change: function () {
			if ( sprite.update ) { sprite.angle = 0; sprite.update = null; }
			else sprite.update = function ( dt ) {
				sprite.angle += 10 * dt;
			}
		}
	} );

	// properties
	var props = example.addChild( 'ui/property-list', {
		flex: 1,
		valueWidth: 130,
		showAll: false,
		showHeader: false,
		rightClickMenu: false,
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
					]},
					'originalWidth': { disabled: true },
					'originalHeight': { disabled: true },
				};
				props.groups = [
					{ name: 'Texture', properties: [ 'texture', 'originalWidth', 'originalHeight' ] },
				];

				break;
			case 1:
				description.text =
				"Sprite texture can be flipped and tiled in horizontal or vertical direction."

				props.properties = {
					'flipX': true,
					'flipY': true,
					'tileX': true,
					'tileY': true,
				};
				props.groups = [
					{ name: "Flip", properties: [ 'flipX', 'flipY' ] },
					{ name: "Tile texture", properties: [ 'tileX', 'tileY' ] }
				];
				break;
			case 2:
				description.text = "Various blending modes, colored outline, multiplicative and additive color tinting are supported, as well as opacity, and stippling.";
				props.properties = {
					'color': { inline: true },
					'addColor': {
						inline: true,
						properties: {
							'r': { min: -1, max: 1, step: 0.1 },
							'g': { min: -1, max: 1, step: 0.1 },
							'b': { min: -1, max: 1, step: 0.1 },
							'a': { min: -1, max: 1, step: 0.1 },
							'hex': false,
						}
					},
					'opacity': { min: 0, max: 1, step: 0.1, target: sprite },
					'stipple': { min: 0, max: 1, step: 0.1 },
					'blendMode': { enum: [
						{ text: "Normal", value: BlendMode.Normal },
						{ text: "Add", value: BlendMode.Add },
						{ text: "Subtract", value: BlendMode.Subtract },
						{ text: "Multiply", value: BlendMode.Multiply },
						{ text: "Screen", value: BlendMode.Screen },
						{ text: "Burn", value: BlendMode.Burn },
						{ text: "Dodge", value: BlendMode.Dodge },
						{ text: "Invert", value: BlendMode.Invert },
						{ text: "Color", value: BlendMode.Color },
						{ text: "Hue", value: BlendMode.Hue },
						{ text: "Saturation", value: BlendMode.Saturation },
						{ text: "Luminosity", value: BlendMode.Luminosity },
						{ text: "Refract", value: BlendMode.Refract },
						{ text: "Cut", value: BlendMode.Cut },
					] },
					'outlineColor': { inline: true, hidden: function( t ){ return (t.outlineRadius === 0); } },
					'outlineOffsetX': { step: 1, hidden: function( t ){ return (t.outlineRadius === 0); } },
					'outlineOffsetY': { step: 1, hidden: function( t ){ return (t.outlineRadius === 0); } },
					'outlineRadius': { min: -16, max: 16, step: 1, reloadOnChange: [ 'outlineOffsetX', 'outlineOffsetY', 'outlineColor' ] },
				};
				props.groups = [
					{ name: "Blending", properties: [ 'blendMode', 'outlineRadius', 'outlineOffsetX', 'outlineOffsetY', 'outlineColor' ] },
					{ name: "Tint", properties: [ 'color', 'addColor' ] },
					{ name: "Opacity", properties: [ 'opacity', 'stipple' ] },]
				break;
			case 3:
				description.text =
				"To help create user interface elements define stretchable regions on " +
				"texture by setting ^Bslice^n properties.";
				props.properties = {
					'sliceLeft': { min: 0, step: 1 },
					'sliceTop': { min: 0, step: 1 },
					'sliceBottom': { min: 0, step: 1 },
					'sliceRight': { min: 0, step: 1 },
					'width': { min: 0, max: Infinity, step: 1 },
					'height': { min: 0, max: Infinity, step: 1 },
				};
				props.groups = [
					{ name: "Slicing", properties: [ 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' ]},
					{ name: "Size", properties: [ 'width', 'height' ]}
				];
				break;

		}

	}

	// show first page
	changePage( 0 );
	scrollbar.focus();
	scene.fadeTo( 1, 2 );
	return scene;
})();