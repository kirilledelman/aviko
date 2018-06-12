(function () {

	// create scene
	var scene = new Scene( {
		name: "Word Fish Menu",
		backgroundColor: 0x223388,
	} );
	// App.debugDraw = true;

	// menu
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
				pad: -10,
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
					new GameObject( 'ui/text', {
						text: "Push any button\nto start",
						multiLine: true,
						font: 'UpheavalPro',
						align: TextAlign.Center,
						antialias: false,
						align: TextAlign.Center,
						size: 16,
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

	// bind controls
	function controllerInput( name, val ) {
		if ( game ) game.controllerInput( name, val, this );
		else if ( val ){
			// delayed start (to allow select+start to act first)
			scene.debounce( 'start', function () {
				game = include( './game' );
				game.scaleScene( App.windowWidth, App.windowHeight );
				sceneForward( game );
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

			// destroy game
			if ( game ) game = null;
			scaleScene( App.windowWidth, App.windowHeight );
			gc();

		// leaving menu to go back
		} else if ( newScene != scene && newScene != game ) {

			// cleanup
			scene.cancelDebouncer( 'start' );
			configurator.off( 'ready', controllerReady );
			App.off( 'resized', scaleScene );
			App.off( 'sceneChanged', arguments.callee );
			for ( var i in Input.controllers )
				Input.controllers[ i ].off( [ 'accept', 'cancel', 'select', 'start', 'vertical', 'horizontal' ], controllerInput );

		}
	});
	scaleScene( App.windowWidth, App.windowHeight );
	return scene;
})();
