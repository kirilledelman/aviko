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

	// API properties
	UI.base.selectPrototype = UI.base.selectPrototype || {

		__proto__: UI.base.componentPrototype,

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		get items(){ return this.__items; },
		set items( v ){
			// check items
			this.__items = [];
			for ( var i in v ) {
				if ( typeof ( v[ i ] ) === 'string' ) this.__items.push( { text: v[ i ], value: v[ i ] } );
				else this.__items.push( v[ i ] );
			}
			this.value = this.__value;
			this.selectedIndex = this.__selectedIndex;
			this.__updateSelectedItem();
		},

		// (*) 'value' property of selected item
		get value(){ return this.__value; },
		set value( v ){
			this.__value = v;
			// find matching value in items
			this.__selectedIndex = -1;
			for ( var i in this.__items ) {
				if ( this.__items[ i ].value === v ) {
					this.__selectedIndex = i;
				}
			}
			// item not found
			if ( this.__selectedIndex == -1 && this.__autoAddValue && this.__items.length ) {
				this.__items.push( { text: v.toString(), value: v } );
				this.selectedIndex = this.__items.length - 1;
			} else {
				this.selectedIndex = this.__selectedIndex; // for inspector to show change
				this.__updateSelectedItem();
			}
		},

		// (Number) 0 based index of item selected in menu
		get selectedIndex(){ return this.__selectedIndex; },
		set selectedIndex( v ){
			if ( isNaN( v ) ) return;
			this.__selectedIndex = Math.max( -1, Math.min( Math.floor( v ), this.__items.length - 1 ) );
			this.__value = this.__selectedIndex > 0 ? this.__items[ this.__selectedIndex ].value : this.__value;
			this.__updateSelectedItem();
		},

		// (*) 'value' property of selected item
		get maxVisibleItems(){ return this.__maxVisibleItems; }, set maxVisibleItems( v ){ this.__maxVisibleItems = v; },

		// (Boolean) when setting .value to value that doesn't exist in items, automatically append this value to items
		get autoAddValue(){ return this.__autoAddValue; }, set autoAddValue( v ){ this.__autoAddValue = v; },

		// (GameObject) instance of 'ui/button.js' used as main area
		get button (){ return this.__button; },

		// (GameObject) instance of 'ui/image.js' used for dropdown icon
		get arrowImage (){ return this.__arrowImage; },
		
		// (Boolean) input disabled
		get disabled(){ return this.__button.disabled; }, set disabled( v ){ this.ui.disabled = this.__button.disabled = v; },

		// (Boolean) whether control is focusable when it's disabled
		get disabledCanFocus(){ return this.__button.disabledCanFocus; }, set disabledCanFocus( f ){ this.__button.disabledCanFocus = f; },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		get cancelToBlur(){ return this.__button.cancelToBlur; }, set cancelToBlur( cb ){ this.__button.cancelToBlur = cb; },

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		get focusGroup(){ return this.__button.ui.focusGroup; }, set focusGroup( f ){ this.__button.ui.focusGroup = f; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		get pad(){ return this.__button.ui.pad; }, set pad( v ){ this.__button.ui.pad = v; },

		// (Number) inner padding top
		get padTop(){ return this.__button.ui.padTop; }, set padTop( v ){ this.__button.ui.padTop = v; },

		// (Number) inner padding right
		get padRight(){ return this.__button.ui.padRight; }, set padRight( v ){ this.__button.ui.padRight = v; },

		// (Number) inner padding bottom
		get padBottom(){ return this.__button.ui.padBottom; }, set padBottom( v ){ this.__button.ui.padBottom = v; },

		// (Number) inner padding left
		get padLeft(){ return this.__button.ui.padLeft; }, set padLeft( v ){ this.__button.ui.padLeft = v; },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		get spacing(){ return this.__button.ui.spacing; }, set spacing( v ){ this.__button.ui.spacing = v; },

		// (Number) spacing between children when layoutType is Vertical
		get spacingX(){ return this.__button.ui.spacingX; }, set spacingX( v ){ this.__button.ui.spacingX = v; },

		// (Number) spacing between children when layoutType is Horizontal
		get spacingY(){ return this.__button.ui.spacingY; }, set spacingY( v ){ this.__button.ui.spacingY = v; },

		// (String) tooltip displayed on mouseOver
		get tooltip() { return this.__button.tooltip; }, set tooltip( v ) { this.__button.tooltip = v; },

		// updates icon and text to display currently selected item
		__updateSelectedItem: function () {
			// something selected
			if ( this.__selectedIndex >= 0 ) {
				var item = this.__items[ this.__selectedIndex ];
				this.__button.text = item.text;
				this.__button.icon = item.icon;
			// invalid, show empty
			} else {
				this.__button.text = " ";
				this.__button.icon = null;
			}
		},
	
		// opens dropdown on click
		__buttonClick: function ( btn ) {
			if ( btn != 1 ) return;
			this.__showDropdown( true );
		},
	
		// refire focus event on gameObject
		__buttonFocusChanged: function ( newFocus ) {
			this.fire( 'focusChanged', newFocus );
		},
	
		__showDropdown: function ( show ) {
			// hide previously shown dropdown
			if ( this.__dropdown ) {
				this.__dropdown.parent = null;
				this.__dropdown = null;
				Input.mouseDown = null;
			}
			// show
			if ( show ) {
				this.scrollIntoView();
				var items = this.__items;
				if ( !items || !items.length ) return;
				var gp = this.__button.localToGlobal( 0, 0, true );
				// scrollable container
				this.__dropdown = new GameObject( './scrollable', {
					layoutType: Layout.Vertical,
					layoutAlignX: LayoutAlign.Stretch,
					layoutAlignY: LayoutAlign.Start,
					wrapEnabled: false,
					focusGroup: 'dropdown',
					height: this.__button.height,
					minWidth: this.__button.width,
					update: this.__updateDropdownPosition.bind( this ),
					x: gp.x, y: gp.y + this.__button.height,
					opacity: 0,
					scrollbars: false,
					style: this.__baseStyle.menu,
					ignoreCamera: true,
					fixedPosition: true,
				} );
				// add items
				var item = null, selectedItem = null;
				for ( var i = 0; i < items.length; i++ ) {
					item = new GameObject( './button', {
						value: items[ i ].value,
						icon: items[ i ].icon,
						text: items[ i ].text,
						name: items[ i ].text,
						item: items[ i ],
						minWidth: this.__button.width,
						disabled: !!items[ i ].disabled,
						focusGroup: 'dropdown',
						select: this,
						click: this.__itemSelected,
						mouseOver: this.__itemSetFocus,
						navigation: this.__itemNavigation,
						style: this.__baseStyle.item,
					} );
					if ( !i || i == this.__selectedIndex ) selectedItem = item;
					if ( i == this.__selectedIndex && this.__baseStyle.itemCheck !== undefined ) {
						item.addChild( './image', this.__baseStyle.itemCheckStyle );
					}
					item.state = 'off';
					this.__dropdown.addChild( item );
				}
				// link top/bottom
				this.__dropdown.getChild( 0 ).focusUp = item;
				item.focusDown = this.__dropdown.getChild( 0 );
				// add to scene, positioning will occur on update
				App.overlay.addChild( this.__dropdown );
				this.__dropdown.fadeTo( 1, 0.15 );
				this.__dropdown.async( function() {
					selectedItem.focus();
					selectedItem.scrollIntoView();
					this.scrollbars = 'auto';
				}, 0.15 );
				Input.mouseDown = this.__mouseDownOutside.bind( this );
			}
		},
	
		__updateDropdownPosition: function () {
			var gp = this.__button.localToGlobal( 0, 0, true );
			var itemHeight = this.__dropdown.getChild( 0 ).height;
			var desiredHeight = Math.min( this.__dropdown.scrollHeight, itemHeight * this.__maxVisibleItems );
			var buttonBottom = gp.y + this.__button.height;
			var availSpaceBelow = ( App.windowHeight - buttonBottom );
			var availSpaceAbove = ( gp.y );
	
			// fits below button
			if ( desiredHeight < availSpaceBelow || availSpaceBelow > availSpaceAbove ) {
				desiredHeight = Math.min( availSpaceBelow, desiredHeight );
				this.__dropdown.setTransform( gp.x, buttonBottom );
			} else {
				desiredHeight = Math.min( availSpaceAbove, desiredHeight );
				this.__dropdown.setTransform( gp.x, gp.y - desiredHeight );
			}
	
			// size
			this.__dropdown.width = Math.max( this.__dropdown.scrollWidth, this.__button.width );
			this.__dropdown.height = desiredHeight;
	
		},
	
		__itemNavigation: function ( name, value ) {
			if ( name == 'cancel' ) {
				this.select.__showDropdown( false );
				this.select.__button.focus();
				stopAllEvents();
			}
		},
	
		__itemSelected: function () {
			stopAllEvents();
			this.select.__showDropdown( false );
			if ( this.select.value !== this.value ) {
				this.select.value = this.value;
				this.select.fire( 'change', this.select.value, this.item );
			}
			this.select.__button.focus();
		},
	
		__itemSetFocus: function () {
			if ( !this.disabled ) this.focus();
		},
	
		// click outside to close
		__mouseDownOutside: function ( btn, x, y ) {
			// add scrollbar width, if visible
			var ww = this.__dropdown.width;
			if ( this.__dropdown.verticalScrollbar && this.__dropdown.verticalScrollbar.active ) {
				ww += this.__dropdown.verticalScrollbar.width;
			}
			// close if outside
			if ( x < this.__dropdown.x || x > this.__dropdown.x + ww ||
				y < this.__dropdown.y || y > this.__dropdown.y + this.__dropdown.height ) {
				this.__showDropdown( false );
				stopAllEvents();
			}
		}
		
	};
	
	// internal props
	go.name = "Select";
	go.ui = new UI( {
		autoMoveFocus: false,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		focusable: false,

    } );
	go.__button = go.addChild( './button', {
		text: "   ",
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Center,
		wrapEnabled: false,
		flex: 1,
		click: UI.base.selectPrototype.__buttonClick.bind( go ),
		focusChanged: UI.base.selectPrototype.__buttonFocusChanged.bind( go ),
	} );
	go.__arrowImage = new GameObject( './image', {
		selfAlign: LayoutAlign.Center,
		mode: 'icon',
	} );
	go.__button.label.flex = 1;
	go.__button.label.parent.addChild( go.__arrowImage );
	go.__value = '';
	go.__selectedIndex = -1;
	go.__items = [];
	go.__dropdown = null;
	go.__maxVisibleItems = 10;
	go.__autoAddValue = false;
	go.__proto__ = UI.base.selectPrototype;
	go.init();
	go.serializeMask.push( 'children', 'itemCheck', 'item', 'menu' );

	// add property-list inspectable info
	UI.base.addInspectables( go, 'Select',
		[ 'items', 'value', 'selectedIndex', 'disabled', 'disabledCanFocus', 'cancelToBlur' ],
        {   'items': { inline: true },
            'value': { readOnly: true },
            'selectedIndex': { min: 0, reloadOnChange: 'value' }
        }, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.select );
	UI.base.applyProperties( go, go.__baseStyle );
	go.__button.state = 'auto';

})(this);
