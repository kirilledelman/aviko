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
		get items(){ return this.__items; }, set items( v ){ this.__items = v; this.__updateItems(); },

		// (Number) 0 based index of item focused / highlighted in menu, note that spacers count as items
		get selectedIndex(){ return this.__selectedIndex; },
		set selectedIndex( v ){
			if ( isNaN( v ) ) return;
			this.__selectedIndex = Math.max( -1, Math.min( Math.floor( v ), this.__items.length ) );
			if ( this.__selectedIndex >= 0 && this.__container.container.numChildren >= this.__selectedIndex + 1 ) {
				if ( this.__noFocus ) {
					for ( var i = 0, nc = this.__container.container.numChildren; i < nc; i++ ){
						var item = this.__container.getChild( i );
						if ( item.item ) {
							item.state = ( i == this.__selectedIndex ? 'over' : 'auto' );
							if ( i == this.__selectedIndex ) item.scrollIntoView();
						}
					}
				} else {
					var si = this.__container.getChild( this.__selectedIndex );
					if ( si && si.item ) si.focus();
				}
			}
		},

		// (Boolean) focus-less mode, where items' "over" state is used instead of "focus". Used in textfield autocomplete
		get noFocus(){ return this.__noFocus; }, set noFocus( v ){ this.__noFocus = v; },

		// (*) maximim number of visible items in menu before scrollbar is displayed
		get maxVisibleItems(){ return this.__maxVisibleItems; }, set maxVisibleItems( v ){ this.__maxVisibleItems = v; this.__updateSize(); },

		// (ui/scrollable) scrollable container
		get container (){ return this.__container; },

		// (String) 'down' or 'up' - positioning of popup relative to either target, or x, y
		get preferredDirection(){ return this.__preferredDirection; }, set preferredDirection( v ){ this.__preferredDirection = v; },

		// (Object) used to override style (collection of properties) other than default after creating / during init.
		// Here because popup menu doesn't have own UI object + not calling addSharedProperties
		get style(){ return this.__baseStyle; }, set style( v ){ UI.base.mergeStyle( this.__baseStyle, v ); UI.base.applyProperties( this, v ); },

		__updateItems: function () {

			// clean up
			this.__container.removeAllChildren();
			this.__container.opacity = 0;
			this.cancelDebouncer( this.__updateItemsDebouncer );
	
			// add items
			this.__selectedItem = null;
			var curItem = 0, numItems = this.__items.length;
			var progressiveAdd = (function () {
	
				// add maxVisibleItems
				var maxItems = Math.min( numItems, curItem + this.__maxVisibleItems );
				for ( var i = curItem; i < maxItems; i++ ) {
					var button, text;
					var item = this.__items[ i ];
					var icon = null;
					var disabled = false;
					if ( item === null ) {
						if ( i == this.__items.length - 1 || i == 0 ) continue;
						this.__container.addChild( './panel', {
							style: this.__baseStyle.separator
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
					button = this.__container.addChild( './button', {
						item: item,
						icon: icon,
						text: text,
						name: text,
						popup: this,
						index: i,
						focusable: !this.__noFocus,
						disabled: disabled,
						focusGroup: 'popup',
						click: this.__itemSelected,
						mouseOver: this.__itemSetFocus,
						navigation: this.__itemNavigation,
						focusChanged: this.__itemFocusChanged,
						style: this.__baseStyle.item,
					} );
					if ( i == this.__selectedIndex ) this.__selectedItem = button;
					// if this is first button in 2nd+ batch
					if ( i > 0 && i == curItem ) {
						// unlink previous last item focusDown to this button
						this.__container.getChild( curItem - 1 ).focusDown = null;
					}
				}
	
				// link top/bottom buttons
				this.__container.getChild( 0 ).focusUp = button;
				button.focusDown = this.__container.getChild( 0 );
	
				// update
				curItem = maxItems;
				if ( this.__selectedItem ) {
					if ( this.__noFocus ) {
						this.__selectedItem.state = 'over';
					} else {
						this.__selectedItem.focus();
					}
					this.__selectedItem.scrollIntoView();
				}
	
				// finished?
				if ( curItem >= this.__items.length ) {
					// if nothing was added bail
					if ( !this.__container.container.numChildren ) {
						this.parent = null;
						return;
					}
	
					this.__container.scrollbars = 'auto';
	
				// add more next frame
				} else {
					this.__updateItemsDebouncer = 'updateItems' + curItem;
					this.debounce( this.__updateItemsDebouncer, progressiveAdd, 0.1 );
				}
	
				// size
				this.__updateSize();
				this.__container.requestLayout();
	
				// fade in
				if ( this.__container.opacity == 0 ) {
					this.__container.on( 'layout', function () {
						this.__container.fadeTo( 1, 0.25 );
					}.bind( this ), true );
				}
	
			}).bind( this );
			
			// start
			progressiveAdd();
			Input.mouseDown = this.__mouseDownOutside;
		},

		// resizes container
		__updateSize: function () {
			this.debounce( 'updateSize', function() {
				this.__container.width = this.__container.scrollWidth;
				var totalHeight = 0, numItems = 0;
				for ( var i = 0; i < this.__container.container.numChildren && numItems < this.__maxVisibleItems; i++ ) {
					var btn = this.__container.getChild( i );
					totalHeight += btn.height + this.__container.spacingY + btn.marginTop + btn.marginBottom;
					if ( items[ i ] !== null ) numItems++;
				}
				this.__container.height = this.__container.minHeight = totalHeight;
			} );
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
			this.popup.fire( 'selected', this.item );
			this.popup.parent = null;
		},
	
		__itemSetFocus: function () {
			this.popup.selectedIndex = this.index;
		},
	
		__itemFocusChanged: function ( newFocus ) {
			var focusedItem = null;
			if ( newFocus && newFocus.parent == this.popup.container.container ) focusedItem = newFocus.gameObject.item;
			if ( newFocus == this.ui ) this.popup.fire( 'change', focusedItem );
		},
	
		removed: function () {
			// remove mousedown callback
			if ( Input.mouseDown == this.__mouseDownOutside ) Input.mouseDown = null;
		}
		
	};
	
	// initialize
	go.name = "Popup menu";
	go.__container = go.addChild( './scrollable', {
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
		layout: function () { go.__updateSize(); },
		blocking: true,
		mouseOver: function(){ cancelDebouncer( 'showTooltip' ); }
	} );
	go.__items = [];
	go.__target = null;
	go.__selectedIndex = -1;
	go.__maxVisibleItems = 10;
	go.__selectedItem = null;
	go.__preferredDirection = 'down';
	go.__noFocus = false;
	go.__updateItemsDebouncer = '';
	go.__proto__ = UI.base.popupMenuPrototype;
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
