/*

 Edit this file to change the appearance of Aviko ui components
 
 
 
 
*/

UI.style = UI.style ? UI.style : {
	
	baseSize: 16,
	font: 'Arial',
	
	textColor: 0x0,
	disabledTextColor: 0x666666,
	backgroundColor: 0xd7d7d7,
	selectionColor: 0x0073b9,
	
	// textures
	texturesFolder: './textures/',
	
	button: 'ui:button',
	buttonOver: 'ui:button-over',
	buttonDown: 'ui:button-down',
	buttonDisabled: 'ui:button-disabled',
	buttonSlice: [ 16, 16, 16, 16 ], // top, right, bottom, left
	
	input: 'ui:input',
	inputFocus: 'ui:input-focus',
	inputDisabled: 'ui:input-disabled',
	inputSlice: [ 8, 8, 8, 8 ],
	inputPadding: [ 8, 8, 8, 8 ]
	
};
