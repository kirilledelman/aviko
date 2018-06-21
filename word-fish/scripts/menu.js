(function () {

	// vars
	var game = null;
	var prompt;
	var language = 1;

	// create scene
	var scene = new Scene( {
		name: "Word Fish Menu",
		backgroundColor: 0x223388,
		music: new Sound( 'Pinwheel.ogg' ),
		select: new Sound( 'select.wav' )
	} );

	// menu container (scaled up to fit screen)
	var menu = scene.addChild( {
		name: "Menu Container",
		scale: 2,
		children: [
			// logo
			new GameObject( {
				name: "Logo",
				render: new RenderSprite( 'player:fish-title' ),
				x: 100, y: 0
			} ),
			// Russian / English prompt
			prompt = new GameObject( {
				name: "Prompt",
				render: new RenderText( {
					text: "^4[Select]^c to toggle\n\n^B^3RUSSIAN^c^b ^9ENGLISH^c" +
					"\n\nAny other button to start",
					multiLine: true,
					font: 'Fairfax', // boldFont: 'FairfaxBold',
					align: TextAlign.Center,
					antialias: false,
					size: 11,
					color: 0xFFFFFF,
				} ),
				x: 85, y: 140,
			} ),
			// How to exit
			new GameObject( {
				render: new RenderText( {
					text: "^6^BSELECT + START^c^b to exit",
					font: 'Fairfax', // boldFont: 'FairfaxBold',
					align: TextAlign.Center,
					size: 11,
					antialias: false,
					color: 0xFFFFFF,
				} ),
				opacity: 0.7,
				x: 90, y: 220,
			} ),
		],
	} );

	// bind controls
	function controllerInput( name, val ) {

		// if game in progress
		if ( game ) {

			// forward input to its handler
			game.controllerInput( name, val, this );

		// otherwise
		} else if ( val ){

			// select button?
			if ( name == 'select' ) {
				(new Sound( 'select' )).play();
				// toggle language
				language = ( language + 1 ) % 2;
				prompt.render.text = "^4[Select]^c to toggle\n\n" +
					( language ? "^B^3RUSSIAN^c^b ^9ENGLISH^c" : "^9RUSSIAN^c ^B^3ENGLISH^c^b" ) +
					"\n\nAny other button to start";

			// any other button
			} else {

				// delayed start (to allow select+start to act first)
				scene.debounce( 'start', function () {
					game = include( 'game' );
					game.language = language;
					game.scaleScene( App.windowWidth, App.windowHeight );
					sceneForward( game );
					scene.select.play();
					scene.music.stop();
				}, 0.5 );

			}
		}
	}

	// controller ready handler
	function controllerReady( cont ) {

		// bind all axis and buttons to be handled by same func
		cont.on( [ 'accept', 'cancel', 'select', 'start', 'vertical', 'horizontal' ], controllerInput );

	}
	configurator.on( 'ready', controllerReady );
	configurator.refresh(); // causes ready to be called on all connected controllers

	// scene scaling and centering
	function scaleScene( w, h ) {
		var sh = w / 640;
		var sv = h / 480;
		menu.scale = 2 * Math.min( sh, sv );
		menu.x = 0.5 * ( w - menu.width * menu.scale);
		menu.y = 0.5 * ( h - menu.height * menu.scale );
		if ( game ) game.scaleScene( w, h );
	}
	App.on( 'resized', scaleScene );
	scaleScene( App.windowWidth, App.windowHeight );

	// scene change handler
	App.on( 'sceneChanged', function ( newScene, oldScene ) {

		// re-entering menu
		if ( newScene == scene ){

			// unpause
			App.timeScale = 1;

			// destroy game
			if ( game ) game = null;
			scaleScene( App.windowWidth, App.windowHeight );
			gc();

			// play theme
			scene.music.play( 0 );

		// leaving menu
		} else {

			// stop music
			scene.music.stop();

		}
	});

	// done
	return scene;
})();
