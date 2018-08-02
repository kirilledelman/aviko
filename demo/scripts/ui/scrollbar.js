/*

	Scrollbar

	Usage:

		var sb = App.scene.addChild( 'ui/scrollbar' );
		sb.orientation = 'vertical';
		sb.totalSize = 100;
		sb.handleSize = 20;
		sb.scroll = function ( scrollPos ) {
			log( "Scrollbar position changed:", scrollPos );
		}

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'scroll' - when scrollbar is scrolled by dragging handle

*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.scrollbarPrototype = UI.base.scrollbarPrototype || {

		__proto__: UI.base.componentPrototype,
		
		// (String) 'horizontal' or 'vertical' - scrollbar orientation
		get orientation(){ return this.__orientation; },
		set orientation( o ){
			if ( o != 'vertical' && o != 'horizontal' ) return;
			this.__orientation = o;
			// re-apply style
			UI.base.applyProperties( this, this.__baseStyle[ this.__orientation + 'Style' ] );
			this.requestLayout();
		},

		// (Number) - position of the handle - 0 to (totalSize - handleSize)
		get position(){
			if ( this.__discrete ) return Math.round( this.__position / this.__handleSize ) * this.__handleSize;
			else return this.__position;
		},
		set position( p ){
			if ( p != this.__position ) {
				this.__position = Math.max( 0, Math.min( p, this.__totalSize - this.__handleSize ) );
				if ( this.__discrete ) this.__position = Math.round( this.__position / this.__handleSize ) * this.__handleSize;
				this.requestLayout();
			}
		},

		// (Number) - total size of scrollable content (note that actual size of scrollbar is set with width/height, or anchors)
		get totalSize(){ return this.__totalSize; },
		set totalSize( v ) {
			if ( v != this.__totalSize ) {
				this.__totalSize = v;
				this.__handle.active = (v > 0);
				this.requestLayout();
			}
		},

		// (Number) - the size of visible "window" into content (less than totalSize)
		get handleSize(){ return this.__handleSize; },
		set handleSize( v ){
			if ( v != this.__handleSize ) {
				this.__handleSize = v;
				this.requestLayout();
			}
		},

		// (Boolean) values can only be changed in increments of handleSize (used for scrollbars used as paginators)
		get discrete(){ return this.__discrete; }, set discrete( cb ){ this.__discrete = cb; },

		// (Boolean) pressing Escape (or 'cancel' controller button) will blur the control
		get cancelToBlur(){ return this.__cancelToBlur; }, set cancelToBlur( cb ){ this.__cancelToBlur = cb; },

		// (Boolean) pressing Enter (or 'accept' controller button) will "pageDown" scrollbar page-by-page.
		get acceptToCycle(){ return this.__acceptToCycle; }, set acceptToCycle( a ){ this.__acceptToCycle = a; },

		// (Boolean) pressing Enter (or 'accept' controller button) enters mode where directional buttons can be used to scroll.
		// if true, unless in scrolling mode, directional buttons move focus instead.
		get acceptToScroll(){ return this.__acceptToScroll; }, set acceptToScroll( a ){ this.__acceptToScroll = a; },

		// (Boolean) input disabled
		get disabled(){ return disabled; },
		set disabled( v ){
			 this.ui.disabled = this.__disabled = v;
			 this.ui.focusable = !v;
			 if ( v && this.ui.focused ) this.ui.blur();
			 this.state = 'auto';
		 },

		// (ui/button) button serving as >> or 'pageUp' button
		get plusButton(){ return this.__plusButton; },
		set plusButton( b ){
			if ( this.__plusButton && this.__plusButton.click == this.__pageScroll ) this.__plusButton.click = this.__plusButton.__scrollbar = null;
			if ( b ) {
				b.click = this.__pageScroll;
				b.__scrollbar = this;
			}
			this.__plusButton = b;
			//this.dispatchLate( 'layout' );
		},

		// (ui/button) button serving as << or 'pageDown' button
		get minusButton(){ return this.__minusButton; },
		set minusButton( b ){
			if ( this.__minusButton && this.__minusButton.click == this.__pageScroll ) this.__minusButton.click = this.__minusButton.__scrollbar = null;
			if ( b ) {
				b.click = this.__pageScroll;
				b.__scrollbar = this;
			}
			this.__minusButton = b;
			// go.dispatchLate( 'layout' );
		},

		// (GameObject) - reference to scroll handle
		get handle(){ return this.__handle; },

		// (String) or (Color) or (Number) or (null|false)- scrollbar background set to sprite, or solid color, or nothing
		get background(){ return this.__background; },
		set background( b ){
			this.__background = b;
			if ( b === null || b === false ) {
				this.render = null;
			} else if ( typeof( b ) == 'string' ) {
				this.__bg.texture = b;
				this.__bg.resize( this.ui.width, this.ui.height );
				this.render = this.__bg;
			} else {
				this.__shp.color = b;
				this.render = this.__shp;
			}
		},

		// (Number) corner roundness when background is solid color
		get cornerRadius(){ return this.__shp.radius; },
		set cornerRadius( b ){
			this.__shp.radius = b;
			this.__shp.shape = b > 0 ? Shape.RoundedRectangle : Shape.Rectangle;
		},

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

		__focusChanged: function ( newFocus ) {
			// focused
			var go = this.gameObject;
		    if ( newFocus == this ) {
			    go.scrollIntoView();
			    go.state = 'focus';
		    } else {
			    go.state = 'auto';
				Input.off( 'mouseMove', go.__handleMouseMoveGlobal );
				Input.off( 'mouseUp', go.__handleMouseUpGlobal );
		    }
			go.fire( 'focusChanged', newFocus );
		},

		// lay out components
		__layout: function( w, h ) {
			var ui = this;
			var go = this.gameObject;
			var handle = go.__handle;
			
			w = Math.max( w, ui.padLeft + ui.padRight );
			h = Math.max( h, ui.padTop + ui.padBottom );
	
			// hide handle if no scrolling needed
			handle.active = go.__handleSize < go.__totalSize;
	
			// position handle
			if ( go.__totalSize > 0 && handle.active ) {
				go.__shp.resize( w, h );
				go.__bg.resize( w, h );
				if ( go.__orientation == 'vertical' ) {
					var availSize = h - ui.padTop - ui.padBottom;
					handle.x = ui.padLeft;
					handle.y = ui.padTop + Math.round( ( go.__position / go.__totalSize ) * availSize );
					handle.width = w - ui.padLeft - ui.padRight;
					handle.height = Math.round( ( go.__handleSize / go.__totalSize ) * availSize );
				} else {
					var availSize = w - ui.padLeft - ui.padRight;
					handle.y = ui.padTop;
					handle.x = ui.padLeft + Math.round( ( go.__position / go.__totalSize ) * availSize );
					handle.height = h - ui.padTop - ui.padBottom;
					handle.width = Math.round( ( go.__handleSize / go.__totalSize ) * availSize );
				}
			}
	
			// +-
			if ( go.__plusButton ) go.__plusButton.disabled = !handle.active || ( go.__position >= go.__totalSize - go.__handleSize );
			if ( go.__minusButton ) go.__minusButton.disabled = !handle.active || ( go.__position <= 0 );
		},
	
		// move handle while dragging
		__handleMouseMoveGlobal: function ( x, y ) {
			var ui = this.ui;
			if ( this.state == 'dragging' ) {
				var lp = this.globalToLocal( x, y, true );
				var newPos = this.__prevPos = this.__position;
				if ( this.__orientation == 'vertical' ) {
					var availSize = ui.height - ui.padTop - ui.padBottom;
					var hy = Math.max( ui.padTop,
		                        Math.min( ui.padTop + availSize - this.__handle.render.height,
		                                  lp.y - this.__grabY ) );
					this.position = Math.round( this.__totalSize * ( (hy - ui.padTop) / availSize ) );
				} else {
					var availSize = ui.width - ui.padLeft - ui.padRight;
					var hx = Math.max( ui.padLeft,
		                        Math.min( ui.padLeft + availSize - this.__handle.render.width,
		                                  lp.x - this.__grabX ) );
					this.position = Math.round( this.__totalSize * ( (hx - ui.padLeft) / availSize ) );
				}
				newPos = this.position;
				if ( this.__prevPos != newPos ) this.fire( 'scroll', newPos );
			}
			stopAllEvents();
		},
	
		// mouse down on handle
		__handleMouseDown: function ( btn, x, y, wx, wy ) {
			if ( this.__disabled ) return;
	
			if ( this.ui.focusable ) this.ui.focus();
	
			// position of mouse on handle
			this.__grabX = x; this.__grabY = y;
	
			// start drag
			this.state = 'dragging';
			Input.mouseMove = this.__handleMouseMoveGlobal.bind( this );
			Input.mouseUp = this.__handleMouseUpGlobal.bind( this );
			stopAllEvents();
		},
	
		// mouse up
		__handleMouseUpGlobal: function ( btn, x, y ) {
			go.state = 'auto';
			Input.mouseMove = null;
			Input.mouseUp = null;
			stopAllEvents();
		},
	
		// wheel - dispatch 'scroll' on gameObject
		__mouseWheel: function ( wy, wx ) {
			stopEvent();
			var go = this.gameObject;
			if ( go.__disabled ) return;
			var s = 0;
			if ( go.__orientation == 'horizontal' ) {
				s = ( wx == 0 ? wy : wx );
				if ( go.__discrete ) s = s > 0 ? go.__handleSize : -go.__handleSize;
			} else {
				s = -wy;
				if ( discrete ) s = s > 0 ? go.__handleSize : -go.__handleSize;
			}
			var prevPos = go.position;
			go.position += s;
			var newPos = go.position;
			if ( newPos != prevPos ) go.fire( 'scroll', newPos );
		},
	
		// down on scroll pane
		__mouseDown: function( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled ) return;
	
			if ( this.focusable ) this.focus();
			go.state = 'down';
	
			// try to center handle on point
			go.__handle.ui.fire( 'mouseDown', btn, go.__handle.render.width * 0.5, go.__handle.render.height * 0.5, wx, wy );
	
			// do first mousemove
			go.__handle.ui.fire( 'mouseMove', wx, wy );
		},
	
		// up
		__mouseUp: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.__disabled || go.state == 'dragging' ) return;
			stopAllEvents();
			go.state = 'auto';
			go.fire( currentEventName(), btn, x, y, wx, wy );
		},
	
		// rollover / rollout
		__mouseOverOut: function ( x, y, wx, wy ) {
			var go = this.gameObject;
			if ( go.state != 'dragging' ) go.state = 'auto';
			go.fire( currentEventName(), x, y, wx, wy );
		},
	
		//
		__navigation: function ( name, value ) {
	
			var go = this.gameObject;
			var dir = ( value > 0 ? 1 : -1 );
			var prevPos = position;
	
			// pressed direction in the same axis as is orientation
			if ( name == go.__orientation && ( !go.__acceptToScroll || go.state == 'scrolling' ) ) {
	
				go.async( function() {
					if ( this.__orientation == 'horizontal' ) {
						if ( !this.__acceptToCycle ) this.position += this.__handleSize * dir;
						if ( this.__prevPos == this.__position ) this.ui.moveFocus( dir, 0 );
						else this.fire( 'scroll', this.position );
					} else {
						if ( !this.__acceptToCycle ) this.position += this.__handleSize * dir;
						if ( this.__prevPos == this.__position ) this.ui.moveFocus( 0, dir );
						else this.fire( 'scroll', this.position );
					}
				} );
	
			// blur
			} else if ( name == 'cancel' ) {
	
				if ( go.__cancelToBlur ) this.blur();
				else if ( go.__acceptToScroll ) go.state = 'auto';
	
			// accept
			} else if ( name == 'accept' ) {
	
				// cycle scroll
				if ( go.__acceptToCycle ) {
					go.async( function () {
						if ( this.__position < ( this.__totalSize - this.__handleSize ) ) {
							this.position = this.__position + this.__handleSize;
						} else {
							this.position = 0;
						}
						this.fire( 'scroll', this.position );
					} );
	
				// enter manual scroll mode
				} else if ( this.__acceptToScroll ) {
	
					go.state = (go.state == 'scrolling' ? 'auto' : 'scrolling');
	
				// work like tab
				} else {
	
					this.moveFocus( 1 );
	
				}
	
			// move focus
			} else if ( name == 'horizontal' ) {
	
				this.moveFocus( dir, 0 );
	
			} else {
	
				this.moveFocus( 0, dir );
	
			}
	
		},
	
		// tab
		__keyDown: function ( code, shift, ctrl, alt, meta ) {
	
			if ( code == Key.Tab ) this.moveFocus( shift ? -1 : 1 );
	
		},
		
		// page up / page down buttons handlers
		__pageScroll: function () {
			var go = this.__scrollbar;
			if ( this == go.plusButton ) {
				go.position += go.handleSize * dir;
			} else {
				go.position -= go.handleSize * dir;
			}
			go.fire( 'scroll', go.position );
		}
		
	};

	// init
	go.name = "Scrollbar";
	go.ui = new UI( {
        layoutType: Layout.Anchors,
		fitChildren: false,
		autoMoveFocus: false,
		focusable: true,
		minWidth: 8,
		minHeight: 8,
		focusChanged: UI.base.scrollbarPrototype.__focusChanged,
		layout: UI.base.scrollbarPrototype.__layout,
		mouseWheel: UI.base.scrollbarPrototype.__mouseWheel,
		mouseDown: UI.base.scrollbarPrototype.__mouseDown,
		mouseUp: UI.base.scrollbarPrototype.__mouseUp,
		mouseUpOutside: UI.base.scrollbarPrototype.__mouseUp,
		mouseOver: UI.base.scrollbarPrototype.__mouseOverOut,
		mouseOut: UI.base.scrollbarPrototype.__mouseOverOut,
		navigation: UI.base.scrollbarPrototype.__navigation,
		keyDown: UI.base.scrollbarPrototype.__keyDown,
	} );
	go.__background = false;
	go.__handle = go.addChild( './button', {
		name: "Handle",
		disabled: true,
		layoutType: Layout.Anchors,
		fixedPosition: true,
		states: {}
	} );
	go.__handle.ui.focusable = false;
	go.__handle.ui.mouseDown = UI.base.scrollbarPrototype.__handleMouseDown.bind( go );
	go.__bg = new RenderSprite();
	go.__shp = new RenderShape( { shape: Shape.Rectangle, centered: false, radius: 0, filled: true } );
	go.__cancelToBlur = false;
	go.__acceptToCycle = false;
	go.__acceptToScroll = false;
	go.__scrolling = false;
	go.__disabled = false;
	go.__position = 0;
	go.__discrete = false;
	go.__totalSize = 100;
	go.__handleSize = 20;
	go.__orientation = 'vertical';
	go.__plusButton = null;
	go.__minusButton = null;
	go.__grabX = 0;
	go.__grabY = 0;
	go.__proto__ = UI.base.scrollbarPrototype;
	go.init();
	go.serializeMask.push( 'children', 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' );
	go.cloneMask = [ 'plusButton', 'minusButton' ];

	// add property-list inspectable info
	UI.base.addInspectables( go, 'Scrollbar',
		[ 'orientation', 'position', 'totalSize', 'handleSize', 'discrete', 'acceptToCycle', 'cancelToBlur',
			'disabled', 'plusButton', 'minusButton' ],
        { orientation: { enum: [ 'vertical', 'horizontal' ] } }, 1 );

	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.scrollbar );
	UI.base.applyProperties( go, go.__baseStyle );
	go.orientation = orientation;
	go.state = 'auto';

})(this);
