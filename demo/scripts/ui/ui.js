/*

 Edit this file to change the appearance of Aviko ui components
 
 
 
 
*/

UI.style = UI.style ? UI.style : {

	// font size and name
	baseSize: 16,
	font: 'Arial',

	// text and text selection color
	textColor: 0x0,
	selectionColor: 0x0073b9,
	
	// textures folder (path relative to this file)
	texturesFolder: './textures/ui:', // prepended to each texture's name, ':' is sliced texture frame selector

	// container panel
	panel: 'gradient', // set panel.fill = 'sprite' for sprite background (default)
	panelSlice: 0,
	panelBackgroundColor: 0xd7d7d7, // set panel.fill = 'color' for solid color
	panelPadding: [ 8, 8, 8, 8 ], // top, right, bottom, left
	panelSpacing: [ 4, 4 ], // horizontal, vertical

	// text input
	input: 'input',
	inputFocus: 'input-focus',
	inputDisabled: 'input-disabled',
	inputSlice: [ 8, 8, 8, 8 ], // top, right, bottom, left
	inputPadding: [ 8, 8, 8, 8 ], // top, right, bottom, left

	button: 'button',
	buttonOver: 'button-over',
	buttonDown: 'button-down',
	buttonDisabled: 'button-disabled',
	buttonSlice: [ 16, 16, 16, 16 ], // top, right, bottom, left
	buttonPadding: [ 8, 8, 8, 8 ], // top, right, bottom, left

};


UI.base = UI.base ? UI.base : {

	// shared between UI components
	addSharedProperties: function( go ) {

		// create properties on gameObject
		var mappedProps = [

			// (Number) current width of the control
			[ 'width',  function (){ return go.ui.width; }, function ( w ){ go.ui.width = w; } ],

			// (Number) current height of the control
			[ 'height',  function (){ return go.ui.height; }, function ( h ){ go.ui.height = h; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusLeft',  function (){ return go.ui.focusLeft; }, function ( f ){ go.ui.focusLeft = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusRight',  function (){ return go.ui.focusRight; }, function ( f ){ go.ui.focusRight = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusUp',  function (){ return go.ui.focusLeft; }, function ( f ){ go.ui.focusUp = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusDown',  function (){ return go.ui.focusLeft; }, function ( f ){ go.ui.focusDown = f; } ],

			// (Layout.None, Layout.Anchors, Layout.Vertical, Layout.Horizontal, Layout.Grid) - how to lay out children
			[ 'layoutType',  function (){ return go.ui.layoutType; }, function ( v ){ go.ui.layoutType = v; } ],

			// (Boolean) for Horizontal and Vertical layout types, expand children to fill cross axis
			[ 'expandChildren',  function (){ return go.ui.expandChildren; }, function ( v ){ go.ui.expandChildren = v; } ],

			// (Boolean) for Horizontal and Vertical layout types, adjust own height and width to fit all children
			[ 'fitChildren',  function (){ return go.ui.expandChildren; }, function ( v ){ go.ui.expandChildren = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorLeft',  function (){ return go.ui.anchorLeft; }, function ( v ){ go.ui.anchorLeft = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorRight',  function (){ return go.ui.anchorRight; }, function ( v ){ go.ui.anchorRight = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorTop',  function (){ return go.ui.anchorTop; }, function ( v ){ go.ui.anchorTop = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorBottom',  function (){ return go.ui.anchorBottom; }, function ( v ){ go.ui.anchorBottom = v; } ],

			// (Number) offset from anchorLeft
			[ 'left',  function (){ return go.ui.left; }, function ( v ){ go.ui.left = v; } ],

			// (Number) offset from anchorLeft
			[ 'right',  function (){ return go.ui.right; }, function ( v ){ go.ui.right = v; } ],

			// (Number) offset from anchorLeft
			[ 'top',  function (){ return go.ui.top; }, function ( v ){ go.ui.top = v; } ],

			// (Number) offset from anchorLeft
			[ 'bottom',  function (){ return go.ui.bottom; }, function ( v ){ go.ui.bottom = v; } ],

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
			[ 'pad',  function (){ return go.ui.pad; }, function ( v ){ go.ui.pad = v; } ],

			// (Number) inner padding top
			[ 'padTop',  function (){ return go.ui.padTop; }, function ( v ){ go.ui.padTop = v; } ],

			// (Number) inner padding right
			[ 'padRight',  function (){ return go.ui.padRight; }, function ( v ){ go.ui.padRight = v; } ],

			// (Number) inner padding bottom
			[ 'padBottom',  function (){ return go.ui.padBottom; }, function ( v ){ go.ui.padBottom = v; } ],

			// (Number) inner padding left
			[ 'padLeft',  function (){ return go.ui.padLeft; }, function ( v ){ go.ui.padLeft = v; } ],

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
			[ 'margin',  function (){ return go.ui.margin; }, function ( v ){ go.ui.margin = v; } ],

			// (Number) outer margin top
			[ 'marginTop',  function (){ return go.ui.marginTop; }, function ( v ){ go.ui.marginTop = v; } ],

			// (Number) outer margin right
			[ 'marginRight',  function (){ return go.ui.marginRight; }, function ( v ){ go.ui.marginRight = v; } ],

			// (Number) outer margin bottom
			[ 'marginBottom',  function (){ return go.ui.marginBottom; }, function ( v ){ go.ui.marginBottom = v; } ],

			// (Number) outer margin left
			[ 'marginLeft',  function (){ return go.ui.marginLeft; }, function ( v ){ go.ui.marginLeft = v; } ],

			// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
			[ 'spacing',  function (){ return go.ui.spacing; }, function ( v ){ go.ui.spacing = v; } ],

			// (Number) spacing between children when layoutType is Vertical
			[ 'spacingX',  function (){ return go.ui.spacingX; }, function ( v ){ go.ui.spacingX = v; } ],

			// (Number) spacing between children when layoutType is Horizontal
			[ 'spacingY',  function (){ return go.ui.spacingY; }, function ( v ){ go.ui.spacingY = v; } ]
		];
		// map them
		for ( var i = 0; i < mappedProps.length; i++ ) {
			Object.defineProperty( go, mappedProps[ i ][ 0 ], {
				get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
			} );
		}

	}

}
