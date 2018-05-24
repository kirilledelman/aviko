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

	// internal props
	var ui = new UI(), bg, shp, background = false;
	var constructing = true;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) or (Color) or (Number) or (Image) or (null|false) - set background to sprite, or solid color, or nothing
		'background': {
			get: function (){ return background; },
			set: function ( v ){
				background = v;
				if ( v === false || v === null || v === undefined ){
					go.render = null;
				} else if ( typeof( v ) == 'string' ) {
					bg.image = null;
					bg.texture = v;
					go.render = bg;
				} else if ( typeof( v ) == 'object' && v.constructor == Image ){
					bg.image = v;
					bg.texture = null;
					go.render = bg;
				} else {
					shp.color = v;
					go.render = shp;
				}
			}
		},

		// (Number) corner roundness when background is solid color
		'cornerRadius': {
			get: function (){ return shp.radius; },
			set: function ( b ){
				shp.radius = b;
				shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
			}
		},

		// (Number) outline thickness when background is solid color
		'lineThickness': { get: function (){ return shp.lineThickness; }, set: function ( b ){ shp.lineThickness = b; } },

		// (String) or (Color) or (Number) or (Boolean) - color of shape outline when background is solid
		'outlineColor': { get: function (){ return shp.outlineColor; }, set: function ( c ){ shp.outlineColor = (c === false ? '00000000' : c ); } },

		// (Boolean) when background is solid color, controls whether it's a filled rectangle or an outline
		'filled': { get: function (){ return shp.filled; }, set: function( v ){ shp.filled = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		'slice': { get: function (){ return bg.slice; }, set: function( v ){ bg.slice = v; }  },

		// (Number) texture slice top
		'sliceTop': { get: function (){ return bg.sliceTop; }, set: function( v ){ bg.sliceTop = v; }, serialized: false },

		// (Number) texture slice right
		'sliceRight': { get: function (){ return bg.sliceRight; }, set: function( v ){ bg.sliceRight = v; }, serialized: false },

		// (Number) texture slice bottom
		'sliceBottom': { get: function (){ return bg.sliceBottom; }, set: function( v ){ bg.sliceBottom = v; }, serialized: false },

		// (Number) texture slice left
		'sliceLeft': { get: function (){ return bg.sliceLeft; }, set: function( v ){ bg.sliceLeft = v; }, serialized: false },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "Panel";

	bg = new RenderSprite( background );
	go.render = bg;

	// solid color background
	shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});

	// UI
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = ui.padLeft + ui.padRight;
	ui.height = ui.minHeight = ui.padTop + ui.padBottom;
	ui.layoutType = Layout.Anchors;
	ui.fitChildren = true;
	ui.focusable = false;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {
		shp.resize( w, h );
		bg.resize( w, h );
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.panel );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);
