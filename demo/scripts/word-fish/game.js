(function(){

	// vars
	var inOrderMode = true;
	var points = 0;
	var curWord = -1;
	var curLetter = 0;
	var word = null;
	var controlEnabled = false, collisionEnabled = false, acceptingEnabled = false;
	var russianAlphabet = ['А','Б','В','Г','Д','Е','Ё','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я' ];
	var englishAlphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'.split( '' );
	var alphabet;
	var container, wordContainer, game, score, player, wordIcon;

	// words - todo - move to an external json file
	var allWords = [
		{ icon: 'clown', word: 'CLOWN' }
	];
	alphabet = englishAlphabet; // russianAlphabet;

	// shuffle
	var j, x, i;
    for (i = allWords.length - 1; i > 0; i--) {
        j = Math.floor( Math.random() * ( i + 1 ) );
        x = allWords[ i ];
        allWords[i] = allWords[j];
        allWords[j] = x;
    }

	// scene
	var scene = new Scene( {
		name: "Word Fish",
		backgroundColor: 0x001133,
		gridSize: 32,
		success: new Sound( './sound/success.wav' ),
		begin: new Sound( './sound/begin.wav' ),
		music: new Sound( './sound/8BitClouds.ogg' ),
	} );

	// add all the static components
	addComponents();

	// spawns letter
	function spawnLetter( numLetters ) {

		// ignore if game is over, or too many already on screen
		if ( App.scene != scene || game.letters.length > 4 ) return;

		// multiple if needed
		if ( !numLetters ) numLetters = 1;
		for ( var i = 0; i < numLetters; i++ ) {

			// pick letter
			var ltr = alphabet[ 0 ];
			var tricky = false; //Math.random() > 0.9 ? ( Math.random() * 0.25 ) : false;
			var useful = 0;

			// check if useful letters are already swimming
			for ( var j = 0; j < game.letters.length; j++ ){
				if ( game.letters[ j ].useful ) { useful++; }
			}
			// allowed letters
			var usefulLetters = [];
			if ( inOrderMode ) usefulLetters.push( word.word[ Math.max( 0, curLetter ) ] );
			else {
				// ones not yet uncovered
				for ( var j = 1; j < wordContainer.numChildren; j++ ){
					var l = wordContainer.getChild( j );
					if ( l.hidden ) usefulLetters.push( l.name );
				}
			}

			// in order?
			if ( inOrderMode ) {
				// only one is needed
				useful = ( useful == 0 );
			// free mode
			} else {
				// can have up to 2 useful
				useful = ( useful < 2 );
			}

			// sometimes, don't pick useful when needed
			if ( Math.random() <= 0.2 || !usefulLetters.length ) useful = false;

			// pick useful letter
			if ( useful ) {

				ltr = usefulLetters[ Math.floor( Math.random() * usefulLetters.length ) ];

			// pick unuseful letter
			} else {

				do {
					ltr = alphabet[ Math.floor( alphabet.length * Math.random() ) ];
				} while ( usefulLetters.indexOf( ltr ) >= 0 );
			}


			// avoid picking same Y position twice in a row
			var lane;
			while ( ( lane = Math.floor( 1 + Math.random() * 5 ) ) == game.lastLane );
			game.lastLane = lane;

			// spawn
			var letter = game.addChild( './letter', {
				x: 310 + Math.round( Math.random() * 2 ) * scene.gridSize,
				y: lane * scene.gridSize,
				lane: lane,
				letter: ltr,
				name: ltr,
				game: game,
				tricky: tricky,
				useful: useful
			}, 2 );
			game.letters.push( letter );
		}
	}
	game.spawnLetter = spawnLetter;

	// collision/etc
	function checkCollision() {
		for ( var i = game.letters.length - 1; i >= 0; i-- ){
			var letter = game.letters[ i ];
			var dx = Math.abs( letter.x - player.x ), dy = Math.abs( letter.y - player.y );
			if ( collisionEnabled && dy < scene.gridSize * 0.5 && letter.x > player.x && dx < scene.gridSize ) {
				playerTouchedLetter( letter );
				continue;
			}
			for ( var j = game.letters.length - 1; j >= 0; j-- ){
				if ( i == j ) continue;
				letter2 = game.letters[ j ];
				dx = Math.abs( letter.x - letter2.x );
				if ( letter.lane == letter2.lane && dx < scene.gridSize ) {
					letterTouchedLetter( letter, letter2 );
				}
			}
		}
		player.debounce( 'checkCollision', checkCollision, 0.25 );
	}

	//
	function playerTouchedLetter( ltr ) {
		// check if it's useful
		var bottomLetter = null;
		for ( var i = 1; i < wordContainer.numChildren; i++ ) {
			var bl = wordContainer.getChild( i );
			if ( bl.hidden && bl.name == ltr.name && ( !inOrderMode || i - 1 == curLetter ) ) {
				bottomLetter = bl;
				break;
			}
		}

		// useful
		if ( bottomLetter && acceptingEnabled ) {
			bottomLetter.accept( true );
			player.eat( ltr );
		// discard
		} else {
			player.discard( ltr );
		}
	}

	function letterTouchedLetter ( a, b ) {
		// sink
		if ( Math.random() > 0.5 ) {
			a.sink();
		// move over
		} else {
			a.bump();
		}
	}

	// called at the beginning of the game, and after each word is completed
	function nextWord() {

		// clean up / animate previous word away
		if ( word ) {

			// letters and icon fall down
			for ( var i = 0; i < wordContainer.numChildren; i++ ) {
				var letter = wordContainer.getChild( i );
				var tw = new Tween( letter,
                    [ 'y', 'angle' ], [ letter.y, 0 ], [ letter.y + 64, Math.random() * 90 - 45 ],
                    1, Ease.In ).finished = function ( tween ) {
					// if not wordIcon, remove on complete
					if ( tween.target != wordIcon ) tween.target.parent = null;
				}
			}

			scene.success.play();
		}

		// pick new word
		curWord = ( curWord + 1 ) % allWords.length;
		word = allWords[ curWord ];
		if ( typeof ( word.word ) == 'string' ) word.word = word.word.split( '' );
		collisionEnabled = false;
		acceptingEnabled = false;

		// when the old word is gone
		wordIcon.async( function () {

			// set new word icon
			wordIcon.render.texture = word.icon;
			wordIcon.scale = 1;
			wordIcon.angle = 0;
			wordIcon.x = 320 + wordIcon.render.originalWidth * 0.5;
			wordIcon.y = -( wordIcon.render.originalHeight * 0.5 + 16 );
			// move icon to center of screen
			wordIcon.moveTo( 152, wordIcon.y, 0.5, Ease.Out ).finished = function (){
				// wait a bit
				wordIcon.async( function () {
					// scale down and move to default pos
					wordIcon.scaleTo( 32 / wordIcon.render.originalHeight, 0.7, Ease.InOut );
					wordIcon.moveTo( 32, -32, 0.5, Ease.In ).finished = function () {
						wordIcon.moveTo( 32, 20, 0.5, Ease.Out, Ease.Bounce );
						collisionEnabled = true;
						scene.begin.play();
					};
					// add letters
					for ( var i = 0; i < word.word.length; i++ ) {
						var letter = wordContainer.addChild( {
							name: word.word[ i ],
							index: i,
							y: 12,
							render: new RenderText( {
								font: 'blogger-sans-bold',
								text: word.word[ i ],
								outlineColor: 0x0,
								outlineRadius: 2,
								outlineOffsetY: 1,
								size: 20,
								antialias: false,
							} ),
							hidden: true,
							makeCurrent: makeCurrent,
							accept: accept
						} );
						letter.render.measure();
						letter.cover = letter.addChild( {
							name: "Cover",
							render: new RenderShape( {
								shape: Shape.RoundedRectangle,
								radius: 4,
								width: 30, height: 30,
								centered: true,
								color: 0x192666,
							} ),
							x: Math.floor( ( letter.render.width - 30 ) * 0.5 + 15 ),
							y: 8,
						} );
						letter.cover.defaultX = letter.cover.x;
						letter.cover.defaultY = letter.cover.y;
						letter.x = 300 + ( 64 + i * 32 ) + ( 6 - letter.cover.x );
						// animate each letters in sequence
						letter.async( function () {
							this.moveBy( -300, 0, 1, Ease.Out, Ease.Bounce );
							if ( this.index == word.word.length - 1 ) {
								acceptingEnabled = true;
							}
						}, 0.4 * i );
					} // end for

					// animation to highlight letter as current
					function makeCurrent() {
						var coverCopy = clone( this.cover );
						coverCopy.setTransform( this.cover.defaultX, this.cover.defaultY, 0 );
						coverCopy.render.color = 0xFFFFFF;
						coverCopy.render.stipple = 1;
						coverCopy.scale = 2;
						coverCopy.z = 1;
						coverCopy.scaleTo( 1, 1, Ease.Out );
						this.addChild( coverCopy );
						(new Tween( coverCopy.render, 'stipple', 1, 0, 1 )).finished = function(){
							this.cover.render.color = coverCopy.render.color;
							coverCopy.parent = null;
						}.bind( this );
					}

					// animation to show, or hide letter again (penalty)
					function accept( val ) {
						// letter is accepted
						if ( val ) {
							// uncover
							this.hidden = false;
							this.render.addColor = [ 0.2, 0.8, 1, 0 ];
							this.cover.render.color = 0xFFFFFF;
							this.cover.rotateTo( ( this.index % 2 ? 1 : -1 ) * 360 * ( 2 + Math.random() * 2 ), 2, Ease.Out );
							this.cover.scaleTo( 0.7, 2, Ease.In );
							this.cover.z = 1;
							var dx = 20 - Math.random() * 40;
							this.cover.moveBy( dx * 0.5, -(32 + 16 * Math.random()), 0.5, Ease.Out ).finished = function (){
								this.cover.render.color.hexTo( '192666', 1 );
								this.cover.moveBy( dx * 0.5, 200, 1, Ease.In ).finished = function (){
									// reset
									this.cover.active = false;
									this.cover.z = 0;
									this.cover.scale = 1;
									this.cover.setTransform( this.cover.defaultX, this.cover.defaultY );
									this.render.addColor.rgbaTo( 0, 0, 0, 2, Ease.Out );
								}.bind( this );
							}.bind( this );
							// next letter
							nextLetter();
						} else {
							// TODO
						}
					}

					// if in order mode, highlight next letter
					if ( inOrderMode ) {
						curLetter = -1;
						wordContainer.async( nextLetter, 0.25 );
					}

				}, 1 );
			};
		}, 1.25 );

	}

	// called after a letter is accepted
	function nextLetter() {

		// count how many letters left
		var left = 0, letter;
		for ( var i = 1; i < wordContainer.numChildren; i++ ) {
			letter = wordContainer.getChild( i );
			if ( letter.hidden ) left++;
		}

		// no more - word complete
		if ( !left || ( inOrderMode && curLetter == word.word.length - 1 ) ) {

			// increase score
			points += 10 * word.word.length;
			score.render.text = ( '000000' + points.toString() ).substr( -6 );
			score.render.color.set( 0.2, 0.8, 1 );
			score.render.color.rgbaTo( 1, 1, 1, 1, 1 );
			score.scale = 1.25;
			score.scaleTo( 1, 0.25, Ease.Out );

			// next word
			scene.async( nextWord, 1 );

		} else if ( inOrderMode ) {

			// next letter
			curLetter++;
			letter = wordContainer.getChild( curLetter + 1 );
			letter.makeCurrent();
		}

	}

	// adds static components - background, water, sky etc.
	function addComponents() {

		// outer container scaled to fit/center in window
		container = new GameObject( { name: "Container" } );

		// main game container - sprites etc are added here
		game = new GameObject( { name: "Game", travelSpeed: 30 } );
		game.letters = [];

		// render target that clips game container
		var renderImage = new Image( 304, 224, game );

		// clips game container using RenderSprite+Image
		var gameContainer = new GameObject( {
			name: "Game Container",
			x: 8, y: 8, render: new RenderSprite( renderImage )
		});

		// game score (upper right)
		score = new GameObject( {
			name: "Score",
			render: RenderText( {
				text: '000000',
				align: TextAlign.Right,
				font: 'UpheavalPro',
				autoResize: true,
				size: 16, pivotX: 0.5, pivotY: 0.5
			}),
			x: 278, y: 17,
		} );

		// assemble
		gameContainer.addChild( game );
		container.addChild( gameContainer );
		container.addChild( score );
		scene.addChild( container );

		// game view
		var sky = new GameObject( {
			name: "Sky",
			script: './sky',
			game: game
		} );

		// water
		var water = new GameObject( {
			name: "Water",
			script: './water',
			game: game
		} );

		// player
		player = new GameObject( {
			script: './player',
			name: "Player",
			x: -64, y: 96,
			removedFromScene: function (){ player.cancelDebouncer( 'checkCollision' ); log( "bye" ); }
		} );

		// word/icon container
		wordContainer = container.addChild( {
			name: "Bottom Overlay",
			y: 180,
			render: new RenderShape( {
				shape: Shape.Rectangle,
				width: 304, height: 44,
				centered: false, color: '00000099',
			} )
		} );

		// word icon
		wordIcon = wordContainer.addChild( {
			name: "Word Icon",
			x: 32, y: 20,
			render: new RenderSprite( {
				pivotX: 0.5, pivotY: 0.5,
			})
		} );

		// set children
		game.children = [ sky, water, player, wordContainer ];
		player.debounce( 'checkCollision', checkCollision );

	}

	// process input - move player
	function processInput( name, val, controller ) {

		if ( !controlEnabled ) return;

		function KeepSwimmingVertically( v ) {
			if ( typeof( v ) === 'undefined' ) v = controller.get( 'vertical' );
			if ( v ) {
				player.swimVertical( Math.round( player.y / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
				scene.debounce( 'keepSwimmingVertically', KeepSwimmingVertically, 0.45 );
			}
		}
		function KeepSwimmingHorizontally( v ) {
			if ( typeof( v ) === 'undefined' ) v = controller.get( 'horizontal' );
			if ( v ) {
				player.swimHorizontal( Math.round( player.x / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
				scene.debounce( 'KeepSwimmingHorizontally', KeepSwimmingHorizontally, 0.3 );
			}
		}
		if ( name == 'vertical' ) {
			if ( val ) {
				scene.debounce( 'keepSwimmingVertically', KeepSwimmingVertically, 0.45 );
			} else {
				scene.cancelDebouncer( 'keepSwimmingVertically' );
			}
			KeepSwimmingVertically( val );
		} else if ( name == 'horizontal' ) {
			if ( val ) {
				scene.debounce( 'KeepSwimmingHorizontally', KeepSwimmingHorizontally, 0.3 );
			} else {
				scene.cancelDebouncer( 'KeepSwimmingHorizontally' );
			}
			KeepSwimmingHorizontally( val );

		// TEST
		} else if ( name == 'accept' && val ) {

			var h = 0;
			for( var i = 1; i < wordContainer.numChildren; i++ ) {
				var l = wordContainer.getChild( i );
				if ( l.hidden ) {
					l.accept( true );
					break;
				}
				else h++;
			}

		// pause
		} else if ( name == 'start' && val ) {

			if ( App.timeScale ) {
				App.timeScale = 0;
				score.render.text = "^6PAUSED";
			} else {
				App.timeScale = 1;
				score.render.text = ('000000' + points.toString()).substr(-6);
			}
		}
	}
	scene.controllerInput = processInput;

	// scale/recenter
	function scaleScene( w, h ) {
		var sh = w / 640;
		var sv = h / 480;
		container.scale = 2 * Math.min( sh, sv );
		container.x = 0.5 * ( w - 320 * container.scale );
		container.y = 0.5 * ( h - 240 * container.scale );
	}
	scene.scaleScene = scaleScene;

	// START sequence - player enter, show word
	player.swimHorizontal( 32 );
	player.swimVertical( 64 );
	scene.async( function () {
		nextWord();
		spawnLetter( 3 );
		controlEnabled = true;
	}, 1 );

	// music
	scene.music.play( 0 );

	return scene;
})();