/*

	General container panel

	Used to help with controls arrangement, has background property

	Usage:

		var p = App.scene.addChild( 'ui/panel' );
		p.background = 0x666666;
		p.layoutType = Layout.Vertical;
		p.addChild( .. );

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js


*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.panelPrototype = UI.base.panelPrototype || {

		__proto__: UI.base.componentPrototype,

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
					this.__background = this.__shp.color;
				}
				this.requestLayout();
			},

		// (Number) corner roundness when background is solid color
		get cornerRadius(){ return this.__shp.radius; },
		set cornerRadius( b ){
				this.__shp.radius = b;
				this.__shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
			},

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

		__layout: function( w, h ) {
			this.gameObject.__shp.resize( w, h );
			this.gameObject.__bg.resize( w, h );
			this.gameObject.fire( 'layout', w, h );
		}
		
	};

	// initialize
	go.name = "Panel";
	go.ui = new UI( {
		autoMoveFocus: false,
		minWidth: 8,
		minHeight: 8,
		layoutType: Layout.Anchors,
		fitChildren: true,
		focusable: false,
		layout: UI.base.panelPrototype.__layout
	} );
	go.__background = false;
	go.__bg = /* go.render = */ new RenderSprite();
	go.__shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});
	go.__proto__ = UI.base.panelPrototype;
	go.__init();
	go.serializeMask.push( 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' );
	
	// add property-list inspectable info
	UI.base.addInspectables( go, 'Panel',
	[ 'background', 'outlineColor', 'lineThickness', 'filled', 'cornerRadius', 'sliceLeft', 'sliceTop', 'sliceRight', 'sliceBottom' ], null, 1 );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.panel );
	UI.base.applyProperties( go, go.__baseStyle );

})(this);
