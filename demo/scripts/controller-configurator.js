/*

	Helper script for automatically configuring a newly connected controller.
	When a new controller is connected, if it's not already configured, this
	interface will be presented, and required axis/buttons configured in sequence.
	The configuration will be saved to ./config/NAME_OF_CONTROLLER.json. To reset
	controller just delete that file, or call configurator `reset` method.

	If multiple controllers are added while configurator is displayed, they'll be
	configured also, in sequence, without configurator closing.

	Example/Usage:

		var configurator = include( 'ui/controller-configurator' );

		// axis - array of axis bindings
		configurator.axis = [
			{
				id: 'horizontal', // axis name, as used in controller.on( 'horizontal', handler )
				minus: 'Left', // text displayed when configuring '-axis_id
				plus:  'Right', // text displayed when configuring '+axis_id'
				description: 'Player movement' // optional text displayed as description for both direction
			}, {
				id: 'vertical',
				minus: 'Player jump',
				plus:  'Player duck'
			}
		];

		// buttons - array of button actions
		configurator.buttons = [
			{
				id: 'accept', // action name, as used in controller.on( 'accept', handler )
				description: 'Jump / Select in menus' // text displayed as description, when mapping
			}, {
				id: 'cancel',
				description: 'Fire / Cancel in menus'
			},
		];

		// callbacks/events

		configurator.controllerAdded = function ( controller ) {
			// called whenever a new controller is connected
			// call stopEvent() if you don't want to use this controller
			// if this controller isn't configured, configurator call willShow event
			// and will display configuration interface
		}

		configurator.controllerRemoved = function ( controller ) {
			// called when controller is disconnected, or if
			// user aborted configuring this controller
		}

		configurator.willShow = function ( controller ) {
			// called when configurator is about to show itself for controller
			// this is called after .controllerAdded callback, if controller isn't configured
			// use this callback to pause your game, or add/modify visuals in configurator.container
		}

		configurator.willHide = function () {
			// called after the last plugged in and unconfigured controller has been
			// configured, and configurator is about to hide itself
		}

		configurator.ready = function ( controller ) {
			// called after controller has finished configuring
			// or was configured previously
			// can use this callback to add new player into game
			// or confirm that controller is available and ready
		}

	Methods:

		configurator.reset( controller ) - deletes configuration file for controller, removes bindings

		configurator.configure( controller ) - re-configures controller, replaces config if successful

		configurator.refresh(); - dispatches 'controllerAdded' for all connected controllers

 */

(function( params ) {

	var go = new GameObject( { name: "Controller Configurator" } );
	var axis = [];
	var buttons = [];
	var container, scene, title, subtitle, prompt, action, error, instruction;
	var controllersToConfigure = [];
	var currentlyConfiguring = null;
	var currentButton, currentAxis, axisDirection, currentSection;
	var buttonsFirst = false;
	var showDelay = 1.5;
	var style = {};
	var currentlyPressed = null;
	var waitForAxis = null, waitForAxisDirection = 0;
	var waitForHat = null, waitForHatDirection = 0, waitForHatValue = 0;
	var ignoreNextEvent = false;

	// API properties
	var mappedProps = {

		// (Array) of { id, minus, plus, description, descriptionMinus, descriptionPlus }
		'axis': { get: function () { return axis; }, set: function ( v ) { axis = v; } },

		// (Array) of { id, description }
		'buttons': { get: function () { return buttons; }, set: function ( v ) { buttons = v; } },

		// (Boolean) buttons before axis
		'buttonsFirst': { get: function () { return buttonsFirst; }, set: function ( v ) { buttonsFirst = v; } },

		// (GameObject) container when configurator is open
		'container': { get: function () { return container; } },

		// (GameObject) (ui/text.js) when configurator is open
		'title': { get: function () { return title; } },

		// (GameObject) (ui/text.js) when configurator is open
		'subtitle': { get: function () { return subtitle; } },

		// (GameObject) (ui/text.js) when configurator is open
		'prompt': { get: function () { return prompt; } },

		// (GameObject) (ui/text.js) when configurator is open
		'action': { get: function () { return action; } },

		// (GameObject) (ui/text.js) when configurator is open
		'error': { get: function () { return error; } },

		// (GameObject) (ui/text.js) when configurator is open
		'instruction': { get: function () { return instruction; } },

		// (Boolean) true when configurator is open
		'isOpen': { get: function () { return !!currentlyConfiguring; } },

		// (Number) delay in seconds before configurator is displayed
		'showDelay': { get: function () { return showDelay; }, set: function ( v ) { showDelay = v; } },

		// (Object) used to apply style (collection of properties) - use to preconfigure look of title, prompt etc.
		'style': {
			get: function () { return style; },
			set: function ( v ) {
				style = v;
				if ( container ) applyProperties( go, style );
			}
		},

	};
	Object.defineProperties( go, mappedProps );

	// sets properties on an object recursively
	function applyProperties( obj, props ) {
		if ( !( props && obj ) ) return;
		for ( var p in props ) {
			var objType = typeof( obj[ p ] );
			var pType = typeof( props[ p ] );
			if ( pType === 'object' && props[ p ].constructor !== Array &&
				objType === 'object' && obj[ p ] !== null ) {
				applyProperties( obj[ p ], props[ p ] );
			} else {
				obj[ p ] = props[ p ];
			}
		}
	}

	// API functions

	go.reset = function ( controller ) {
		controller.reset( true );
		var index = controllersToConfigure.indexOf( controller );
		if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
	}

	go.configure = configureController;

	go.refresh = function () {
		for ( var i = 0, nc = Input.controllers.length; i < nc; i++ ) {
			controllerAdded ( Input.controllers[ i ] );
		}
	}

	// optional init parameter ( can pass as include( 'ui/controller-configurator', { ... init ... } ) )

	if ( params != global ) applyProperties( go, params );

	// display elements - feel free to modify, or adjust in willShow

	// create container
	container = new GameObject( {
		render: new RenderSprite( new Image( App.windowWidth, App.windowHeight ) ),
		ui: new UI( {
			width: App.windowWidth,
			height: App.windowHeight,
			layoutType: Layout.Vertical,
			layoutAlignX: LayoutAlign.Stretch
		}),
		resize: function ( w, h ){
			container.render.resize( w, h );
			container.ui.resize( w, h );
		}
	} );
	container.render.color.set( 1, 1, 1, 0.15 );

	// new scene
	scene = new Scene( { backgroundColor: 0xFFFFFF } );
	scene.addChild( container );

	// automatic initial scale
	var sx = App.windowWidth * 0.05;
	var sy = App.windowHeight * 0.05;
	var largeFontSize = Math.max( 16, sx * 1.2 );
	var mediumFontSize = Math.max( 12, sx );
	var smallFontSize = Math.max( 9, sx * 0.5 );
	function layoutText( w, h ) {
		this.gameObject.render.width = w;
		this.gameObject.render.measure();
		this.minHeight = this.gameObject.render.scrollHeight;
		this.gameObject.render.resize( w, h );
	}
	// add text
	title = container.addChild( {
		render: new RenderText ( {
			bold: true,
			align: TextAlign.Center,
			size: largeFontSize,
			color: 0x006699,
			wrap: true,
			multiLine: true,
			autoSize: false,
			text: "New Controller Detected",
		} ),
		ui: new UI( { marginTop: sy, minHeight: largeFontSize, layout: layoutText } )
	} );
	subtitle = container.addChild( {
		render: new RenderText ( {
			align: TextAlign.Center,
			autoSize: false,
			bold: true,
			size: largeFontSize,
			color: 0x003366,
			wrap: true,
			multiLine: true,
			text: "_controller_"
		}),
		ui: new UI( { marginTop: sy * 0.5, minHeight: largeFontSize, layout: layoutText } ),
	} );
	container.addChild( new GameObject( { ui: new UI( { flex: 1 } ) } ) ); // spacer
	prompt = container.addChild( {
		render: new RenderText( {
			align: TextAlign.Center,
			autoSize: false,
			size: mediumFontSize,
			color: 0x0,
			wrap: true,
			multiLine: true,
			text: "Press a button or axis for"
		}),
		ui: new UI( { marginTop: sy * 2, minHeight: mediumFontSize, layout: layoutText } ),
	} );
	action = container.addChild( {
		render: new RenderText( {
			bold: true,
			align: TextAlign.Center,
			autoSize: false,
			size: mediumFontSize,
			color: 0xD03399,
			wrap: true,
			multiLine: true,
			text: "_description_"
		}),
		ui: new UI( { minHeight: mediumFontSize, layout: layoutText } ),
	} );
	error = container.addChild( {
		render: new RenderText( {
			align: TextAlign.Center,
			autoSize: false,
			size: smallFontSize,
			color: 0x901010,
			pad: sy,
			wrap: true,
			multiLine: true,
		}),
		ui: new UI( { minHeight: smallFontSize } ),
	} );
	container.addChild( new GameObject( { ui: new UI( { flex: 1 } ) } ) ); // spacer
	instruction = container.addChild( {
		render: new RenderText( {
			align: TextAlign.Center,
			autoSize: false,
			size: smallFontSize,
			color: 0x666666,
			wrap: true,
			multiLine: true,
			text: "press + hold any button to skip, press + hold any two buttons to abort"
		}),
		ui: new UI( { minHeight: smallFontSize, marginBottom: 8, layout: layoutText } ),
	} );

	// internals

	Input.on( 'controllerRemoved', function ( controller ) {

		// currently configuring, do next without finishing
		if ( currentlyConfiguring == controller ) {
			nextController( false );
			// remove from list
		} else {
			var index = controllersToConfigure.indexOf( controller );
			if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
		}
		// callback
		go.fire( 'controllerRemoved', controller );

	}.bind( go ) );

	var controllerAdded = function ( controller ) {

		// ask if want to use this one
		if ( !go.fire( 'controllerAdded', controller ) ) return;

		// configure
		if ( !controller.configured ) {
			async( function () { configureController( controller ); }, showDelay );
		} else go.fire( 'ready', controller );

	}.bind( go );
	Input.on( 'controllerAdded', controllerAdded );

	function configureController ( controller ) {

		// add to list
		if ( controllersToConfigure.indexOf( controller ) == -1 ) controllersToConfigure.push( controller );

		// if already open, will config after current is finished
		if ( currentlyConfiguring ) return;

		// draw current scene to image
		container.render.image.autoDraw = App.scene;
		App.pushScene( scene );

		// reapply style
		go.style = style;

		// listeners
		App.on( 'resized', container.resize );
		Input.on( 'keyDown', keyDown );
		Input.on( 'keyUp', keyUp );
		Input.on( 'joyDown', joyDown );
		Input.on( 'joyUp', joyUp );
		Input.on( 'joyAxis', joyAxis );
		Input.on( 'joyHat', joyHat );

		currentlyConfiguring = null;
		nextController( false );

	}

	function nextButton () {

		currentSection = 'buttons';
		error.render.text = "";
		prompt.render.text = (currentlyConfiguring.name == 'Keyboard' ? "Press a key for" : "Press a button for");
		currentButton++;
		if ( currentButton >= buttons.length ) {
			if ( buttonsFirst ) nextAxis();
			else nextController( true );
			return;
		}

		// update description
		action.render.text = buttons[ currentButton ].description;
	}

	function nextAxis () {

		currentSection = 'axis';
		error.render.text = "";
		prompt.render.text = (currentlyConfiguring.name == 'Keyboard' ? "Press a key for" : "Move joystick or press a button for");
		axisDirection++;
		if ( axisDirection >= 2 ) {
			axisDirection = 0;
			currentAxis++;
			if ( currentAxis >= axis.length ) {
				if ( !buttonsFirst ) nextButton();
				else nextController( true );
				return;
			}
		}

		// show description
		if ( currentAxis < 0 ) currentAxis = 0;
		var a = axis[ currentAxis ];
		var desc = "";
		if ( a.description ) desc = a.description + ' / ';
		if ( axisDirection == 0 ) {
			desc += a.minus ? a.minus : "Minus";
		} else {
			desc += a.plus ? a.plus : "Plus";
		}
		action.render.text = desc;
	}

	function keyDown ( k, s, a, c, g, r ) {

		// ignore repeat key
		if ( r ) return;

		// if configuring a gamepad, ignore keyboard presses
		if ( currentlyConfiguring.name != 'Keyboard' ) {
			// allow exit by pressing escape
			if ( k == Key.Escape ) return closeConfigurator();
			return;
		}

		// set timer
		debounce( 'configuratorSkip', skipToNext, 2 );

		// already pressed something else? abort
		if ( currentlyPressed && currentlyPressed.key != k ) return closeConfigurator();

		// save currently pressed
		currentlyPressed = { key: k };

		// check if already in use
		error.render.text = "";
		var inUse = checkInUse();
		if ( inUse ) {
			currentlyPressed = null;
			error.render.text = "Key already in use by ^B" + inUse;
			return;
		}

	}

	function keyUp ( k ) {

		// if configuring a gamepad, ignore keyboard presses
		if ( currentlyConfiguring.name != 'Keyboard' ) return;

		cancelDebouncer( 'configuratorSkip' );

		// skipping until new press
		if ( !currentlyPressed ) return;
		currentlyPressed = null;

		// accept
		if ( currentSection == 'buttons' ) {
			buttons[ currentButton ].accepted = { key: k };
			nextButton();
		} else {
			axis[ currentAxis ][ 'accepted' + axisDirection ] = { key: k };
			nextAxis();
		}

	}

	function joyDown ( k, c ) {

		// controller must match
		if ( currentlyConfiguring != c ) return;

		// already pressed something else? abort
		if ( currentlyPressed && currentlyPressed.button != k ) return closeConfigurator();

		// save currently pressed
		currentlyPressed = { button: k };

		// set timer
		debounce( 'configuratorSkip', skipToNext, 2 );

		// check if already in use
		error.render.text = "";
		var inUse = checkInUse();
		if ( inUse ) {
			currentlyPressed = null;
			error.render.text = "Button already in use by ^B" + inUse;
			return;
		}

	}

	function joyUp ( k, c ) {

		// controller must match
		if ( currentlyConfiguring != c ) return;

		// clear hold to skip
		cancelDebouncer( 'configuratorSkip' );

		// skipping until new press
		if ( !currentlyPressed ) return;
		currentlyPressed = null;

		// accept
		if ( currentSection == 'buttons' ) {
			buttons[ currentButton ].accepted = { button: k };
			nextButton();
		} else {
			axis[ currentAxis ][ 'accepted' + axisDirection ] = { button: k };
			nextAxis();
		}

	}

	function joyAxis ( v, a, c ) {

		// controller must match
		if ( currentlyConfiguring != c ||
			currentSection == 'buttons' ) return;

		// if not waiting to release
		if ( waitForAxis !== null ) {

			// check if axis is in use
			error.render.text = "";
			currentlyPressed = { axis: a };
			var inUse = checkInUse();
			currentlyPressed = null;
			if ( inUse ) {
				error.render.text = "Axis already in use by ^B" + inUse;
				return;
			}

			// if doing second direction, and first direction was button, show error
			if ( axisDirection == 1 ) {
				if ( axis[ currentAxis ].accepted0 && axis[ currentAxis ].accepted0.button != undefined ) {
					error.render.text = "Opposite direction must also be a button";
					return;
				}
			}
		}

		// clear reset
		if ( waitForAxis == a && Math.abs( v ) <= c.deadZone ) {
			// all good
			axis[ currentAxis ][ 'accepted' + axisDirection ] = { axis: a, dir: waitForAxisDirection };
			if ( axisDirection == 0 ) {
				// no need to define opposite axis
				axisDirection++;
			}
			nextAxis();
			waitForAxis = null;
			return;
		}

		// threshold
		if ( Math.abs( v ) > 0.75 ) {
			// wait for clear event to accept
			waitForAxis = a;
			waitForAxisDirection = v;
		}

	}

	function joyHat ( x, y, h, c ) {

		// controller must match
		if ( currentlyConfiguring != c ||
			currentSection == 'buttons' ) return;

		var val = ( y < 0 ? 0 :
			( y > 0 ? 2 :
				( x > 0 ? 1 :
					( x < 0 ? 3 : -1 ) ) ) );
		var dir = ( y != 0 ? 'y' : ( x != 0 ? 'x' : '') );

		// if not waiting to release
		if ( waitForHat !== null ) {

			// check if axis is in use
			error.render.text = "";
			currentlyPressed = { hat: h, dir: dir, val: val };
			var inUse = checkInUse();
			currentlyPressed = null;
			if ( inUse ) {
				error.render.text = "Hat already in use by ^B" + inUse;
				return;
			}

			// if doing second direction, and first direction was button, show error
			if ( axisDirection == 1 ) {
				if ( axis[ currentAxis ].accepted0 && axis[ currentAxis ].accepted0.button != undefined ) {
					error.render.text = "Opposite direction must also be a button";
					return;
				}
			}
		}

		// clear reset
		if ( waitForHat == h && val == -1 ) {
			// all good
			axis[ currentAxis ][ 'accepted' + axisDirection ] = {
				hat: h,
				dir: waitForHatDirection,
				val: waitForHatValue
			};
			if ( axisDirection == 0 ) {
				// no need to define opposite axis
				axisDirection++;
			}
			nextAxis();
			waitForHat = null;
			return;
		}

		// wait for clear event to accept
		waitForHat = h;
		waitForHatDirection = dir;
		waitForHatValue = val;

	}

	function checkInUse () {
		for ( var i in buttons ) {
			var b = buttons[ i ].accepted;
			if ( b != undefined && (
					( b.key != undefined && b.key === currentlyPressed.key ) ||
					( b.button != undefined && b.button === currentlyPressed.button )
				) ) return buttons[ i ].description;

		}
		for ( var i in axis ) {
			var b = axis[ i ].accepted0;
			if ( b != undefined && (
					( b.axis != undefined && b.axis === currentlyPressed.axis ) ||
					( b.key != undefined && b.key === currentlyPressed.key ) ||
					( b.hat != undefined && b.hat === currentlyPressed.hat && b.dir == currentlyPressed.dir ) ||
					( b.button != undefined && b.button === currentlyPressed.button )
				) )
				return axis[ i ].description ? axis[ i ].description : axis[ i ].minus;
			b = axis[ i ].accepted1;
			if ( b != undefined && (
					( b.axis != undefined && b.axis === currentlyPressed.axis ) ||
					( b.key != undefined && b.key === currentlyPressed.key ) ||
					( b.hat != undefined && b.hat === currentlyPressed.hat && b.dir == currentlyPressed.dir ) ||
					( b.button != undefined && b.button === currentlyPressed.button )
				) )
				return axis[ i ].description ? axis[ i ].description : axis[ i ].plus;
		}

		return false; // return description of axis or btn
	}

	// skip a button or axis
	function skipToNext () {

		currentlyPressed = null;
		var skipText;
		if ( currentSection == 'buttons' ) {

			skipText = buttons[ currentButton ].description;
			buttons[ currentButton ].accepted = null;
			nextButton();

		} else {

			skipText = (axis[ currentAxis ].description ? axis[ currentAxis ].description + " / " : "" );
			skipText += ( axisDirection ? axis[ currentAxis ].plus : axis[ currentAxis ].minus );
			axis[ currentAxis ][ 'accepted' + axisDirection ] = null;
			nextAxis();

		}

		// set error text
		error.render.text = "^N^0Skipped ^B" + skipText;
	}

	function closeConfigurator () {

		// clear references
		container.render.autoDraw = null;
		controllersToConfigure = [];
		currentlyConfiguring = null;

		// clear listeners
		cancelDebouncer( 'configuratorSkip' );
		App.off( 'resized', container.resize );
		Input.off( 'keyDown', keyDown );
		Input.off( 'keyUp', keyUp );
		Input.off( 'joyDown', joyDown );
		Input.off( 'joyUp', joyUp );
		Input.off( 'joyAxis', joyAxis );
		Input.off( 'joyHat', joyHat );

		// notify
		go.fire( 'willHide' );

		// remove scene
		App.popScene();

	}

	// called in the beginning, and after each controller in queue is finished
	function nextController ( saveCurrent ) {
		// apply bindings
		if ( saveCurrent ) {
			currentlyConfiguring.reset();
			for ( var i in buttons ) {
				var b = buttons[ i ].accepted;
				if ( b ) {
					if ( b.key != undefined ) {
						currentlyConfiguring.bind( buttons[ i ].id, b.key );
					} else if ( b.button != undefined ) {
						currentlyConfiguring.bind( buttons[ i ].id, Key.JoyButton, b.button );
					}
				}
			}
			for ( var i in axis ) {
				var b = axis[ i ].accepted0;
				if ( b ) {
					if ( b.key != undefined ) {
						currentlyConfiguring.bindAxis( '-' + axis[ i ].id, b.key );
					} else if ( b.button != undefined ) {
						currentlyConfiguring.bindAxis( '-' + axis[ i ].id, Key.JoyButton, b.button );
					} else if ( b.axis != undefined ) {
						if ( b.dir < 0 ) {
							currentlyConfiguring.bindAxis( '+' + axis[ i ].id, Key.JoyAxis, b.axis );
						} else {
							currentlyConfiguring.bindAxis( '-' + axis[ i ].id, Key.JoyAxis, b.axis ); // reversed
						}
					} else if ( b.hat != undefined ) {
						if ( b.dir == 'x' ) {
							if ( b.val != 1 ) {
								currentlyConfiguring.bindAxis( '+' + axis[ i ].id, Key.JoyHatX, b.hat );
							} else {
								currentlyConfiguring.bindAxis( '-' + axis[ i ].id, Key.JoyHatX, b.hat );
							}
						} else {
							if ( b.val != 2 ) {
								currentlyConfiguring.bindAxis( '+' + axis[ i ].id, Key.JoyHatY, b.hat );
							} else {
								currentlyConfiguring.bindAxis( '-' + axis[ i ].id, Key.JoyHatY, b.hat );
							}
						}
					}
				}
				b = axis[ i ].accepted1;
				if ( b ) {
					if ( b.key != undefined ) {
						currentlyConfiguring.bindAxis( '+' + axis[ i ].id, b.key );
					} else if ( b.button != undefined ) {
						currentlyConfiguring.bindAxis( '+' + axis[ i ].id, Key.JoyButton, b.button );
					}
				}

			}

			// save & notify
			if ( currentlyConfiguring.bindings != null ) {
				currentlyConfiguring.configured = true;
				currentlyConfiguring.save();
				go.fire( 'ready', currentlyConfiguring );
			}
		}

		// next controller
		var index = controllersToConfigure.indexOf( currentlyConfiguring );
		if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
		if ( controllersToConfigure.length ) {
			currentlyConfiguring = controllersToConfigure[ 0 ];
			subtitle.render.text = currentlyConfiguring.name;
			subtitle.opacity = 0;
			subtitle.fadeTo( 1, 1 );
		} else {
			return closeConfigurator();
		}

		// reset
		currentButton = -1;
		currentAxis = -1;
		axisDirection = -1;
		error.text = "";
		for ( var i in buttons ) buttons[ i ].accepted = null;
		for ( var i in axis ) axis[ i ].accepted0 = axis[ i ].accepted1 = null;

		// callback
		go.fire( 'willShow', currentlyConfiguring );

		// start configuring
		if ( buttonsFirst ) {
			nextButton();
		} else {
			nextAxis();
		}

	}

	return go;

})( this )

