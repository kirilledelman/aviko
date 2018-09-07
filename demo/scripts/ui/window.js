/*

	Draggable / resizable container

	Usage:

		var w = App.scene.addChild( 'ui/window' );
		w.title = "Hello";
		w.background = 0x666666;
		w.layoutType = Layout.Vertical;
		w.addChild( .. );

	padding on top is reserved for window header

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'closed' - dispatched after close button is clicked (.active has been set to false)


*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.windowPrototype = UI.base.windowPrototype || {

		__proto__: UI.base.componentPrototype,

		// (ui/panel) reference to window header
		get header(){ return this.__header; },

		// (ui/text) reference to window title
		get titleText(){ return this.__titleText; },

		// (ui/button) reference to window close button
		get closeButton(){ return this.__closeButton; },

		// (String) window title
		get title(){ return this.__titleText.text; }, set title( t ){ this.__titleText.text = t; },

		// (Boolean) window can be dragged
		get draggable(){ return this.__draggable; }, set draggable( d ){ this.__draggable = d; },

		// (Boolean) window can be resized
		get resizable(){ return this.__resizable; }, set resizable( r ){ this.__resizable = r; },

		// (Boolean) window blocks UI events outside its bounds
		get modal(){ return !!this.__modalBackground; },
			set modal( m ){
				if ( m && !this.__modalBackground ) {
					this.__modalBackground = new GameObject( './panel', {
						width: App.windowWidth,
						height: App.windowHeight,
						style: this.__baseStyle.modalBackground,
						blocking: true,
					} );
					if ( this.parent ) {
						this.parent.addChild( this.__modalBackground, this.parent.children.indexOf( this ) );
					}
					this.__draggable = this.__resizable = false;
				} else if ( this.__modalBackground && !m ) {
					this.__modalBackground.parent = null;
					this.__modalBackground = null;
				}
			},

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

		// close window
		close: function () {
			if ( this.__modalBackground ) {
				this.parent = null;
			} else {
				this.active = false;
			}
			this.fire( 'closed' );
		},
		
		__layout: function( w, h ) {
			var go = this.gameObject;
			go.__shp.resize( w, h );
			go.__bg.resize( w, h );
			// resize title
			go.__header.width = w ;
			this.padTop = go.__header.height + go.__header.marginBottom;
			// modal is centered
			if ( go.__modalBackground ) {
				go.__modalBackground.resize( App.windowWidth, App.windowHeight );
				go.setTransform( ( App.windowWidth - w ) * 0.5, ( App.windowHeight - h ) * 0.5 );
			}
			go.fire( 'layout', w, h );
		},
	
		// dragging window
		__dragCallback: function ( x, y ) {
			var go = this.gameObject;
			if ( go.__dragging == 'window' ) {
				go.setTransform( x - go.__dragOffsetX, y - go.__dragOffsetY );
			} else if ( go.__dragging == 'width' ) {
				this.width = Math.max( this.minWidth, x - go.x );
			} else if ( go.__dragging == 'height' ) {
				this.height = Math.max( this.minHeight, y - go.y );
			} else if ( go.__dragging == 'widthheight' ) {
				this.width = Math.max( this.minWidth, x - go.x );
				this.height = Math.max( this.minHeight, y - go.y );
			}
			stopAllEvents();
		},
	
		__mouseUpCallback: function ( btn, x, y ) {
			var go = this.gameObject;
			if ( go.__dragging ) {
				Input.mouseMove = null;
				Input.mouseUp = null;
				go.eventMask.length = 0;
				go.opacity = 1;
				go.__dragging = false;
			}
			stopAllEvents();
		},
	
		__mouseDown: function ( btn, x, y ) {
			var go = this.gameObject;
			if ( go.__dragging ) return;
			go.__dragOffsetX = x; go.__dragOffsetY = y;
			stopAllEvents();
			if ( go.__draggable && y < go.__header.height ) {
				go.__dragging = 'window';
			} else if ( go.__resizable ) {
				if ( x >= this.width - this.padRight ) {
					go.__dragging = 'width';
				}
				if ( y >= this.height - this.padBottom ) {
					go.__dragging = ( go.__dragging ? go.__dragging : '' ) + 'height';
				}
				if ( !go.__dragging ) return;
			} else return;
			go.eventMask = [ 'mouseUp', 'mouseMove' ];
			Input.mouseMove = go.__dragCallback.bind( this );
			Input.mouseUp = go.__mouseUpCallback.bind( this );
			go.opacity = 0.7;
		},
	
		// add modal background under window if present
		added: function () {
			cancelDebouncer( 'showTooltip' );
			if ( go.__modalBackground ) {
				go.parent.addChild( go.__modalBackground, this.parent.children.indexOf( this ) );
			}
		},
	
		// remove modal background if present
		removed: function () {
			if ( go.__modalBackground ) go.__modalBackground.parent = null;
		},
		
	};
	
	// init
	go.name = "Window";
	go.ui = new UI( {
		autoMoveFocus: false,
		layoutType: Layout.Vertical,
		fitChildren: false,
		focusable: false,
		blocking: true,
		layout: UI.base.windowPrototype.__layout,
		mouseDown: UI.base.windowPrototype.__mouseDown
    } );
	go.__background = false;
	go.__modalBackground = null;
	go.__draggable = true;
	go.__resizable = true;
	go.__dragOffsetX = 0;
	go.__dragOffsetY = 0;
	go.__dragging = false;
	go.__bg = go.render = new RenderSprite();
	go.__shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});
	go.__header = go.addChild( './panel', {
		fixedPosition: true,
		x: 0, y: 0,
		layoutType: Layout.Horizontal,
		layoutAlignY: LayoutAlign.Center,
		fitChildren: false,
	});
	go.__titleText = go.__header.addChild( './text', {
		flex: 1
	} );
	go.__closeButton = go.__header.addChild( './button', {
		click: UI.base.windowPrototype.close.bind( go ),
		focusGroup: 'window'
	} );
	go.__proto__ = UI.base.windowPrototype;
	go.init();
	go.serializeMask.push( 'added', 'removed', 'close' );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.window );
	UI.base.applyProperties( go, go.__baseStyle );

})(this);
