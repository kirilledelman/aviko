(function( go ) {

	var letter = '';
	var rotVel = ((Math.random() > 0.5) ? 1 : -1) * ( 1 + Math.random() * 2 );
	var velMultiplier = 1 + Math.random() * 1;
	var wobbleSpeed = 5 + Math.random() * 5;
	var rot = 0;
	var missed = false;

	Object.defineProperty( go, 'letter', {
		get: function(){ return letter; },
		set: function( l ){
			letter = l;
			go.render = new RenderText( {
				text: l,
				color: 0xFFFFFF,
				outlineColor: [ 0.05, 0.07, 0.2, 1 ],
				outlineRadius: 2,
				outlineOffsetY: 1,
				size: 32 - Math.random() * 16,
				antialias: false,
				autoResize: true,
				pivotX: 0.5, pivotY: 0.5
			} );
			go.render.color.hsv( Math.random(), 0.2, 0.9 );
		}
	} );

	// red outline to indicate missed letter
	Object.defineProperty( go, 'missed', {
		get: function(){ return missed; },
		set: function( m ){
			missed = m;
			go.render.outlineColor = m ? [ 0.8, 0.3, 0.2, 1 ] : [ 0.05, 0.07, 0.2, 1 ];
		}
	} );

	// think function
	go.update = function ( dt ) {

		// move
		//rot += dt * rotVel;
		//go.angle = Math.round( rot / 5 ) * 15;
		go.x -= dt * go.game.travelSpeed * velMultiplier;
		go.y = go.lane * go.game.scene.gridSize + 0.25 * wobbleSpeed * Math.cos( App.time * wobbleSpeed * 2 );

		if ( go.useful && go.x < go.game.player.x ) {
			go.missed = true;
			go.useful = false;
		}

		// when offscreen disappear
		if ( go.x < -32 ) go.parent = null;

	}

	// just fall down
	go.sink = function () {
		go.update = null;
		go.render.color.hexTo( '3366FF', 1 );
		// remove
		var index = go.game.letters.indexOf( go );
		if ( index >= 0 ) go.game.letters.splice( index, 1 );
		//
		go.moveTo( go.x - go.game.travelSpeed * velMultiplier, go.y + 220, 1.5, Ease.In ).finished = function () {
			// remove
			var index = go.game.letters.indexOf( go );
			if ( index >= 0 ) go.game.letters.splice( index, 1 );
			// destroy
			go.parent = null;
		}
	}

	// move to another lane
	go.bump = function ( randomLane ) {
		var newLane = randomLane ?
			( Math.floor( go.lane + 1 + Math.random() * 8 ) % 5 ) :
			( go.lane < 5 ? ( go.lane + 1 ) : ( go.lane - 1 ) );
		var t = new Tween( go, 'lane', go.lane, newLane, 1, Ease.InOut );
	}

	// letter is removed
	go.removed = function () {
		// remove from letter array
		var index = go.game.letters.indexOf( go );
		if ( index >= 0 ) go.game.letters.splice( index, 1 );

		// schedule next letter spawn
		go.game.async( go.game.spawnLetter, 0.5 + Math.random() * 0.5 );
	}

} )( this );