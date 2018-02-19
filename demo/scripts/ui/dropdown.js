/*

	Dropdown menu

	Used to select a single value from a list.
	Uses ui/button and ui/scrollable components for functionality.

	Usage:
		panel.addChild( 'ui/dropdown', {
		items: [
			{ value: "First", text: "Item 1", icon: "optional_icon" },
			{ value: "Second", text: "Item 2", icon: "optional_icon_2" },
		],
		value: "Second",
		change: function ( v ) {
			log( "value changed", v );
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
	var maxVisibleItems = 10;
	go.serializeMask = { 'ui':1, 'render':1, 'children':1 };

	// API properties
	var mappedProps = [

		// (*) 'value' property of selected item
		[ 'items',  function (){ return items; },
			function ( v ){
				items = v;
				go.value = value;
				go.selectedIndex = selectedIndex;
				go.updateSelectedItem();
			} ],

		// (*) 'value' property of selected item
		[ 'value',  function (){ return value; },
			function ( v ){
				value = v;
				// find matching value in items
				selectedIndex = -1;
				for ( var i in items ) {
					if ( items[ i ].value === v ) {
						selectedIndex = i;
					}
				}
				go.updateSelectedItem();
			} ],

		// (*) 'value' property of selected item
		[ 'selectedIndex',  function (){ return selectedIndex; },
			function ( v ){
				if ( isNaN( v ) ) return;
				selectedIndex = Math.max( -1, Math.min( Math.floor( v ), items.length ) );
				value = selectedIndex > 0 ? items[ selectedIndex ] : value;
				go.updateSelectedItem();
			} ],

		// (*) 'value' property of selected item
		[ 'maxVisibleItems',  function (){ return maxVisibleItems; }, function ( v ){ maxVisibleItems = v; } ],

		// (GameObject) instance of 'ui/button.js' used as main area
		[ 'button',  function (){ return button; } ],

		// (*) 'value' property of selected item
		[ 'arrowImage',  function (){ return arrowImage; } ],

		// (String) or null - texture on icon
		[ 'arrowIcon',  function (){ return arrowImage.texture; }, function ( v ){
			arrowImage.image.texture = v;
			arrowImage.active = !!v;
		} ],

		// (Boolean) input disabled
		[ 'disabled',  function (){ return button.disabled; },
		 function ( v ){
			 button.disabled = v;
		 } ],

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		[ 'cancelToBlur',  function (){ return button.cancelToBlur; }, function ( cb ){ button.cancelToBlur = cb; } ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// API functions


	// create components

	// set name
	if ( !go.name ) go.name = "Dropdown";

	// main button
	button = new GameObject( './button', {
		text: "   ",
		layoutAlign: LayoutAlign.Stretch
	} );
	go.addChild( button );

	// add dropdown arrow
	arrowImage = new GameObject( './image' );
	button.label.flex = 1;
	button.label.parent.addChild( arrowImage );

	// UI
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = ui.padLeft + ui.padRight;
	ui.height = ui.minHeight = ui.padTop + ui.padBottom;
	ui.layoutType = Layout.Vertical;
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
	button.click = function () {
		go.showDropdown( !dropdown );
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
			Input.off( 'mouseDown', go.mouseDownOutside );
		}
		// show
		if ( show ) {
			go.scrollIntoView();
			if ( !items || !items.length ) return;
			var gp = button.localToGlobal( 0, 0 );
			// scrollable container
			dropdown = new GameObject( './scrollable', {
				layoutType: Layout.Vertical,
				layoutAlign: LayoutAlign.Stretch,
				height: button.height,
				width: button.width,
				update: go.updateDropdownPosition,
				x: gp.x, y: gp.y + button.height,
				opacity: 0,
				style: UI.style.dropdown.menu,
			} );
			// add items
			var item, selectedItem;
			for ( var i = 0; i < items.length; i++ ) {
				item = new GameObject( './button', {
					value: items[ i ].value,
					icon: items[ i ].icon,
					text: items[ i ].text,
					name: items[ i ].text,
					disabled: !!items[ i ].disabled,
					focusGroup: 'dropdown',
					click: go.itemSelected,
					navigation: go.itemNavigation,
					style: UI.style.dropdown.item,
				} );
				if ( !i || i == selectedIndex ) selectedItem = item;
				dropdown.addChild( item );
			}
			// link top/bottom
			dropdown.getChild( 0 ).focusUp = item;
			item.focusDown = dropdown.getChild( 0 );
			// add to scene, positioning will occur on update
			go.scene.addChild( dropdown );
			dropdown.fadeTo( 1, 0.15 );
			dropdown.async( function() {
				if ( dropdown ) {
					selectedItem.focus();
					selectedItem.scrollIntoView();
				}
			}, 0.15 );
			Input.on( 'mouseDown', go.mouseDownOutside );
		}
	}

	go.updateDropdownPosition = function () {
		var gp = button.localToGlobal( 0, 0 );
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
		go.value = this.value;
		go.showDropdown( false );
		button.focus();
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
		}
	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.dropdown );

})(this);
