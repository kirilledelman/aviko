new (function (){

	// create scene
	var scene = new Scene( {
		name: "Shapes",
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
		text: "Shapes",
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
		text: "Aviko can render a variety of geometric shapes by assigning an instance of ^B^1RenderShape^n^c class to " +
		"^BGameObject^b's ^B.render^b property.\n\n" +
		"The shape itself is determined by the value of ^B.shape^b property, set to one of constant values in ^BShapes^b enumeration:\n" +
		"^IArc, Circle, Ellipse, Line, Polygon, Rectangle, RoundedRectangle, Sector, Triangle, Chain.^i\n\n" +
		"The shape parameters are shared between different shape types, and can be changed on the fly.\n\n" +
		"Some shapes can be solid (filled), and have an additional colored outline.",

	} );

	// back to main menu button
	var backButton = leftColumn.addChild( 'ui/button', {
		text: "Back to main menu",
		click: function () {
			App.popScene();
			App.scene.ui.requestLayout();
			transitionScene( App.scene, scene, 1 );
			scene = null;
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
		render: new RenderShape( {
			shape: Shape.Circle,
			color: 0xF06620,
			radius: 40,
			lineThickness: 2,
			startAngle: 45,
			x: 40, y: 30,
			y1: -50, x2: -40, y2: 60,
		} ),
		x: 150, y: 70,
	} );
	// generate star -> points
	var r0 = 40, r1 = 20;
	for ( var i = 0; i < 5; i++ ) {
		var th = Math.PI * 0.2;
		sprite.render.points.push( r0 * Math.cos( th * i * 2 ), r0 * Math.sin( th * i * 2 ) );
		sprite.render.points.push( r1 * Math.cos( th * ( 1 + i * 2 ) ), r1 * Math.sin( th * ( 1 + i * 2 ) ) );
	}

	// properties
	var props = rightColumn.addChild( 'ui/property-list', {
		flex: 1,
		minWidth: 300,
		valueWidth: 130,
		showAll: true,
		showBackButton: false,
		showContextMenu: false,
		showMoreButton: false,
		target: sprite.render,
		properties: {
			active: false,
		}
	} );

	backButton.focus();
	return scene;
})();