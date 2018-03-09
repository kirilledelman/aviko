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
		marginBottom: 40,
		color: Color.Text,
		width: 290,
		wrap: true,
		flex: 1,
		multiLine: true,
		focusRect: true,
	} );

	// page selector scrollbar
	var scrollbar = description.addChild( 'ui/scrollbar', {
		orientation: 'horizontal',
		totalSize: 3,
		position: 0,
		handleSize: 1,
		anchorTop: 1,
		top: -15,
		bottom: -40,
		left: -4,
		right: -4,
		discrete: true,
		scroll: changePage
	});

	// page number on scrollbar handle
	var pageNumber = scrollbar.handle.addChild( 'ui/text', {
		text: 'Page 1',
		color: Color.Text,
		align: TextAlign.Center
	} );

	// back to main menu button
	scene.addChild( 'ui/button', {
		text: "Back to main menu",
		selfAlign: LayoutAlign.Start,
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
		width: 300,
		background: 0xF0F0F0,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Stretch,
	} );

	// sample sprite
	var spriteContainer = example.addChild( 'ui/scrollable', {
		height: 200,
		flex: 1,
		layoutType: Layout.None,
		scrollbars: false,
		layout: function () {
			this.scrollWidth = this.width;
			this.scrollHeight = this.height;
		}
	} );

	var sprite = spriteContainer.addChild( {
		render: new RenderSprite( 'queen.png' ),
		scale: 0.5,
		x: 150, y: 100,
	} );
	sprite.render.pivotX = sprite.render.pivotY = 0.5;

	// properties
	var propsContainer = example.addChild( 'ui/scrollable', {
		flex: 1,
		pad: 5,
		valueWidth: 150,
	} );

	// add property list to scrollable container
	var props = propsContainer.addChild( 'ui/property-list', {
		showAll: false,
		target: sprite.render,
		layout: function () {
			// resize to fit
			propsContainer.scrollHeight = this.height + propsContainer.padTop + propsContainer.padBottom;
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
					'flipX': true,
				};
				props.groups = [ { name: 'Texture', properties: [ 'texture' ] } ];
				sprite.update = null;
				sprite.angle = 0;

				break;
			case 1:
				description.text =
				"Sprites can be tiled in x or y direction, and flipped. Multiplicative " +
				"and additive color tinting is supported, as well as opacity, and stippling."

				props.properties = {
					'flipX': true,
					'flipY': true,
					'pivotX': { min: 0, max: 1, step: 0.1 },
					'pivotY': { min: 0, max: 1, step: 0.1 },
					'tileX': true,
					'tileY': true,
				};
				props.groups = [
					{ name: "Flip", properties: [ 'flipX', 'flipY' ] },
					{ name: "Pivot", properties: [ 'flipX', 'flipY' ] },
					{ name: "Tile texture", properties: [ 'tileX', 'tileY' ] } ];
				sprite.update = function( dt ) { this.angle += dt * 10; }

				break;
			case 2:
				description.text =
				"To help create user interface elements define stretchable regions on " +
				"texture by setting ^Bslice^n property.";
				props.properties = {
					'sliceLeft': true,
					'sliceTop': true,
					'sliceBottom': true,
					'sliceRight': true,
				};
				props.groups = [ { name: "9-slice", properties: [ 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' ]} ];
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