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

// set screen size
App.setWindowSize( 640, 480, 1 );
App.windowResizable = true;
App.fixedWindowResolution = false;

/*
// auto-configure controller
var configurator = include( 'controller-configurator', {
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
		// if pressing select + start
		controller.on( ['select','start' ], maybeExit );
	}
} );
*/

// inspector - hold right mouse button down to activate
var inspector = include( 'ui/inspector' );


// used by controller to pop scene or exit app
function maybeExit() {
	// 'this' here is Controller instance
	if ( this.get( 'select' ) && this.get( 'start' ) ) {
		stopEvent();
		sceneBack();
	}
}

// smooth transition back, or exit, if in main menu
function sceneBack() {
	// if can pop scene
	if ( App.sceneStack.length > 1 ){
		var scene = App.popScene();
		transitionScene( App.scene, scene, 1 );
	// otherwise quit
	} else {
		quit();
	}
}

// pushes new scene in via a transition
function sceneForward( sub ){
	if (sub.ui) sub.ui.async( sub.ui.requestLayout, 0.3 );
	this.async( function () {
		transitionScene( sub, App.scene, -1 );
		App.pushScene( sub );
	}, 0.25 );
}

// auto-show mouse as soon as it moves
Input.showCursor = false;
Input.on( 'mouseMove', function() { Input.showCursor = true; }, true );

// set some color globals
Color.Background = 0xFFFFFF;
Color.Title = 0x106633;
Color.Text = 0x333333;

// show main menu
App.scene = include( 'main-menu' );

// helper for scene transition - adds an image of current scene on top of newScene, and starts fading/moving animation
function transitionScene( newScene, oldScene, dir ) {
	// draw old scene to image
	var ghost = new GameObject( {
		render: new RenderSprite( {
			image: new Image( App.windowWidth, App.windowHeight, oldScene ),
		} ),
		x: dir * App.windowWidth,
		parent: newScene
	} );
	// slide new scene in
	new Tween( newScene, 'cameraX', -ghost.x, 0, 0.5, Ease.Out );
	ghost.fadeTo( 0, 0.5 ).finished = function () {
		ghost = ghost.parent = null;
		gc();
	}
	// done
	return newScene;
}



