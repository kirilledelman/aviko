log( "Привет из main.js!" );

App.setWindowSize( 400, 240, 2 );
App.windowResizable = true;

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
			description: 'Primary / Select'
		}, {
			id: 'cancel',
			description: 'Secondary / Back'
		},
	],
} );

var text = App.scene.addChild( 'ui/text', { text: "Click to add points, Enter to make dynamic body, Shift+Enter to make static.", wrap: true, x: 0, width: 400 } );
//App.scene.gravityY = 10;

var poly;

Input.mouseDown = function ( btn, x, y ) {

	if ( !poly ) {
		poly = new GameObject({
			render: new RenderShape( Shape.Polygon ),
			parent: App.scene,
			opacity: 0.5
		});
		poly.outline = new GameObject({
			render: new RenderShape( {
				shape: Shape.Chain,
				color: 0xff0000
			} ),
			parent: App.scene,
		});
	}

	poly.render.points.push( x, y );
	poly.outline.render.points.push( x, y );

}

Input.keyDown = function ( k, s ) {

	if ( k == Key.Enter && poly ) {
		poly.body = new Body();
		poly.outline.parent = null;
		poly.outine = null;
		poly.opacity = 0.8;
		poly.body.shape = new BodyShape( Shape.Polygon, poly.render.points );
		if ( s ) poly.body.type = BodyType.Static;
		else {
			poly.ui = new UI();
			poly.ui.mouseOver = function(){ log( "over" ); this.gameObject.render.color = 0x66FF66; };
			poly.ui.mouseOut = function(){ log( "out" ); this.gameObject.render.color = 0xFFFFFF; };

		}


		poly = null;
	}
}


Input.mouseWheel = function ( y, x ) {
	if ( y ) text.revealEnd += y / Math.abs( y );
}