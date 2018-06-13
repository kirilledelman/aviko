(function( go ){

	go.render = new RenderShape( Shape.Polygon );
	go.render.color.set( 0.1, 0.15, 0.4, 0.9 );
	go.render.outlineColor.set( 0.2, 0.8, 1, 1 );
	go.render.lineThickness = 2;

	// waves
	var wavePoints = 32;
	var distanceBetweenPoints = 304 / ( wavePoints - 1 );
	var pts = [ -10, 16 ];
	for ( var i = 0; i < wavePoints; i++ ) {
		pts.push( 304 * ( i / ( wavePoints - 1 ) ), 16 );
	}
	pts.push( 314, 20, 314, 240, -10, 240 );
	go.render.points = pts;

	// update
	go.update = function ( dt ){
		// update waves
		var phase = go.game.travelSpeed / ( 2.0 * Math.PI );
		for ( var i = 0; i < wavePoints; i++ ) {
			go.render.points[ i * 2 + 3 ] = 20 + Math.sin( App.time * phase + i ) * 2;
		}
	}

	// start emitting bubbles
	function emitBubble() {
		var size = Math.round( 2 + Math.random() * 3 );
		var bubble = go.addChild( {
			name: "Bubble",
			render: new RenderShape( {
				shape: Shape.Ellipse,
				width: size,
				height: size * 0.7,
				filled: true,
				lineThickness: 1,
				outlineColor: [ 0.2, 0.8, 1, 1 ],
				color: [ 0.2, 0.8, 1, 0.75 ],
			}, 0 ),
			climbSpeed: size * 10,
			x: go.game.travelSpeed + 320 * Math.random(),
			y: 220,
			opacity: 0.5,
			update: function( dt ) {
				this.x -= go.game.travelSpeed * dt + Math.cos( App.time * this.climbSpeed * 0.07 ) * 0.5 * App.timeScale;
				this.y -= this.climbSpeed * dt;
				if ( this.y < 40 ) {
					this.render.stipple = 1.0 - ( this.y - 20 ) / 20 ;
				}
				if ( this.x < -10 || this.y <= 20 ) this.parent = null;
			}
		} );
		// next bubble
		if ( App.scene == go.scene ) go.debounce( 'bubble', emitBubble, 0.5 );
	}
	go.debounce( 'bubble', emitBubble, 0.5 );

	// start emitting sun rays
	function emitRay() {
		var ray = go.addChild( {
			name: "Sunbeam",
			render: new RenderSprite( './grad.png', {
				height: 32 + Math.random() * 32,
				width: 4 + Math.random() * 8,
				color: 0x6699FF,
				pivotX: 0.5, pivotY: 0
			} ),
			x: go.game.travelSpeed + 320 * Math.random(),
			y: 10,
			angle: Math.round( ( 90 * Math.random() - 30 ) / 15 ) * 15,
			angleDelta: Math.random() * 10 - 5,
			opacity: 0,
			life: 0, timeToLive: 1 + 0.5 * Math.random(),
			update: function( dt ) {
				this.x -= go.game.travelSpeed * dt;
				this.opacity = 0.5 * Math.sin( ( this.life / this.timeToLive ) * Math.PI );
				this.angle += this.angleDelta * dt;
				this.life += dt;
				if ( this.life >= this.timeToLive || this.x < -10 ) this.parent = null;
			}
		} );
		// next ray
		if ( App.scene == go.scene ) go.debounce( 'ray', emitRay, 0.2 + 0.5 * Math.random() );
	}
	go.debounce( 'ray', emitRay, 0.5 );

})( this );