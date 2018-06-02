/*

	Aviko UI components are intended to help rapid development.

	To customize default appearance, edit UI.style structure below.

	These are just default properties applied at creation and can be overridden.
	See components' source files for all available properties. Some shared properties are
	defined in this file in "shared functionality" block (below UI.style structure)

    To customize individual components, use .style property to override many properties
    at once. E.g:

	    var bigButtonStyle = { pad: 10, label: { size: 20 } };
	    var smallButtonStyle = { pad: 4, label: { size: 8 } };

	    container.addChild( 'ui/button', {
	        text: "Big Button",
	        style: bigButtonStyle
	    });
	    container.addChild( 'ui/button', {
	        text: "Small Button",
	        style: smallButtonStyle
	    });
 
*/

UI.style = UI.style || {

	// basic container panel - ui/panel.js
	panel: { },

	// text label - ui/text.js
	text: {
		font: 'Roboto-Regular',
		boldFont: 'Roboto-Black',
		italicFont: 'Roboto-Italic',
		boldItalicFont: 'Roboto-BlackItalic',
		size: 14,
		textAlign: TextAlign.Left,
		color: 0xFFFFFF,
		wrap: false,
	},

	// text input field - ui/textfield.js
	textfield: {
		font: 'Roboto-Regular',
		boldFont: 'Roboto-Black',
		italicFont: 'Roboto-Italic',
		boldItalicFont: 'Roboto-BlackItalic',
		bold: true,
		size: 14,
		color: 0x0,
		selectionColor: 0x0066a5,
		selectionTextColor: 0xFFFFFF,
		outlineColor: 0x0066a5,
		pad: 4,
		cornerRadius: 0,
		acceptToEdit: true,
		focusRect: true,
		states:{
			off: {
				background: 0xf6f6f6,
				lineThickness: 0,
			},
			scroll: {
				background: 0xe5eff4,
				lineThickness: 2,
			},
			focus: {
				background: 0xd9e2e7,
				lineThickness: 2,
			},
			disabled: {
				background: 0xcccccc,
				lineThickness: 0,
			},
		},
		// autocomplete menu (ui/popupMenu.js)
		popupMenu: {
			// items in the popup menu (ui/button.js)
			item: {
				label: { size: 12, },
				pad: [ 4, 8, 4, 8 ],
			},
		},
	},

	// scrollable container - ui/scrollable.js
	scrollable: {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		spacing: 4,
		scrollbars: 'auto'
	},

	// scrollbar - ui/scrollbar.js
	scrollbar: {
		minWidth: 20,
		minHeight: 20,
		pad: 2,
		focusRect: true,
		borderRadius: 2,

		// handle - ui/button.js
		handle: {
			pad: 0,
			borderRadius: 0,
			label: {
				size: 12,
				color: 0x0,
				bold: false,
				align: TextAlign.Center,
				wrap: false,
			}
		},

		states: {
			off: {
				background: 0xd7d7d7,
				handle: {
					background: 0xb6b6b6,
				},
			},
			focus: {
				background: 0xe3e3e3,
				handle: {
					background: 0xcecccc,
				}
			},
			scrolling: { // scrolling with directional keys mode with .acceptToScroll = true
				handle: {
					background: 0xFFFFFF,
				}
			},
			dragging: { // dragging handle with mouse
				handle: {
					background: 0xb6b6b6,
				}
			},

			disabled: {
				background: 0xcccccc,
				handle: {
					background: 0xcfcfcf,
				}
			},
		},

		// apply only to horizontal
		horizontalStyle: { },

		// apply only to vertical
		verticalStyle: { },

	},

	// image container - ui/image.js
	image: { },

	// button - ui/button.js
	button: {
		pad: 12,
		focusRect: true,
		cornerRadius: 3,

		// apply to button's label (ui/text.js)
		label: {
			color: 0xFFFFFF,
			size: 18,
			bold: true,
		},

		states:{
			off: {
				background: 0x0066a5,
			},
			over: {
				background: 0x1a92dc,
			},
			focus: {
				background: 0x1a92dc,
			},
			down: {
				background: 0x004b7a,
			},
			disabled: {
				background: 0xc0c0c0,
			},
		}
	},

	// checkbox - ui/checkbox.js
	checkbox: {

		pad: 2,
		focusRect: true,

		// apply to checkbox label (ui/text.js)
		label: {
			color: 0x0,
			size: 14,
			marginLeft: 4,
			padTop: -1,
		},

		// "checkbox" button (ui/button.js)
		checkbox: {
			// "checkmark" image
			icon: '#iVBORw0KGgoAAAANSUhEUgAAAAoAAAAKCAYAAACNMs+9AAAAG0lEQVR4XmMgFjBCFf4noIGRiVgTR6ZCogMcAHcwARAJJhF3AAAAAElFTkSuQmCC ',
			minWidth: 16,
			minHeight: 16,
		},

		states: {
			off: {
				checkbox: { background: 0xf6f6f6, },
				label: { color: 0x0 }
			},
			over: {
				checkbox: { background: 0xe5eff4, },
				label: { color: 0x0 }
			},
			focus: {
				checkbox: { background: 0xd9e2e7, },
				label: { color: 0x0 }
			},
			down: {
				checkbox: { background: 0x0066a5, },
				label: { color: 0x0 }
			},
			disabled: {
				checkbox: { background: 0xcccccc, },
				label: { color: 0x333333 }
			},
		}

	},

	// select-style dropdown menu - ui/select.js
	select: {

		maxVisibleItems: 5,
		pad: 0,

		// style applied to dropdown button itself - ui/button.js
		button: {
			label: {
				bold: true,
				size: 14,
				color: 0x0
			},
			pad: 4,
			outlineColor: 0x0066a5,
			cornerRadius: 2,
			states: {
				off: {
					background: 0xf6f6f6,
					lineThickness: 0,
					label: { color: 0x0 }
				},
				over: {
					background: 0xf9f9f9,
					lineThickness: 0,
					label: { color: 0x0 }
				},
				down: {
					background: 0x0066a5,
					lineThickness: 0,
					label: { color: 0xFFFFFF }
				},
				focus: {
					background: 0xd9e2e7,
					lineThickness: 2,
					label: { color: 0x0 }
				},
				disabled: {
					background: 0xcccccc,
					lineThickness: 0,
					label: { color: 0x333333 }
				},
			}
		},

		// image used for arrow on the right side of the dropdown - ui/image.js
		arrowImage: {
			marginLeft: 4,
			pad: 0,
			texture: '#iVBORw0KGgoAAAANSUhEUgAAAAwAAAAHCAYAAAA8sqwkAAAANElEQVR4XmMgF7gANf4ngF1QDBcREQnDpQEqh+kYbJpwKoZpR9ZEUDFMk6ysbCIIM1ADAABNKxlRej1XTAAAAABJRU5ErkJggg==',
		},

		// scrollable container with items (ui/scrollable.js)
		menu: {
			spacingY: 0,
			pad: 0,
			render: {
				outlineColor: '00000033',
				outlineOffsetY: 1,
				outlineRadius: 2
			}
		},

		// items in the dropdown list (ui/button.js)
		item: {
			label: {
				size: 14,
				flex: 1
			},
			layoutAlignX: LayoutAlign.Start,
			focusRect: false,
			cornerRadius: 0,
			pad: [ 4, 8, 4, 8 ],
			states:{
				off: {
					background: 0xFFFFFF,
					label: { color: 0x0 }
				},
				over: {
					background: 0x006eb2,
					label: { color: 0xFFFFFF }
				},
				focus: {
					background: 0x0066a5,
					label: { color: 0xFFFFFF }
				},
				down: {
					background: 0x004b7a,
					label: { color: 0xFFFFFF }
				},
				disabled: {
					background: 0xf6f6f6,
					label: { color: 0x333333 }
				},
			}
		},

		// currently selected item checkmark (ui/image.js)
		itemCheck: {
			texture: '#iVBORw0KGgoAAAANSUhEUgAAAA4AAAAKCAYAAACE2W/HAAAAqElEQVR4XmOgB1ABWjIJiFnRLePHY7suUO4rEP8H4g5kdfJAzlsgrgNiJjQDLID8nyBNLCws52RkZISQ5dNAElDcC6RhznEHsv+CxNnZ2Q+rqqqiaGLg4OBgEBISKkDSPA/IjofxgfKb1dTU+BiwASYmJgZRUdF4JM0g//zn4uJaCrSJm4EQkJCQ8Ic5j5eXd4qkpCQrA7FASUnJQ1xcvJOfn5+JgdoAAB8wISrAnWSvAAAAAElFTkSuQmCC',
			mode: 'icon',
			marginLeft: 4,
		}

	},

	// select-style dropdown menu - ui/select.js
	popupMenu: {

		maxVisibleItems: 8,
		pad: 0,

		// scrollable container with items (ui/scrollable.js)
		container: {
			spacingY: 0,
			minWidth: 20,
			minHeight: 20,
			pad: 0,
			render: {
				outlineColor: '00000033',
				outlineOffsetY: 1,
				outlineRadius: 2
			}
		},

		// items in the popup menu (ui/button.js)
		item: {
			label: {
				size: 14,
				flex: 1,
				bold: false,
			},
			layoutAlignX: LayoutAlign.Start,
			focusRect: false,
			cornerRadius: 0,
			pad: [ 4, 8, 4, 8 ],
			states: {
				off: {
					background: 0xFFFFFF,
					label: { color: 0x0 }
				},
				over: {
					background: 0x006eb2,
					label: { color: 0xFFFFFF }
				},
				focus: {
					background: 0x0066a5,
					label: { color: 0xFFFFFF }
				},
				down: {
					background: 0x004b7a,
					label: { color: 0xFFFFFF }
				},
				disabled: {
					background: undefined,
					label: { color: 0xA0A0A0 }
				},
			}
		},

		// separator (ui/panel)
		separator: {
			background: 0xD0D0D0,
			minHeight: 1,
		}

	},

	// window - ui/window.js
	window: {

		background: 0xE0E0E0,
		cornerRadius: 4,
		pad: [ 30, 6, 6, 6 ],

		// ui/panel.js instance
		header: {
			pad: 2,
			background: 0x004b7a,
			marginBottom: 4,
			cornerRadius: 4,
			minHeight: 30,
		},

		// ui/text.js instance
		titleText: {
			color: 0xFFFFFF,
			size: 12,
			pad: 4,
			bold: true,
		},

		// ui/button.js
		closeButton: {
			pad: [ 0, 4, 0, 4 ],
			size: 12,
			text: "X",
		},

		// ui/panel.js
		modalBackground: {
			background: 0xFFFFFF,
			opacity: 0.9,
		}
	},

	// tooltip control (ui/tooltip.js)
	tooltip: {

		// (ui/panel) container
		panel: {
			pad: 8,
			background: 0xFFFFFF,
			outlineColor: 0x999999,
			lineThickness: 2,
			cornerRadius: 2,
		},

		label: {
			size: 12,
			color: 0x0,
			maxWidth: 200,
			minWidth: 60,
		}
	},

	// property list for editors (ui/property-list.js)
	propertyList: {

		valueWidth: 130, // width of value part. height of rows is set in value.AnyType
		spacingX: 2, // distance between label and value
		spacingY: 4, // distance between rows
		pad: 4,

		// applied to group label - note that first group's marginTop will be forced to 0
		group: {
			bold: true,
			color: 0x666666,
			align: TextAlign.Center,
			marginTop: 12,
			marginBottom: 4,
		},

		// applied to "no editable properties" and "(null)" text
		empty: {
			bold: true,
			color: 0x666666,
			align: TextAlign.Center,
			marginTop: 4,
			flex: 1,
			wrap: true,
			forceWrap: true
		},

		// applied to each field label
		label: {
			pad: [ 4, 4, 0, 0 ],
			align: TextAlign.Right,
			size: 14,
			color: 0x0,
		},

		// container for buttons on top of prop list
		header: {
			pad: 2,
			spacing: 2,
		},

		backButton: {
			label: {
				size: 12,
				bold: false,
				flex: 1,
				wrap: true,
				align: TextAlign.Left,
			},
			background: false,
			pad: [ 2, 4, 2, 4 ],
			spacingX: 4,
			image: { imageObject: { angle: 90 } }, // down arrow rotated back <
			states: {
				off: { background: 0xF0F0F0, label: { color: 0x0 } },
				focus: { background: 0x006eb2, label: { color: 0xFFFFFF } },
				disabled: { background: false, label: { color: 0x0, opacity: 1 } },
				over: { background: 0x006eb2, label: { color: 0xFFFFFF } },
				down: { background: 0x004b7a, label: { color: 0xFFFFFF } },
			}
		},

		moreButton: {
			label: {
				size: 12,
				bold: false,
			},
			pad: [ 2, 4, 2, 4 ],
			states: {
				off: { background: 0xF0F0F0, label: { color: 0x0 } },
				focus: { background: 0x006eb2, label: { color: 0xFFFFFF } },
				disabled: { background: 0xF0F0F0, label: { color: 0x999999 } },
				over: { background: 0x006eb2, label: { color: 0xFFFFFF } },
				down: { background: 0x004b7a, label: { color: 0xFFFFFF } },
			}
		},

		// applied to field values based on type
		values: {

			// all types
			any: {
				acceptToEdit: true,
				cancelToBlur: false,
				blurOnClickOutside: true,
				fitChildren: false,
				minHeight: 25,
				states: { disabled: { label: { color: 0x666666 } }, }
			},

			// Number - ui/textfield
			number: {
				numeric: true,
				selectAllOnFocus: true,
			},

			// String - ui/textfield
			string: { },

			// Function - ui/button
			func: {
				layoutAlignX: LayoutAlign.Start,
				spacing: 4,
				label: { bold: true, size: 14, color: 0x0 },
				pad: 4,
				outlineColor: 0x0066a5,
				cornerRadius: 2,
				states: {
					off: {
						background: 0xf6f6f6,
						lineThickness: 0,
						label: { color: 0x0 }
					},
					over: {
						background: 0xf6f6f6,
						lineThickness: 0,
						label: { color: 0x0 }
					},
					down: {
						background: 0x0066a5,
						lineThickness: 0,
						label: { color: 0xFFFFFF }
					},
					focus: {
						background: 0xd9e2e7,
						lineThickness: 2,
						label: { color: 0x0 }
					},
					disabled: {
						background: 0xcccccc,
						lineThickness: 0,
						label: { color: 0x666666 }
					},
				}
			},

			// Boolean - ui/checkbox
			boolean: { },

			// Enum - ui/dropdown
			enum: { },

			// Object - ui/button
			object: {
				icon: '#iVBORw0KGgoAAAANSUhEUgAAAAwAAAAHCAYAAAA8sqwkAAAANElEQVR4XmMgF7gANf4ngF1QDBcREQnDpQEqh+kYbJpwKoZpR9ZEUDFMk6ysbCIIM1ADAABNKxlRej1XTAAAAABJRU5ErkJggg==',
				layoutAlignX: LayoutAlign.Start,
				spacing: 4,
				label: {
					bold: true,
					size: 14,
					color: 0x0
				},
				pad: 4,
				outlineColor: 0x0066a5,
				cornerRadius: 2,
				states: {
					off: {
						background: 0xf6f6f6,
						lineThickness: 0,
						label: { color: 0x0 }
					},
					over: {
						background: 0xf6f6f6,
						label: { color: 0x0 }
					},
					down: {
						background: 0x0066a5,
						label: { color: 0xFFFFFF }
					},
					focus: {
						background: 0xd9e2e7,
						lineThickness: 2,
						label: { color: 0x0 }
					},
					disabled: {
						background: 0xcccccc,
						lineThickness: 0,
						label: { color: 0x666666 }
					},
				}
			},

			// inline sub-property list ui/property-list
			inline: {
				marginLeft: 16 // indent
			},

		},

	},

	// (ui/resizer.js)
	resizer: {

		minWidth: 6,
		minHeight: 6,

		states: {
			off: { background: false },
			over: { background: 0xFFFFFF },
			dragging: { background: 0x0066a5 },
			collapsed: { background: 0x333333 },
		}
	},

	// settings for focus rectangle (ui/panel.js)
	focusRect: {

		// texture or color
		background: 0x0066a5,

		opacity: 0.5,

		// rounded corner, if background is color
		cornerRadius: 4,

		// outline thickness if background is color
		lineThickness: 2,

		// filled = solid rectangle, false = outline
		filled: false,

		// pixels offset outside focused control
		offset: 2

	}

};


/*

Shared functionality between Aviko ui components

*/

UI.base = UI.base || {

	// layout-specific properties shared between UI components
	addSharedProperties: function( go, ui ) {

		// create properties on gameObject
		var mappedProps = {

			// (Number) current width of the control
			'width': { get: function (){ return ui.width; }, set: function( w ){ ui.width = w; } },

			// (Number) current height of the control
			'height': { get: function (){ return ui.height; }, set: function( h ){ ui.height = h; } },

			// (Boolean) allows keyboard / navigation focus
			'focusable': { get: function (){ return ui.focusable; }, set: function( v ){ ui.focusable = v; } },

			// (Boolean) is field currently focused
			'focused': { get: function (){ return ui.focused; } },

			// (UI) or (GameObject) or null - object to focus to the left of this control
			'focusLeft': { get: function (){ return ui.focusLeft; }, set: function( f ){ ui.focusLeft = f; } },

			// (UI) or (GameObject) or null - object to focus to the left of this control
			'focusRight': { get: function (){ return ui.focusRight; }, set: function( f ){ ui.focusRight = f; } },

			// (UI) or (GameObject) or null - object to focus to the left of this control
			'focusUp': { get: function (){ return ui.focusUp; }, set: function( f ){ ui.focusUp = f; } },

			// (UI) or (GameObject) or null - object to focus to the left of this control
			'focusDown': { get: function (){ return ui.focusDown; }, set: function( f ){ ui.focusDown = f; } },

			// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
			'focusGroup': { get: function (){ return ui.focusGroup; }, set: function( f ){ ui.focusGroup = f; } },

			// (Boolean) - enables/disables focus rectangle (getter returns Render component)
			'focusRect': {
				get: function (){ return ui.focusRect; },
				set: function ( fr ){
					if ( fr ) {
						ui.on( 'layout', UI.base._layoutFocusRect );
						ui.on( 'focusChanged', UI.base._focusChangedRect );
					} else {
						ui.off( 'layout', UI.base._layoutFocusRect );
						ui.off( 'focusChanged', UI.base._focusChangedRect );
						if ( ui.focusRectPanel ) {
							ui.focusRectPanel = ui.focusRectPanel.parent = null;
						}
					}
					ui.focusRect = fr;
				}
			},

			// (Boolean) blocks mouse events from reaching objects higher in in hierarchy tree
			'blocking': { get: function (){ return ui.blocking; }, set: function( v ){ ui.blocking = v; } },

			// (Layout.None, Layout.Anchors, Layout.Vertical, Layout.Horizontal, Layout.Grid) - how to lay out children
			'layoutType': { get: function (){ return ui.layoutType; }, set: function( v ){ ui.layoutType = v; } },

			// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on X axis
			'layoutAlignX': { get: function (){ return ui.layoutAlignX; }, set: function( v ){ ui.layoutAlignX = v; } },

			// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on Y axis
			'layoutAlignY': { get: function (){ return ui.layoutAlignY; }, set: function( v ){ ui.layoutAlignY = v; } },

			// (Boolean) for Horizontal, Vertical, and Grid layout types, adjust own height and width to fit all children
			'fitChildren': { get: function (){ return ui.fitChildren; }, set: function( v ){ ui.fitChildren = v; } },

			// (Boolean) for Horizontal and Vertical layouts, allow wrap of children into rows
			'wrapEnabled': { get: function (){ return ui.wrapEnabled; }, set: function( v ){ ui.wrapEnabled = v; } },

			// (Integer) for Horizontal and Vertical layouts, auto wrap after this many items per row
			'wrapAfter': { get: function (){ return ui.wrapAfter; }, set: function( v ){ ui.wrapAfter = v; } },

			// (Boolean) for Horizontal and Vertical layouts, force parent to wrap to new row after this element
			'forceWrap': { get: function (){ return ui.forceWrap; }, set: function( v ){ ui.forceWrap = v; } },

			// (LayoutAlign.Default, LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) overrides parent container's layoutAlignX/Y for this object
			'selfAlign': { get: function (){ return ui.selfAlign; }, set: function( v ){ ui.selfAlign = v; } },

			// (Number) stretch this element to fill empty space in vertical and horizontal layouts, 0 = no stretch, otherwise proportion rel. to other flex elems
			'flex': { get: function (){ return ui.flex; }, set: function( v ){ ui.flex = v; } },

			// (Boolean) if true, parent will ignore this element while performing layout
			'fixedPosition': { get: function (){ return ui.fixedPosition; }, set: function( v ){ ui.fixedPosition = v; } },

			// (Boolean) reverse child layout order
			'reversed': { get: function (){ return ui.reversed; }, set: function( v ){ ui.reversed = v; } },

			// (Number) minimum width allowed by layout
			'minWidth': { get: function (){ return ui.minWidth; }, set: function( v ){ ui.minWidth = v; } },

			// (Number) minimum height allowed by layout
			'minHeight': { get: function (){ return ui.minHeight; }, set: function( v ){ ui.minHeight = v; } },

			// (Number) maximum width allowed by layout
			'maxWidth': { get: function (){ return ui.maxWidth; }, set: function( v ){ ui.maxWidth = v; } },

			// (Number) maximum height allowed by layout
			'maxHeight': { get: function (){ return ui.maxHeight; }, set: function( v ){ ui.maxHeight = v; } },

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			'anchorLeft': { get: function (){ return ui.anchorLeft; }, set: function( v ){ ui.anchorLeft = v; } },

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			'anchorRight': { get: function (){ return ui.anchorRight; }, set: function( v ){ ui.anchorRight = v; } },

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			'anchorTop': { get: function (){ return ui.anchorTop; }, set: function( v ){ ui.anchorTop = v; } },

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			'anchorBottom': { get: function (){ return ui.anchorBottom; }, set: function( v ){ ui.anchorBottom = v; } },

			// (Number) offset from anchorLeft
			'left': { get: function (){ return ui.left; }, set: function( v ){ ui.left = v; } },

			// (Number) offset from anchorLeft
			'right': { get: function (){ return ui.right; }, set: function( v ){ ui.right = v; } },

			// (Number) offset from anchorLeft
			'top': { get: function (){ return ui.top; }, set: function( v ){ ui.top = v; } },

			// (Number) offset from anchorLeft
			'bottom': { get: function (){ return ui.bottom; }, set: function( v ){ ui.bottom = v; } },

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
			'margin': { get: function (){ return ui.margin; }, set: function( v ){ ui.margin = v; } },

			// (Number) outer margin top
			'marginTop': { get: function (){ return ui.marginTop; }, set: function( v ){ ui.marginTop = v; }, serialized: false, enumerable: false, configurable: true },

			// (Number) outer margin right
			'marginRight': { get: function (){ return ui.marginRight; }, set: function( v ){ ui.marginRight = v; }, serialized: false },

			// (Number) outer margin bottom
			'marginBottom': { get: function (){ return ui.marginBottom; }, set: function( v ){ ui.marginBottom = v; }, serialized: false },

			// (Number) outer margin left
			'marginLeft': { get: function (){ return ui.marginLeft; }, set: function( v ){ ui.marginLeft = v; }, serialized: false },

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
			'pad': { get: function (){ return ui.pad; }, set: function( v ){ ui.pad = v;} },

			// (Number) inner padding top
			'padTop': { get: function (){ return ui.padTop; }, set: function( v ){ ui.padTop = v; }, serialized: false },

			// (Number) inner padding right
			'padRight': { get: function (){ return ui.padRight; }, set: function( v ){ ui.padRight = v; }, serialized: false },

			// (Number) inner padding bottom
			'padBottom': { get: function (){ return ui.padBottom; }, set: function( v ){ ui.padBottom = v; }, serialized: false },

			// (Number) inner padding left
			'padLeft': { get: function (){ return ui.padLeft; }, set: function( v ){ ui.padLeft = v; }, serialized: false },

			// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
			'spacing': { get: function (){ return ui.spacing; }, set: function( v ){ ui.spacing = v; }, serialized: false },

			// (Number) spacing between children when layoutType is Vertical
			'spacingX': { get: function (){ return ui.spacingX; }, set: function( v ){ ui.spacingX = v; } },

			// (Number) spacing between children when layoutType is Horizontal
			'spacingY': { get: function (){ return ui.spacingY; }, set: function( v ){ ui.spacingY = v; } },

			// (Number) relative X offset from object's layout position
			'offsetX': { get: function (){ return ui.offsetX; }, set: function( v ){ ui.offsetX = v; } },

			// (Number) relative Y offset from object's layout position
			'offsetY': { get: function (){ return ui.offsetY; }, set: function( v ){ ui.offsetY = v; } },

			// (Color) | (Number) .color property of current render component
			'color': { get: function (){ return go.render ? go.render.color : null; }, set: function( v ){ if ( go.render ) go.render.color = v; } },

			// (Color) | (Number) .addColor property of current render component
			'addColor': { get: function (){ return go.render ? go.render.addColor : null; }, set: function( v ){ if ( go.render ) go.render.addColor = v; } },

			// (String) if set, will show tooltip on mouseOver
			'tooltip': {
				get: function (){ return ui.tooltip; },
				set: function( v ){
					ui.tooltip = v;
					if ( v ) {
						go.on( 'mouseOver', UI.base._showTooltip );
					} else {
						go.off( 'mouseOver', UI.base._showTooltip );
					}
				}
			},

			// (Object) used to override style (collection of properties) other than default after creating / during init
			'style': {
				get: function (){ return go.baseStyle; },
				set: function ( v ){
					// merge into current baseStyle
					UI.base.mergeStyle( go.baseStyle, v ) ;
					UI.base.applyProperties( go, v );
					go.state = 'auto'; // reset state
				}, // serialized: false
			},

			// (Object) used to change state of control. Holds definitions from initialization
			'states': { get: function (){ return go._states; }, set: function( v ){ go._states = clone( v ); }, serialized: false },

			// (String) used to change state of control (e.g. 'focus', 'off', 'disabled' etc). Applies properties from object in .states style property
			'state': {
				get: function (){ return go._state ? go._state : 'off'; },
				set: function ( v ){
					if ( !go._states ) return;
					if ( v == 'auto' ) {
						// apply states in order
						UI.base.applyProperties( go, go._states[ v = 'off' ] );
						if ( ui.focused && go._states[ 'focus' ] ) { UI.base.applyProperties( go, go._states[ v = 'focus' ] ); }
						if ( ui.disabled ) {
							v = 'disabled';
							if ( go._states[ 'disabled' ] ) { UI.base.applyProperties( go, go._states[ v ] ); }
						} else {
							if ( ui.down && go._states[ 'down' ] ) { UI.base.applyProperties( go, go._states[ v = 'down' ] ); }
							else if ( ui.over && go._states[ 'over' ] ) { UI.base.applyProperties( go, go._states[ v = 'over' ] ); }
						}
					} else {
						UI.base.applyProperties( go, go._states[ v ] );
					}
					go._state = v;
				}, serialized: false
			},

		};

		// map them to gameObject
		this.mapProperties( go, mappedProps );

		// Shared API functions

		// resize control
		go.resize = function ( w, h ) { ui.resize( w, h ); }

		// set focus to the control (if it accepts focus)
		go.focus = UI.base._focus; //

		// remove focus from control
		go.blur = UI.base._blur; //

		// called from "focusChanged" to scroll this component into view
		go.scrollIntoView = UI.base._scrollIntoView;

	},

	// adds group with properties to __propertyListConfig
	addInspectables: function ( go, groupName, props, propsExtended ) {
		if ( go.__propertyListConfig === undefined ) go.__propertyListConfig = { properties: {}, groups: [] };
		go.__propertyListConfig.groups.push( { name: groupName, properties: props } );
		for ( var p in props ) {
			go.__propertyListConfig.properties[ props[ p ] ] = ( propsExtended ? propsExtended[ props[ p ] ] : true ) || true;
		}
	},

	// sets properties on an object
	applyProperties: function ( go, props ) {
		if ( !( props && go ) ) return;
		for ( var p in props ) {
			if ( p === 'style' ) continue; // last
			// object with same name (but not an array)?
			if ( typeof( props[ p ] ) === 'object' && props[ p ].constructor !== Array &&
				typeof( go[ p ] ) === 'object' && go[ p ] !== null ) {
				// apply properties to it
				UI.base.applyProperties( go[ p ], props[ p ] );
			} else {
				// just set property
				go[ p ] = props[ p ];
			}
		}
		// style is set last
		if ( typeof( props[ 'style' ] ) === 'object' ) go.style = props.style;
	},

	// merges style into base style
	mergeStyle: function ( baseStyle, overrideStyle ) {
		if ( !( baseStyle && overrideStyle ) ) return;
		for ( var p in overrideStyle ) {
			var ot = typeof( overrideStyle[ p ] );
			var bt = typeof( baseStyle[ p ] );
			if ( ot === 'object' && !( overrideStyle[ p ] && overrideStyle[ p ].constructor == Array ) ) {
				if ( bt === 'undefined' ) baseStyle[ p ] = {};
				UI.base.mergeStyle( baseStyle[ p ], overrideStyle[ p ] );
			} else {
				baseStyle[ p ] = overrideStyle[ p ];
			}
		}
		return baseStyle;
	},

	// creates properties with getter/setters
	addMappedProperties: function ( go, mappedProps ) {
		if ( go != global ) go.serializeMask = go.serializeMask ? go.serializeMask : {};
		if ( go.__propertyListConfig === undefined ) go.__propertyListConfig = { properties: {}, groups: [] };
		for ( var i = 0; i < mappedProps.length; i++ ) {
			var hidden = ( mappedProps[ i ].length >= 4 && mappedProps[ i ][ 3 ] );
			Object.defineProperty( go, mappedProps[ i ][ 0 ], {
				get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: !hidden, configurable: true,
			} );
			go.__propertyListConfig.properties[ mappedProps[ i ][ 0 ] ] = false; // hide from inspector
			if ( hidden && go != global ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = true; }
		}
	},

	// creates properties with getter/setters
	mapProperties: function( go, mappedProps ) {
		if ( go !== global ) go.serializeMask = go.serializeMask || [];
		if ( go.__propertyListConfig === undefined ) go.__propertyListConfig = { properties: {}, groups: [] };
		for ( var i in mappedProps ) {
			var prop = mappedProps[ i ];
			prop.configurable = true;
			if ( typeof( prop.enumerable ) === 'undefined' ) prop.enumerable = true;
			go.__propertyListConfig.properties[ i ] = false; // hide from inspector
			if ( prop.serialized === false && go !== global ) {
				if ( go.serializeMask.constructor === Array ) go.serializeMask.push( i );
				else go.serializeMask[ i ] = true;
			}
		}
		Object.defineProperties( go, mappedProps );
	},

	// textfield callback for autocompleting object properties
	// returns:
	// {
	//      suggestions: Array of suggestions to display in a popup, or null to hide popup, e.g. [ { text: "suggestion 1", value: "replace value" } ... ]
	//      replaceStart: position in current field text, from which an accepted suggestion will replace text up to current caret
	// }
	autocompleteObjectProperty: function ( textfield ) {

		var expr = textfield.findExpression( textfield.caretPosition - 1, /[a-z0-9\.$_]/i, null );
		var exprLen = expr.length;
		var lastPeriod = 0;
		var replaceStart = textfield.caretPosition; // if we support partial / midword matches in the future
		var suggestions = [];
		var target = ( textfield.target || global );
		var numeric = /^\d+$/;
		function funcParen( o, p, d ) { return ( typeof ( o[ p ] ) === 'function' ) ? ( d === 2 ? '()' : '(' ) : ''; }

		// ends with . - show all available properties
		if ( expr.substr( -1 ) == '.' ) {

			var obj = eval( expr.substr( 0, exprLen - 1 ), target );
			if ( typeof( obj ) !== 'undefined' && obj !== null && !( typeof( obj ) === 'object' && obj.constructor.name.indexOf( 'Error' ) >= 0 ) ) {
				// add all properties
				var props = Object.getProperties( obj, false, true, true );
				for ( var i = 0, np = props.length; i < np; i++  ) {
					var p = props[ i ];
					if ( p.substr( 0, 2 ) == '__' || numeric.test( p ) ) continue;
					suggestions.push( { text: p + funcParen( obj, p, 2 ), value: p + funcParen( obj, p, 1 ) } );
				}
			}

		// object, ends with partially completed property name - suggest matches
		} else if ( ( lastPeriod = expr.lastIndexOf( '.' ) ) > 0 ) {

			var obj = eval( expr.substr( 0, lastPeriod ), target );
			if ( typeof( obj ) !== 'undefined' && obj !== null && !( typeof( obj ) === 'object' && obj.constructor.name.indexOf( 'Error' ) >= 0 ) ) {

				// all matching beginning of property name
				var prop = expr.substr( lastPeriod + 1 );
				var propLen = prop.length;
				var props = Object.getProperties( obj, false, true, true );
				for ( var i = 0, np = props.length; i < np; i++  ) {
					var p = props[ i ];
					if ( p.substr( 0, propLen ) === prop && p.length !== propLen ) {
						suggestions.push( {
							text: "^B" + prop + "^b" + p.substr( propLen ) + funcParen( obj, p, 2 ),
							value: p.substr( -( p.length + ( lastPeriod - exprLen ) ) - 1 ) + funcParen( obj, p, 1 ) } );
					}
					if ( p.substr( 0, 2 ) == '__' || numeric.test( p ) ) continue;
				}

			}

		// doesn't have . in it yet - predict object from global
		} else if ( exprLen ) {

			// all matching beginning of line
			var props = Object.getProperties( global, false, true, true );
			for ( var i = 0, np = props.length; i < np; i++  ) {
				var p = props[ i ];
				if ( p.substr( 0, exprLen ) === expr && p.length !== exprLen ) {
					suggestions.push( {
						text: "^B" + expr + "^b" + p.substr( exprLen ) + funcParen( global, p, 2 ),
						value: p.substr( exprLen - p.length ) + funcParen( global, p, 1 ) } );
				}
			}

		}

		// return result
		return {
			replaceStart: replaceStart,
			suggestions: suggestions
		};
	},

	// focus rectangle layout callback
	_layoutFocusRect: function ( w, h ) {
		var fr = this.focusRectObject;
		if ( !fr || !fr.resize ) return;
		fr.resize( w + fr.offset * 2, h + fr.offset * 2 );
		fr.setTransform( -fr.offset, -fr.offset );
	},

	// focus rectangle focus change callback
	_focusChangedRect: function ( nf ) {
		if ( this.focusRect ) {
			if ( !this.focusRectObject ) {
				this.focusRectObject = this.gameObject.addChild( 'ui/panel', {
					active: ( this == nf ),
					name: "FocusRect",
					fixedPosition: true,
					style: UI.style.focusRect,
				} );
			} else {
				this.focusRectObject.active = ( this == nf );
			}
		}
	},

	_blur: function () { this.ui.blur(); },

	_focus: function () { if ( this.ui.focusable ) this.ui.focus(); },

	_scrollIntoView: function ( expandAmount ) {
		var lpx = 0, lpy = 0, lw = this.width, lh = this.height;
		// params are used by input to scroll caret into view
		if ( arguments.length != 0 ) {
			lpx = arguments[ 0 ]; lpy = arguments[ 1 ];
			lw = arguments[ 2 ]; lh = arguments[ 3 ];
		}
		// find nearest scrollable
		var p = this.parent;
		var c = this;
		var scrollable = null;
		while( p ) {
			if ( p.ui && p.render && p.render && p.render.image && p.render.image.autoDraw == c ){
				scrollable = p;
				break;
			}
			c = p;
			p = p.parent;
		}
		if ( !scrollable || scrollable[ 'scrollTop' ] === undefined || scrollable[ 'scrollLeft' ] === undefined ) return;

		// convert coordinate to scrollable's system
		var sx = scrollable.scrollLeft, sy = scrollable.scrollTop;
		var sw = scrollable.width, sh = scrollable.height;
		var gp = this.localToGlobal( lpx, lpy );
		var lp = scrollable.globalToLocal( gp.x, gp.y );
		var t = lp.y + scrollable.scrollTop,
			b = t + lh;
		var l = lp.x + scrollable.scrollLeft,
			r = l + lw;

		// expand b-h and l-r a bit
		var m = Math.min( lw, lh ) * ( expandAmount ? expandAmount : 0 );
		if ( lh + m * 2 < sh ) { t -= m; b += m; }
		if ( lw + m * 2 < sw ) { l -= m; r += m; }

		// make sure it's in view
		if ( b > sy + sh && b - t < sh ) { // bottom
			scrollable.scrollTop = b - sh;
		} else if ( t < sy ) { // top
			scrollable.scrollTop = t;
		}
		if ( r > sx + sw && r - l < sw ) { // right
			scrollable.scrollLeft = r - sw;
		} else if ( l < sx ) { // left
			scrollable.scrollLeft = l;
		}
	},

	_showTooltip: function () {
		var show = function() {
			if ( this.ui && this.ui.tooltip ) {
				var t = new GameObject( './tooltip', {
					target: this,
					text: this.ui.tooltip,
					preferredDirection: 'up',
				} );
			}
		}.bind( this );

		var cancel = function () {
			cancelDebouncer( 'showTooltip' );
		};

		// after delay
		debounce( 'showTooltip', show, 2 );
		this.on( 'mouseOut', cancel, true );
	},

}

