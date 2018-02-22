/*

	Helper script for automatically configuring a newly connected controller.
	When a new controller is connected, if it's not already configured, this
	interface will be presented, and required axis/buttons configured in sequence.
	The configuration will be saved to ./config/NAME_OF_CONTROLLER.json. To reset
	controller just delete that file, or call configurator `reset` method.

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
			return true;
		}

		configurator.controllerRemoved = function ( controller ) {
			// called when controller is disconnected, or if
			// user aborted configuring this controller
		}

		configurator.willShow = function ( controller, container ) {
			// called when configurator is about to show itself for controller
			// this is called after .controllerAdded callback, if controller isn't configured
			// 'container' is GameObject in which the UI is displayed
			// use this callback to pause your game, or add visuals to container
		}

		configurator.finished = function ( controller, success ) {
			// called after controller has been configured
			// success = true if configured successfully, false if aborted
		}

	Methods:

		configurator.reset( controller ) - deletes configuration file for controller, removes bindings

		configurator.configure( controller ) - re-configures controller, replaces config if successful


 */

include( './ui' );
new (function(){

	var axis = [];
	var buttons = [];
	var container, scene, title, subtitle, prompt, action, instruction;
	var controllersToConfigure = [];
	var currentlyConfiguring = null;
	var currentButton, currentAxis, axisDirection, currentSection;
	var buttonsFirst = false;
	var callbacks = {};
	var showDelay = 1.5;
	var style = {};
	var currentlyPressed = null;
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
		[ 'instruction', function (){ return instruction; } ],

		// (Boolean) true when configurator is open
		[ 'open', function (){ return !!currentlyConfiguring; } ],

		// (Function) callback for when controller is connected
		[ 'controllerAdded',  function (){ return callbacks.controllerAdded; }, function ( v ){ callbacks.controllerAdded = v; }, true ],

		// (Function) callback for when controller is disconnected
		[ 'controllerRemoved',  function (){ return callbacks.controllerRemoved; }, function ( v ){ callbacks.controllerRemoved = v; }, true ],

		// (Function) callback before configurator is displayed
		[ 'willShow',  function (){ return callbacks.willShow; }, function ( v ){ callbacks.willShow = v; }, true ],

		// (Function) callback after configuration completes
		[ 'finished',  function (){ return callbacks.finished; }, function ( v ){ callbacks.finished = v; }, true ],

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

	this[ 'reset' ] = function ( controller ) {  controller.reset( true ); }

	this[ 'configure' ] = configureController;

	// handlers

	Input.on( 'controllerAdded', function ( controller ) {
		// if have callback
		if ( callbacks.controllerAdded ) {
			// ask if want to use this one
			if ( callbacks.controllerAdded( controller ) === false ) return;
		}
		// configure
		if ( !controller.configured ) {
			async( function () { configureController( controller ); }, showDelay );
		}
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

	// internal

	function configureController ( controller ) {

		// add to list
		if ( controllersToConfigure.indexOf( controller ) == -1 ) controllersToConfigure.push( controller );

		// if already open, will config after current is finished
		if ( currentlyConfiguring ) return;

		// create container
		container = new GameObject( './panel', {
			background: new Image( App.windowWidth, App.windowHeight ),
			width: App.windowWidth,
			height: App.windowHeight,
			layoutType: Layout.Vertical,
			layoutAlign: LayoutAlign.Stretch
		} );
		// draw current scene to image
		container.background.autoDraw = App.scene;
		container.render.color.set( 1, 1, 1, 0.15 );
		// new scene
		scene = new Scene( { backgroundColor: 0xFFFFFF } );
		scene.addChild( container );
		App.pushScene( scene );

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
			text: controller.name
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
		container.addChild( new GameObject( { ui: new UI( { flex: 1 } ) } ) ); // spacer
		instruction = container.addChild( './text', {
			align: TextAlign.Center,
			size: sx * 0.5,
			color: 0x666666,
			pad: sy,
			wrap: true,
			text: "press and hold any button to skip, press and hold any two buttons to abort"
		} );

		// adjust container on resize
		App.on( 'resized', container.resize );

		// reapply style
		this.style = style;


		// callback
		if ( callbacks.willShow ) callbacks.willShow( controller, container );

		// listeners
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
		currentButton++;
		if ( currentButton >= buttons.length ) {
			if ( buttonsFirst ) nextAxis();
			else nextController( true );
			return;
		}

		// description
		action.text = buttons[ currentButton ].description;
	}

	function nextAxis() {

		currentSection = 'axis';
		axisDirection++;
		if ( axisDirection >= 2 ) {
			axisDirection = -1;
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

	function keyDown( k ) {

		// already pressed something else? abort
		if ( currentlyPressed && currentlyPressed.key != k ) return closeConfigurator();

		// save currently pressed
		currentlyPressed = { key: k };

		// set timer
		debounce( 'configuratorSkip', skipToNext, 2 );

	}

	function keyUp( k ) {

		cancelDebouncer( 'configuratorSkip' );

		// skipping until new press
		if ( !currentlyPressed ) return;

		// check if already in use
		var inUse = checkInUse();
		if ( inUse ) {
			showAlreadyInUse( inUse );
			currentlyPressed = null;
			return;
		}

		// accept
		currentlyPressed = null;
		if ( currentSection == 'buttons' ) {
			buttons[ currentButton ] = { key: k };
			nextButton();
		} else {
			axis[ currentAxis ][ 'accepted' + axisDirection ] = null;
			nextAxis();
		}

	}

	function joyDown( k, c ) {

	}

	function joyUp( k, c ) {

	}

	function joyAxis( v, a, c ) {

	}

	function joyHat( v, h, c ) {

	}

	function checkInUse() {

		// todo - check if currentlyPressed is already in any buttons or axis


		return false; // return description of axis or btn

	}

	function showAlreadyInUse( name ) {
		// TODO - flash prompt
	}

	function skipToNext() {

		currentlyPressed = null;
		if ( currentSection == 'buttons' ) {

			buttons[ currentButton ].accepted = null;
			nextButton();

		} else {

			axis[ currentAxis ][ 'accepted' + axisDirection ] = null;
			nextAxis();

		}
	}

	function closeConfigurator() {

		// remove resize event
		App.off( 'resized', container.resize );

		// clear references
		scene = container = title = subtitle = prompt = action = instruction = null;
		controllersToConfigure = [];
		currentlyConfiguring = null;

		// clear listeners
		Input.off( 'keyDown', keyDown );
		Input.off( 'keyUp', keyUp );
		Input.off( 'joyDown', joyDown );
		Input.off( 'joyUp', joyUp );
		Input.off( 'joyAxis', joyAxis );
		Input.off( 'joyHat', joyHat );

		// remove scene
		App.popScene();

	}

	function nextController( saveCurrent ) {
		if ( saveCurrent ) {
			// apply bindings
			// todo

			// save
			currentlyConfiguring.save();
		}
		// next
		var index = controllersToConfigure.indexOf( currentlyConfiguring );
		if ( index >= 0 ) controllersToConfigure.splice( index, 1 );
		if ( controllersToConfigure.length ) currentlyConfiguring = controllersToConfigure[ 0 ];

		// reset
		currentSection = 'axis';
		currentButton = -1;
		currentAxis = -1;
		axisDirection = -1;
		for ( var i in buttons ) buttons[ i ].accepted = null;
		for ( var i in axis ) axis[ i ].accepted0 = axis[ i ].accepted1 = null;

		// start
		if ( buttonsFirst ) {
			nextButton();
		} else {
			nextAxis();
		}

	}



})();

