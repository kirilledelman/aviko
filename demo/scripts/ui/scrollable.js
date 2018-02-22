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
	var vsb = null, hsb = null;

	// API properties
	var mappedProps = [

		// (Array) - set child objects at once - overrides built-in
		[ 'children',  function (){ return container.children; }, function ( v ){
			container.children = v; // overridden
		}],

		// (String) 'auto' or (Boolean) - whether scrollbars are shown automatically, always visible, or never
		[ 'scrollbars',  function (){ return scrollbars; }, function ( s ){
			scrollbars = s;
			go.debounce( 'updateScrollbars', go.updateScrollbars );
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
			container.ui.width = w;
			go.debounce( 'updateScrollbars', go.updateScrollbars );
		} ],

		// (Number) height of the scrollable container area
		[ 'scrollHeight',  function (){ return container.ui.height; }, function ( h ){
			container.ui.height = h;
			go.debounce( 'updateScrollbars', go.updateScrollbars );
		} ],

		// (Number) current y scroll position
		[ 'scrollTop',  function (){ return -container.y; }, function ( t ){
			var py = container.y;
			container.y = Math.min( 0, Math.max( -(container.ui.height - ui.height), -t ) );
			if ( py != container.y ) {
				go.fireLate( 'scroll' );
				go.debounce( 'updateScrollbars', go.updateScrollbars );
			}
		} ],

		// (Number) current x scroll position
		[ 'scrollLeft',  function (){ return -container.x; }, function ( l ){
			var px = container.x;
			container.x = Math.min( 0, Math.max( -(container.ui.width - ui.width), -l ) );
			if ( px != container.x ) {
				go.fireLate( 'scroll' );
				go.debounce( 'updateScrollbars', go.updateScrollbars );
			}
		} ],

		// (ui/scrollbar.js) - returns vertical scrollbar instance
		[ 'verticalScrollbar',  function (){ return vsb; } ],

		// (ui/scrollbar.js) - returns horizontal scrollbar instance
		[ 'horizontalScrollbar',  function (){ return hsb; } ],

		// (GameObject) the actual container to which all children are added
		[ 'container',  function (){ return container; } ],

	];
	UI.base.addSharedProperties( go, container.ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// remapped functions - forward calls to container
	go.addChild = function() { return container.addChild.apply( container, arguments ); }
	go.removeChild = function() { return container.removeChild.apply( container, arguments ); }
	go.getChild = function() { return container.getChild.apply( container, arguments ); }

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
		} else {
			// vertical
			if ( !vsb ) {
				// position to the right
				vsb = new GameObject( './scrollbar' );
				vsb.orientation = 'vertical';
				vsb.parent = go;
				vsb.anchorTop = vsb.anchorBottom = vsb.anchorRight = 0;
				vsb.anchorLeft = -1;
				vsb.right = -vsb.width;
				vsb.scroll = function ( ny ) { go.scrollTop = ny; }
			}
			// horizontal
			if ( !hsb ) {
				// position to the right
				hsb = new GameObject( './scrollbar' );
				hsb.orientation = 'horizontal';
				hsb.parent = go;
				hsb.anchorLeft = hsb.anchorBottom = hsb.anchorRight = 0;
				hsb.anchorTop = -1;
				hsb.bottom = -hsb.height;
				hsb.scroll = function ( nx ) { go.scrollLeft = nx; }
			}
		}

		// update vertical scrollbars params
		if ( vsb ) {
			vsb.totalSize = container.ui.height;
			vsb.handleSize = ui.height;
			vsb.position = -container.y;
			// hide / show
			var ac = (scrollbars == 'auto' ? (container.ui.height > ui.height) : scrollbars);
			if ( ac != vsb.active ) {
				vsb.active = ac;
				if ( ac && go.parent ) go.parent.dispatchLate( 'layout' );
			}
		}
		// update horizontal scrollbars params
		if ( hsb ) {
			hsb.totalSize = container.ui.width;
			hsb.handleSize = ui.width;
			hsb.position = -container.x;
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

		// auto-size container for lists
		if ( container.ui.layoutType == Layout.Vertical || container.layoutType == Layout.Grid ) {
			container.ui.width = Math.max( container.ui.minWidth, w );
		} else if ( container.ui.layoutType == Layout.Horizontal ) {
			container.ui.height = Math.max( container.ui.minHeight, h );
		}
		spr.resize( w, h );
		img.resize( w, h );
		go.scrollLeft = go.scrollLeft; // use setter to clip value
		go.scrollTop = go.scrollTop; // use setter to clip value
	}

	// container resizing should update scrollbars
	container.ui.layout = function() { go.debounce( 'updateScrollbars', go.updateScrollbars ); }

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
	UI.base.applyDefaults( go, UI.style.scrollable );

})(this);
