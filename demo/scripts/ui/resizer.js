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

	// internal props
	var ui = new UI(), bg, shp, background = false;
	var constructing = true;
	var target = null;
	var minSize = 0;
	var maxSize = 0;
	var mode = false;
	var disabled = false;
	var collapsible = false;
	var moved = false;
	var reversedOffset = false;
	var dragX, dragY, dragging = false, startSize = 0;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (GameObject) target to resize
		'target': { get: function () { return target; }, set: function ( t ) { target = t; } },

		// (Number) minimum width or height of target allowed
		'minSize': { get: function(){ return minSize; }, set: function ( m ) { minSize = m; } },

		// (Number) maximum width or height of target allowed
		'maxSize': { get: function(){ return maxSize; }, set: function ( m ) { maxSize = m; } },

		// (Boolean) clicking on resizer will hide/unhide its target
		'collapsible': { get: function(){ return collapsible; }, set: function ( m ) { collapsible = m; } },

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

		// (Boolean) disabled
		'disabled': { get: function (){ return disabled; }, set: function( v ) { disabled = v; if ( !v && dragging ) ui.mouseUp(); } },

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
	UI.base.addInspectables( go, 'Resizer',
	[ 'target', 'minSize', 'maxSize', 'collapsible', 'disabled', 'background', 'outlineColor', 'lineThickness', 'filled', 'cornerRadius', 'sliceLeft', 'sliceTop', 'sliceRight', 'sliceBottom' ],
	null, 1 );

	// create components

	// set name
	if ( !go.name ) go.name = "Resizer";

	bg = new RenderSprite( background );
	go.render = bg;

	// solid color background
	shp = new RenderShape( Shape.Rectangle, {
		radius: 0,
		filled: true,
		centered: false
	});

	//
	function setup(){
		var p = null;
		mode = false;
		if ( target && target.parent && target.parent.ui ) p = target.parent.ui;
		else if ( go.parent && go.parent.ui ) p = go.parent.ui;
		if ( !p || !target || !target.ui ) return;

		if ( p.layoutType == Layout.Horizontal ) {
			minSize = minSize ? minSize : target.ui.minWidth;
			maxSize = maxSize ? maxSize : target.ui.maxWidth;
			mode = 1;
		} else if ( p.layoutType == Layout.Vertical ) {
			minSize = minSize ? minSize : target.ui.minHeight;
			maxSize = maxSize ? maxSize : target.ui.maxHeight;
			mode = 2;
		}
		// resize direction
		var cc = go.parent.children;
		reversedOffset = cc.indexOf( go ) > cc.indexOf( target ) ? -1 : 1;
	}

	// UI
	ui.autoMoveFocus = false;
	ui.layoutType = Layout.None;
	ui.focusable = false;
	go.ui = ui;

	// lay out components
	ui.layout = function( w, h ) {
		shp.resize( w, h );
		bg.resize( w, h );
	}

	ui.mouseOver = function () {
		go.style = 'auto';
	}

	ui.mouseOut = ui.mouseUp = function () {
		go.style = ( target && !target.active ) ? 'collapsed' : 'off';
	}

	ui.mouseDown = function ( btn, x, y, wx, wy ) {
		setup();
		if ( btn != 1 || disabled || dragging || !mode ) return;
		go.state = 'dragging';
		stopAllEvents();
		dragging = true;
		dragX = wx;
		dragY = wy;
		moved = false;
		startSize = ( mode == 1 ? target.ui.width : target.ui.height );
		Input.mouseMove = dragResize;
		Input.mouseUp = dragEnd;
	}

	function dragResize( wx, wy ) {
		stopAllEvents();
		var val;

		// a little threshold
		if ( !moved ) {
			if ( Math.abs( wx - dragX ) > 2 || Math.abs( dragY - wy ) > 2 ) moved = true;
		}

		// resize
		if( moved ) {
			if ( !target.active ) {
				target.active = true;
				dragX = wx; dragY = wy;
			}
			if ( mode == 1 ) {
				val = startSize + ( dragX - wx ) * reversedOffset;
				target.ui.minWidth = Math.min( maxSize ? maxSize : 99999, Math.max( minSize, val ) );
			} else if ( mode == 2 ) {
				val = startSize + ( dragY - wy ) * reversedOffset;
				target.ui.minHeight = Math.min( maxSize ? maxSize : 99999, Math.max( minSize, val ) );
			}
		}
	}

	// up
	function dragEnd ( btn, wx, wy ) {
		if ( btn != 1 || !dragging ) return;
		stopAllEvents();
		dragging = false;
		Input.mouseMove = Input.mouseUp = null;
		// if collapsible
		if ( collapsible && !moved ) {
			target.active = !target.active;
			go.state = target.active ? 'off' : 'collapsed';
		} else go.state = 'off';
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.resizer );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

})(this);
