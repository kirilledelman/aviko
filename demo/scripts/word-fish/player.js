(function( go ){

	go.render = new RenderSprite( 'smiley' );
	go.render.pivotX = go.render.pivotY = 0.5;
	go.render.flipX = true;
	go.scale = 0.3;

	// animated swimming
	var velX = 0, velY = 0, minVel = 8, maxVel = 64, velEaseDist = 8,
		destX, destY;

	//
	go.swimVertical = function ( val ) {
		if ( destX === undefined ) destX = x;
		if ( destY === undefined ) destY = y;
		destY = Math.max( 32, Math.min( 5 * 32 , val ) );
		if ( ( velY < 0 && destY > go.y ) || ( velY > 0 && destY < go.y ) ) velY *= -0.5;
		go.update = swim;
	}

	// relative offset
	go.swimHorizontal = function ( val ) {
		if ( destX === undefined ) destX = x;
		if ( destY === undefined ) destY = y;
		destX = Math.max( 32, Math.min( 96, val ) );
		if ( ( velX < 0 && destX > go.x ) || ( velX > 0 && destX < go.x ) ) velX *= -0.5;
		go.update = swim;
	}

	// easing towards destX, destY
	function swim( dt ){
		// update velocity
		var dist, distAbs, vel;
		dist = destY - go.y;
		if ( dist != 0 ) {
			distAbs = Math.abs( dist );
			vel = Math.min( maxVel, minVel + ( maxVel - minVel ) * ( distAbs / velEaseDist ) );
			if ( dist < 0 ) vel = -vel;
			if ( velY != vel ) {
				velY += ( vel - velY ) * dt * maxVel;
			}
			// tilt up/down
			go.angle = 10 * velY / maxVel;
		} else velY = 0;
		dist = destX - go.x;
		if ( dist != 0 ) {
			distAbs = Math.abs( dist );
			vel = Math.min( maxVel, minVel + ( maxVel - minVel ) * ( distAbs / velEaseDist ) );
			if ( dist < 0 ) vel = -vel;
			if ( velX != vel ) {
				velX += ( vel - velX ) * dt * maxVel;
			}
			vel = Math.abs( velX );
			if ( distAbs < velEaseDist ) { // slowing
				go.scaleX = Math.max( go.scaleY * 0.75, go.scaleX - go.scaleY * dt * 2 )
			} else if ( vel < maxVel ) { // accelerating
				go.scaleX = Math.min( go.scaleY * 1.25, go.scaleX + go.scaleY * dt * 4 );
			}
		} else velX = 0;

		// update
		if ( velY > 0 ) {
			go.y = Math.min( destY, velY * dt + go.y );
		} else if ( velY < 0 ) {
			go.y = Math.max( destY, velY * dt + go.y );
		} else go.angle = 0;

		if ( velX > 0 ) {
			go.x = Math.min( destX, velX * dt + go.x );
		} else if ( velX < 0 ) {
			go.x = Math.max( destX, velX * dt + go.x );
		}
		go.scaleX = go.scaleX + (go.scaleY - go.scaleX) * dt * 4;

		// turn off when reached dest
		if ( go.x == destX && go.y == destY && Math.abs( go.scaleX - go.scaleY ) < 0.001 ) {
			go.angle = 0;
			go.scaleX = go.scaleY;
			velX = 0; velY = 0;
			go.update = null;
		}
	}

	go.eat = function( ltr ) {
		log( "Ate ", ltr.letter );
		ltr.parent = null;
	}

	go.discard = function( ltr ) {
		log( "Discarded ", ltr.letter );
		// ltr.parent = null;
		ltr.sink();
	}


})( this );