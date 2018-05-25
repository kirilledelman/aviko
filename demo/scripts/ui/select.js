/*

	Dropdown menu

	Used to select a single value from a list.
	Uses ui/button and ui/scrollable components for functionality.

	Usage:

		App.scene.addChild( 'ui/select', {
		items: [
			{ value: "First", text: "Item 1", icon: "optional_icon" },
			{ value: "Second", text: "Item 2", icon: "optional_icon_2" },
			{ value: "Third", text: "Item 3", icon: "optional_icon_3", disabled: true },
		],
		value: "Second",
		change: function ( v ) {
			log( "Value selected:", v );
		}
	} );

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'change' - a new value was selected
		'focusChanged' - when control focus is set or cleared (same as UI event)

*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI();
	var button;
	var value = undefined;
	var selectedIndex = -1;
	var items = [];
	var arrowImage;
	var dropdown;
	var constructing = true;
	var maxVisibleItems = 10;
	go.serializeMask = [ 'ui', 'render', 'children' ];

	// API properties
	var mappedProps = {

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		'items': { get: function (){ return items; }, set: function( v ){
				items = v;
				go.value = value;
				go.selectedIndex = selectedIndex;
				go.updateSelectedItem();
			}  },

		// (*) 'value' property of selected item
		'value': { get: function (){ return value; }, set: function( v ){
				value = v;
				// find matching value in items
				selectedIndex = -1;
				for ( var i in items ) {
					if ( items[ i ].value === v ) {
						selectedIndex = i;
					}
				}
				go.updateSelectedItem();
			}  },

		// (Number) 0 based index of item selected in menu
		'selectedIndex': {
			get: function (){ return selectedIndex; },
			set: function( v ){
				if ( isNaN( v ) ) return;
				selectedIndex = Math.max( -1, Math.min( Math.floor( v ), items.length ) );
				value = selectedIndex > 0 ? items[ selectedIndex ] : value;
				go.updateSelectedItem();
			}
		},

		// (*) 'value' property of selected item
		'maxVisibleItems': { get: function (){ return maxVisibleItems; }, set: function( v ){ maxVisibleItems = v; }  },

		// (GameObject) instance of 'ui/button.js' used as main area
		'button': { get: function (){ return button; } },

		// (GameObject) instance of 'ui/image.js' used for dropdown icon
		'arrowImage': { get: function (){ return arrowImage; } },

		// (String) or null - texture on icon
		'arrowIcon': {
			get: function (){ return arrowImage.texture; },
			set: function( v ){
				arrowImage.image.texture = v;
				arrowImage.active = !!v;
			}
		},

		// (Boolean) input disabled
		'disabled': { get: function (){ return button.disabled; }, set: function( v ){ ui.disabled = button.disabled = v; }  },

		// (Boolean) whether control is focusable when it's disabled
		'disabledCanFocus': { get: function (){ return button.disabledCanFocus; }, set: function ( f ){ button.disabledCanFocus = f; } },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		'cancelToBlur': { get: function (){ return button.cancelToBlur; }, set: function( cb ){ button.cancelToBlur = cb; }  },

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		'focusGroup': { get: function (){ return button.ui.focusGroup; }, set: function( f ){ button.ui.focusGroup = f; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		'pad': { get: function (){ return button.ui.pad; }, set: function( v ){ button.ui.pad = v; }  },

		// (Number) inner padding top
		'padTop': { get: function (){ return button.ui.padTop; }, set: function( v ){ button.ui.padTop = v; }, serialized: false  },

		// (Number) inner padding right
		'padRight': { get: function (){ return button.ui.padRight; }, set: function( v ){ button.ui.padRight = v; }, serialized: false },

		// (Number) inner padding bottom
		'padBottom': { get: function (){ return button.ui.padBottom; }, set: function( v ){ button.ui.padBottom = v; }, serialized: false },

		// (Number) inner padding left
		'padLeft': { get: function (){ return button.ui.padLeft; }, set: function( v ){ button.ui.padLeft = v; }, serialized: false },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		'spacing': { get: function (){ return button.ui.spacing; }, set: function( v ){ button.ui.spacing = v; }, serialized: false },

		// (Number) spacing between children when layoutType is Vertical
		'spacingX': { get: function (){ return button.ui.spacingX; }, set: function( v ){ button.ui.spacingX = v; } },

		// (Number) spacing between children when layoutType is Horizontal
		'spacingY': { get: function (){ return button.ui.spacingY; }, set: function( v ){ button.ui.spacingY = v; } },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

	// API functions


	// create components

	// set name
	if ( !go.name ) go.name = "Select";

	// main button
	button = new GameObject( './button', {
		text: "   ",
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Center,
		wrapEnabled: false,
		flex: 1
	} );
	go.addChild( button );

	// add dropdown arrow
	arrowImage = new GameObject( './image', {
		selfAlign: LayoutAlign.Center,
		mode: 'icon',
	} );
	button.label.flex = 1;
	button.label.parent.addChild( arrowImage );

	// UI
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = ui.padLeft + ui.padRight;
	ui.height = ui.minHeight = ui.padTop + ui.padBottom;
	ui.layoutType = Layout.Vertical;
	ui.layoutAlignX = LayoutAlign.Stretch;
	//ui.layoutAlignY = LayoutAlign.Stretch;
	ui.focusable = false;
	go.ui = ui;

	// updates icon and text to display currently selected item
	go.updateSelectedItem = function () {
		// something selected
		if ( selectedIndex >= 0 ) {
			var item = items[ selectedIndex ];
			button.text = item.text;
			button.icon = item.icon;
		// invalid, show empty
		} else {
			button.text = " ";
			button.icon = null;
		}
	};

	// opens dropdown on click
	button.click = function ( btn ) {
		go.showDropdown( true );
	}

	// refire focus event on gameObject
	button.focusChanged = function ( newFocus ) {
		go.fire( 'focusChanged', newFocus );
	}

	go.showDropdown = function( show ) {
		// hide previously shown dropdown
		if ( dropdown ) {
			dropdown.parent = null;
			dropdown = null;
			Input.mouseDown = null;
		}
		// show
		if ( show ) {
			go.scrollIntoView();
			if ( !items || !items.length ) return;
			var gp = button.localToGlobal( 0, 0, true );
			// scrollable container
			dropdown = new GameObject( './scrollable', {
				layoutType: Layout.Vertical,
				layoutAlignX: LayoutAlign.Stretch,
				layoutAlignY: LayoutAlign.Start,
				wrapEnabled: false,
				focusGroup: 'dropdown',
				height: button.height,
				minWidth: button.width,
				update: go.updateDropdownPosition,
				x: gp.x, y: gp.y + button.height,
				opacity: 0,
				scrollbars: false,
				style: go.baseStyle.menu,
				ignoreCamera: true,
				fixedPosition: true,
			} );
			// add items
			var item, selectedItem;
			for ( var i = 0; i < items.length; i++ ) {
				item = new GameObject( './button', {
					value: items[ i ].value,
					icon: items[ i ].icon,
					text: items[ i ].text,
					name: items[ i ].text,
					item: items[ i ],
					minWidth: button.width,
					disabled: !!items[ i ].disabled,
					focusGroup: 'dropdown',
					click: go.itemSelected,
					mouseOver: go.itemSetFocus,
					navigation: go.itemNavigation,
					style: go.baseStyle.item,
				} );
				if ( !i || i == selectedIndex ) selectedItem = item;
				if ( i == selectedIndex && go.itemCheck != undefined ) {
					item.addChild( './image', go.itemCheck );
				}
				item.state = 'off';
				dropdown.addChild( item );
			}
			// link top/bottom
			dropdown.getChild( 0 ).focusUp = item;
			item.focusDown = dropdown.getChild( 0 );
			// add to scene, positioning will occur on update
			App.overlay.addChild( dropdown );
			dropdown.fadeTo( 1, 0.15 );
			dropdown.async( function() {
				if ( dropdown ) {
					selectedItem.focus();
					selectedItem.scrollIntoView();
					dropdown.scrollbars = 'auto';
				}
			}, 0.15 );
			Input.mouseDown = go.mouseDownOutside;
		}
	}

	go.updateDropdownPosition = function () {
		var gp = button.localToGlobal( 0, 0, true );
		var itemHeight = dropdown.getChild( 0 ).height;
		var desiredHeight = Math.min( dropdown.scrollHeight, itemHeight * maxVisibleItems );
		var buttonBottom = gp.y + button.height;
		var availSpaceBelow = ( App.windowHeight - buttonBottom );
		var availSpaceAbove = ( gp.y );

		// fits below button
		if ( desiredHeight < availSpaceBelow || availSpaceBelow > availSpaceAbove ) {
			desiredHeight = Math.min( availSpaceBelow, desiredHeight );
			dropdown.setTransform( gp.x, buttonBottom );
		} else {
			desiredHeight = Math.min( availSpaceAbove, desiredHeight );
			dropdown.setTransform( gp.x, gp.y - desiredHeight );
		}

		// size
		dropdown.width = Math.max( dropdown.scrollWidth, button.width );
		dropdown.height = desiredHeight;

	}

	go.itemNavigation = function ( name, value ) {
		if ( name == 'cancel' ) {
			go.showDropdown( false );
			button.focus();
			stopAllEvents();
		}
	}

	go.itemSelected = function () {
		stopAllEvents();
		go.showDropdown( false );
		if ( go.value !== this.value ) {
			go.value = this.value;
			go.fire( 'change', go.value, this.item );
		}
		button.focus();
	}

	go.itemSetFocus = function () {
		if ( !this.disabled ) this.focus();
	}

	// click outside to close
	go.mouseDownOutside = function ( btn, x, y ) {
		// add scrollbar width, if visible
		var ww = dropdown.width;
		if ( dropdown.verticalScrollbar && dropdown.verticalScrollbar.active ) {
			ww += dropdown.verticalScrollbar.width;
		}
		// close if outside
		if ( x < dropdown.x || x > dropdown.x + ww ||
			y < dropdown.y || y > dropdown.y + dropdown.height ) {
			go.showDropdown( false );
			stopAllEvents();
		}
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.select );
	UI.base.applyProperties( go, go.baseStyle );
	button.state = 'auto';
	constructing = false;

})(this);
