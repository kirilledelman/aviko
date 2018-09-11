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
		size: 14,
		textAlign: TextAlign.Left,
		color: 0xFFFFFF,
		wrap: false,
	},

	// text input field - ui/textfield.js
	textfield: {
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
	
	componentPrototype: {

		__proto__: GameObject.prototype,
		
		// (Number) current width of the control
		get width(){ return this.ui.width; }, set width( w ){ this.ui.width = w; },

		// (Number) current height of the control
		get height(){ return this.ui.height; }, set height( h ){ this.ui.height = h; },

		// (Boolean) allows keyboard / navigation focus
		get focusable(){ return this.ui.focusable; }, set focusable( v ){ this.ui.focusable = v; },

		// (Boolean) is field currently focused
		get focused(){ return this.ui.focused; },

		// (UI) or (GameObject) or null - object to focus to the left of this control
		get focusLeft(){ return this.ui.focusLeft; }, set focusLeft( f ) { this.ui.focusLeft = f; },

		// (UI) or (GameObject) or null - object to focus to the left of this control
		get focusRight(){ return this.ui.focusRight; }, set focusRight( f ){ this.ui.focusRight = f; },
	
		// (UI) or (GameObject) or null - object to focus to the left of this control
		get focusUp(){ return this.ui.focusUp; }, set focusUp( f ){ this.ui.focusUp = f; },

		// (UI) or (GameObject) or null - object to focus to the left of this control
		get focusDown(){ return this.ui.focusDown; }, set focusDown( f ){ this.ui.focusDown = f; },

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		get focusGroup(){ return this.ui.focusGroup; }, set focusGroup( f ){ this.ui.focusGroup = f; },

		// (Boolean) - enables/disables focus rectangle (getter returns Render component)
		get focusRect(){ return this.ui.focusRect; },
		set focusRect( fr ){
			if ( fr ) {
				this.ui.on( 'layout', UI.base._layoutFocusRect );
				this.ui.on( 'focusChanged', UI.base._focusChangedRect );
			} else {
				this.ui.off( 'layout', UI.base._layoutFocusRect );
				this.ui.off( 'focusChanged', UI.base._focusChangedRect );
				if ( this.ui.focusRectPanel ) {
					this.ui.focusRectPanel = this.ui.focusRectPanel.parent = null;
				}
			}
			this.ui.focusRect = fr;
		},

		// (Boolean) blocks mouse events from reaching objects higher in in hierarchy tree
		get blocking(){ return this.ui.blocking; }, set blocking( v ){ this.ui.blocking = v; },

		// (Boolean) mouse events are disabled
		get mouseDisabled(){ return this.ui.mouseDisabled; }, set mouseDisabled( v ){ this.ui.mouseDisabled = v; },

		// (Layout.None, Layout.Anchors, Layout.Vertical, Layout.Horizontal, Layout.Grid) - how to lay out children
		get layoutType(){ return this.ui.layoutType; }, set layoutType( v ){ this.ui.layoutType = v; },

		// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on X axis
		get layoutAlignX(){ return this.ui.layoutAlignX; }, set layoutAlignX( v ){ this.ui.layoutAlignX = v; },

		// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on Y axis
		get layoutAlignY(){ return this.ui.layoutAlignY; }, set layoutAlignY( v ){ this.ui.layoutAlignY = v; },

		// (Boolean) for Horizontal, Vertical, and Grid layout types, adjust own height and width to fit all children
		get fitChildren(){ return this.ui.fitChildren; }, set fitChildren( v ){ this.ui.fitChildren = v; },

		// (Boolean) for Horizontal and Vertical layouts, allow wrap of children into rows
		get wrapEnabled(){ this.ui.wrapEnabled; }, set wrapEnabled( v ){ this.ui.wrapEnabled = v; },

		// (Integer) for Horizontal and Vertical layouts, auto wrap after this many items per row
		get wrapAfter(){ return this.ui.wrapAfter; }, set wrapAfter( v ){ this.ui.wrapAfter = v; },

		// (Boolean) for Horizontal and Vertical layouts, force parent to wrap to new row after this element
		get forceWrap(){ return this.ui.forceWrap; }, set forceWrap( v ){ this.ui.forceWrap = v; },

		// (LayoutAlign.Default, LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) overrides parent container's layoutAlignX/Y for this object
		get selfAlign(){ return this.ui.selfAlign; }, set selfAlign( v ){ this.ui.selfAlign = v; },

		// (Number) stretch this element to fill empty space in vertical and horizontal layouts, 0 = no stretch, otherwise proportion rel. to other flex elems
		get flex(){ return this.ui.flex; }, set flex( v ){ this.ui.flex = v; },

		// (Boolean) if true, parent will ignore this element while performing layout
		get fixedPosition(){ return this.ui.fixedPosition; }, set fixedPosition( v ){ this.ui.fixedPosition = v; },

		// (Boolean) reverse child layout order
		get reversed(){ return this.ui.reversed; }, set reversed( v ){ this.ui.reversed = v; },

		// (Number) minimum width allowed by layout
		get minWidth(){ return this.ui.minWidth; }, set minWidth( v ){ this.ui.minWidth = v; },

		// (Number) minimum height allowed by layout
		get minHeight(){ return this.ui.minHeight; }, set minHeight( v ){ this.ui.minHeight = v; },

		// (Number) maximum width allowed by layout
		get maxWidth(){ return this.ui.maxWidth; }, set maxWidth( v ){ this.ui.maxWidth = v; },

		// (Number) maximum height allowed by layout
		get maxHeight(){ return this.ui.maxHeight; }, set maxHeight( v ){ this.ui.maxHeight = v; },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		get anchorLeft(){ return this.ui.anchorLeft; }, set anchorLeft( v ){ this.ui.anchorLeft = v; },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		get anchorRight(){ return this.ui.anchorRight; }, set anchorRight( v ){ this.ui.anchorRight = v; },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		get anchorTop(){ return this.ui.anchorTop; }, set anchorTop( v ){ this.ui.anchorTop = v; },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		get anchorBottom(){ return this.ui.anchorBottom; }, set anchorBottom( v ){ this.ui.anchorBottom = v; },

		// (Number) offset from anchorLeft
		get left(){ return this.ui.left; }, set left( v ){ this.ui.left = v; },

		// (Number) offset from anchorLeft
		get right(){ return this.ui.right; }, set right( v ){ this.ui.right = v; },

		// (Number) offset from anchorLeft
		get top(){ return this.ui.top; }, set top( v ){ this.ui.top = v; },

		// (Number) offset from anchorLeft
		get bottom(){ return this.ui.bottom; }, set bottom( v ){ this.ui.bottom = v; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
		get margin(){ return this.ui.margin; }, set margin( v ){ this.ui.margin = v; },

		// (Number) outer margin top
		get marginTop(){ return this.ui.marginTop; }, set marginTop( v ){ this.ui.marginTop = v; },

		// (Number) outer margin right
		get marginRight(){ return this.ui.marginRight; }, set marginRight( v ){ this.ui.marginRight = v; },

		// (Number) outer margin bottom
		get marginBottom(){ return this.ui.marginBottom; }, set marginBottom( v ){ this.ui.marginBottom = v; },

		// (Number) outer margin left
		get marginLeft(){ return this.ui.marginLeft; }, set marginLeft( v ){ this.ui.marginLeft = v; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		get pad(){ return this.ui.pad; }, set pad( v ){ this.ui.pad = v; },

		// (Number) inner padding top
		get padTop(){ return this.ui.padTop; }, set padTop( v ){ this.ui.padTop = v; },

		// (Number) inner padding right
		get padRight(){ return this.ui.padRight; }, set padRight( v ){ this.ui.padRight = v; },

		// (Number) inner padding bottom
		get padBottom(){ return this.ui.padBottom; }, set padBottom( v ){ this.ui.padBottom = v; },

		// (Number) inner padding left
		get padLeft (){ return this.ui.padLeft; }, set padLeft( v ){ this.ui.padLeft = v; },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		get spacing(){ return this.ui.spacing; }, set spacing( v ){ this.ui.spacing = v; },

		// (Number) spacing between children when layoutType is Vertical
		get spacingX(){ return this.ui.spacingX; }, set spacingX( v ){ this.ui.spacingX = v; },

		// (Number) spacing between children when layoutType is Horizontal
		get spacingY(){ return this.ui.spacingY; }, set spacingY( v ){ this.ui.spacingY = v; },

		// (Number) relative X offset from object's layout position
		get offsetX(){ return this.ui.offsetX; }, set offsetX( v ){ this.ui.offsetX = v; },

		// (Number) relative Y offset from object's layout position
		get offsetY(){ return this.ui.offsetY; }, set offsetY( v ){ this.ui.offsetY = v; },

		// (Color) | (Number) .color property of current render component
		get color(){ return this.render ? this.render.color : null; }, set color( v ){ if ( this.render ) this.render.color = v; },

		// (Color) | (Number) .addColor property of current render component
		get addColor(){ return this.render ? this.render.addColor : null; }, set addColor( v ){ if ( this.render ) this.render.addColor = v; },

		// (String) if set, will show tooltip on mouseOver
		get tooltip(){ return this.ui.tooltip ? this.ui.tooltip : ""; },
		set tooltip( v ){
			this.ui.tooltip = v;
			if ( v ) {
				this.ui.on( 'mouseOver', UI.base._showTooltip );
			} else {
				this.ui.off( 'mouseOver', UI.base._showTooltip );
			}
		},

		// (Object) used to override style (collection of properties) other than default after creating / during init
		get style(){ return this.__baseStyle; },
		set style( v ){
			// merge into current baseStyle
			UI.base.mergeStyle( this.__baseStyle, v ) ;
			UI.base.applyProperties( this, v );
			this.state = 'auto'; // reset state
		},

		// (String) used to change state of control (e.g. 'focus', 'off', 'disabled' etc).
		// Applies properties from corresponding object in .states property
		get state(){ return this.__state ? this.__state : 'off'; },
		set state( v ){
			this.__state = v;
			if ( !this.__states ) return;
			if ( v == 'auto' ) {
				// apply states in order
				UI.base.applyProperties( this, this.__states[ v = 'off' ] );
				if ( this.ui.focused && this.__states[ 'focus' ] ) { UI.base.applyProperties( this, this.__states[ v = 'focus' ] ); }
				if ( this.ui.disabled ) {
					v = 'disabled';
					if ( this.__states[ 'disabled' ] ) { UI.base.applyProperties( this, this.__states[ v ] ); }
				} else {
					if ( this.ui.down && this.__states[ 'down' ] ) { UI.base.applyProperties( this, this.__states[ v = 'down' ] ); }
					else if ( this.ui.over && this.__states[ 'over' ] ) { UI.base.applyProperties( this, this.__states[ v = 'over' ] ); }
				}
			} else {
				UI.base.applyProperties( this, this.__states[ v ] );
			}
		},

		// (Object) definitions for each state
		get states(){ return this.__states; }, set states( v ){ this.__states = v ? clone( v ) : v; },

		resize: function ( w, h ) { this.ui.resize( w, h ); },
	
		blur: function () { this.ui.blur(); },
	
		focus: function () { if ( this.ui.focusable ) this.ui.focus(); },
	
		scrollIntoView: function ( expandAmount ) {
			var lpx = 0, lpy = 0, lw, lh;
			if ( typeof( this.width ) == 'undefined' && this.ui ){
				lw = this.ui.width; lh = this.ui.height;
			} else {
				lw = this.width; lh = this.height;
			}
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
		
		// call this to initialize __propertyListConfig and serializeMask / cloneMask
		__init: function () {
		
			this.serializeMask = [
				'states', 'state', 'ui', 'render', 'body', 'parent', 'style',
				'marginTop', 'marginLeft', 'marginBottom', 'marginRight', 'spacing',
				'padTop', 'padLeft', 'padBottom', 'padRight',
				'focus', 'blur', 'resize', 'scrollIntoView' ];
			
			this.__propertyListConfig = {
				groups: [ { name: 'UI',
					properties: [
					'focusGroup',
					'layoutType', 'layoutAlignX', 'layoutAlignY', 'selfAlign', 'reversed',
					'spacingX', 'spacingY', 'fixedPosition', 'fitChildren', 'wrapEnabled', 'wrapAfter', 'forceWrap',
					'width', 'height', 'minWidth', 'minHeight', 'maxWidth', 'maxHeight', 'flex',
					'anchorLeft', 'anchorTop', 'anchorRight', 'anchorBottom', 'left', 'top', 'right', 'bottom',
					'padLeft', 'padTop', 'padRight', 'padBottom', 'marginLeft', 'marginTop', 'marginRight', 'marginBottom',
					'tooltip' ] } ],
				properties: {
					'layoutType': { enum: UI.base._enumUILayout },
					'layoutAlignX': { enum: UI.base._enumUILayoutAlign },
					'layoutAlignY': { enum: UI.base._enumUILayoutAlign },
					'selfAlign': { enum: UI.base._enumUILayoutAlign },
					'background': {
						inline: true,
						validate: function( v ){ var isNum = parseInt( v, 16 ); if( isNaN( isNum ) ) return v; return isNum; },
						reloadOnChange: [ 'cornerRadius', 'outlineColor', 'lineThickness', 'filled', 'sliceTop', 'sliceRight', 'sliceBottom', 'sliceLeft', ],
					},
					'outlineColor': { inline: true, hidden: UI.base._isBackgroundNotShape, },
				    'cornerRadius': { inline: true, hidden: UI.base._isBackgroundNotShape,  },
					'lineThickness': { inline: true, hidden: UI.base._isBackgroundNotShape,  },
					'filled': { inline: true, hidden: UI.base._isBackgroundNotShape,  },
					'sliceTop': { inline: true, hidden: UI.base._isBackgroundNotTexture,  },
					'sliceRight': { inline: true, hidden: UI.base._isBackgroundNotTexture,  },
					'sliceBottom': { inline: true, hidden: UI.base._isBackgroundNotTexture,  },
					'sliceLeft': { inline: true, hidden: UI.base._isBackgroundNotTexture,  },
				}
			};
			
			// log( this, "constructor()" );
		},
		
	},
	
	// helpers for property-list config
	_enumUILayout: [ { text: "None", value: Layout.None }, { text: "Horizontal", value: Layout.Horizontal }, { text: "Vertical", value: Layout.Vertical }, { text: "Anchors", value: Layout.Anchors }, ],
	_enumUILayoutAlign: [ { text: "Default", value: LayoutAlign.Default }, { text: "Start", value: LayoutAlign.Start }, { text: "Center", value: LayoutAlign.Center }, { text: "End", value: LayoutAlign.End }, { text: "Stretch", value: LayoutAlign.Stretch }, ],
	_isBackgroundNotShape: function(go){ return (go.background === false || ( go.render && go.render.constructor != RenderShape ) ); },
	_isBackgroundNotTexture: function(go){ return (go.background === false || ( go.render && go.render.constructor != RenderSprite ) ); },
	
	// adds group with properties to __propertyListConfig ( used by ui components to add component-specific properties )
	// go - object
	// groupName - group to add (or replace if group w same name exists in GameObject.prototype.__propertyListConfig)
	// props: array of property names, in order
	// propsExtended: (optional) object with property definitions to override
	// pos: (optional) position where this group should be inserted
	addInspectables: function ( go, groupName, props, propsExtended, pos ) {
		// add config if none
		if ( typeof( go.__propertyListConfig ) === 'undefined' ) go.__propertyListConfig = { properties: {}, groups: [] };
		// find group with same name
		var group = null;
		for ( var i = 0, ng = go.__propertyListConfig.groups.length; i < ng; i++ ) {
			if ( go.__propertyListConfig.groups[ i ].name === groupName ) { group = go.__propertyListConfig.groups[ i ]; break; }
		}
		// add group or append props
		if ( group === null ) go.__propertyListConfig.groups.push( group = { name: groupName, properties: props, pos: pos } );
		else {
			group.pos = pos;
			for ( var i = 0, np = props.length; i < np; i++ ) if ( group.properties.indexOf( props[ i ] ) < 0 ) props.push( props[ i ] );
		}
		// write property defs
		for ( var p in props ) {
			if ( typeof( go.__propertyListConfig.properties[ props[ p ] ] ) === 'undefined' ) {
				go.__propertyListConfig.properties[ props[ p ] ] = ( propsExtended ? propsExtended[ props[ p ] ] : true ) || true;
			}
		}
		for ( var p in propsExtended ) {
			if ( typeof( go.__propertyListConfig.properties[ p ] ) === 'undefined' ) {
				go.__propertyListConfig.properties[ p ] = propsExtended[ p ];
			}
		}
	},

	// sets properties on an object recursively
	applyProperties: function ( go, props ) {
		if ( !( props && go ) ) return;
		for ( var p in props ) {
			var goType = typeof( go[ p ] );
			var pType = typeof( props[ p ] );
			if ( p === 'style' ) continue; // last
			// object with same name (but not an array)?
			if ( pType === 'object' && props[ p ].constructor !== Array &&
				goType === 'object' && go[ p ] !== null ) {
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
	mapProperties: function( go, mappedProps ) {
		if ( go !== global ) go.serializeMask = go.serializeMask || [];
		if ( go.__propertyListConfig === undefined ) go.__propertyListConfig = { properties: {}, groups: [] };
		for ( var i in mappedProps ) {
			var prop = mappedProps[ i ];
			prop.configurable = true;
			if ( typeof( prop.enumerable ) === 'undefined' ) prop.enumerable = true;
			if ( go.__propertyListConfig.properties[ i ] === undefined ) {
				go.__propertyListConfig.properties[ i ] = false; // hide from inspector by default
			}
			if ( ( prop.serialized === false || prop.set === undefined ) && go !== global ) {
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
		var target = ( textfield.autocompleteParam || global );
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

	// textfield callback for autocompleting file path
	// textfield.autocompleteParam in form 'startdirectory;extension1,extension2...'
	autocompleteFilePath: function ( textfield ) {
		
		// list files matching text in box
		var tft = textfield.text;
		var exts = [];
		var startPath = '';
		if ( typeof( textfield.autocompleteParam ) === 'string' ) {
			var parts = textfield.autocompleteParam.split( ';' );
			if ( parts.length > 1 ) {
				startPath = parts[ 0 ];
				exts = parts[ 1 ].split( ',' );
			} else exts = parts[ 0 ].split( ',' );
		}
		var files = listDirectory( tft, exts, startPath );
		var suggestions = [];
		var lastSlash = tft.lastIndexOf( '/' );
		var tail = lastSlash >= 0 ? tft.substr( lastSlash + 1 ) : tft;
		for ( var i = 0; i < files.length; i++ ){
			var f = files[ i ];
			suggestions.push( {
				text: "^B" + f.substr( 0, tail.length ) + "^b" + f.substr( tail.length ),
				value: f.substr( tail.length ) } );
		}

		// return result
		return {
			replaceStart: textfield.caretPosition,
			suggestions: suggestions
		};

	},

	// textfield callback for autocompleting texture path, with optional ":frameName" at end
	autocompleteTexturePath: function ( textfield ) {

		// TODO
		var tft = textfield.text;
		
		// if contains ":"
		var colonPos = tft.lastIndexOf( ':' );
		if ( colonPos == -1 ) {
			// set default autocompleteParam
			if ( typeof( textfield.autocompleteParam ) === 'undefined' ) {
				textfield.autocompleteParam = 'textures;png,jpg,jpeg';
			}
			///  return UI.base.auto
		}
		
		// list files matching text in box
		var exts = [];
		var startPath = '';
		if ( typeof( textfield.autocompleteParam ) === 'string' ) {
			var parts = textfield.autocompleteParam.split( ';' );
			if ( parts.length > 1 ) {
				startPath = parts[ 0 ];
				exts = parts[ 1 ].split( ',' );
			} else exts = parts[ 0 ].split( ',' );
		}
		var files = listDirectory( tft, exts, startPath );
		var suggestions = [];
		var lastSlash = tft.lastIndexOf( '/' );
		var tail = lastSlash >= 0 ? tft.substr( lastSlash + 1 ) : tft;
		for ( var i = 0; i < files.length; i++ ){
			var f = files[ i ];
			suggestions.push( {
				text: "^B" + f.substr( 0, tail.length ) + "^b" + f.substr( tail.length ),
				value: f.substr( tail.length ) } );
		}

		// return result
		return {
			replaceStart: textfield.caretPosition,
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
					serializeable: false
				} );
			} else {
				this.focusRectObject.active = ( this == nf );
			}
		}
	},

	_resize: function ( w, h ) { this.ui.resize( w, h ); },
	
	_blur: function () { this.ui.blur(); },

	_focus: function () { if ( this.ui.focusable ) this.ui.focus(); },

	_scrollIntoView: function ( expandAmount ) {
		var lpx = 0, lpy = 0, lw, lh;
		if ( typeof( this.width ) == 'undefined' && this.ui ){
			lw = this.ui.width; lh = this.ui.height;
		} else {
			lw = this.width; lh = this.height;
		}
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
			if ( this.tooltip ) {
				var t = new GameObject( './tooltip', {
					target: this.gameObject,
					text: this.tooltip,
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

