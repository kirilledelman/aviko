new (function (){

	var scene = new Scene( {
		name: "Sprites",
		backgroundColor: Color.Background
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Start,
		pad: 20,
		wrapEnabled: true,
		spacing: 10,
		width: App.windowWidth,
		height: App.windowHeight
	} );

	// title
	scene.addChild( 'ui/text', {
		name: "Title",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		selfAlign: LayoutAlign.Stretch,
		text: "Sprites"
	} );

	// scrollable description
	var description = scene.addChild( 'ui/textfield', {
		size: 12,
		disabled: true,
		disabledBackground: false,
		focusBackground: false,
		scrollingBackground: 0xF0F0F0,
		cornerRadius: 2,
		background: false,
		cancelToBlur: false,
		pad: 5,
		// marginBottom: 40,
		color: Color.Text,
		width: 290,
		wrap: true,
		flex: 1,
		multiLine: true,
		focusRect: true,
	} );

	// page selector scrollbar
	var scrollbar = scene.addChild( 'ui/scrollbar', {
		orientation: 'horizontal',
		width: 290,
		totalSize: 4,
		position: 0,
		handleSize: 1,
		anchorTop: 1,
		top: -15,
		bottom: -45,
		left: -4,
		right: -4,
		discrete: true,
		scroll: changePage,
	});

	// page number on scrollbar handle
	var pageNumber = scrollbar.handle.addChild( 'ui/text', {
		text: 'Page 1',
		color: Color.Text,
		marginTop: -6,
		align: TextAlign.Center
	} );

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
		width: 300,
		spacingY: 5,
		background: 'checker.png',
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	// sample sprite
	var spriteContainer = example.addChild( 'ui/scrollable', {
		height: 120,
		layoutType: Layout.None,
		scrollbars: false,
		layout: function () {
			this.scrollWidth = this.width;
			this.scrollHeight = this.height;
		}
	} );

	var sprite = spriteContainer.addChild( {
		render: new RenderSprite( 'queen.png' ),
		scale: 0.2,
		x: 150, y: 70,
	} );
	sprite.render.pivotX = sprite.render.pivotY = 0.5;

	// properties
	var props = example.addChild( 'ui/property-list', {
		flex: 1,
		valueWidth: 150,
		showAll: false,
		target: sprite.render,
		//updateInterval: 1,
		change: function ( obj, prop, val, oldVal ) {
			log ( obj, '.', prop, ' = ', val, ' from ', oldVal );
		}
	} );

	// multiple subsections of slide
	function changePage( p ) {
		pageNumber.text = 'page ' + (p + 1);
		switch ( p ){
			case 0:
				description.text =
				"Aviko renders sprites using ^B^1RenderSprite^n^c class.\n" +
				"It can display images loaded from ^Bpng^n, and ^Bjpg^n files, as " +
				"well as dynamic textures, using ^B^1Image^n^c class.\n\n" +
				"Sprite sheets (texture atlases) in JSON format can be used to " +
				"combine multiple small sprites into one texture to increase performance";

				props.properties = {
					'texture': { enum: [
						{ text: "queen.png", value: "/textures/queen.png" },
						{ text: "king.png", value: "/textures/king.png" },
					] },
					'originalWidth': { disabled: true },
					'originalHeight': { disabled: true },
					'width': { min: 0, max: Infinity, step: 1 },
					'height': { min: 0, max: Infinity, step: 1 },
				};
				props.groups = [ { name: 'Texture', properties: [ 'texture', 'width', 'height', 'originalWidth', 'originalHeight' ] } ];

				break;
			case 1:
				description.text =
				"Sprites can be tiled, or flipped in horizontal or vertical direction."

				props.properties = {
					'pivotX': { min: 0, max: 1, step: 0.1 },
					'pivotY': { min: 0, max: 1, step: 0.1 },
					'flipX': true,
					'flipY': true,
					'tileX': true,
					'tileY': true,
				};
				props.groups = [
					{ name: "Flip", properties: [ 'flipX', 'flipY' ] },
					{ name: "Tile texture", properties: [ 'tileX', 'tileY' ] } ,
					{ name: "Pivot", properties: [ 'pivotX', 'pivotY' ] },]

				break;
			case 2:
				description.text = "Multiplicative and additive color tinting is supported, as well as opacity, and stippling.";
				props.properties = {
					'color': { inline: true,
						showAll: false,
						properties: {
						'r': { min: 0, max: 1, step: 0.1 },
						'g': { min: 0, max: 1, step: 0.1 },
						'b': { min: 0, max: 1, step: 0.1 },
						'a': { min: 0, max: 1, step: 0.1 }
					} },
					'addColor': { inline: true,
						showAll: false,
						properties: {
						'r': { min: 0, max: 1, step: 0.1 },
						'g': { min: 0, max: 1, step: 0.1 },
						'b': { min: 0, max: 1, step: 0.1 },
						'a': { min: 0, max: 1, step: 0.1 }
					} },
					'opacity': { min: 0, max: 1, step: 0.1 },
					'stipple': { min: 0, max: 1, step: 0.1 },
					'stippleAlpha': true,
				};
				props.groups = [
					{ name: "Tint", properties: [ 'color', 'addColor' ] },
					{ name: "Opacity", properties: [ 'opacity', 'stipple', 'stippleAlpha' ] },]
				break;
			case 3:
				description.text =
				"To help create user interface elements define stretchable regions on " +
				"texture by setting ^Bslice^n property.";
				props.properties = {
					'sliceLeft': true,
					'sliceTop': true,
					'sliceBottom': true,
					'sliceRight': true,
				};
				props.groups = [ { name: "Slicing", properties: [ 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' ]} ];
				sprite.update = null;
				sprite.angle = 0;

				break;

		}

	}

	Input.keyDown = function () { App.scene.ui.requestLayout(); }

	// show first page
	changePage( 0 );
	scrollbar.focus();
	return scene;
})();