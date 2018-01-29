/*

	Image

	Usage:
		var img = app.scene.addChild( 'ui/image' );
		img.texture = 'mom.png';
		img.mode = 'fit';
		img.width = 32; img.height = 32;

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js


 */

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), sc, rs, ri;
	var mode = 'icon';
	go.serializeMask = { 'ui':1, 'render':1 };

	// API properties
	var mappedProps = [

		// (String) texture path
		[ 'texture', function (){ return rs.texture; }, function ( v ){ rs.texture = v; go.updateParams(); } ],

		// (Image) instance of Image object (alternative to .texture property)
		[ 'image',  function (){ return rs.image; }, function ( v ){ rs.image = v; go.updateParams(); } ],

		// (String) 'icon', 'fit', 'fitx', 'fity', 'fill', or 'stretch'
		[ 'mode',  function (){ return mode; }, function ( v ){ mode = v; go.updateParams(); } ]

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

	// create components
	sc = go.addChild();
	sc.serialized = false;
	rs = new RenderSprite();
	rs.pivotX = rs.pivotY = 0.5;
	sc.render = rs;

	// UI
	ui.layoutType = Layout.Anchors;
	ui.focusable = false;
	go.ui = ui;

	// lay out components
	ui.layout = function( x, y, w, h ) {

		go.setTransform( x, y );
		sc.setTransform( w * 0.5, h * 0.5 );
		if ( !rs.texture ) return;

		// mode
		if ( mode == 'icon' ){
			// nothing
		} else if ( mode == 'fit' ) {
			sc.scale = Math.min( w / rs.width, h / rs.height );
		} else if ( mode == 'fitx' ) {
			ui.height = h = w * ( rs.width / rs.height );
			sc.scale = Math.min( w / rs.width, h / rs.height );
		} else if ( mode == 'fity' ) {
			ui.width = w = h / ( rs.width / rs.height );
			sc.scale = Math.min( w / rs.width, h / rs.height );
		} else if ( mode == 'stretch' ) {
			rs.setSize( w, h );
		} else if ( mode == 'fill' ) {
			sc.scale = Math.max( w / rs.width, h / rs.height );
		}
		// image size
		if ( go.render && (ri.width != w || ri.height != h) ) {
			go.render.setSize( w, h );
		}
	}

	// update after params change
	go.updateParams = function () {

		if ( mode == 'icon' ){
			// min size is set to icon size
			ui.width = ui.minWidth = rs.width;
			ui.height = ui.minHeight = rs.height;
		}

		if ( mode == 'fill' ) {
			if ( !ri ) {
				ri = new Image( ui.width, ui.height );
				go.render = new RenderSprite( ri );
				ri.autoDraw = sc;
			}
		} else if ( ri ) {
			// clear render image (cropping)
			go.render = ri = null;
		}

		go.fireLate( 'layout' );
	}

	// apply defaults
	if ( UI.style && UI.style.image ) for ( var p in UI.style.image ) go[ p ] = UI.style.image[ p ];

})(this);