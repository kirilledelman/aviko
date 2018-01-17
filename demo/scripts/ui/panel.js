/*

General container panel




*/

include( './ui' );
(function(go) {

	// internal props
	var ui, spr, shp, background = UI.style.texturesFolder + UI.style.panel;

	// API properties
	var mappedProps = [

		// (string) or (Color) or (Number) or (null|false)- set background to sprite, or solid color, or nothing
		[ 'background',  function (){ return background; }, function ( v ){
			background = v;
			if ( typeof( v ) == 'string' ) {
				spr.texture = v;
				go.render = spr;
			} else if ( v === false || v === null || v === undefined ){
				go.render = null;
			} else {
				shp.color = v;
				go.render = shp;
			}
		}]

	];
	UI.base.addSharedProperties( go ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
	}

	// create components
	spr = new RenderSprite( background );
	spr.slice = UI.style.panelSlice;

	// solid color background
	shp = new RenderShape( Shape.Rectangle );
	shp.filled = true; shp.centered = false;
	shp.color = UI.style.panelBackgroundColor;

	// UI
	ui = new UI();
	ui.autoMoveFocus = false;
	ui.width = ui.minWidth = UI.style.panelPadding[ 3 ] + UI.style.panelPadding[ 1 ];
	ui.height = ui.minHeight = UI.style.panelPadding[ 0 ] + UI.style.panelPadding[ 2 ];
	ui.layoutType = Layout.Anchors;
	ui.spacing = UI.style.panelSpacing;
	ui.focusable = false;

	// components are added after component is awake
	go.awake = function () {
		go.ui = ui;
		go.render = ( typeof( background ) == 'string' ? spr : shp );
	};

	// lay out components
	ui.layout = function( x, y, w, h ) {
		go.setTransform( x, y );
		shp.width = spr.width = w;
		shp.height = spr.height = h;
	}

})(this);
