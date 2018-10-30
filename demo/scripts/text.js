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
		formatting: true,
		text:
		"Aviko renders text using ^B^1RenderText^b^c class.\n\n" +
		"Text can include inline formatting for ^Bbold^b, ^Iitalic^i, and ^2t^3e^4xt ^5co^8lor^c.\n\n" +
		"Aviko includes Google's Roboto as a built-in font, and supports ^BTTF^b format for custom fonts.\n\n" +
		"^B^1RenderText^b^c also includes properties and methods to help implement text input fields in Javascript (provided with the included UI components)."

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
	var textContainer = rightColumn.addChild( 'ui/scrollable', {
		minHeight: 150,
		layoutType: Layout.None,
	} );
	
	// background for sprite
	textContainer.addChild( 'ui/image', {
		width: 300,
		height: 150,
		mode: 'fit',
		texture: 'checker.png',
	} );

	// background for sprite
	var text = textContainer.addChild( 'ui/text', {
		width: 280,
		maxWidth: 280,
		x: 150, y: 70,
		pivot: 0.5,
		autoSize: true,
		multiLine: true,
		color: 0x0,
		size: 30,
		align: TextAlign.Center,
		text: "Sample text:\n^BBold^b, ^IItalic^i,\n^3PURPLE!",
	} );
	
	// properties
	var props = rightColumn.addChild( 'ui/property-list', {
		flex: 1,
		minWidth: 300,
		valueWidth: 130,
		showAll: true,
		showContextMenu: false,
		showMoreButton: false,
		target: text.renderText,
	} );

	// overrides
	props.properties = {
		active: false,
		italicFont: false,
		boldItalicFont: false,
		font: { readOnly: true },
		boldFont: { readOnly: true },
		colors: { inline: true, readOnly: true },
		showSelection: false, selectionStart: false, selectionEnd: false,
		selectionTextColor: false, selectionColor: false,
		showCaret: false, caretPosition: false,
		autoSize: false, pivotX: false, pivotY: false,
	};

	backButton.focus();
	return scene;
})();