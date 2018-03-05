new (function (){

	var scene = new Scene( {
		name: "Sprites",
		backgroundColor: Color.Background
	} );

	scene.ui = new UI( {
		layoutType: Layout.Vertical,
		layoutCrossAlign: LayoutAlign.Stretch,
		pad: [ 10, 20, 10, 20 ],
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

	// container
	var cont = scene.addChild( 'ui/panel', {
		background: null,
		pad:0,
		spacing: 10,
		marginBottom: 10,
		layoutType: Layout.Horizontal,
		layoutCrossAlign: LayoutAlign.Stretch,
		flex: 1
	} );

	// scrollable description
	var description = cont.addChild( 'ui/textfield', {
		size: 12,
		disabled: true,
		disabledBackground: false,
		focusBackground: false,
		scrollingBackground: 0xF0F0F0,
		cornerRadius: 2,
		background: false,
		cancelToBlur: false,
		pad: 4,
		marginBottom: 40,
		color: Color.Text,
		width: 300,
		wrap: true,
		multiLine: true,
		focusRect: true,
	} );

	var example = cont.addChild( 'ui/panel', {
		flex: 1
	} );

	// scrollbar
	var scrollbar = description.addChild( 'ui/scrollbar', {
		orientation: 'horizontal',
		totalSize: 3,
		position: 0,
		handleSize: 1,
		width: 300,
		anchorTop: 1,
		top: -15,
		bottom: -40,
		left: -4,
		right: -4,
		discrete: true,
		scroll: changePage
	});

	example.addChild( 'ui/property-list', {
		showAll: false,
		properties: {
			'x': true,
			'y': true,
		},
		groups: [
			{ name: "Position", properties: [ 'x', 'y' ] }
		],
		target: scrollbar
	} );

	// page number on scrollbar handle
	var pageNumber = scrollbar.handle.addChild( 'ui/text', {
		text: 'page 1',
		color: Color.Text,
		align: TextAlign.Center
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
				"combine multiple small sprites into one texture to increase performance"
				break;
			case 1:
				description.text =
				"Sprites can be tiled in x or y direction, and flipped. Multiplicative " +
				"and additive color tinting is supported, as well as opacity, and stippling."
				break;
			case 2:
				description.text =
				"To help create user interface elements define stretchable regions on " +
				"texture by setting ^Bslice^n property.";
				break;

		}

	}

	// back to main menu button
	scene.addChild( 'ui/button', {
		text: "Back to main menu",
		selfAlign: LayoutAlign.Start,
		click: function () {
			App.popScene();
			transitionScene( App.scene, scene, 1 );
			scene = null;
		}
	} );

	// show first page
	changePage( 0 );
	scrollbar.focus();
	return scene;
})();