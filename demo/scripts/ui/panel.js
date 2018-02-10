/*

	General container panel

	Used to help with controls arrangement, has background property

	Usage:
		var p = app.scene.addChild( 'ui/panel' );
		s.background = 0x666666;
		s.layoutType = Layout.Vertical;
		s.addChild( .. );

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js


*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), bg, shp, background = false;
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) or (Color) or (Number) or (null|false)- set background to sprite, or solid color, or nothing
		[ 'background',  function (){ return background; }, function ( v ){
			background = v;
			if ( typeof( v ) == 'string' ) {
				bg.texture = v;
				go.render = bg;
			} else if ( v === false || v === null || v === undefined ){
				go.render = null;
			} else {
				shp.color = v;
				go.render = shp;
			}
		}],

		// (Number) corner roundness when background is solid color
		[ 'cornerRadius',  function (){ return shp.radius; }, function ( b ){
			shp.radius = b;
			shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		} ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - background texture slice
		[ 'slice',  function (){ return bg.slice; }, function ( v ){ bg.slice = v; } ],

		// (Number) texture slice top
		[ 'sliceTop',  function (){ return bg.sliceTop; }, function ( v ){ bg.sliceTop = v; }, true ],

		// (Number) texture slice right
		[ 'sliceRight',  function (){ return bg.sliceRight; }, function ( v ){ bg.sliceRight = v; }, true ],

		// (Number) texture slice bottom
		[ 'sliceBottom',  function (){ return bg.sliceBottom; }, function ( v ){ bg.sliceBottom = v; }, true ],

		// (Number) texture slice left
		[ 'sliceLeft',  function (){ return bg.sliceLeft; }, function ( v ){ bg.sliceLeft = v; }, true ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

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
	ui.focusable = false;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {
		shp.width = bg.width = w;
		shp.height = bg.height = h;
	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.panel );

})(this);
