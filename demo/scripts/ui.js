new (function (){

	// create scene
	var scene = new Scene( {
		name: "UI",
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
		name: "UI",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		text: "Input",
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
		"^B^1UI^b^c is a class that facilitates creation of user interface elements. " +
		"It has dual function:\n• Helps dispatch input events to on-screen ^B^1GameObject^n^c instances " +
		"(mouse clicks, keyboard/controller, managing focus, etc.).\n• Provides a way to automatically arrange " +
		"objects out on screen in simple layouts - rows, columns, or anchored to parent's edge.\n\n" +
		"^B^1GameObject^n^c has an optional ^B.ui^b property. Assign an instance of ^B^1UI^b^c class to it to use this functionality.\n\n" +
		"Aviko provides a small library of pre-built common UI elements - feel free to modify and use them in your projects!"
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

	//
	var container = rightColumn.addChild( 'ui/scrollable', {
		flex: 1,
		minWidth: 300,
		layoutType: Layout.Vertical,
		pad: 4,
	} );
	
	// components
	
	container.addChild( 'ui/text.js', {
		text: "ui/text:",
		bold: true,
		color: Color.Title,
	} );
	container.addChild( 'ui/text', {
		text: "UI text component\nmultiline, and ^Bformattable^b",
		size: 16,
		color: Color.Text,
	} );
	
	container.addChild( 'ui/text', {
		text: "ui/checkbox.js:",
		bold: true,
		color: Color.Title,
		marginTop: 8,
	} );
	container.addChild( 'ui/checkbox', {
		text: "Checkbox",
	} );
	
	container.addChild( 'ui/text', {
		text: "ui/button.js:",
		bold: true,
		color: Color.Title,
		marginTop: 8,
	} );
	container.addChild( 'ui/button', {
		text: "Button",
		icon: 'smiley',
		iconSize: 20,
	} );
	
	container.addChild( 'ui/text', {
		text: "ui/scrollbar.js:",
		bold: true,
		color: Color.Title,
		marginTop: 8,
	} );
	container.addChild( 'ui/scrollbar', {
		orientation: 'horizontal',
		minWidth: 292,
	} );
	
	container.addChild( 'ui/text', {
		text: "ui/select.js:",
		bold: true,
		color: Color.Title,
		marginTop: 8,
	} );
	container.addChild( 'ui/select', {
		items: [ 'Item one', 'Item two', 'Item three', 'Item four' ],
		value: 'Item one'
	} );
	
	container.addChild( 'ui/text', {
		text: "ui/textfield.js:",
		bold: true,
		color: Color.Title,
		marginTop: 8,
	} );
	container.addChild( 'ui/textfield', {
		text: "Editable text",
		selectAllOnFocus: true,
	} );
	
	backButton.focus();
	return scene;
})();