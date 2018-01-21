/*

Scrollable container




*/

include( './ui' );
(function(go) {

	// internal props
	var ui, spr, img, container = go.addChild();
	container.ui = new UI();
	go.serializeMask = {};
	var scrollbars = false;
	var vsb = null, hsb = null;

	// API properties
	var mappedProps = [

		// (Array) - set child objects at once
		[ 'children',  function (){ return container.children; }, function ( v ){
			container.children = v; // overridden
		}],

		// (Number) current width of the control
		[ 'scrollbars',  function (){ return scrollbars; }, function ( s ){
			scrollbars = s;
			go.updateScrollbars();
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
			go.updateScrollbars();
		} ],

		// (Number) height of the scrollable container area
		[ 'scrollHeight',  function (){ return container.ui.height; }, function ( h ){
			container.ui.height = h;
			go.updateScrollbars();
		} ],

		// (Number) current y scroll position
		[ 'scrollTop',  function (){ return -container.y; }, function ( t ){
			var py = container.y;
			container.y = Math.min( 0, Math.max( -(container.ui.height - ui.height), -t ) );
			if ( py != container.y ) {
				go.updateScrollbars();
				go.fireLate( 'scroll' );
			}
		} ],

		// (Number) current x scroll position
		[ 'scrollLeft',  function (){ return -container.x; }, function ( l ){
			var px = container.x;
			container.x = Math.min( 0, Math.max( -(container.ui.width - ui.width), -l ) );
			if ( px != container.x ) {
				go.fireLate( 'scroll' );
				go.updateScrollbars();
			}
		} ],

	];
	UI.base.addSharedProperties( go, container.ui ); // add common UI properties (ui.js)
	for ( var i = 0; i < mappedProps.length; i++ ) {
		Object.defineProperty( go, mappedProps[ i ][ 0 ], {
			get: mappedProps[ i ][ 1 ], set: mappedProps[ i ][ 2 ], enumerable: (mappedProps[ i ][ 2 ] != undefined), configurable: true
		} );
		if ( mappedProps[ i ].length >= 4 ){ go.serializeMask[ mappedProps[ i ][ 0 ] ] = mappedProps[ i ][ 3 ]; }
	}

	// remapped functions - forward calls to container
	go.addChild = function() { return container.addChild.apply( container, arguments ); }
	go.removeChild = function() { return container.removeChild.apply( container, arguments ); }
	go.getChild = function() { return container.getChild.apply( container, arguments ); }

	// create components

	// UI
	ui = new UI();
	ui.layoutType = Layout.Anchors;
	ui.focusable = false;

	// container
	container.ui.focusable = false;

	// image
	img = new Image();
	spr = new RenderSprite( img );

	// don't serialize components/properties
	go.serializeMask = { 'ui':1, 'render':1 };

	// components are added after component is awake
	go.awake = function () {
		go.ui = ui;
		go.render = spr;
		img.autoDraw = container;
	};

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
				vsb.parent = go;
				vsb.anchorTop = vsb.anchorBottom = vsb.anchorRight = 0;
				vsb.anchorLeft = -1;
				vsb.right = -vsb.width;
				vsb.scroll = function ( ny ) { go.scrollTop = ny; }
			}
		}

		// update scrollbars params
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
	}

	// lay out components
	ui.layout = function( x, y, w, h ) {
		go.setTransform( x, y );

		// auto-size container for lists
		if ( container.ui.layoutType == Layout.Vertical || container.layoutType == Layout.Grid ) {
			container.ui.width = w;
		} else if ( container.ui.layoutType == Layout.Horizontal ) {
			container.ui.height = h;
		}
		spr.width = img.width = w;
		spr.height = img.height = h;

		// set to current, to make sure scroll position is within limits
		go.scrollLeft = go.scrollLeft;
		go.scrollTop = go.scrollTop;
		go.async( go.updateScrollbars, 0 );
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
	if ( UI.style && UI.style.scrollable ) for ( var p in UI.style.scrollable ) go[ p ] = UI.style.scrollable[ p ];

})(this);
