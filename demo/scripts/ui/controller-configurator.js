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

		// callbacks (all optional; note that these callbacks aren't events)

		configurator.controllerAdded = function ( controller ) {
			// called whenever a new controller is connected
			// return false if you don't want to use this controller
			// otherwise, this callback can be used to add handlers for
			// buttons and axis etc.
			// if this controller isn't configured, configurator will display
			// its interface after you return true, and call willShow() callback
			return true;
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
			// called after controller has been configured
			// or was configured previously
			// can use this callback to add new player into game
			// or ensure controller is available and ready
		}

	Methods:

		configurator.reset( controller ) - deletes configuration file for controller, removes bindings

		configurator.configure( controller ) - re-configures controller, replaces config if successful


 */

include( './ui' );
new (function( params ){

	var axis = [];
	var buttons = [];
	var container, scene, title, subtitle, prompt, action, error, instruction;
	var controllersToConfigure = [];
	var currentlyConfiguring = null;
	var currentButton, currentAxis, axisDirection, currentSection;
	var buttonsFirst = false;
	var callbacks = {};
	var showDelay = 1.5;
	var style = {};
	var currentlyPressed = null;
	var waitForAxis = null, waitForAxisDirection = 0;
	var waitForHat = null, waitForHatDirection = 0, waitForHatValue = 0;
	var ignoreNextEvent = false;

	// API properties
	var mappedProps = [

		// (Array) of { id, minus, plus, description, descriptionMinus, descriptionPlus }
		[ 'axis',  function (){ return axis; }, function ( v ){ axis = v; } ],

		// (Array) of { id, description }
		[ 'buttons',  function (){ return buttons; }, function ( v ){ buttons = v; } ],

		// (Boolean) buttons before axis
		[ 'buttonsFirst',  function (){ return buttonsFirst; }, function ( v ){ buttonsFirst = v; } ],

		// (GameObject) container when configurator is open
		[ 'container', function (){ return container; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'title', function (){ return title; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'subtitle', function (){ return subtitle; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'prompt', function (){ return prompt; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'action', function (){ return action; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'error', function (){ return error; } ],

		// (GameObject) (ui/text.js) when configurator is open
		[ 'instruction', function (){ return instruction; } ],

		// (Boolean) true when configurator is open
		[ 'isOpen', function (){ return !!currentlyConfiguring; } ],

		// (Function) callback for when controller is connected
		[ 'controllerAdded',  function (){ return callbacks.controllerAdded; }, function ( v ){ callbacks.controllerAdded = v; }, true ],

		// (Function) callback for when controller is disconnected
		[ 'controllerRemoved',  function (){ return callbacks.controllerRemoved; }, function ( v ){ callbacks.controllerRemoved = v; }, true ],

		// (Function) callback before configurator is displayed for another controller
		[ 'willShow',  function (){ return callbacks.willShow; }, function ( v ){ callbacks.willShow = v; }, true ],

		// (Function) callback before configurator is dismissed
		[ 'willHide',  function (){ return callbacks.willHide; }, function ( v ){ callbacks.willHide = v; }, true ],

		// (Function) callback after controller is deemed ready to use
		[ 'ready',  function (){ return callbacks.ready; }, function ( v ){ callbacks.ready = v; }, true ],

		// (Number) delay in seconds before configurator is displayed
		[ 'showDelay',  function (){ return showDelay; }, function ( v ){ showDelay = v; } ],

		// (Object) used to apply style (collection of properties) - use to preconfigure look of title, prompt etc.
		[ 'style',  function (){ return style; }, function ( v ){
			style = v;
			if ( container ) UI.base.applyDefaults( this, style );
		} ],

	];
	UI.base.addMappedProperties( this, mappedProps );

	// API functions

	this[ 'reset' ] = function ( controller ) {
		controller.reset( true );
		var index = controllersToConfigure.indexOf( controller );
		if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
	}

	this[ 'configure' ] = configureController;

	// optional init parameter ( can pass as include( 'ui/controller-configurator', { ... init ... } ) )

	if ( params != global ) UI.base.applyDefaults( this, params );

	// display elements - feel free to modify, or adjust in willShow callback

	// create container
	container = new GameObject( './panel', {
		background: new Image( App.windowWidth, App.windowHeight ),
		width: App.windowWidth,
		height: App.windowHeight,
		layoutType: Layout.Vertical,
		layoutAlign: LayoutAlign.Stretch
	} );
	container.render.color.set( 1, 1, 1, 0.15 );

	// new scene
	scene = new Scene( { backgroundColor: 0xFFFFFF } );
	scene.addChild( container );

	// automatic initial scale
	var sx = App.windowWidth * 0.05;
	var sy = App.windowHeight * 0.05;

	// add text
	title = container.addChild( './text', {
		bold: true,
		align: TextAlign.Center,
		size: sx * 1.2,
		marginTop: sy,
		wrap: true,
		color: 0x006699,
		text: "New Controller Detected"
	} );
	subtitle = container.addChild( './text', {
		align: TextAlign.Center,
		bold: true,
		size: sx,
		color: 0x003366,
		wrap: true,
		marginTop: sy * 0.5,
		text: "_controller_"
	} );
	container.addChild( new GameObject( { ui: new UI( { flex: 1 } ) } ) ); // spacer
	prompt = container.addChild( './text', {
		align: TextAlign.Center,
		size: sx,
		color: 0x0,
		marginTop: sy * 2,
		wrap: true,
		text: "Press a button or axis for"
	} );
	action = container.addChild( './text', {
		bold: true,
		align: TextAlign.Center,
		size: sx,
		color: 0xD03399,
		wrap: true,
		text: "_description_"
	} );
	error = container.addChild( './text', {
		align: TextAlign.Center,
		size: sx * 0.5,
		color: 0x901010,
		pad: sy,
		wrap: true,
		text: ""
	} );
	container.addChild( new GameObject( { ui: new UI( { flex: 1 } ) } ) ); // spacer
	instruction = container.addChild( './text', {
		align: TextAlign.Center,
		size: sx * 0.5,
		color: 0x666666,
		pad: sy,
		wrap: true,
		text: "press and hold any button to skip, press and hold any two buttons to abort"
	} );

	// internals

	Input.on( 'controllerAdded', function ( controller ) {

		// if have callback
		if ( callbacks.controllerAdded ) {
			// ask if want to use this one
			if ( callbacks.controllerAdded( controller ) === false ) return;
		}
		// configure
		if ( !controller.configured ) {
			async( function () { configureController( controller ); }, showDelay );
		} else if ( callbacks.ready ) callbacks.ready( controller );

	}.bind( this ));

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
		if ( callbacks.controllerRemoved ) callbacks.controllerRemoved( controller );

	}.bind( this ));

	function configureController ( controller ) {

		// add to list
		if ( controllersToConfigure.indexOf( controller ) == -1 ) controllersToConfigure.push( controller );

		// if already open, will config after current is finished
		if ( currentlyConfiguring ) return;

		// draw current scene to image
		container.background.autoDraw = App.scene;
		App.pushScene( scene );

		// reapply style
		this.style = style;

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

	function nextButton() {

		currentSection = 'buttons';
		error.text = "";
		prompt.text = (currentlyConfiguring.name == 'Keyboard' ? "Press a key for" : "Press a button for");
		currentButton++;
		if ( currentButton >= buttons.length ) {
			if ( buttonsFirst ) nextAxis();
			else nextController( true );
			return;
		}

		// update description
		action.text = buttons[ currentButton ].description;
	}

	function nextAxis() {

		currentSection = 'axis';
		error.text = "";
		prompt.text = (currentlyConfiguring.name == 'Keyboard' ? "Press a key for" : "Move joystick or press a button for");
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
		action.text = desc;
	}

	function keyDown( k, s, a, c, g, r ) {

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
		error.text = "";
		var inUse = checkInUse();
		if ( inUse ) {
			currentlyPressed = null;
			error.text = "Key already in use by ^B" + inUse;
			return;
		}

	}

	function keyUp( k ) {

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

	function joyDown( k, c ) {

		// controller must match
		if ( currentlyConfiguring != c ) return;

		// already pressed something else? abort
		if ( currentlyPressed && currentlyPressed.button != k ) return closeConfigurator();

		// save currently pressed
		currentlyPressed = { button: k };

		// set timer
		debounce( 'configuratorSkip', skipToNext, 2 );

		// check if already in use
		error.text = "";
		var inUse = checkInUse();
		if ( inUse ) {
			currentlyPressed = null;
			error.text = "Button already in use by ^B" + inUse;
			return;
		}

	}

	function joyUp( k, c ) {

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

	function joyAxis( v, a, c ) {

		// controller must match
		if ( currentlyConfiguring != c ||
			currentSection == 'buttons' ) return;

		// if not waiting to release
		if ( waitForAxis !== null ) {

			// check if axis is in use
			error.text = "";
			currentlyPressed = { axis: a };
			var inUse = checkInUse();
			currentlyPressed = null;
			if ( inUse ) {
				error.text = "Axis already in use by ^B" + inUse;
				return;
			}

			// if doing second direction, and first direction was button, show error
			if ( axisDirection == 1 ) {
				if ( axis[ currentAxis ].accepted0 && axis[ currentAxis ].accepted0.button != undefined ) {
					error.text = "Opposite direction must also be a button";
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

	function joyHat( x, y, h, c ) {

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
			error.text = "";
			currentlyPressed = { hat: h, dir: dir, val: val };
			var inUse = checkInUse();
			currentlyPressed = null;
			if ( inUse ) {
				error.text = "Hat already in use by ^B" + inUse;
				return;
			}

			// if doing second direction, and first direction was button, show error
			if ( axisDirection == 1 ) {
				if ( axis[ currentAxis ].accepted0 && axis[ currentAxis ].accepted0.button != undefined ) {
					error.text = "Opposite direction must also be a button";
					return;
				}
			}
		}

		// clear reset
		if ( waitForHat == h && val == -1 ) {
			// all good
			axis[ currentAxis ][ 'accepted' + axisDirection ] = { hat: h, dir: waitForHatDirection, val: waitForHatValue };
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

	function checkInUse() {
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
	function skipToNext() {

		currentlyPressed = null;
		var skipText;
		if ( currentSection == 'buttons' ) {

			skipText = buttons[ currentButton ].description;
			buttons[ currentButton ].accepted = null;
			nextButton();

		} else {

			skipText = (axis[ currentAxis ].description ? axis[ currentAxis ].description + " / ": "" );
			skipText += ( axisDirection ? axis[ currentAxis ].plus : axis[ currentAxis ].minus );
			axis[ currentAxis ][ 'accepted' + axisDirection ] = null;
			nextAxis();

		}

		// set error text
		error.text = "^N^0Skipped ^B" + skipText;
	}

	function closeConfigurator() {

		// clear references
		container.background.autoDraw = null;
		controllersToConfigure = [];
		currentlyConfiguring = null;

		// clear listeners
		App.off( 'resized', container.resize );
		Input.off( 'keyDown', keyDown );
		Input.off( 'keyUp', keyUp );
		Input.off( 'joyDown', joyDown );
		Input.off( 'joyUp', joyUp );
		Input.off( 'joyAxis', joyAxis );
		Input.off( 'joyHat', joyHat );

		// notify
		if ( callbacks.willHide ) callbacks.willHide();

		// remove scene
		App.popScene();

	}

	// called in the beginning, and after each controller in queue is finished
	function nextController( saveCurrent ) {
		// apply bindings
		if ( saveCurrent ) {
			currentlyConfiguring.reset();
			for ( var i in buttons ) {
				var b = buttons[ i ].accepted;
				if ( b ){
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
				if ( callbacks.ready ) callbacks.ready( currentlyConfiguring );
			}
		}

		// next controller
		var index = controllersToConfigure.indexOf( currentlyConfiguring );
		if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
		if ( controllersToConfigure.length ) {
			currentlyConfiguring = controllersToConfigure[ 0 ];
			subtitle.text = currentlyConfiguring.name;
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
		if ( callbacks.willShow ) callbacks.willShow( currentlyConfiguring );

		// start configuring
		if ( buttonsFirst ) {
			nextButton();
		} else {
			nextAxis();
		}

	}

})( this );

