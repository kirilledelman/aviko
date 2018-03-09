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
App.setWindowSize( 640, 360, 1 ); // 720p @ 2x

// auto-configure controller
include( 'ui/controller-configurator', {
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
} );

/*var p = App.scene.addChild( 'ui/panel', {
	x: 10, y: 10, width: 100, height: 100,
	pad: 16,
	background: 0xD0D0D0,
	layoutType: Layout.Vertical,
	layoutAlignX: LayoutAlign.Stretch,
	layoutAlignY: LayoutAlign.Start,

} );

var s = p.addChild( 'ui/scrollbar', {
	orientation: 'horizontal',

} );

Input.mouseMove = function ( x, y ) {

	if ( Input.get( Key.MouseButton ) && Input.get( Key.LeftShift ) ) {
		p.ui.resize( x - p.x, y - p.y );
	}

}*/


// auto-show mouse as soon as it moves
Input.showCursor = false;
Input.on( 'mouseMove', function() { Input.showCursor = true; }, true );

// set some constants
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
			blendMode: BlendMode.PremultipliedAlpha
		} ),
		x: dir * App.windowWidth,
		parent: newScene
	} );
	// slide new scene in
	newScene.x = -ghost.x;
	newScene.moveTo( 0, 0, 0.5, Ease.Out );
	ghost.fadeTo( 0, 0.5 ).finished = function () { ghost = ghost.parent = null; }
	// done
	return newScene;
}
