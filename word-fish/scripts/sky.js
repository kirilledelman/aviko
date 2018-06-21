(function( go ){

	// solid sky
	go.render = new RenderShape( Shape.Rectangle );
	go.render.centered = false;
	go.render.color = 0x6699FF;
	go.render.resize( 304, 20 );
	go.render.renderAfterChildren = true;

	// gradient below
	go.addChild( {
		render: new RenderSprite( 'grad', {
			color: 0x6699FF,
			pivotX: 0, pivotY: 0,
			width: 304, height: 64,
			stippleAlpha: true,
		} ), y: 20
	} );


})( this );