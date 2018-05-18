/*

	Popup menu with list items

	Used as right-click context menu, autosuggest, or drop down menu
	Popup menu adds itself to App.overlay automatically

	Usage:

		var p = new GameObject( 'ui/popup-menu', {
			items: [
				{ text: "One", icon: 'texture' },
				null, // null object is a separator
				"String is ok too"
			],
			selected: function ( item ) { ... }
		} );

		// or to open from another UI element:

		var p = new GameObject( 'ui/popup-menu', {
			target: anotherObject,
			items: [
				"One", "Two"
			],
			selected: function ( item ) { ... }
		} );

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'selected' : item is clicked on
		'change' : an item is highlighted
*/

include( './ui' );
(function(go) {

	// internal props
	var container, items = [];
	var constructing = true;
	var target = null, selectedIndex = -1, maxVisibleItems = 10, selectedItem = null;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (GameObject) object to which this menu is attached
		[ 'target',  function (){ return target; }, function ( v ){ target = v; } ],

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		[ 'items',  function (){ return items; },
			function ( v ){
				items = v;
				go.updateItems();
			} ],

		// (Number) 0 based index of item focused / highlighted in menu, note that spacers count as items
		[ 'selectedIndex',  function (){ return selectedIndex; },
			function ( v ){
				if ( isNaN( v ) ) return;
				selectedIndex = Math.max( -1, Math.min( Math.floor( v ), items.length ) );
				if ( selectedIndex >= 0 && container.container.numChildren >= selectedIndex + 1 ) {
					var si = container.getChild( selectedIndex );
					if ( si && si.focus) si.focus();
				}
			} ],

		// (*) maximim number of visible items in menu before scrollbar is displayed
		[ 'maxVisibleItems',  function (){ return maxVisibleItems; }, function ( v ){ maxVisibleItems = v; go.updateSize(); } ],

		// (ui/scrollable) scrollable container
		[ 'container',  function (){ return container; } ],

	];
	UI.base.addMappedProperties( go, mappedProps );

	// set name
	if ( !go.name ) go.name = "Popup menu";

	// create components
	container = go.addChild( './scrollable', {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		fitChildren: true,
		wrapEnabled: false,
		focusGroup: 'popup',
		style: UI.style.popupMenu.menu,
		ignoreCamera: true,
		fixedPosition: true,
	} );
	go.updateItems = function () {

		// clean up
		container.removeAllChildren();

		// add items
		var button;
		selectedItem = null;
		for ( var i = 0; i < items.length; i++ ) {
			var item = items[ i ];
			var icon = null;
			var disabled = false;
			if ( item === null ) {
				if ( i == items.length - 1 || i == 0 ) continue;
				container.addChild( './panel', {
					style: go.baseStyle.separator
				});
				continue;
			}
			if ( typeof ( item ) === 'string' ) {
				text = item;
			} else if ( typeof( item ) === 'object' ) {
				text = item.text;
				icon = item.icon;
				disabled = item.disabled;
			} else continue;
			// make item
			button = container.addChild( './button', {
				item: item,
				icon: icon,
				text: text,
				name: text,
				disabled: disabled,
				focusGroup: 'popup',
				click: go.itemSelected,
				mouseOver: go.itemSetFocus,
				navigation: go.itemNavigation,
				focusChanged: go.itemFocusChanged,
				style: go.baseStyle.item,
			} );
			if ( i == selectedIndex ) selectedItem = button;
			button.state = 'off';
		}

		// if nothing was added bail
		if ( !container.container.numChildren ) {
			go.parent = null;
			return;
		}
		// link top/bottom buttons
		container.getChild( 0 ).focusUp = button;
		button.focusDown = container.getChild( 0 );
		container.ui.on( 'layout', function () {
			this.async( function () {
				if ( selectedItem ) {
					selectedItem.focus();
					selectedItem.scrollIntoView();
				}
				go.updateSize();
				container.fadeTo( 1, 0.15 );
			});
		}, true );
		container.opacity = 0;
		Input.mouseDown = go.mouseDownOutside;
	}

	// resizes container
	go.updateSize = function () {
		container.width = container.scrollWidth;
		var totalHeight = 0, numItems = 0;
		for ( var i = 0; i < container.container.numChildren && numItems < maxVisibleItems; i++ ) {
			var btn = container.getChild( i );
			totalHeight += btn.height + container.spacingY + btn.marginTop + btn.marginBottom;
			if ( items[ i ] !== null ) numItems++;
		}
		container.minHeight = totalHeight;
	}

	// positions menu next to target
	go.update = function () {
		// place next to target
		var x = go.x, y = go.y;
		if ( target ) {
			var tw = 0;
			if ( target.ui ) {
				tw = target.ui.width;
			} else if ( target.render ) {
				tw = target.render.width;
			}
			var tx = target.worldX;
			var ty = target.worldY;
			x = ( tx + tw + container.width >= App.windowWidth ) ? ( tx - container.width ) : ( tx + tw );
			y = ( ty + container.height >= App.windowHeight ) ? ( ty - container.height ) : ty;
			if ( x < 0 ) x = Math.max( 0, tx + tw );
			if ( y < 0 ) y = Math.max( 0, ty );
		// fit to screen
		} else  {
			if ( x + container.width > App.windowWidth ) x = App.windowWidth - container.width;
			else if ( x < 0 ) x = 0;
			if ( y + container.height > App.windowHeight ) y = App.windowHeight - container.height;
			else if ( y < 0 ) y = 0;
		}
		go.setTransform( x, y );
	}

	// click outside to close
	go.mouseDownOutside = function ( btn, x, y ) {
		// add scrollbar width, if visible
		var ww = container.width;
		if ( container.verticalScrollbar && container.verticalScrollbar.active ) {
			ww += container.verticalScrollbar.width;
		}
		// close if outside
		if ( x < go.x || x > go.x + ww ||
			y < go.y || y > go.y + container.height ) {
			go.parent = null;
			stopAllEvents();
			//Input.on( 'mouseUp', function() { stopAllEvents(); }, true ); // ignore next mouseup
		}
	}

	// cancel closes popup
	go.itemNavigation = function ( name, value ) {
		if ( name == 'cancel' ) {
			go.parent = null;
			stopAllEvents();
		}
	}

	// item clicked
	go.itemSelected = function () {
		stopAllEvents();
		go.fire( 'selected', this.item );
		go.parent = null;
	}

	// focuses item on mouse over
	go.itemSetFocus = function () {
		if ( !this.disabled ) this.focus();
	}

	// 'change' event
	go.itemFocusChanged = function ( newFocus ) {
		var focusedItem = null;
		if ( newFocus && newFocus.parent == container.container ) focusedItem = newFocus.gameObject.item;
		if ( newFocus == this.ui ) go.fire( 'change', focusedItem );
	}

	// removed from parent
	go.removed = function () {
		// remove mousedown callback
		if ( Input.mouseDown == go.mouseDownOutside ) Input.mouseDown = null;
	}

	// apply defaults
	go.baseStyle = Object.create( UI.style.popupMenu );
	UI.base.applyProperties( go, go.baseStyle );

	// add self to overlay
	App.overlay.addChild( go );
	constructing = false;

})(this);
