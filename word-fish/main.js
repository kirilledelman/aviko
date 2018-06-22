// set screen size and pixel doubling
App.setWindowSize( 320, 240, 2 );

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
log (" hi, configurator=", configurator );

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
	// if (sub.ui) sub.ui.async( sub.ui.requestLayout, 0.3 );
	this.async( function () {
		transitionScene( sub, App.scene, -1 );
		App.pushScene( sub );
	}, 0.25 );
}

// scene transition - adds an image of current scene on top of newScene, and starts fading/moving animation
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
	newScene.requestLayout();
	return newScene;
}

// show main menu
App.scene = include( 'menu' );
