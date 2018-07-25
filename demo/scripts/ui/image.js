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

	// API properties
	UI.base.imagePrototype = UI.base.imagePrototype || {

		__proto__: UI.base.componentPrototype,

		// (String) texture path
		get texture(){ return this.__rs.texture; }, set texture( v ){ this.__rs.texture = v; this.__updateParams(); },

		// (GameObject) object displaying the image
		get imageObject(){ return this.__sc; },

		// (Image) instance of Image object (alternative to .texture property)
		get image(){ return this.__rs.image; },
		set image( v ) { this.__rs.image = v; this.__updateParams(); },

		// (String) 'icon', 'fit', 'fill', or 'stretch'
		get mode(){ return this.__mode; }, set mode( v ){ this.__mode = v; this.__updateParams(); },

		// (Boolean) image is flipped X
		get flipX(){ return this.__rs.flipX; }, set flipX( v ){ this.__rs.flipX = v; },

		// (Boolean) image is flipped X
		get flipY(){ return this.__rs.flipY; }, set flipY( v ){ this.__rs.flipY = v; },

		// (Number) image rotation
		get rotation(){ return this.__sc.angle; }, set rotation( v ){ this.__sc.angle = v; },

		// (Color) | (Number) .addColor property of current render component
		get addColor(){ return this.__rs.addColor; }, set addColor( v ){ this.__rs.addColor = v; },

		// (Color) | (Number) .color property of current render component
		get color(){ return this.__rs.color; }, set color( v ){ this.__rs.color = v; },

		__layout: function( w, h ) {
			var go = this.gameObject;
			var rs = go.__rs, sc = go.__sc;
			
			// center container
			sc.setTransform( w * 0.5, h * 0.5 );
			if ( !(rs.texture || rs.image ) ) return;
	
			// mode
			var _scale = 1;
			var _w = rs.originalWidth, _h = rs.originalHeight;
			if ( go.__mode == 'fit' ) {
				_scale = Math.min( w / rs.originalWidth, h / rs.originalHeight );
			} else if ( go.__mode == 'stretch' ) {
				_w = w; _h = h;
			} else if ( go.__mode == 'fill' ) {
				_scale = Math.max( w / rs.originalWidth, h / rs.originalHeight );
			}
			sc.scale = _scale;
			rs.resize( _w, _h );
	
			if ( go.render ) go.render.resize( w, h );
	
			// refire
			go.fire( 'layout', w, h );
		},
	
		__updateParams: function () {
			// update after params change
			if ( this.__mode == 'icon' && (this.__rs.texture || this.__rs.image) ){
				// min size is set to icon size
				this.ui.minWidth = this.__rs.originalWidth;
				this.ui.minHeight = this.__rs.originalHeight;
			}
			// fill mode uses Image to crop
			if ( this.__mode == 'fill' ) {
				if ( !this.__ri ) {
					this.__ri = new Image( this.ui.width, this.ui.height );
					this.render = new RenderSprite( this.__ri );
					this.__ri.autoDraw = this.__sc;
				}
			} else if ( this.__ri ) {
				// clear render image (cropping)
				this.__ri = null;
				this.render = null;
			}
			this.ui.requestLayout( 'image/updateParams' );
		},
		
	};
	
	// initialize
	go.name = "Image";
	go.ui = new UI( {
		layoutType: Layout.Anchors,
		focusable: false,
		minWidth: 8, minHeight: 8,
		layout: UI.base.imagePrototype.__layout,
    } );
	go.__mode = 'icon';
	go.__sc = go.addChild( { name: "Image.Container", serializeable: false });
	go.__rs = go.__sc.render = new RenderSprite( { pivot: 0.5 } );
	go.__proto__ = UI.base.imagePrototype;
	go.init();

	// add property-list inspectable info
	UI.base.addInspectables( go, 'Button',
		[ 'mode', 'texture', 'rotation', 'flipX', 'flipY', 'color', 'addColor' ],
		{ 'texture': { autocomplete: 'file', autocompleteParam: 'textures;png,jpg,jpeg' },
		  'mode': { enum: [ 'icon', 'fit', 'fill', 'stretch' ] },
		  'color': { inline: true },
		  'addColor': { inline: true }, }, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.image );
	UI.base.applyProperties( go, go.__baseStyle );

})(this);