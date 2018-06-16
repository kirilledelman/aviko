(function () {

	// create scene
	var scene = new Scene( {
		name: "Word Fish Menu",
		backgroundColor: 0x223388,
		music: new Sound( './sound/Pinwheel.ogg' ),
		select: new Sound( './sound/select.wav' )
	} );
	// App.debugDraw = true;

	// menu
	var prompt;
	var menu = scene.addChild( 'ui/panel', {
		name: "Menu",
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		fitChildren: false,
		fixedPosition: true,
		width: 320,
		height: 240,
		scale: 2,
		pad: 5,
		children: [
			new GameObject( 'ui/text', {
				text: "WORD\nFISH!",
				name: "Title",
				multiLine: true,
				font: 'blogger-sans-bold',
				align: TextAlign.Center,
				antialias: false,
				outlineColor: 0x0,
				outlineRadius: 4,
				outlineOffsetY: 2,
				characterSpacing: -4, lineSpacing: -20,
				marginTop: 20,
				size: 64,
				color: 0xFFFFFF,
				update: function () {
					this.renderText.pivotX = 0.03 * Math.sin( App.time * 2 );
					this.renderText.pivotY = 0.1 * Math.cos( App.time * 2 );
				}
			} ),
			new GameObject( 'ui/panel', {
				layoutType: Layout.Horizontal,
				layoutAlignY: LayoutAlign.Center,
				layoutAlignX: LayoutAlign.Stretch,
				spacing: 10,
				flex: 1,
				children: [
					prompt = new GameObject( 'ui/text', {
						text: "^4[Select]^c to toggle\n\n^B^3RUSSIAN^c^b ^9ENGLISH^c" +
						"\n\nAny other button to start",
						multiLine: true,
						font: 'Fairfax', //'UpheavalPro',
						boldFont: 'FairfaxBold',
						align: TextAlign.Center,
						antialias: false,
						align: TextAlign.Center,
						size: 11, // 16,
						color: 0xFFFFFF,
					} )
				],
			}),
			new GameObject( 'ui/text', {
				text: "^6^BSELECT + START^c^b to exit",
				font: 'Fairfax',
				boldFont: 'FairfaxBold',
				align: TextAlign.Center,
				size: 11,
				antialias: false,
				opacity: 0.75,
				color: 0xFFFFFF,
			} )
		],
	} );
	var game = null;
	var language = 1;

	// bind controls
	function controllerInput( name, val ) {
		if ( game ) game.controllerInput( name, val, this );
		else if ( val ){
			if ( name == 'select' ) {
				language = ( language + 1 ) % 2;
				prompt.text = "^4[Select]^c to toggle\n\n" +
					( language ? "^B^3RUSSIAN^c^b ^9ENGLISH^c" : "^9RUSSIAN^c ^B^3ENGLISH^c^b" ) +
					"\n\nAny other button to start";
				return;
			}

			// delayed start (to allow select+start to act first)
			scene.debounce( 'start', function () {
				game = include( './game' );
				game.language = language;
				game.scaleScene( App.windowWidth, App.windowHeight );
				sceneForward( game );
				scene.select.play();
				scene.music.stop();
			}, 0.5 );
		}
	}
	function controllerReady( cont ) {
		cont.on( [ 'accept', 'cancel', 'select', 'start', 'vertical', 'horizontal' ], controllerInput );
	}
	configurator.on( 'ready', controllerReady );
	configurator.refresh();

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
	App.on( 'sceneChanged', function ( newScene, oldScene ) {
		// re-entering this scene from game
		if ( game && oldScene == game ){
			// unpause
			App.timeScale = 1;
			// destroy game
			if ( game ) game = null;
			scaleScene( App.windowWidth, App.windowHeight );
			gc();

			// play theme
			oldScene.music.stop();
			scene.music.play( 0 );

		// leaving menu to go back
		} else if ( newScene != scene && newScene != game ) {

			// cleanup
			scene.cancelDebouncer( 'start' );
			configurator.off( 'ready', controllerReady );
			App.off( 'resized', scaleScene );
			App.off( 'sceneChanged', arguments.callee );
			for ( var i in Input.controllers )
				Input.controllers[ i ].off( [ 'accept', 'cancel', 'select', 'start', 'vertical', 'horizontal' ], controllerInput );

			// stop music
			scene.music.stop();

		}
	});
	scaleScene( App.windowWidth, App.windowHeight );
	scene.music.play( 0 );
	return scene;
})();
