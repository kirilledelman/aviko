/*

	Control to resize another UI element in a horizontal or vertical layout

	Usage:

		var p = container.addChild( 'ui/resizer' );
		p.target = anotherGameObjectWithUI;
		p.minSize = 300;
		p.maxSize = 600;

	resizer determines the direction (horizontal/vertical) from parent's layout

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js


*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.resizerPrototype = UI.base.resizerPrototype || {

		__proto__: UI.base.componentPrototype,

		// (GameObject) target to resize
		get target() { return this.__target; }, set target( t ) { this.__target = t; },

		// (Number) minimum width or height of target allowed
		get minSize(){ return this.__minSize; }, set minSize( m ) { this.__minSize = m; },

		// (Number) maximum width or height of target allowed
		get maxSize(){ return this.__maxSize; }, set maxSize( m ) { this.__maxSize = m; },

		// (Boolean) clicking on resizer will hide/unhide its target
		get collapsible(){ return this.__collapsible; }, set collapsible( m ) { this.__collapsible = m; },

		// (String) or (Color) or (Number) or (Image) or (null|false) - set background to sprite, or solid color, or nothing
		get background(){ return this.__background; },
		set background( v ){
			this.__background = v;
			if ( v === false || v === null || v === undefined ){
				this.render = null;
			} else if ( typeof( v ) == 'string' ) {
				this.__bg.image = null;
				this.__bg.texture = v;
				this.render = this.__bg;
			} else if ( typeof( v ) == 'object' && v.constructor == Image ){
				this.__bg.image = v;
				this.__bg.texture = null;
				this.render = this.__bg;
			} else {
				this.__shp.color = v;
				this.render = this.__shp;
			}
		},

		// (Number) corner roundness when background is solid color
		get cornerRadius(){ return this.__shp.radius; }, set cornerRadius( b ){ this.__shp.radius = b; this.__shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle; },

		// (Boolean) disabled
		get disabled(){ return this.__disabled; }, set disabled( v ) { this.__disabled = v; if ( !v && this.__dragging ) this.ui.mouseUp(); },

		// (Number) outline thickness when background is solid color
		get lineThickness(){ return this.__shp.lineThickness; }, set lineThickness( b ){ this.__shp.lineThickness = b; },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		get outlineColor(){ return this.__shp.outlineColor; }, set outlineColor( c ){ this.__shp.outlineColor = (c === false ? '00000000' : c ); },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		get filled(){ return this.__shp.filled; }, set filled( v ){ this.__shp.filled = v; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		get slice(){ return this.__bg.slice; }, set slice( v ){ this.__bg.slice = v; },

		// (Number) texture slice top
		get sliceTop(){ return this.__bg.sliceTop; }, set sliceTop( v ){ this.__bg.sliceTop = v; },

		// (Number) texture slice right
		get sliceRight(){ return this.__bg.sliceRight; }, set sliceRight( v ){ this.__bg.sliceRight = v; },

		// (Number) texture slice bottom
		get sliceBottom(){ return this.__bg.sliceBottom; }, set sliceBottom( v ){ this.__bg.sliceBottom = v; },

		// (Number) texture slice left
		get sliceLeft(){ return this.__bg.sliceLeft; }, set sliceLeft( v ){ this.__bg.sliceLeft = v; },

		__setup: function (){
			var p = null;
			this.__mode = false;
			if ( this.__target && this.__target.parent && this.__target.parent.ui ) p = this.__target.parent.ui;
			else if ( this.parent && this.parent.ui ) p = this.parent.ui;
			if ( !p || !this.__target || !this.__target.ui ) return;
	
			if ( p.layoutType == Layout.Horizontal ) {
				this.__minSize = this.__minSize ? this.__minSize : this.__target.ui.minWidth;
				this.__maxSize = this.__maxSize ? this.__maxSize : this.__target.ui.maxWidth;
				this.__mode = 1;
			} else if ( p.layoutType == Layout.Vertical ) {
				this.__minSize = this.__minSize ? this.__minSize : this.__target.ui.minHeight;
				this.__maxSize = this.__maxSize ? this.__maxSize : this.__target.ui.maxHeight;
				this.__mode = 2;
			}
			// resize direction
			var cc = this.parent.children;
			this.__reversedOffset = cc.indexOf( this ) > cc.indexOf( this.__target ) ? -1 : 1;
		},
	
		__layout: function( w, h ) {
			this.gameObject.__shp.resize( w, h );
			this.gameObject.__bg.resize( w, h );
		},
	
		__mouseOver: function () {
			this.gameObject.state = 'auto';
		},
	
		__mouseOutUp: function () {
			this.gameObject.state = ( this.gameObject.__target && !this.gameObject.__target.active ) ? 'collapsed' : 'off';
		},
	
		__mouseDown: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			go.__setup();
			if ( btn != 1 || go.__disabled || go.__dragging || !go.__mode ) return;
			go.state = 'dragging';
			stopAllEvents();
			go.__dragging = true;
			go.__dragX = wx;
			go.__dragY = wy;
			go.__moved = false;
			go.__startSize = ( go.__mode == 1 ? go.__target.ui.width : go.__target.ui.height );
			go.__dragResize = go.__dragResize.bind( go );
			go.__dragEnd = go.__dragEnd.bind( go );
			Input.mouseMove = go.__dragResize;
			Input.mouseUp = go.__dragEnd;
		},
	
		__dragResize: function ( wx, wy ) {
			stopAllEvents();
			var val;
	
			// a little threshold
			if ( !this.__moved ) {
				if ( Math.abs( wx - this.__dragX ) > 2 || Math.abs( this.__dragY - wy ) > 2 ) this.__moved = true;
			}
	
			// resize
			if( this.__moved ) {
				if ( !this.__target.active ) {
					this.__target.active = true;
					this.__dragX = wx; this.__dragY = wy;
				}
				if ( this.__mode == 1 ) {
					val = this.__startSize + ( this.__dragX - wx ) * this.__reversedOffset;
					this.__target.ui.minWidth = Math.min( this.__maxSize ? this.__maxSize : 99999, Math.max( this.__minSize, val ) );
				} else if ( this.__mode == 2 ) {
					val = this.__startSize + ( this.__dragY - wy ) * this.__reversedOffset;
					this.__target.ui.minHeight = Math.min( this.__maxSize ? this.__maxSize : 99999, Math.max( this.__minSize, val ) );
				}
			}
		},
	
		__dragEnd: function ( btn, wx, wy ) {
			if ( btn != 1 || !this.__dragging ) return;
			stopAllEvents();
			this.__dragging = false;
			Input.mouseMove = Input.mouseUp = null;
			// if collapsible
			if ( this.__collapsible && !this.__moved ) {
				this.__target.active = !this.__target.active;
				this.state = this.__target.active ? 'off' : 'collapsed';
			} else this.state = 'off';
		},
		
	};

	// initialize
	go.name = "Resizer";
	go.ui = new UI( {
		autoMoveFocus: false,
		layoutType: Layout.None,
		focusable: false,
		layout: UI.base.resizerPrototype.__layout,
		mouseOver: UI.base.resizerPrototype.__mouseOver,
		mouseOut: UI.base.resizerPrototype.__mouseOutUp,
		mouseDown: UI.base.resizerPrototype.__mouseDown,
		mouseUp: UI.base.resizerPrototype.__mouseOutUp,
    } );
	go.__bg = new RenderSprite();
	go.render = go.__bg;
	go.__shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});
	go.__target = null;
	go.__minSize = 0;
	go.__maxSize = 0;
	go.__mode = false;
	go.__disabled = false;
	go.__collapsible = false;
	go.__moved = false;
	go.__reversedOffset = false;
	go.__dragX = 0;
	go.__dragY = 0;
	go.__dragging = false;
	go.__startSize = 0;
	go.__proto__ = UI.base.resizerPrototype;
	go.__init();
	go.cloneMask = [ 'target' ];
	go.serializeMask.push( 'sliceLeft', 'sliceRight', 'sliceBottom', 'sliceTop' );
	
	// add property-list inspectable info
	UI.base.addInspectables( go, 'Resizer',
	[ 'target', 'minSize', 'maxSize', 'collapsible', 'disabled', 'background', 'outlineColor', 'lineThickness', 'filled', 'cornerRadius', 'sliceLeft', 'sliceTop', 'sliceRight', 'sliceBottom' ],
	null, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.resizer );
	UI.base.applyProperties( go, go.__baseStyle );

})(this);
