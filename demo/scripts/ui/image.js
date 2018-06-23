/*

	Image

	Usage:

		var img = App.scene.addChild( 'ui/image' );
		img.texture = 'mom.png';
		img.mode = 'fit';
		img.width = 32;
		img.height = 32;

	look at mappedProps in source code below for additional properties,
	also has shared layout properties from ui/ui.js


 */

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI(), sc, rs, ri;
	var mode = 'icon';
	var constructing = true;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) texture path
		'texture': { get: function (){ return rs.texture; }, set: function( v ){ rs.texture = v; go.updateParams(); }  },

		// (GameObject) object displaying the image
		'imageObject': { get: function (){ return sc; } },

		// (Image) instance of Image object (alternative to .texture property)
		'image': {
			get: function (){ return rs.image; },
			set: function ( v ) {
				rs.image = v;
				go.updateParams();
			}
		},

		// (String) 'icon', 'fit', 'fill', or 'stretch'
		'mode': { get: function (){ return mode; }, set: function( v ){ mode = v; go.updateParams(); } },

		// (Boolean) image is flipped X
		'flipX': { get: function (){ return rs.flipX; }, set: function( v ){ rs.flipX = v; } },

		// (Boolean) image is flipped X
		'flipY': { get: function (){ return rs.flipY; }, set: function( v ){ rs.flipY = v; } },

		// (Number) image rotation
		'rotation': { get: function (){ return sc.angle; }, set: function( v ){ sc.angle = v; } },

		// (Color) | (Number) .addColor property of current render component
		'addColor': { get: function (){ return rs.addColor; }, set: function( v ){ rs.addColor = v; } },

		// (Color) | (Number) .color property of current render component
		'color': { get: function (){ return rs.color; }, set: function( v ){ rs.color = v; } },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );
	UI.base.addInspectables( go, 'Button',
		[ 'mode', 'texture', 'rotation', 'flipX', 'flipY', 'color', 'addColor' ],
		{ 'texture': { autocomplete: 'file', autocompleteParam: 'textures;png,jpg,jpeg' },
		  'mode': { enum: [ 'icon', 'fit', 'fill', 'stretch' ] },
		  'color': { inline: true },
		  'addColor': { inline: true }, }, 1 );

	// create components

	// set name
	go.name = "Image";

	sc = go.addChild();
	sc.name = "Image.Container";
	sc.serializeable = false;
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

		// background

		// center container
		sc.setTransform( w * 0.5, h * 0.5 );
		if ( !(rs.texture || rs.image ) ) return;

		// mode
		var _scale = 1;
		var _w = rs.originalWidth, _h = rs.originalHeight;
		if ( mode == 'fit' ) {
			/*if ( go.parent && go.parent.ui ) {
				// parent is vertical list, fit-x
				if ( go.parent.ui.layoutType == Layout.Vertical ) {
					ui.minHeight = h = w * ( rs.width / rs.height );
				// parent is horizontal list, fit-y
				} else if ( go.parent.ui.layoutType == Layout.Horizontal ) {
					ui.minWidth = w = h / ( rs.width / rs.height );
				}
			}*/
			_scale = Math.min( w / rs.originalWidth, h / rs.originalHeight );
		} else if ( mode == 'stretch' ) {
			_w = w; _h = h;
		} else if ( mode == 'fill' ) {
			_scale = Math.max( w / rs.originalWidth, h / rs.originalHeight );
		}
		sc.scale = _scale;
		rs.resize( _w, _h );

		if ( go.render ) go.render.resize( w, h );

		// refire
		go.fire( 'layout', w, h );
	}

	// update after params change
	go.updateParams = function () {
		if ( mode == 'icon' && (rs.texture || rs.image) ){
			// min size is set to icon size
			ui.minWidth = rs.originalWidth;
			ui.minHeight = rs.originalHeight;
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
			ri = null;
			go.render = null;
		}
		ui.requestLayout( 'image/updateParams' );
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.image );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);