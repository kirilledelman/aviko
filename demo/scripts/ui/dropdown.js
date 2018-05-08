/*

	Dropdown menu

	Used to select a single value from a list.
	Uses ui/button and ui/scrollable components for functionality.

	Usage:

		App.scene.addChild( 'ui/dropdown', {
		items: [
			{ value: "First", text: "Item 1", icon: "optional_icon" },
			{ value: "Second", text: "Item 2", icon: "optional_icon_2" },
			{ value: "Third", text: "Item 3", icon: "optional_icon_3", disabled: true },
		],
		context: someObject,
		selected: function ( v, context ) {
			log( "Value selected:", v );
		}
	} );

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'selected' - item was clicked

*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI();
	var dropdown;
	var items = [];
	var constructing = true;
	var maxVisibleItems = 10;
	go.serializeMask = { 'ui':1, 'render':1, 'children':1 };

	// API properties
	var mappedProps = [

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		[ 'items',  function (){ return items; },
			function ( v ){
				items = v;
				go.refresh();
			} ],

		// (Number) maximum items visible in a list before scrollbar appears
		[ 'maxVisibleItems',  function (){ return maxVisibleItems; }, function ( v ){ maxVisibleItems = v; } ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		[ 'pad',  function (){ return dropdown.pad; }, function ( v ){ dropdown.pad = v; } ],

		// (Number) inner padding top
		[ 'padTop',  function (){ return dropdown.padTop; }, function ( v ){ dropdown.padTop = v; }, true ],

		// (Number) inner padding right
		[ 'padRight',  function (){ return dropdown.padRight; }, function ( v ){ dropdown.padRight = v; }, true ],

		// (Number) inner padding bottom
		[ 'padBottom',  function (){ return dropdown.padBottom; }, function ( v ){ dropdown.padBottom = v; }, true ],

		// (Number) inner padding left
		[ 'padLeft',  function (){ return dropdown.padLeft; }, function ( v ){ dropdown.padLeft = v; }, true ],

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		[ 'spacing',  function (){ return dropdown.spacing; }, function ( v ){ dropdown.spacing = v; }, true ],

		// (Number) spacing between children when layoutType is Vertical
		[ 'spacingX',  function (){ return dropdown.spacingX; }, function ( v ){ dropdown.spacingX = v; } ],

		// (Number) spacing between children when layoutType is Horizontal
		[ 'spacingY',  function (){ return dropdown.spacingY; }, function ( v ){ dropdown.spacingY = v; } ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// API functions


	// create components

	// set name
	if ( !go.name ) go.name = "Dropdown";

	// UI
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = ui.padLeft + ui.padRight;
	ui.height = ui.minHeight = ui.padTop + ui.padBottom;
	ui.layoutType = Layout.Vertical;
	ui.layoutAlignX = LayoutAlign.Stretch;
	ui.layoutAlignY = LayoutAlign.Stretch;
	ui.focusable = false;
	go.ui = ui;

	// recreates items
	go.refresh = function () {
	};

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

			// scrollable container
			dropdown = new GameObject( './scrollable', {
				layoutType: Layout.Vertical,
				layoutAlignX: LayoutAlign.Stretch,
				layoutAlignY: LayoutAlign.Start,
				wrapEnabled: false,
				height: 16,
				minWidth: 16,
				opacity: 0,
				style: go.baseStyle.menu,
				ignoreCamera: true,
			} );
			// add items
			var item, selectedItem;
			for ( var i = 0; i < items.length; i++ ) {
				item = new GameObject( './button', {
					value: items[ i ].value,
					icon: items[ i ].icon,
					text: items[ i ].text,
					name: items[ i ].text,
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

	go.itemNavigation = function ( name ) {
		if ( name == 'cancel' ) {
			go.showDropdown( false );
			stopAllEvents();
		}
	}

	go.itemSelected = function () {
		stopAllEvents();
		go.showDropdown( false );
		go.fire( 'select', go.value );
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
	go.baseStyle = Object.create( UI.style.dropdown );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);
