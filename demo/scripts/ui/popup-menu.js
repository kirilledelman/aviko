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
	var preferredDirection = 'down';
	var noFocus = false;
	var updateItemsDebouncer = '';
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (GameObject) object to which this menu is attached
		'target': { get: function (){ return target; }, set: function( v ){ target = v; }  },

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		'items': { get: function (){ return items; }, set: function( v ){
				items = v;
				go.updateItems();
			}  },

		// (Number) 0 based index of item focused / highlighted in menu, note that spacers count as items
		'selectedIndex': { get: function (){ return selectedIndex; }, set: function( v ){
				if ( isNaN( v ) ) return;
				selectedIndex = Math.max( -1, Math.min( Math.floor( v ), items.length ) );
				if ( selectedIndex >= 0 && container.container.numChildren >= selectedIndex + 1 ) {
					if ( noFocus ) {
						for ( var i = 0, nc = container.container.numChildren; i < nc; i++ ){
							var item = container.getChild( i );
							if ( item.item ) {
								item.state = ( i == selectedIndex ? 'over' : 'auto' );
								if ( i == selectedIndex ) item.scrollIntoView();
							}
						}
					} else {
						var si = container.getChild( selectedIndex );
						if ( si && si.item ) si.focus();
					}
				}
			}  },

		// (Boolean) focus-less mode, where items' "over" state is used instead of "focus". Used in textfield autocomplete
		'noFocus': { get: function (){ return noFocus; }, set: function( v ){ noFocus = v; }  },

		// (*) maximim number of visible items in menu before scrollbar is displayed
		'maxVisibleItems': { get: function (){ return maxVisibleItems; }, set: function( v ){ maxVisibleItems = v; go.updateSize(); }  },

		// (ui/scrollable) scrollable container
		'container': { get: function (){ return container; } },

		// (String) 'down' or 'up' - positioning of popup relative to either target, or x, y
		'preferredDirection': { get: function (){ return preferredDirection; }, set: function ( v ){ preferredDirection = v; } },

		// (Object) used to override style (collection of properties) other than default after creating / during init.
		// Here because popup menu doesn't have own UI object + not calling addSharedProperties
		'style': { get: function (){ return go.baseStyle; }, set: function ( v ){ UI.base.mergeStyle( go.baseStyle, v ) ; UI.base.applyProperties( go, v ); }, },

	};
	UI.base.mapProperties( go, mappedProps );

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
		scrollbarsFocusable: false,
		style: UI.style.popupMenu.menu,
		ignoreCamera: true,
		fixedPosition: true,
		scrollbars: false,
		layout: function () { go.updateSize(); },
		blocking: true,
		mouseOver: function(){ cancelDebouncer( 'showTooltip' ); }
	} );
	go.updateItems = function () {

		// clean up
		container.removeAllChildren();
		container.opacity = 0;
		go.cancelDebouncer( updateItemsDebouncer );

		// log( "^BItem:\n", stringify( go.baseStyle.item, true ) );

		// add items
		selectedItem = null;
		// var d = new Date();
		var curItem = 0, numItems = items.length;
		var progressiveAdd = function () {

			// add maxVisibleItems
			var maxItems = Math.min( numItems, curItem + maxVisibleItems );
			for ( var i = curItem; i < maxItems; i++ ) {
				var button, text;
				var item = items[ i ];
				var icon = null;
				var disabled = false;
				if ( item === null ) {
					if ( i == items.length - 1 || i == 0 ) continue;
					container.addChild( './panel', {
						style: go.baseStyle.separator
					} );
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
					index: i,
					focusable: !noFocus,
					disabled: disabled,
					focusGroup: 'popup',
					click: go.itemSelected,
					mouseOver: go.itemSetFocus,
					navigation: go.itemNavigation,
					focusChanged: go.itemFocusChanged,
					style: go.baseStyle.item,
				} );
				if ( i == selectedIndex ) selectedItem = button;
				// if this is first button in 2nd+ batch
				if ( i > 0 && i == curItem ) {
					// unlink previous last item focusDown to this button
					container.getChild( curItem - 1 ).focusDown = null;
				}
			}

			// link top/bottom buttons
			container.getChild( 0 ).focusUp = button;
			button.focusDown = container.getChild( 0 );

			// update
			curItem = maxItems;
			if ( selectedItem ) {
				if ( noFocus ) {
					selectedItem.state = 'over';
				} else {
					selectedItem.focus();
				}
				selectedItem.scrollIntoView();
			}

			// finished?
			if ( curItem >= items.length ) {
				// if nothing was added bail
				if ( !container.container.numChildren ) {
					go.parent = null;
					return;
				}

				container.scrollbars = 'auto';

			// add more next frame
			} else {
				updateItemsDebouncer = 'updateItems' + curItem;
				go.debounce( updateItemsDebouncer, progressiveAdd, 0.1 );
			}

			// size
			go.updateSize();
			container.requestLayout();

			// fade in
			if ( container.opacity == 0 ) {
				container.on( 'layout', function () {
					container.fadeTo( 1, 0.25 );
				}, true );
			}

		};

		// log( "Items took " + ((new Date()).getTime() - d ) * 0.001 + "s" );

		progressiveAdd();
		Input.mouseDown = go.mouseDownOutside;
	}

	// resizes container
	go.updateSize = function () {
		this.debounce( 'updateSize', function() {
			container.width = container.scrollWidth;
			var totalHeight = 0, numItems = 0;
			for ( var i = 0; i < container.container.numChildren && numItems < maxVisibleItems; i++ ) {
				var btn = container.getChild( i );
				totalHeight += btn.height + container.spacingY + btn.marginTop + btn.marginBottom;
				if ( items[ i ] !== null ) numItems++;
			}
			container.height = container.minHeight = totalHeight;
		} );
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
			if ( preferredDirection == 'up' ) {
				y = ( ty - container.height < 0 ) ? ty : ( ty - container.height );
			} else {
				y = ( ty + container.height >= App.windowHeight ) ? ( ty - container.height ) : ty;
			}
			if ( x < 0 ) x = Math.max( 0, tx + tw );
			if ( y < 0 ) y = Math.max( 0, ty );
		// fit to screen
		} else  {
			if ( x + container.width > App.windowWidth ) x = App.windowWidth - container.width;
			else if ( x < 0 ) x = 0;
			if ( preferredDirection == 'up' ) {
				container.y = -container.height;
				if ( y > App.windowHeight ) y = App.windowHeight;
				else if ( y - container.height < 0 ) y = y - container.height;
			} else {
				container.y = 0;
				if ( y + container.height > App.windowHeight ) y = App.windowHeight - container.height;
				else if ( y < 0 ) y = 0;
			}
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
		var lp = container.globalToLocal( x, y );
		if ( lp.x < 0 || lp.x > ww ||
			lp.y < 0 || lp.y > container.height ) {
			go.parent = null;
			stopAllEvents();
			go.cancelDebouncer( 'updateItems' );
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
		if ( this.disabled ) return;
		stopAllEvents();
		go.fire( 'selected', this.item );
		go.parent = null;
	}

	// focuses item on mouse over
	go.itemSetFocus = function () {
		go.selectedIndex = this.index;
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
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.popupMenu );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

	// add self to overlay
	App.overlay.addChild( go );

	// cancel any tooltips
	cancelDebouncer( 'showTooltip' );

})(this);
