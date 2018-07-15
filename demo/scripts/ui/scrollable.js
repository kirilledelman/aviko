/*

	Scrollable container

	If scrollWidth / scrollHeight are bigger than control's width/height, control will
	be scrollable to reveal its contents via mouse wheel, or scroll bars (automatically shown)

	Usage:
		var s = App.scene.addChild( 'ui/scrollable' );
		s.layoutType = Layout.Vertical;
		s.width = 80; s.height = 100;
		s.addChild( .. );
		s.addChild( .. );


	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'scroll' - when content scroll position has changed
		'layout' - during layout

*/

include( './ui' );
(function(go) {

	// internal props
	var ui, spr, img, container = go.addChild();
	container.ui = new UI();
	var scrollbars = '';
	var scrollbarsFocusable = true;
	var constructing = true;
	var vsb = null, hsb = null;
	go.serializeMask = [ 'ui', 'render', 'children', 'updateScrollbars', 'addChild', 'removeChild', 'getChild', 'removeAllChildren' ];

	// API properties
	var mappedProps = {

		// (String) 'auto' or (Boolean) - whether scrollbars are shown automatically, always visible, or never
		'scrollbars': { get: function (){ return scrollbars; }, set: function( s ){
			if ( s != scrollbars ) {
				scrollbars = s;
				go.fireLate( 'updateScrollbars' );
				go.requestLayout();
			}
		}  },

		// (Boolean) whether scrollbars can receive focus
		'scrollbarsFocusable': { get: function (){ return scrollbarsFocusable; }, set: function( v ){
			scrollbarsFocusable = v;
			if ( vsb ) vsb.focusable = scrollbarsFocusable;
			if ( hsb ) hsb.focusable = scrollbarsFocusable;
		}  },


		// (Number) current width of the control
		'width': { get: function (){ return ui.width; }, set: function( w ){
			ui.width = w;
		}  },

		// (Number) current height of the control
		'height': { get: function (){ return ui.height; }, set: function( h ){
			ui.height = h;
		}  },

		// (Number) width of the scrollable container area
		'scrollWidth': { get: function (){ return container.ui.width; }, set: function( w ){
			if ( container.ui.width != w ) {
				container.ui.width = w;
				go.fireLate( 'updateScrollbars' );
			}
		}  },

		// (Number) height of the scrollable container area
		'scrollHeight': { get: function (){ return container.ui.height; }, set: function( h ){
			if ( container.ui.height != h ) {
				container.ui.height = h;
				go.fireLate( 'updateScrollbars' );
			}
		}  },

		// (Number) current y scroll position
		'scrollTop': { get: function (){ return -container.y; }, set: function( t ){
			var py = container.y;
			container.y = Math.min( 0, Math.max( -(container.ui.height - ui.height), -t ) );
			if ( py != container.y ) {
				go.fireLate( 'scroll' );
				go.fireLate( 'updateScrollbars' );
			}
		}  },

		// (Number) current x scroll position
		'scrollLeft': { get: function (){ return -container.x; }, set: function( l ){
			var px = container.x;
			container.x = Math.min( 0, Math.max( -(container.ui.width - ui.width), -l ) );
			if ( px != container.x ) {
				go.fireLate( 'scroll' );
				go.fireLate( 'updateScrollbars' );
			}
		}  },

		// (ui/scrollbar.js) - returns vertical scrollbar instance
		'verticalScrollbar': { get: function (){ return vsb; } },

		// (ui/scrollbar.js) - returns horizontal scrollbar instance
		'horizontalScrollbar': { get: function(){ return hsb; } },

		// (GameObject) the actual container to which all children are added
		'container': { get: function (){ return container; } },

		// (Array) - set child objects at once (for serialization)
		'containerChildren': {
			get: function (){ return container.children; },
			set: function( v ){
				container.children = v; // overridden
			}
		},

		// map back to this ui (not container)

		// (Boolean) for Horizontal and Vertical layouts, force parent to wrap to new row after this element
		'forceWrap': { get: function (){ return ui.forceWrap; }, set: function( v ){ ui.forceWrap = v; }  },

		// (LayoutAlign.Default, LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) overrides parent container's layoutAlignX/Y for this object
		'selfAlign': { get: function (){ return ui.selfAlign; }, set: function( v ){ ui.selfAlign = v; }  },

		// (Number) stretch this element to fill empty space in vertical and horizontal layouts, 0 = no stretch, otherwise proportion rel. to other flex elems
		'flex': { get: function (){ return ui.flex; }, set: function( v ){ ui.flex = v; }  },

		// (Boolean) if true, parent will ignore this element while performing layout
		'fixedPosition': { get: function (){ return ui.fixedPosition; }, set: function( v ){ ui.fixedPosition = v; }  },

		// (Number) minimum width allowed by layout
		'minWidth': { get: function (){ return ui.minWidth; }, set: function( v ){ ui.minWidth = v; }  },

		// (Number) minimum height allowed by layout
		'minHeight': { get: function (){ return ui.minHeight; }, set: function( v ){ ui.minHeight = v; }  },

		// (Number) maximum width allowed by layout
		'maxWidth': { get: function (){ return ui.maxWidth; }, set: function( v ){ ui.maxWidth = v; }  },

		// (Number) maximum height allowed by layout
		'maxHeight': { get: function (){ return ui.maxHeight; }, set: function( v ){ ui.maxHeight = v; }  },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		'anchorLeft': { get: function (){ return ui.anchorLeft; }, set: function( v ){ ui.anchorLeft = v; }  },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		'anchorRight': { get: function (){ return ui.anchorRight; }, set: function( v ){ ui.anchorRight = v; }  },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		'anchorTop': { get: function (){ return ui.anchorTop; }, set: function( v ){ ui.anchorTop = v; }  },

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		'anchorBottom': { get: function (){ return ui.anchorBottom; }, set: function( v ){ ui.anchorBottom = v; }  },

		// (Number) offset from anchorLeft
		'left': { get: function (){ return ui.left; }, set: function( v ){ ui.left = v; }  },

		// (Number) offset from anchorLeft
		'right': { get: function (){ return ui.right; }, set: function( v ){ ui.right = v; }  },

		// (Number) offset from anchorLeft
		'top': { get: function (){ return ui.top; }, set: function( v ){ ui.top = v; }  },

		// (Number) offset from anchorLeft
		'bottom': { get: function (){ return ui.bottom; }, set: function( v ){ ui.bottom = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
		'margin': { get: function (){ return ui.margin; }, set: function( v ){ ui.margin = v; }  },

		// (Number) outer margin top
		'marginTop': { get: function (){ return ui.marginTop; }, set: function( v ){ ui.marginTop = v; }, serialized: false },

		// (Number) outer margin right
		'marginRight': { get: function (){ return ui.marginRight; }, set: function( v ){ ui.marginRight = v; }, serialized: false },

		// (Number) outer margin bottom
		'marginBottom': { get: function (){ return ui.marginBottom; }, set: function( v ){ ui.marginBottom = v; }, serialized: false },

		// (Number) outer margin left
		'marginLeft': { get: function (){ return ui.marginLeft; }, set: function( v ){ ui.marginLeft = v; }, serialized: false },

	};
	UI.base.addSharedProperties( go, container.ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );
	UI.base.addInspectables( go, 'Scrollable',
		[ 'scrollbars', 'scrollbarsFocusable', 'scrollWidth', 'scrollHeight', 'containerChildren' ],
        { 'addChild': false, 'removeChild': false, 'getChild': false, 'removeAllChildren': false }, 1 );
	
	// remapped functions - forward calls to container
	go.addChild = function() { return container.addChild.apply( container, arguments ); }
	go.removeChild = function() { return container.removeChild.apply( container, arguments ); }
	go.getChild = function() { return container.getChild.apply( container, arguments ); }
	go.removeAllChildren = function() { return container.removeAllChildren.apply( container, arguments ); }

	// create components

	// set name
	go.name = "Scrollable";

	// UI
	ui = new UI();
	ui.layoutType = Layout.Anchors;
	ui.focusable = false;
	go.ui = ui;

	// container
	container.name = 'Container';
	container.ui.focusable = false;
	container.ui.fixedPosition = true;

	// image
	img = new Image();
	img.autoDraw = container;
	spr = new RenderSprite( img );
	go.render = spr;

	// init / detach scrollbars
	go.updateScrollbars = function () {
		if ( !scrollbars ) {
			// detach
			if ( vsb ) vsb.parent = null;
			if ( hsb ) hsb.parent = null;
			vsb = hsb = null;
		} else {
			// vertical
			if ( !vsb ) {
				// position to the right
				vsb = new GameObject( './scrollbar' );
				vsb.orientation = 'vertical';
				vsb.focusGroup = container.ui.focusGroup;
				vsb.parent = go;
				vsb.anchorTop = vsb.anchorBottom = vsb.anchorRight = 0;
				vsb.anchorLeft = -1;
				vsb.right = -vsb.width;
				vsb.scroll = function ( ny ) { go.scrollTop = ny; }
				vsb.focusUp = vsb.focusDown = vsb;
				vsb.focusable = scrollbarsFocusable;
			}
			// horizontal
			if ( !hsb ) {
				// position to the right
				hsb = new GameObject( './scrollbar' );
				hsb.orientation = 'horizontal';
				hsb.focusGroup = container.ui.focusGroup;
				hsb.parent = go;
				hsb.anchorLeft = hsb.anchorBottom = hsb.anchorRight = 0;
				hsb.anchorTop = -1;
				hsb.bottom = -hsb.height;
				hsb.scroll = function ( nx ) { go.scrollLeft = nx; }
				hsb.focusLeft = hsb.focusRight = hsb;
				hsb.focusable = scrollbarsFocusable;
			}
		}

		// update vertical scrollbars params
		if ( vsb ) {
			vsb.totalSize = Math.floor( container.ui.height );
			vsb.handleSize = Math.floor( ui.height );
			container.y = Math.min( 0, Math.max( -(container.ui.height - ui.height), container.y ) );
			vsb.position = Math.floor( -container.y );
			// hide / show
			var ac = (scrollbars == 'auto' ? (container.ui.height > ui.height) : scrollbars);
			if ( ac != vsb.active ) {
				vsb.active = ac;
				if ( ac && go.parent ) go.parent.requestLayout( 'scrollable/verticalScrollbar' );
			}
		}
		// update horizontal scrollbars params
		if ( hsb ) {
			hsb.totalSize = Math.floor( container.ui.width );
			hsb.handleSize = Math.floor( ui.width );
			container.x = Math.min( 0, Math.max( -(container.ui.width - ui.width), container.x ) );
			hsb.position = Math.floor( -container.x );
			// hide / show
			var ac = (scrollbars == 'auto' ? (container.ui.width > ui.width) : scrollbars);
			if ( ac != hsb.active ) {
				hsb.active = ac;
				if ( ac && go.parent ) go.parent.requestLayout( 'scrollable/horizontalScrollbar');
			}
		}
	}

	// lay out components
	ui.layout = function( w, h ) {
		var sizeChanged = ( w != spr.width || h != spr.height );
		spr.resize( w, h );
		if ( sizeChanged ) go.updateScrollbars();

		// refire
		go.fire( 'layout', w, h );
	}

	// container resizing updates scrollbars
	container.ui.layout = function() {

		go.debounce( 'updateScrollbars', go.updateScrollbars );

	}

	// scrolling
	ui.mouseWheel = function ( wy, wx ) {
		// scroll
		var st = go.scrollTop, sl = go.scrollLeft;
		go.scrollTop = Math.max( 0, Math.min( go.scrollTop - wy, go.scrollHeight - go.height ));
		go.scrollLeft = Math.max( 0, Math.min( go.scrollLeft + wx, go.scrollWidth - go.width ));

		// stop event if scrolled
		if ( sl != go.scrollLeft || st != go.scrollTop ) stopEvent();
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.scrollable );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;

})(this);
