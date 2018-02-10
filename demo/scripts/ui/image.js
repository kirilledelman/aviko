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

		// (String) 'icon', 'fit', 'fill', or 'stretch'
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

	// set name
	if ( !go.name ) go.name = "Image";

	sc = go.addChild();
	sc.name = "Image.Container";
	sc.serialized = false;
	rs = new RenderSprite();
	rs.pivotX = rs.pivotY = 0.5;
	sc.render = rs;

	// UI
	ui.layoutType = Layout.Anchors;
	ui.focusable = false;
	ui.minWidth = ui.minHeight = 8;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {

		sc.setTransform( w * 0.5, h * 0.5 );
		if ( !(rs.texture || rs.image ) ) return;

		// mode
		if ( mode == 'icon' ){
			// do nothing
		} else if ( mode == 'fit' ) {
			if ( go.parent && go.parent.ui ) {
				// parent is vertical list, fit-x
				if ( go.parent.ui.layoutType == Layout.Vertical ) {
					ui.height = h = w * ( rs.width / rs.height );
				// parent is horizontal list, fit-y
				} else if ( go.parent.ui.layoutType == Layout.Horizontal ) {
					ui.width = w = h / ( rs.width / rs.height );
				}
			}
			sc.scale = Math.min( w / rs.width, h / rs.height );
		} else if ( mode == 'stretch' ) {
			rs.resize( w, h );
		} else if ( mode == 'fill' ) {
			sc.scale = Math.max( w / rs.width, h / rs.height );
		}
		// image size
		if ( go.render && (ri.width != w || ri.height != h) ) {
			go.render.resize( w, h );
		}
	}

	// update after params change
	go.updateParams = function () {
		if ( mode == 'icon' && (rs.texture || rs.image) ){
			// min size is set to icon size
			ui.minWidth = rs.width;
			ui.minHeight = rs.height;
		}
		// fill mode uses Image to crop
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
		go.fire( 'layout' );
		ui.requestLayout( 'image/updateParams' );
	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.image );

})(this);