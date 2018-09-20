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

	// API properties
	UI.base.popupMenuPrototype = UI.base.popupMenuPrototype || {

		__proto__: GameObject.prototype,

		// (GameObject) object to which this menu is attached
		get target(){ return this.__target; }, set target( v ){ this.__target = v; },

		// (Array) in form of [ { text:"Label text", value:(*), icon:"optional icon", disabled:(Boolean) } ...]
		get items(){ return this.__items; }, set items( v ){
			// recycle all old ones
			var ch = this.__container.container.children;
			for ( var i = 0, nc = ch.length; i < nc; i++ ) this.__disposeItem( ch[ i ] );
			this.__items = v;
			this.debounce( 'updateItems', this.__updateItems );
		},

		// (Number) 0 based index of item focused / highlighted in menu, note that spacers count as items
		get selectedIndex(){ return this.__selectedIndex; },
		set selectedIndex( v ){ this.__selectedIndex = v; this.debounce( 'updateItems', this.__updateItems ); },

		// (Boolean) focus-less mode, where items' "over" state is used instead of "focus". Used in textfield autocomplete
		get noFocus(){ return this.__noFocus; }, set noFocus( v ){ this.__noFocus = v; },

		// (*) maximim number of visible items in menu before scrollbar is displayed
		get maxVisibleItems(){ return this.__maxVisibleItems; }, set maxVisibleItems( v ){ this.__maxVisibleItems = v; this.debounce( 'updateItems', this.__updateItems ); },

		// (ui/scrollable) scrollable container
		get container (){ return this.__container; },

		// (String) 'down' or 'up' - positioning of popup relative to either target, or x, y
		get preferredDirection(){ return this.__preferredDirection; }, set preferredDirection( v ){ this.__preferredDirection = v; },

		// (Object) used to override style (collection of properties) other than default after creating / during init.
		// Here because popup menu doesn't have own UI object + not calling addSharedProperties
		get style(){ return this.__baseStyle; }, set style( v ){ UI.base.mergeStyle( this.__baseStyle, v ); UI.base.applyProperties( this, v ); },

		// makes a new row or a separator, or returns a cached one
		__makeItem: function( item ) {
			// dequeue
			var row = null;
			if ( item.text ) {
				if ( this.__cachedRows.length ) {
					row = this.__cachedRows.pop();
					row.icon = item.icon,
					row.text = item.text,
					row.name = item.text,
					row.index = item.index,
					row.disabled = !!item.disabled;
				} else {
					row = new GameObject( './button', {
						icon: item.icon,
						text: item.text,
						name: item.text,
						popup: this,
						index: item.index,
						focusable: !this.__noFocus,
						disabled: !!item.disabled,
						focusGroup: 'popup',
						mouseDown: this.__itemSelected,
						mouseOver: this.__itemSetFocus,
						navigation: this.__itemNavigation,
						focusChanged: this.__itemFocusChanged,
						style: this.__baseStyle.item,
						left: 0,
						bottom: -1,
						right: 0,
						top: 0,
					} );
					row.label.autoSize = true;
				}
			} else {
				if ( this.__cachedSeparators.length ) {
					row = this.__cachedSeparators.pop();
				} else {
					row = new GameObject( './panel', {
						style: this.__baseStyle.separator
					} );
				}
				
			}
			
			item.row = row;
			row.item = item;
			return row;
		},
		
		// puts row back into cached array
		__disposeItem: function ( row ) {
			// remove
			row.parent = null;
			if ( row.item.row == row ) row.item.row = null;
			row.item = null;
			if ( row.click )
				this.__cachedRows.push( row );
			else
				this.__cachedSeparators.push( row );
		},
		
		// updates items visible in window
		__updateItems: function ( noScroll ) {

			// ignore if removed
			if ( !this.parent ) return;
			
			// if item size is unknown
			if ( !this.__itemHeight ) {
				if ( !this.__items.length ) return;
				// add item, wait for it to finish lay out
				var t = this.__makeItem( this.__items[ 0 ] );
				t.layout = function ( w, h ) {
					if ( h ) {
						this.__itemHeight = h - 2;
						t.layout = null;
						this.__updateItems();
					}
				}.bind( this );
				this.__container.addChild( t );
				return;
			}
			
			// set height
			this.__container.height = this.__container.minHeight = Math.min( this.__items.length, this.__maxVisibleItems ) * this.__itemHeight;
			
			// scan items until we find first one to display
			this.__selectedItem = null;
			var curItem = 0, numItems = this.__items.length;
			var foundLast = false, foundFirst = false;
			var curY = 0, scrollTop = this.__container.scrollTop, visibleHeight = this.__container.height;
			var minY = scrollTop - this.__itemHeight, maxY = scrollTop + visibleHeight + this.__itemHeight;
			var contWidth = this.__container.minWidth;
			var lastFocusableItem = null;
			
			for ( var i = 0; i < numItems; i++ ) {
				var item = this.__items[ i ];
				var itemHeight = 1;
				var isSeparator = false;
				
				// conform
				if ( item === null ) {
					this.__items[ i ] = item = {};
				} else if ( typeof ( item ) === 'string' ) {
					this.__items[ i ] = item = { text: item, value: item };
				} else if ( typeof( item ) !== 'object' ) continue;
				
				// setup
				isSeparator = ( typeof ( item.text ) === 'undefined' );
				itemHeight = isSeparator ? 1 : this.__itemHeight;
				item.index = i;
				item.y = curY;
				
				// mark selected item
				if ( this.__selectedIndex == i ) this.__selectedItem = item;
				
				// if item needs to be visible
				if ( !foundLast &&
					( curY >= minY && curY < maxY ) ||
					( curY + itemHeight >= minY && curY + itemHeight < maxY ) ) {
					
					// first?
					if ( foundFirst === false ) {
						foundFirst = i;
					}
					
					// add and position it
					if ( !item.row ) {
						item.row = this.__makeItem( item );
						item.row.y = curY;
						this.__container.addChild( item.row );
					}
					
					// link up
					if ( lastFocusableItem ) {
						lastFocusableItem.row.focusDown = item.row;
						item.row.focusUp = lastFocusableItem.row;
					}
					
					// measure width
					if ( item.row.label ) {
						item.row.label.renderText.measure();
						contWidth = Math.max( item.row.label.renderText.width + 16, contWidth );
					}
					
					// if passed last item
					foundLast = (curY + itemHeight >= maxY || i >= numItems - 1 );
					if ( foundLast ) {
						// size visible items to width
						var j = i;
						while ( j >= foundFirst ) {
							item = this.__items[ j ];
							if ( item.row ) item.row.width = contWidth;
							j--;
						}
						// force layout event
						this.__container.container.dispatch( 'layout', 'update' );
					}
					
					// row
					if ( !isSeparator ) {
						lastFocusableItem = item;
						item.row.state = ( i == this.__selectedIndex ? 'over' : 'auto' );
					}
					
				// item can be recycled
				} else {
					if ( item.row ) this.__disposeItem( item.row );
				}
				
				// advance Y
				curY += itemHeight;
			}
			
			// height
			this.__container.scrollHeight = curY;
			this.__container.minWidth = contWidth;
			
			// highlight / scroll to selected item
			if ( !noScroll ) {
				if ( this.__selectedItem ) {
					if ( this.__selectedItem.row ) {
						if ( this.__noFocus ) {
							this.__selectedItem.row.state = 'over';
						} else {
							this.__selectedItem.row.focus();
						}
						this.__selectedItem.row.scrollIntoView();
					} else {
						this.async( function () {
							this.__container.scrollTop = this.__selectedItem.y;
						}, 0.1 );
					}
				}
			}
			
			// fade in
			if ( this.__container.opacity == 0 ) {
				this.__container.on( 'layout', function () {
					this.__container.fadeTo( 1, 0.25 );
				}.bind( this ), true );
			}
			Input.mouseDown = this.__mouseDownOutside;
		},

		// positions menu next to target
		update: function () {
			// place next to target
			var x = this.x, y = this.y;
			var target = this.__target, container = this.__container;
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
				if ( this.__preferredDirection == 'up' ) {
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
				if ( this.__preferredDirection == 'up' ) {
					if ( y > App.windowHeight ) y = App.windowHeight;
					else if ( y - container.height < 0 ) {
						container.y = container.getChild( 0 ).height;
					} else container.y = -container.height;
				} else {
					container.y = 0;
					if ( y + container.height > App.windowHeight ) y = App.windowHeight - container.height;
					else if ( y < 0 ) y = 0;
				}
			}
			this.setTransform( x, y );
		},
	
		// click outside to close
		__mouseDownOutside: function ( btn, x, y ) {
			// add scrollbar width, if visible
			var container = this.__container;
			var ww = container.width;
			if ( container.verticalScrollbar && container.verticalScrollbar.active ) {
				ww += container.verticalScrollbar.width;
			}
			// close if outside
			var lp = container.globalToLocal( x, y );
			if ( lp.x < 0 || lp.x > ww ||
				lp.y < 0 || lp.y > container.height ) {
				this.parent = null;
				stopAllEvents();
				this.cancelDebouncer( 'updateItems' );
			}
		},
	
		// cancel closes popup
		__itemNavigation: function ( name, value ) {
			if ( name == 'cancel' ) {
				this.parent = null;
				stopAllEvents();
			}
		},
	
		__itemSelected: function () {
			if ( this.disabled ) return;
			stopAllEvents();
			this.popup.selectedIndex = this.item.index;
			this.popup.fire( 'selected', this.item );
			this.popup.parent = null;
		},
	
		__itemSetFocus: function () {
			if ( this.text && !this.disabled ) {
				if ( this.popup.__noFocus ) {
					this.popup.selectedIndex = this.item.index;
				} else {
					this.focus();
				}
			}
		},
	
		__itemFocusChanged: function ( newFocus ) {
			var focusedItem = null;
			if ( newFocus && newFocus.parent == this.popup.container.container ) focusedItem = newFocus.gameObject.item;
			if ( newFocus == this.ui ) this.popup.fire( 'change', focusedItem );
		},
	
		__scrolled: function () {
			this.__updateItems( true );
		},
		
		removed: function () {
			// remove mousedown callback
			if ( Input.mouseDown == this.__mouseDownOutside ) Input.mouseDown = null;
		}
		
	};
	
	// initialize
	go.name = "Popup menu";
	go.__items = [];
	go.__cachedRows = [];
	go.__cachedSeparators = [];
	go.__target = null;
	go.__selectedIndex = -1;
	go.__maxVisibleItems = 10;
	go.__selectedItem = null;
	go.__preferredDirection = 'down';
	go.__noFocus = false;
	go.__updateItemsDebouncer = '';
	go.__proto__ = UI.base.popupMenuPrototype;
	go.__container = go.addChild( './scrollable', {
		layoutType: Layout.None,
		/*layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		fitChildren: true,*/
		wrapEnabled: false,
		focusGroup: 'popup',
		scrollbarsFocusable: false,
		style: UI.style.popupMenu.menu,
		ignoreCamera: true,
		fixedPosition: true,
		scrollbars: 'auto',
		blocking: true,
		scroll: go.__scrolled.bind( go ),
		mouseOver: function(){ cancelDebouncer( 'showTooltip' ); }
	} );
	go.__mouseDownOutside = go.__mouseDownOutside.bind( go );
	go.serializeMask = [ 'children', 'update', 'removed' ];
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.popupMenu );
	UI.base.applyProperties( go, go.__baseStyle  );

	// add self to overlay
	App.overlay.addChild( go );

	// cancel any tooltips
	cancelDebouncer( 'showTooltip' );

})(this);
