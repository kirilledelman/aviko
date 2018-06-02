/*

	Tooltip

	Adds itself to App.overlay automatically

	Usage:

		var t = new GameObject( 'ui/tooltip', {
			text: ".......",
			target: objectWithUI
		} );

	tooltip will automatically disappear on mouseOut event from target

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'selected' : item is clicked on
		'change' : an item is highlighted
*/

include( './ui' );
(function(go) {

	// internal props
	var panel, label;
	var constructing = true;
	var target = null;
	var preferredDirection = 'up';
	var autoHideAfter = 5;
	go.serializeMask = [ 'ui', 'render' ];

	// API properties
	var mappedProps = {

		// (String) text in tooltip
		'text': { get: function (){ return label.text; }, set: function( t ) { label.text = t; } },

		// (GameObject) object to which this tooltip is attached
		// null - to not be attached to any object
		'target': {
			get: function (){ return target; },
			set: function( v ){
				target = v;
				if ( target && target.ui ) target.on( 'mouseOut', hide, true );
			} },

		// (ui/panel) container
		'panel': { get: function (){ return panel; } },

		// (ui/text) container
		'label': { get: function (){ return label; } },

		// (String) 'down' or 'up' - positioning of tooltip relative to either target, or x, y
		'preferredDirection': { get: function (){ return preferredDirection; }, set: function ( v ){ preferredDirection = v; } },

		// (Object) used to override style (collection of properties) other than default after creating / during init.
		// Here because tooltip doesn't have own UI object + not calling addSharedProperties
		// 'style': { get: function (){ return go.baseStyle; }, set: function ( v ){ UI.base.mergeStyle( go.baseStyle, v ) ; UI.base.applyProperties( go, v ); }, },

	};
	UI.base.mapProperties( go, mappedProps );

	// set name
	if ( !go.name ) go.name = "Tooltip";

	// create components
	panel = go.addChild( './panel', {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Start,
		width: 20,
		fitChildren: true,
		wrapEnabled: false,
		ignoreCamera: true,
	} );
	panel.ui.mouseOut = hide;

	label = panel.addChild( './text', {
		multiLine: true,
		wrap: true,
		autoSize: true,
	} );

	// positions menu next to target
	go.update = function () {
		// place next to target
		var x = go.x, y = go.y;
		if ( target ) {
			if ( !target.parent ) {
				hide(); return;
			}
			var tw = 0, th = 0;
			if ( target.ui ) {
				tw = target.ui.width;
				th = target.ui.height;
			} else if ( target.render ) {
				tw = target.render.width;
				th = target.render.height;
			}
			var tx = target.worldX;
			var ty = target.worldY;
			x = ( tx + panel.width >= App.windowWidth ) ? ( App.windowWidth - panel.width ) : tx;
			if ( preferredDirection == 'up' ) {
				y = ( ty - panel.height < 0 ) ? ( ty + th ) : ( ty - panel.height );
			} else {
				y = ( ty + th + panel.height >= App.windowHeight ) ? ( ty - panel.height ) : ( ty + th );
			}
			if ( x < 0 ) x = Math.max( 0, tx );
			if ( y < 0 ) y = Math.max( 0, ty + th );
		// fit to screen
		} else {

			if ( x + panel.width > App.windowWidth ) x = App.windowWidth - panel.width;
			else if ( x < 0 ) x = 0;
			if ( preferredDirection == 'up' ) {
				panel.y = -panel.height;
				if ( y > App.windowHeight ) y = App.windowHeight;
				else if ( y - panel.height < 0 ) y = y - panel.height;
			} else {
				panel.y = 0;
				if ( y + panel.height > App.windowHeight ) y = App.windowHeight - panel.height;
				else if ( y < 0 ) y = 0;
			}
		}
		go.setTransform( x, y );
	}

	// removal func
	function hide() { go.parent = null; }

	// callback
	go.removed = function () {
		if ( UI.tooltip === go ) {
			if ( UI.tooltip.target ) UI.tooltip.target.off( 'mouseOut', hide );
			UI.tooltip = null;
		}
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.tooltip );
	UI.base.applyProperties( go, go.baseStyle );
	constructing = false;

	// if there's another tooltip, kill it
	if ( typeof( UI.tooltip ) === 'object' && UI.tooltip !== null ) UI.tooltip.parent = null;

	// become tooltip
	UI.tooltip = go;

	// add self to overlay
	go.opacity = 0;
	App.overlay.addChild( go );
	go.fadeTo( 1, 0.2 );


})(this);
