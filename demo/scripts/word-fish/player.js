(function( go ){

	// animated swimming
	var velX = 0, velY = 0, minVel = 8, maxVel = 64, velEaseDist = 8,
		destX, destY;
	var tex = './textures/fish:Frame00';

	// no frame animation support yet
	var idleAnim = [ 1, 2, 3, 2 ], idleFPS = 4;
	var anim = idleAnim, animFPS = idleFPS;
	var loopAnim = true;
	var seqFrame = 0, prevFrame = -1;

	//
	go.render = new RenderSprite();
	go.render.pivotX = go.render.pivotY = 0.5;

	// absolute Y pos
	go.swimVertical = function ( val ) {
		if ( destX === undefined ) destX = x;
		if ( destY === undefined ) destY = y;
		destY = Math.max( 32, Math.min( 5 * 32 , val ) );
		if ( ( velY < 0 && destY > go.y ) || ( velY > 0 && destY < go.y ) ) velY *= -0.5;
		go.update = swim;
	}

	// absolute X pos
	go.swimHorizontal = function ( val ) {
		if ( destX === undefined ) destX = x;
		if ( destY === undefined ) destY = y;
		destX = Math.max( 32, Math.min( 96, val ) );
		if ( ( velX < 0 && destX > go.x ) || ( velX > 0 && destX < go.x ) ) velX *= -0.5;
		go.update = swim;
	}

	// easing towards destX, destY
	function swim( dt ){
		// frames
		animate( dt );

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
			go.update = animate;
		}
	}

	function animate( dt ) {
		// anim
		seqFrame += animFPS * dt;
		if ( seqFrame >= anim.length ) {
			//seqFrame -= Math.floor( seqFrame );
			seqFrame = 0;
			if ( !loopAnim ) {
				loopAnim = true;
				anim = idleAnim;
				animFPS = idleFPS;
				log( "reverting to idle" );
			}
		}
		var frame = Math.floor( seqFrame );
		if ( frame != prevFrame ) {
			go.render.texture = tex + anim[ frame ];
			prevFrame = frame;
		}
	}

	// eat letter animation
	go.eat = function( ltr ) {
		// eat anim
		anim = [ 4, 5 ];
		animFPS = 2;
		prevFrame = -1;
		seqFrame = 0;
		loopAnim = false;

		// animate crunch
		ltr.update = null;
		ltr.parent = go;
		ltr.setTransform( 20, 0, 0 );
		(new Tween( ltr,
			[ 'scaleX', 'scaleY', 'x', 'stipple' ],
			[ ltr.scaleX, ltr.scaleY, ltr.x, 0 ],
			[ 0.2, 0, 8, 0.5 ],
	        0.5, Ease.Out )).finished =
			function(){
				ltr.parent = null;
			};
		// sound
		go.sfx = new Sound( './sound/eat' + Math.floor(1 + Math.random() * 5) + '.wav' );
		go.sfx.play();
	}

	go.discard = function( ltr ) {
		// bump anim
		anim = [ 6 ];
		animFPS = 0.5;
		seqFrame = 0;
		prevFrame = -1;
		loopAnim = false;

		// sfx
		go.sfx = new Sound( './sound/hit' + Math.floor(1 + Math.random() * 3) + '.wav' );
		go.sfx.play();

		// log( "Discarded ", ltr.letter );
		ltr.sink();
	}


})( this );