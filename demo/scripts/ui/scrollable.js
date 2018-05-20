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


*/

include( './ui' );
(function(go) {

	// internal props
	var ui, spr, img, container = go.addChild();
	container.ui = new UI();
	go.serializeMask = { 'ui':1, 'render':1 };
	var scrollbars = false;
	var scrollbarsFocusable = true;
	var constructing = true;
	var vsb = null, hsb = null;

	// API properties
	var mappedProps = [

		// (Array) - set child objects at once - overrides built-in
		[ 'children',  function (){ return container.children; }, function ( v ){
			container.children = v; // overridden
		}],

		// (String) 'auto' or (Boolean) - whether scrollbars are shown automatically, always visible, or never
		[ 'scrollbars',  function (){ return scrollbars; }, function ( s ){
			if ( s != scrollbars ) {
				scrollbars = s;
				go.fireLate( 'updateScrollbars' );
			}
		} ],

		// (Boolean) whether scrollbars can receive focus
		[ 'scrollbarsFocusable',  function (){ return scrollbarsFocusable; }, function ( v ){
			scrollbarsFocusable = v;
			if ( vsb ) vsb.focusable = scrollbarsFocusable;
			if ( hsb ) hsb.focusable = scrollbarsFocusable;
		} ],


		// (Number) current width of the control
		[ 'width',  function (){ return ui.width; }, function ( w ){
			ui.width = w;
		} ],

		// (Number) current height of the control
		[ 'height',  function (){ return ui.height; }, function ( h ){
			ui.height = h;
		} ],

		// (Number) width of the scrollable container area
		[ 'scrollWidth',  function (){ return container.ui.width; }, function ( w ){
			if ( container.ui.width != w ) {
				container.ui.width = w;
				go.fireLate( 'updateScrollbars' );
			}
		} ],

		// (Number) height of the scrollable container area
		[ 'scrollHeight',  function (){ return container.ui.height; }, function ( h ){
			if ( container.ui.height != h ) {
				container.ui.height = h;
				go.fireLate( 'updateScrollbars' );
			}
		} ],

		// (Number) current y scroll position
		[ 'scrollTop',  function (){ return -container.y; }, function ( t ){
			var py = container.y;
			container.y = Math.min( 0, Math.max( -(container.ui.height - ui.height), -t ) );
			if ( py != container.y ) {
				go.fireLate( 'scroll' );
				go.fireLate( 'updateScrollbars' );
			}
		} ],

		// (Number) current x scroll position
		[ 'scrollLeft',  function (){ return -container.x; }, function ( l ){
			var px = container.x;
			container.x = Math.min( 0, Math.max( -(container.ui.width - ui.width), -l ) );
			if ( px != container.x ) {
				go.fireLate( 'scroll' );
				go.fireLate( 'updateScrollbars' );
			}
		} ],

		// (ui/scrollbar.js) - returns vertical scrollbar instance
		[ 'verticalScrollbar',  function (){ return vsb; } ],

		// (ui/scrollbar.js) - returns horizontal scrollbar instance
		[ 'horizontalScrollbar',  function (){ return hsb; } ],

		// (GameObject) the actual container to which all children are added
		[ 'container',  function (){ return container; } ],

		// map back to this ui (not container)

		// (Boolean) for Horizontal and Vertical layouts, force parent to wrap to new row after this element
		[ 'forceWrap',  function (){ return ui.forceWrap; }, function ( v ){ ui.forceWrap = v; } ],

		// (LayoutAlign.Default, LayoutAlign.Start, LayoutAlign.Center, LayoutAlign.End, LayoutAlign.Stretch) overrides parent container's layoutAlignX/Y for this object
		[ 'selfAlign',  function (){ return ui.selfAlign; }, function ( v ){ ui.selfAlign = v; } ],

		// (Number) stretch this element to fill empty space in vertical and horizontal layouts, 0 = no stretch, otherwise proportion rel. to other flex elems
		[ 'flex',  function (){ return ui.flex; }, function ( v ){ ui.flex = v; } ],

		// (Boolean) if true, parent will ignore this element while performing layout
		[ 'fixedPosition',  function (){ return ui.fixedPosition; }, function ( v ){ ui.fixedPosition = v; } ],

		// (Number) minimum width allowed by layout
		[ 'minWidth',  function (){ return ui.minWidth; }, function ( v ){ ui.minWidth = v; } ],

		// (Number) minimum height allowed by layout
		[ 'minHeight',  function (){ return ui.minHeight; }, function ( v ){ ui.minHeight = v; } ],

		// (Number) maximum width allowed by layout
		[ 'maxWidth',  function (){ return ui.maxWidth; }, function ( v ){ ui.maxWidth = v; } ],

		// (Number) maximum height allowed by layout
		[ 'maxHeight',  function (){ return ui.maxHeight; }, function ( v ){ ui.maxHeight = v; } ],

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		[ 'anchorLeft',  function (){ return ui.anchorLeft; }, function ( v ){ ui.anchorLeft = v; } ],

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		[ 'anchorRight',  function (){ return ui.anchorRight; }, function ( v ){ ui.anchorRight = v; } ],

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		[ 'anchorTop',  function (){ return ui.anchorTop; }, function ( v ){ ui.anchorTop = v; } ],

		// (Number) 0 to 1, or -1 - anchor point to parent's same side (0) opposite side (1), or "auto"(-1)
		[ 'anchorBottom',  function (){ return ui.anchorBottom; }, function ( v ){ ui.anchorBottom = v; } ],

		// (Number) offset from anchorLeft
		[ 'left',  function (){ return ui.left; }, function ( v ){ ui.left = v; } ],

		// (Number) offset from anchorLeft
		[ 'right',  function (){ return ui.right; }, function ( v ){ ui.right = v; } ],

		// (Number) offset from anchorLeft
		[ 'top',  function (){ return ui.top; }, function ( v ){ ui.top = v; } ],

		// (Number) offset from anchorLeft
		[ 'bottom',  function (){ return ui.bottom; }, function ( v ){ ui.bottom = v; } ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - outer margin
		[ 'margin',  function (){ return ui.margin; }, function ( v ){ ui.margin = v; } ],

		// (Number) outer margin top
		[ 'marginTop',  function (){ return ui.marginTop; }, function ( v ){ ui.marginTop = v; }, true ],

		// (Number) outer margin right
		[ 'marginRight',  function (){ return ui.marginRight; }, function ( v ){ ui.marginRight = v; }, true ],

		// (Number) outer margin bottom
		[ 'marginBottom',  function (){ return ui.marginBottom; }, function ( v ){ ui.marginBottom = v; }, true ],

		// (Number) outer margin left
		[ 'marginLeft',  function (){ return ui.marginLeft; }, function ( v ){ ui.marginLeft = v; }, true ],

	];
	UI.base.addSharedProperties( go, container.ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// remapped functions - forward calls to container
	go.addChild = function() { return container.addChild.apply( container, arguments ); }
	go.removeChild = function() { return container.removeChild.apply( container, arguments ); }
	go.getChild = function() { return container.getChild.apply( container, arguments ); }
	go.removeAllChildren = function() { return container.removeAllChildren.apply( container, arguments ); }

	// create components

	// set name
	if ( !go.name ) go.name = "Scrollable";

	// UI
	ui = new UI();
	ui.layoutType = Layout.Anchors;
	ui.focusable = false;
	go.ui = ui;

	// container
	container.name = 'Scrollable.Container';
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
				if ( ac && go.parent ) go.parent.dispatchLate( 'layout' );
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
				if ( ac && go.parent ) go.parent.dispatchLate( 'layout' );
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
	go.baseStyle = Object.create( UI.style.scrollable );
	UI.base.applyProperties( go, go.baseStyle );
	go.state = 'auto';
	constructing = false;

})(this);
