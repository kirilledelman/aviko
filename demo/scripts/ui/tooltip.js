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

	// API properties
	UI.base.tooltipPrototype = UI.base.tooltipPrototype || {

		__proto__: UI.base.componentPrototype,

		// (String) text in tooltip
		get text(){ return this.__label.text; }, set text( t ) { this.__label.text = t; },

		// (GameObject) object to which this tooltip is attached
		// null - to not be attached to any object
		get target(){ return this.__target; },
		set target( v ){
			this.__target = v;
			if ( this.__target && this.__target.ui ) this.__target.on( 'mouseOut', this.__hide, true );
		},

		// (ui/panel) container
		get panel(){ return this.__panel; },

		// (ui/text) container
		get label(){ return this.__label; },

		// (String) 'down' or 'up' - positioning of tooltip relative to either target, or x, y
		get preferredDirection(){ return this.__preferredDirection; }, set preferredDirection( v ){ this.__preferredDirection = v; },
	
		// positions menu next to target
		update: function () {
			// place next to target
			var x = this.x, y = this.y;
			if ( this.__target ) {
				if ( !this.__target.parent ) {
					this.__hide(); return;
				}
				var tw = 0, th = 0;
				if ( this.__target.ui ) {
					tw = this.__target.ui.width;
					th = this.__target.ui.height;
				} else if ( this.__target.render ) {
					tw = this.__target.render.width;
					th = this.__target.render.height;
				}
				var tx = this.__target.worldX;
				var ty = this.__target.worldY;
				x = ( tx + this.__panel.width >= App.windowWidth ) ? ( App.windowWidth - this.__panel.width ) : tx;
				if ( preferredDirection == 'up' ) {
					y = ( ty - this.__panel.height < 0 ) ? ( ty + th ) : ( ty - this.__panel.height );
				} else {
					y = ( ty + th + this.__panel.height >= App.windowHeight ) ? ( ty - this.__panel.height ) : ( ty + th );
				}
				if ( x < 0 ) x = Math.max( 0, tx );
				if ( y < 0 ) y = Math.max( 0, ty + th );
			// fit to screen
			} else {
	
				if ( x + this.__panel.width > App.windowWidth ) x = App.windowWidth - this.__panel.width;
				else if ( x < 0 ) x = 0;
				if ( this.__preferredDirection == 'up' ) {
					this.__panel.y = -this.__panel.height;
					if ( y > App.windowHeight ) y = App.windowHeight;
					else if ( y - this.__panel.height < 0 ) y = y - this.__panel.height;
				} else {
					this.__panel.y = 0;
					if ( y + this.__panel.height > App.windowHeight ) y = App.windowHeight - this.__panel.height;
					else if ( y < 0 ) y = 0;
				}
			}
			this.setTransform( x, y );
		},
	
		// removal func
		__hide: function () { this.parent = null; },
	
		removed: function () {
			if ( UI.tooltip === this ) {
				if ( UI.tooltip.target ) UI.tooltip.target.off( 'mouseOut', this.__hide );
				UI.tooltip = null;
			}
		}
		
	};

	// init
	go.name = "Tooltip";
	go.__target = null;
	go.__preferredDirection = 'up';
	go.__autoHideAfter = 5;
	go.__panel = go.addChild( './panel', {
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Start,
		width: 20,
		fitChildren: true,
		wrapEnabled: false,
		ignoreCamera: true,
	} );
	go.__label = go.__panel.addChild( './text', {
		multiLine: true,
		wrap: true,
		autoSize: true,
	} );
	go.__proto__ = UI.base.tooltipPrototype;
	go.__hide = go.__hide.bind( go );
	go.__panel.ui.mouseOut = go.__hide;
	go.__init();
	go.serializeMask.push( 'update', 'removed' );

	// add property-list inspectable info
	UI.base.addInspectables( go, "Tooltip", [ 'text' ], { }, 1 );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.tooltip );
	UI.base.applyProperties( go, go.__baseStyle );

	// if there's another tooltip, kill it
	if ( typeof( UI.tooltip ) === 'object' && UI.tooltip !== null ) UI.tooltip.parent = null;

	// become tooltip
	UI.tooltip = go;

	// add self to overlay
	go.opacity = 0;
	App.overlay.addChild( go );
	go.fadeTo( 1, 0.2 );


})(this);
