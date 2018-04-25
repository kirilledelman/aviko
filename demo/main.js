/*
	Aviko
	Copyright 2018 Kirill Edelman - GPL
	Open source libraries used in Aviko have their own copyrights and licenses
	SDL2
	SDL_gpu
	SDL_image
	SDL_mixer
	SDL_ttf
	SDL_net
	Mozilla
	Box2D
	LiquidFun

 */


// set screen size and pixel doubling
App.setWindowSize( 640, 480, 1 ); // 720p @ 2x

// auto-configure controller
/* include( 'ui/controller-configurator', {
	axis: [
		{
			id: 'horizontal',
			minus: 'Left',
			plus: 'Right',
		}, {
			id: 'vertical',
			minus: 'Up',
			plus: 'Down',
		}
	],
	buttons: [
		{
			id: 'accept',
			description: 'Primary / Select / (A)'
		}, {
			id: 'cancel',
			description: 'Secondary / Back / (B)'
		}, {
			id: 'select',
			description: 'Select'
		}, {
			id: 'start',
			description: 'Start'
		}
	],
	ready: function ( controller ) {
		// quit if pressing select + start
		controller.on( ['select','start' ], function () {
			if ( this.get( 'select' ) && this.get( 'start' ) ) quit();
		} );
	}
} );*/

// auto-show mouse as soon as it moves
Input.showCursor = false;
Input.on( 'mouseMove', function() { Input.showCursor = true; }, true );

// set some color globals
Color.Background = 0xFFFFFF;
Color.Title = 0x106633;
Color.Text = 0x333333;

// show main menu
App.scene = include( 'main-menu' );

/*var s = new GameObject( {
	render: new RenderSprite( {
		texture: 'test:test',
		slice: 9,
		width: 200, height: 100,
	} ),
	parent: App.scene,
} );

Input.mouseWheel = function( wy, wx ) {
	if ( wy ) s.render.tileY += ( wy > 0 ? 1 : -1 );
	if ( wx ) s.render.tileX += ( wx > 0 ? 1 : -1 );

	log ( s.render.tileX, s.render.tileY );
}*/

// helper for scene transition - adds an image of current scene on top of newScene, and starts fading/moving animation
function transitionScene( newScene, oldScene, dir ) {
	// draw old scene to image
	var ghost = new GameObject( {
		render: new RenderSprite( {
			image: new Image( App.windowWidth, App.windowHeight, oldScene ),
			blendMode: BlendMode.PremultipliedAlpha
		} ),
		x: dir * App.windowWidth,
		parent: newScene
	} );
	// slide new scene in
	newScene.x = -ghost.x;
	newScene.moveTo( 0, 0, 0.5, Ease.Out );
	ghost.fadeTo( 0, 0.5 ).finished = function () { ghost = ghost.parent = null; gc(); }
	// done
	return newScene;
}
