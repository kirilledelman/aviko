/*

	Scrollable container

	If scrollWidth / scrollHeight are bigger than control's width/height, control will
	be scrollable to reveal its contents via mouse wheel, or scroll bars (automatically shown)

	Usage:
		var s = App.scene.addChild( 'ui/scrollable' );
		s.layoutType = Layout.Vertical;
		s.width = 80; s.height = 100;
		s.addChild( .. );
		s.addChild( .. );


	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'scroll' - when content scroll position has changed
		'layout' - during layout

*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.scrollablePrototype = UI.base.scrollablePrototype || {

		__proto__: UI.base.componentPrototype,

		// (String) 'auto' or (Boolean) - whether scrollbars are shown automatically, always visible, or never
		get scrollbars(){ return this.__scrollbars; },
		set scrollbars( s ){
			if ( s != this.__scrollbars ) {
				this.__scrollbars = s;
				this.async( this.__updateScrollbars );
				this.requestLayout();
			}
		},

		// (Boolean) whether scrollbars can receive focus
		get scrollbarsFocusable(){ return this.__scrollbarsFocusable; },
		set scrollbarsFocusable( v ){
			this.__scrollbarsFocusable = v;
			if ( this.__vsb ) this.__vsb.focusable = v;
			if ( this.__hsb ) this.__hsb.focusable = v;
		},
		
		// (Number) width of the scrollable container area
		get scrollWidth(){ return this.__container.ui.width; },
		set scrollWidth( w ){
			if ( this.__container.ui.width != w ) {
				this.__container.ui.width = w;
				this.async( this.__updateScrollbars );
			}
		},

		// (Number) height of the scrollable container area
		get scrollHeight(){ return this.__container.ui.height; },
		set scrollHeight( h ){
			if ( this.__container.ui.height != h ) {
				this.__container.ui.height = h;
				this.async( this.__updateScrollbars );
			}
		},

		// (Number) current y scroll position
		get scrollTop(){ return -this.__container.y; },
		set scrollTop( t ){
			var py = this.__container.y;
			this.__container.y = Math.min( 0, Math.max( -(this.__container.ui.height - this.ui.height), -t ) );
			if ( py != this.__container.y ) {
				this.fireLate( 'scroll' );
				this.async( this.__updateScrollbars );
			}
		},

		// (Number) current x scroll position
		get scrollLeft(){ return -this.__container.x; },
		set scrollLeft( l ){
			var px = this.__container.x;
			this.__container.x = Math.min( 0, Math.max( -(this.__container.ui.width - this.ui.width), -l ) );
			if ( px != this.__container.x ) {
				this.fireLate( 'scroll' );
				this.async( this.__updateScrollbars );
			}
		},

		// (ui/scrollbar.js) - returns vertical scrollbar instance
		get verticalScrollbar (){ return this.__vsb; },

		// (ui/scrollbar.js) - returns horizontal scrollbar instance
		get horizontalScrollbar(){ return this.__hsb; },

		// (GameObject) the actual container to which all children are added
		get container(){ return this.__container; },

		// (Array) - set child objects at once (for serialization)
		get containerChildren (){ return this.__container.children; }, set containerChildren( v ){ this.__container.children = v; },

		// map to container, not ui

		// (Layout.None, Layout.Anchors, Layout.Vertical, Layout.Horizontal, Layout.Grid) - how to lay out children
		get layoutType(){ return this.__container.ui.layoutType; }, set layoutType( v ){ this.__container.ui.layoutType = v; },

		// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on X axis
		get layoutAlignX(){ return this.__container.ui.layoutAlignX; }, set layoutAlignX( v ){ this.__container.ui.layoutAlignX = v; },

		// (LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) for Horizontal and Vertical layout types determines how to align children on Y axis
		get layoutAlignY(){ return this.__container.ui.layoutAlignY; }, set layoutAlignY( v ){ this.ui.layoutAlignY = v; },

		// (Boolean) for Horizontal, Vertical, and Grid layout types, adjust own height and width to fit all children
		get fitChildren(){ return this.__container.ui.fitChildren; }, set fitChildren( v ){ this.__container.ui.fitChildren = v; },

		// (Boolean) for Horizontal and Vertical layouts, allow wrap of children into rows
		get wrapEnabled(){ this.__container.ui.wrapEnabled; }, set wrapEnabled( v ){ this.__container.ui.wrapEnabled = v; },
		
		// (Integer) for Horizontal and Vertical layouts, auto wrap after this many items per row
		get wrapAfter(){ return this.__container.ui.wrapAfter; }, set wrapAfter( v ){ this.__container.ui.wrapAfter = v; },
		
		// (Boolean) reverse child layout order
		get reversed(){ return this.__container.ui.reversed; }, set reversed( v ){ this.__container.ui.reversed = v; },
		
		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		get pad(){ return this.__container.ui.pad; }, set pad( v ){ this.__container.ui.pad = v; },

		// (Number) inner padding top
		get padTop(){ return this.__container.ui.padTop; }, set padTop( v ){ this.__container.ui.padTop = v; },

		// (Number) inner padding right
		get padRight(){ return this.__container.ui.padRight; }, set padRight( v ){ this.__container.ui.padRight = v; },

		// (Number) inner padding bottom
		get padBottom(){ return this.__container.ui.padBottom; }, set padBottom( v ){ this.__container.ui.padBottom = v; },

		// (Number) inner padding left
		get padLeft (){ return this.__container.ui.padLeft; }, set padLeft( v ){ this.__container.ui.padLeft = v; },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		get spacing(){ return this.__container.ui.spacing; }, set spacing( v ){ this.__container.ui.spacing = v; },

		// (Number) spacing between children when layoutType is Vertical
		get spacingX(){ return this.__container.ui.spacingX; }, set spacingX( v ){ this.__container.ui.spacingX = v; },

		// (Number) spacing between children when layoutType is Horizontal
		get spacingY(){ return this.__container.ui.spacingY; }, set spacingY( v ){ this.__container.ui.spacingY = v; },
		
		addChild: function() { return this.__container.addChild.apply( this.__container, arguments ); },
		removeChild: function() { return this.__container.removeChild.apply( this.__container, arguments ); },
		getChild: function() { return this.__container.getChild.apply( this.__container, arguments ); },
		removeAllChildren: function() { return this.__container.removeAllChildren.apply( this.__container, arguments ); },
		
		__updateScrollbars: function () {
			if ( !this.__scrollbars ) {
				// detach
				if ( this.__vsb ) this.__vsb.parent = null;
				if ( this.__hsb ) this.__hsb.parent = null;
				this.__vsb = this.__hsb = null;
			} else {
				// vertical
				if ( !this.__vsb ) {
					// position to the right
					this.__vsb = new GameObject( './scrollbar', {
						orientation: 'vertical',
						focusGroup: this.__container.ui.focusGroup,
						parent: this,
						anchorTop: 0,
						anchorBottom: 0,
						anchorRight: 0,
						anchorLeft: -1,
						scroll: function ( ny ) { this.scrollTop = ny; }.bind( this ),
						focusable: this.__scrollbarsFocusable,
					});
					this.__vsb.focusUp = this.__vsb.focusDown = this.__vsb;
					this.__vsb.right = -this.__vsb.width;
				}
				// horizontal
				if ( !this.__hsb ) {
					// position to the bottom
					this.__hsb = new GameObject( './scrollbar', {
						orientation: 'horizontal',
						focusGroup: container.ui.focusGroup,
						parent: this,
						anchorLeft: 0,
						anchorBottom: 0,
						anchorRight: 0,
						anchorTop: -1,
						scroll: function ( nx ) { this.scrollLeft = nx; }.bind( this ),
						focusable: this.__scrollbarsFocusable
					});
					this.__hsb.focusLeft = this.__hsb.focusRight = this.__hsb;
					this.__bottom = -this.__hsb.height;
				}
			}
	
			// update vertical scrollbars params
			if ( this.__vsb ) {
				this.__vsb.totalSize = Math.floor( this.__container.ui.height );
				this.__vsb.handleSize = Math.floor( this.ui.height );
				this.__container.y = Math.min( 0, Math.max( -(this.__container.ui.height - this.ui.height), this.__container.y ) );
				this.__vsb.position = Math.floor( -this.__container.y );
				// hide / show
				var ac = (this.__scrollbars == 'auto' ? (this.__container.ui.height > this.ui.height) : this.__scrollbars);
				if ( ac != this.__vsb.active ) {
					this.__vsb.active = ac;
					if ( ac && this.parent ) this.parent.requestLayout( 'scrollable/verticalScrollbar' );
				}
			}
			// update horizontal scrollbars params
			if ( this.__hsb ) {
				this.__hsb.totalSize = Math.floor( this.__container.ui.width );
				this.__hsb.handleSize = Math.floor( this.ui.width );
				this.__container.x = Math.min( 0, Math.max( -(this.__container.ui.width - this.ui.width), this.__container.x ) );
				this.__hsb.position = Math.floor( -this.__container.x );
				// hide / show
				var ac = (this.__scrollbars == 'auto' ? (this.__container.ui.width > this.ui.width) : this.__scrollbars);
				if ( ac != this.__hsb.active ) {
					this.__hsb.active = ac;
					if ( ac && this.parent ) this.parent.requestLayout( 'scrollable/horizontalScrollbar');
				}
			}
		},
	
		__layout: function( w, h ) {
			var go = this.gameObject;
			var sizeChanged = ( w != go.__spr.width || h != go.__spr.height );
			go.__spr.resize( w, h );
			if ( sizeChanged ) go.__updateScrollbars();
			go.fire( 'layout', w, h );
		},
	
		__containerLayout: function() {
			this.scrollable.debounce( 'updateScrollbars', this.scrollable.__updateScrollbars );
		},
	
		// scrolling
		__mouseWheel: function ( wy, wx ) {
			// scroll
			var go = this.gameObject;
			var st = go.scrollTop, sl = go.scrollLeft;
			go.scrollTop = Math.max( 0, Math.min( go.scrollTop - wy, go.scrollHeight - go.height ));
			go.scrollLeft = Math.max( 0, Math.min( go.scrollLeft + wx, go.scrollWidth - go.width ));
	
			// if scrolled
			if ( sl != go.scrollLeft || st != go.scrollTop ) {
				// stop event
				stopEvent();
				// dispatch mousemove to counter mouseoff
				go.fire( 'mousemove', Input.mouseX, Input.mouseY );
			}
		}
		
	};

	// create components

	// set name
	go.name = "Scrollable";
	go.ui = new UI( {
		layoutType: Layout.Anchors,
		focusable: false,
		layout: UI.base.scrollablePrototype.__layout,
		mouseWheel: UI.base.scrollablePrototype.__mouseWheel,
		
    });
	go.__container = go.addChild( {
		name: 'Container',
		ui: new UI({
			focusable: false,
			fixedPosition: true,
			scrollable: go,
			layout: UI.base.scrollablePrototype.__containerLayout,
        }),
    } );
	go.__img = new Image();
	go.__img.autoDraw = go.__container;
	go.__spr = new RenderSprite( go.__img );
	go.render = go.__spr;
	go.__scrollbars = '';
	go.__scrollbarsFocusable = true;
	go.__vsb = null;
	go.__hsb = null;
	go.__proto__ = UI.base.scrollablePrototype;
	go.__init();
	go.serializeMask.push( 'children', 'addChild', 'removeChild', 'getChild', 'removeAllChildren' );

	// add property-list inspectable info
	UI.base.addInspectables( go, 'Scrollable',
		[ 'scrollbars', 'scrollbarsFocusable', 'scrollWidth', 'scrollHeight', 'containerChildren' ],
        { 'addChild': false, 'removeChild': false, 'getChild': false, 'removeAllChildren': false }, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.scrollable );
	UI.base.applyProperties( go, go.__baseStyle );
	go.state = 'auto';

})(this);
