/*

	Edit UI.style structure below to change the default appearance of Aviko UI components.

	These are just default properties set at creation and can be overridden. See components
	source files for all available properties. Some shared properties are defined in ui.js
	in "shared functionality" block (below)

    To further customize / use different "substyles" for differently styled components,
    provide .style param. E.g:

    var bigButtonStyle = { .... style properties to init button with .... };
    var smallButtonStyle = { .... style properties to init button with .... };
    container.addChild( 'ui/button', {
        text: "Big Button",
        style: bigButtonStyle
    });
    container.addChild( 'ui/button', {
        text: "Small Button",
        style: smallButtonStyle
    });
 
*/

UI.style = UI.style ? UI.style : {

	// basic container panel - ui/panel.js
	panel: {
		slice: 0,
		layoutType: Layout.Anchors,
		pad: 8,
		spacing: 4,
	},

	// text label - ui/text.js
	text: {
		size: 12,
		textAlign: TextAlign.Left,
		color: 0xFFFFFF,
	},

	// text input field - ui/textfield.js
	textfield: {
		size: 12,
		color: 0x0,
		selectionColor: 0x0073b9,
		slice: 8,
		pad: 8,
		acceptToEdit: true,
		focusRect: true,
		states:{
			off: {
				background: './textures/ui:text-off',
			},
			scroll: {
				background: './textures/ui:text-scroll'
			},
			focus: {
				background: './textures/ui:text-focus',
			},
			disabled: {
				background: './textures/ui:text-disabled'
			},
		}
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
		slice: 6,
		minWidth: 24,
		minHeight: 24,
		pad: 2,
		focusRect: true,
		handle: {
			slice: 8
		},
		states: {
			off: {
				background: './textures/ui:scrollbar-off',
				handle: {
					background: './textures/ui:scrollbar-handle-off',
				}
			},
			focus: {
				background: './textures/ui:scrollbar-focus',
				handle: {
					background: './textures/ui:scrollbar-handle-focus',
				}
			},
			disabled: {
				background: './textures/ui:scrollbar-fisabled',
				handle: {
					background: './textures/ui:scrollbar-handle-disabled',
				}
			},
		},

		// apply only to horizontal
		horizontal: { },

		// apply only to vertical
		vertical: { },
	},

	// image container - ui/image.js
	image: { },

	// button - ui/button.js
	button: {
		slice: 10,
		pad: 8,
		focusRect: true,

		// apply to button's label (ui/text.js)
		label: {
			color: 0x0,
			size: 16
		},

		// apply to icon image (ui/image.js)
		image: {
			width: 20,
			height: 20,
			mode: 'fit'
		},

		states:{
			off: {
				background: './textures/ui:button-off',
			},
			over: {
				background: './textures/ui:button-over',
			},
			focus: {
				background: './textures/ui:button-focus',
			},
			down: {
				background: './textures/ui:button-down',
			},
			disabled: {
				background: './textures/ui:button-disabled',
				label: {
					color: 0x333333
				}
			},
		}
	},

	// checkbox - ui/checkbox.js
	checkbox: {

		slice: 0,
		pad: 0,
		focusRect: true,

		// apply to checkbox label (ui/text.js)
		label: {
			color: 0x0,
			size: 12,
			marginLeft: 8,
			pad: 2,
		},

		// "checkbox" button (ui/button.js)
		checkbox: {
			// "checkmark" image
			icon: './textures/ui:checkbox-check',
		},

		states: {
			off: {
				checkbox: {
					background: './textures/ui:checkbox-off',
				}
			},
			over: {
				checkbox: {
					background: './textures/ui:checkbox-over',
				}
			},
			focus: {
				checkbox: {
					background: './textures/ui:checkbox-focus',
				}
			},
			down: {
				checkbox: {
					background: './textures/ui:checkbox-down',
				}
			},
			disabled: {
				checkbox: {
					background: './textures/ui:checkbox-disabled',
				},
				label: {
					color: 0x333333
				}
			},
		}

	},

	// select-style dropdown menu - ui/dropdown.js
	dropdown: {

		maxVisibleItems: 5,

		// style applied to dropdown button itself - ui/button.js
		button: {
			label: {
				size: 12
			},
			states: {
				off: {
					background: './textures/ui:dropdown-off',
				},
				over: {
					background: './textures/ui:dropdown-over',
				},
				focus: {
					background: './textures/ui:dropdown-focus',
				},
				down: {
					background: './textures/ui:dropdown-down',
				},
				disabled: {
					background: './textures/ui:dropdown-disabled',
					label: {
						color: 0x333333
					}
				},
			}
		},

		// image used for arrow on the right side of the dropdown - ui/image.js
		arrowImage: {
			marginLeft: 4,
			texture: './textures/ui:dropdown-arrow',
		},

		// scrollable container with items (ui/scrollable.js)
		menu: {
			spacingY: 0,
			pad: 0,
		},

		// items in the dropdown list (ui/button.js)
		item: {
			label: {
				size: 12,
				flex: 1
			},
			layoutAlignX: LayoutAlign.Start,
			focusRect: false,
			pad: [ 4, 8, 4, 8 ],
			states:{
				off: {
					background: './textures/ui:item-off',
					label: {
						color: 0x0
					}
				},
				over: {
					background: './textures/ui:item-over',
					label: {
						color: 0xFFFFFF
					}
				},
				focus: {
					background: './textures/ui:item-focus',
					label: {
						color: 0xFFFFFF
					}
				},
				down: {
					background: './textures/ui:item-down',
					label: {
						color: 0xFFFFFF
					}
				},
				disabled: {
					background: './textures/ui:item-disabled',
					label: {
						color: 0x333333
					}
				},
			}
		},

		// currently selected item checkmark (ui/image.js)
		itemCheck: {
			texture: './textures/ui:item-check',
			mode: 'icon',
			marginLeft: 4,
		}

	},

	// property list for editors (ui/property-list.js)
	propertyList: {

		valueWidth: 60, // width of value part. height of rows is set in value.AnyType
		spacingX: 2, // distance between label and value
		spacingY: 2, // distance between rows

		// applied to group label - note that first group's marginTop will be forced to 0
		group: {
			bold: true,
			color: 0x333333,
			align: TextAlign.Left,
			margin: [ 8, 0, 4, 0 ]
		},

		// applied to label
		label: {
			pad: 6,
			align: TextAlign.Right,
			color: 0x0,
		},

		// applied to description
		description: {

		},

		// applied to field value based on type
		value: {
			// all types
			any: {
				height: 20,
				acceptToEdit: true,
				cancelToBlur: false,
				blurOnClickOutside: false,
			},
			// Number - ui/textfield
			number: {
				numeric: true,
				selectAllOnFocus: true,
			},
			// String - ui/textfield
			string: {

			},
			// Object - ui/button
			object: {

			},
			// Enum - ui/dropdown
			enum: {

			}
		}

	},

	// settings for focus rectangle (ui/panel.js)
	focusRect: {

		// default color
		background: 0x0073b9,

		// rounded corner
		cornerRadius: 2,

		// stipple rendering
		render: { stipple: 0.5 },

		// outline thickness
		lineThickness: 2,

		// filled = solid rectangle
		filled: false,

		// pixels outside control
		offset: 2

	}




};


/*

Shared functionality between Aviko ui components

*/

UI.base = UI.base ? UI.base : {

	// layout-specific properties shared between UI components
	addSharedProperties: function( go, ui ) {

		// create properties on gameObject
		var mappedProps = [

			// (Number) current width of the control
			[ 'width',  function (){ return ui.width; }, function ( w ){ ui.width = w; } ],

			// (Number) current height of the control
			[ 'height',  function (){ return ui.height; }, function ( h ){ ui.height = h; } ],

			// (Boolean) allows keyboard / navigation focus
			[ 'focusable',  function (){ return ui.focusable; }, function ( v ){ ui.focusable = v; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusLeft',  function (){ return ui.focusLeft; }, function ( f ){ ui.focusLeft = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusRight',  function (){ return ui.focusRight; }, function ( f ){ ui.focusRight = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusUp',  function (){ return ui.focusUp; }, function ( f ){ ui.focusUp = f; } ],

			// (UI) or (GameObject) or null - object to focus to the left of this control
			[ 'focusDown',  function (){ return ui.focusDown; }, function ( f ){ ui.focusDown = f; } ],

			// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
			[ 'focusGroup',  function (){ return ui.focusGroup; }, function ( f ){ ui.focusGroup = f; } ],

			// (Boolean) - enables/disables focus rectangle (getter returns Render component)
			[ 'focusRect',  function (){ return ui.focusRect ? ui.focusRect.render : null; }, function ( fr ){
				if ( fr === null || fr === false ) {
					// remove
					if ( ui.focusRect ) {
						ui.focusRect.parent = null;
						ui.focusRect = null;
						ui.off( 'layout', layoutFocusRect );
						ui.off( 'focusChanged', focusChangedRect );
					}
				// add
				} else if ( !ui.focusRect ) {
					ui.focusRect = new GameObject( './panel', {
						active: false,
						fixedPosition: true,
						style: UI.style.focusRect,
					} );
					ui.on( 'layout', layoutFocusRect );
					ui.on( 'focusChanged', focusChangedRect );
					go.addChild( ui.focusRect, 0 );
				}
			} ],

			// (Layout.None, Layout.Anchors, Layout.Vertical, Layout.Horizontal, Layout.Grid) - how to lay out children
			[ 'layoutType',  function (){ return ui.layoutType; }, function ( v ){ ui.layoutType = v; } ],

			// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on X axis
			[ 'layoutAlignX',  function (){ return ui.layoutAlignX; }, function ( v ){ ui.layoutAlignX = v; } ],

			// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on Y axis
			[ 'layoutAlignY',  function (){ return ui.layoutAlignY; }, function ( v ){ ui.layoutAlignY = v; } ],

			// (Boolean) for Horizontal, Vertical, and Grid layout types, adjust own height and width to fit all children
			[ 'fitChildren',  function (){ return ui.fitChildren; }, function ( v ){ ui.fitChildren = v; } ],

			// (Boolean) for Horizontal and Vertical layouts, allow wrap of children into rows
			[ 'wrapEnabled',  function (){ return ui.wrapEnabled; }, function ( v ){ ui.wrapEnabled = v; } ],

			// (Integer) for Horizontal and Vertical layouts, auto wrap after this many items per row
			[ 'wrapAfter',  function (){ return ui.wrapAfter; }, function ( v ){ ui.wrapAfter = v; } ],

			// (Boolean) for Horizontal and Vertical layouts, force parent to wrap to new row after this element
			[ 'forceWrap',  function (){ return ui.forceWrap; }, function ( v ){ ui.forceWrap = v; } ],

			// (LayoutAlign.Default, LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) overrides parent container's layoutAlignX/Y for this object
			[ 'selfAlign',  function (){ return ui.selfAlign; }, function ( v ){ ui.selfAlign = v; } ],

			// (Number) stretch this element to fill empty space in vertical and horizontal layouts, 0 = no stretch, otherwise proportion rel. to other flex elems
			[ 'flex',  function (){ return ui.flex; }, function ( v ){ ui.flex = v; } ],

			// (Boolean) if true, parent will ignore this element while performing layout
			[ 'fixedPosition',  function (){ return ui.fixedPosition; }, function ( v ){ ui.fixedPosition = v; } ],

			// (Number) minimum width allowed by layout
			[ 'minWidth',  function (){ return ui.minWidth; }, function ( v ){ ui.minWidth = v; } ],

			// (Number) minimum height allowed by layout
			[ 'minHeight',  function (){ return ui.minHeight; }, function ( v ){ ui.minHeight = v; } ],

			// (Number) maximum width allowed by layout
			[ 'maxWidth',  function (){ return ui.maxWidth; }, function ( v ){ ui.maxWidth = v; } ],

			// (Number) maximum height allowed by layout
			[ 'maxHeight',  function (){ return ui.maxHeight; }, function ( v ){ ui.maxHeight = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorLeft',  function (){ return ui.anchorLeft; }, function ( v ){ ui.anchorLeft = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorRight',  function (){ return ui.anchorRight; }, function ( v ){ ui.anchorRight = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorTop',  function (){ return ui.anchorTop; }, function ( v ){ ui.anchorTop = v; } ],

			// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
			[ 'anchorBottom',  function (){ return ui.anchorBottom; }, function ( v ){ ui.anchorBottom = v; } ],

			// (Number) offset from anchorLeft
			[ 'left',  function (){ return ui.left; }, function ( v ){ ui.left = v; } ],

			// (Number) offset from anchorLeft
			[ 'right',  function (){ return ui.right; }, function ( v ){ ui.right = v; } ],

			// (Number) offset from anchorLeft
			[ 'top',  function (){ return ui.top; }, function ( v ){ ui.top = v; } ],

			// (Number) offset from anchorLeft
			[ 'bottom',  function (){ return ui.bottom; }, function ( v ){ ui.bottom = v; } ],

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
			[ 'margin',  function (){ return ui.margin; }, function ( v ){ ui.margin = v; } ],

			// (Number) outer margin top
			[ 'marginTop',  function (){ return ui.marginTop; }, function ( v ){ ui.marginTop = v; }, true ],

			// (Number) outer margin right
			[ 'marginRight',  function (){ return ui.marginRight; }, function ( v ){ ui.marginRight = v; }, true ],

			// (Number) outer margin bottom
			[ 'marginBottom',  function (){ return ui.marginBottom; }, function ( v ){ ui.marginBottom = v; }, true ],

			// (Number) outer margin left
			[ 'marginLeft',  function (){ return ui.marginLeft; }, function ( v ){ ui.marginLeft = v; }, true ],

			// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
			[ 'pad',  function (){ return ui.pad; }, function ( v ){ ui.pad = v; } ],

			// (Number) inner padding top
			[ 'padTop',  function (){ return ui.padTop; }, function ( v ){ ui.padTop = v; }, true ],

			// (Number) inner padding right
			[ 'padRight',  function (){ return ui.padRight; }, function ( v ){ ui.padRight = v; }, true ],

			// (Number) inner padding bottom
			[ 'padBottom',  function (){ return ui.padBottom; }, function ( v ){ ui.padBottom = v; }, true ],

			// (Number) inner padding left
			[ 'padLeft',  function (){ return ui.padLeft; }, function ( v ){ ui.padLeft = v; }, true ],

			// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
			[ 'spacing',  function (){ return ui.spacing; }, function ( v ){ ui.spacing = v; }, true ],

			// (Number) spacing between children when layoutType is Vertical
			[ 'spacingX',  function (){ return ui.spacingX; }, function ( v ){ ui.spacingX = v; } ],

			// (Number) spacing between children when layoutType is Horizontal
			[ 'spacingY',  function (){ return ui.spacingY; }, function ( v ){ ui.spacingY = v; } ],

			// (Object) used to apply style (collection of properties) other than default after creating / during init
			[ 'style',  function (){ return null; }, function ( v ){ UI.base.applyProperties( go, v ); }, true ],

			// (Object) used to change state of control. Holds definitions from initialization
			[ 'states',  function (){ return go._states; }, function ( v ){ go._states = clone( v ); }, true ],

			// (String) used to change state of control (e.g. 'focus', 'off', 'disabled' etc). Applies properties from object in .states style property
			[ 'state',  function (){ return go._state ? go._state : 'off'; }, function ( v ){
				if ( v == 'auto' ) v = ( ui.disabled ? 'disabled' : ( ui.focused ? 'focus' : ( ui.over ? 'over' : 'off' ) ) );
				go._state = v;
				if ( go._states !== undefined && go._states[ v ] !== undefined ) UI.base.applyProperties( go, go._states[ v ] );
			}, true ],
		];
		// map them to gameObject
		go.serializeMask = go.serializeMask ? go.serializeMask : {};
		this.addMappedProperties( go, mappedProps );

		// Shared API functions

		// set focus to the control (if it accepts focus)
		go[ 'focus' ] = function () { ui.focus(); }

		// remove focus from control
		go[ 'blur' ] = function () { ui.blur(); }

		// resize control
		go[ 'resize' ] = function ( w, h ) { ui.resize( w, h ); }

		// called from "focusChanged" to scroll this component into view
		go[ 'scrollIntoView' ] = function() {
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
			var gp = this.localToGlobal( lpx, lpy );
			var lp = scrollable.globalToLocal( gp.x, gp.y );
			var t = lp.y + scrollable.scrollTop,
				b = t + lh;
			var l = lp.x + scrollable.scrollLeft,
				r = l + lw;

			// make sure it's in view
			if ( b > sy + scrollable.height ) { // bottom
				scrollable.scrollTop = b - scrollable.height;
			} else if ( t < sy ) { // top
				scrollable.scrollTop = t;
			}
			if ( r > sx + scrollable.width ) { // right
				scrollable.scrollLeft = r - scrollable.width;
			} else if ( l < sx ) { // left
				scrollable.scrollLeft = l;
			}
		}

		// focus rectangle layout callback
		function layoutFocusRect ( w, h ) {
			var fr = this.focusRect;
			if ( !fr ) return;
			fr.resize( w + fr.offset * 2, h + fr.offset * 2 );
			fr.setTransform( -fr.offset, -fr.offset );
		};

		// focus rectangle focus change callback
		function focusChangedRect( nf ) {
			if ( this.focusRect.active = (nf == this) ){
				this.focusRect.dispatch( 'layout' );
			}
		};

	},

	// sets properties on an object
	applyProperties: function ( go, props ) {
		if ( !props || !go ) return;
		for ( var p in props ) {
			// object with same name?
			if ( typeof( props[ p ] ) == 'object' && typeof( go[ p ] ) == 'object' ) {
				// apply properties to it
				UI.base.applyProperties( go[ p ], props[ p ] );
			} else {
				// just set property
				go[ p ] = props[ p ];
			}
		}
	},

	// creates properties with getter/setters
	addMappedProperties: function ( go, mappedProps ) {
		go.serializeMask = go.serializeMask ? go.serializeMask : {};
		for ( var i = 0; i < mappedProps.length; i++ ) {
			Object.defineProperty( go, mappedProps[ i ][ 0 ], {
				get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
			} );
			if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
		}
	}

}
