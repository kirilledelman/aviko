(function(){

	// vars
	var inOrderMode = true;
	var points = 0;
	var curWord = -1;
	var curLetter = 0;
	var word = null;
	var controlEnabled = false, collisionEnabled = false, acceptingEnabled = false;
	var alphabet = [
		[ 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z' ],
		[ 'А','Б','В','Г','Д','Е','Ё','Ж','З','И','Й','К','Л','М','Н','О','П','Р','С','Т','У','Ф','Х','Ц','Ч','Ш','Щ','Ъ','Ы','Ь','Э','Ю','Я' ],
	];
	var container, wordContainer, game, score, player, wordIcon;

	// words - todo - move to an external json file
	var allWords = [
		{ icon: 'clown', words: [ 'CLOWN', 'КЛОУН' ] },
		{ icon: 'bee', words: [ 'BEE', 'ПЧЕЛА' ] },
		{ icon: 'chick', words: [ 'CHICK', 'ЦЫПЛЁНОК' ] },
		{ icon: 'panda', words: [ 'PANDA', 'ПАНДА' ] },
		{ icon: 'pizza', words: [ 'PIZZA', 'ПИЦЦА' ] },
		{ icon: 'poop', words: [ 'POOP', 'КАКАШКА' ] },
		{ icon: 'snake', words: [ 'SNAKE', 'ЗМЕЯ' ] },
		{ icon: 'elephant', words: [ 'ELEPHANT', 'СЛОН' ] },
		{ icon: 'balloon', words: [ 'BALLOON', 'ШАР' ] },
		{ icon: 'cat', words: [ 'CAT', 'КОТ' ] },
		{ icon: 'crab', words: [ 'CRAB', 'КРАБ' ] },
		{ icon: 'fish', words: [ 'FISH', 'РЫБА' ] },
		{ icon: 'gecco', words: [ 'GECCO', 'ГЕККО' ] },
		{ icon: 'robot', words: [ 'ROBOT', 'РОБОТ' ] },
		{ icon: 'key', words: [ 'KEY', 'КЛЮЧ' ] },
		{ icon: 'apple', words: [ 'APPLE', 'ЯБЛОКО' ] },
		{ icon: 'ball', words: [ 'BALL', 'МЯЧ' ] },
		{ icon: 'banana', words: [ 'BANANA', 'БАНАН' ] },
		{ icon: 'bear', words: [ 'BEAR', 'МЕДВЕДЬ' ] },
		{ icon: 'box', words: [ 'BOX', 'КОРОБКА' ] },
		{ icon: 'bread', words: [ 'BREAD', 'ХЛЕБ' ] },
		{ icon: 'car', words: [ 'CAR', 'МАШИНА' ] },
		{ icon: 'cow', words: [ 'COW', 'КОРОВА' ] },
		{ icon: 'dog', words: [ 'DOG', 'СОБАКА' ] },
		{ icon: 'dolphin', words: [ 'DOLPHIN', 'ДЕЛЬФИН' ] },
		{ icon: 'door', words: [ 'DOOR', 'ДВЕРЬ' ] },
		{ icon: 'flag', words: [ 'FLAG', 'ФЛАГ' ] },
		{ icon: 'frog', words: [ 'FROG', 'ЛЯГУШКА' ] },
		{ icon: 'guitar', words: [ 'GUITAR', 'ГИТАРА' ] },
		{ icon: 'hare', words: [ 'RABBIT', 'ЗАЯЦ' ] },
		{ icon: 'house', words: [ 'HOUSE', 'ДОМ' ] },
		{ icon: 'lemon', words: [ 'LEMON', 'ЛИМОН' ] },
		{ icon: 'moon', words: [ 'MOON', 'ЛУНА' ] },
		{ icon: 'mouse', words: [ 'MOUSE', 'МЫШЬ' ] },
		{ icon: 'mushroom', words: [ 'MUSHROOM', 'ГРИБ' ] },
		{ icon: 'pencil', words: [ 'PENCIL', 'КАРАНДАШ' ] },
		{ icon: 'piano', words: [ 'PIANO', 'ПИАНИНО' ] },
		{ icon: 'pine', words: [ 'PINE', 'ЁЛКА' ] },
		{ icon: 'plane', words: [ 'PLANE', 'САМОЛЁТ' ] },
		{ icon: 'rainbow', words: [ 'RAINBOW', 'РАДУГА' ] },
		{ icon: 'rocket', words: [ 'ROCKET', 'РАКЕТА' ] },
		{ icon: 'rooster', words: [ 'ROOSTER', 'ПЕТУХ' ] },
		{ icon: 'snail', words: [ 'SNAIL', 'УЛИТКА' ] },
		{ icon: 'train', words: [ 'TRAIN', 'ПОЕЗД' ] },
		{ icon: 'umbrella', words: [ 'UMBRELLA', 'ЗОНТ' ] },
		{ icon: 'whale', words: [ 'WHALE', 'КИТ' ] },
	];

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
		letter: new Sound( './sound/letter.wav' ),
		begin: new Sound( './sound/begin.wav' ),
		music: new Sound( './sound/8BitClouds.ogg' ),
		language: 1
	} );

	// add all the static components
	addComponents();

	// spawns letter
	// numLetters = -1 to force spawn 1 useful letter
	// 0/undefined = one
	// # num letter
	function spawnLetter( numLetters ) {

		// ignore if game is over, or too many already on screen
		if ( App.scene != scene || ( game.letters.length > 3 && !forceUseful ) ) return;

		// multiple if needed
		var forceUseful = ( numLetters == -1 );
		if ( !numLetters || numLetters <= 0 ) numLetters = 1;
		for ( var i = 0; i < numLetters; i++ ) {

			// pick letter
			var ltr = alphabet[ scene.language ][ 0 ];
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

			// force
			useful = useful || forceUseful;

			// pick useful letter
			if ( useful ) {

				ltr = usefulLetters[ Math.floor( Math.random() * usefulLetters.length ) ];

			// pick unuseful letter
			} else {

				do {
					ltr = alphabet[ scene.language ][ Math.floor( alphabet[ scene.language ].length * Math.random() ) ];
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

	// player touch
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

	// touch
	function letterTouchedLetter ( a, b ) {
		// sink
		if ( b.useful ) {
			a.sink();
		// move over
		} else {
			a.bump();
		}
	}

	// called at the beginning of the game, and after each word is completed
	function nextWord() {
		log( "language =", scene.language );

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

			// clear useful and missed
			for ( var i = 0; i < game.letters.length; i++ ) { game.letters[ i ].useful = game.letters[ i ].missed = false; }

			// word accepted sound
			scene.success.play();
		}

		// pick new word
		curWord = ( curWord + 1 ) % allWords.length;
		word = allWords[ curWord ];
		word.word = word.words[ scene.language ];
		// split into array
		var cc = word.word.charCodeAt( 0 );
		if ( cc > 128 ) { // utf8
			// split by pairs
			var ww = [];
			for ( var i = 0; i < word.word.length; i += 2 ) ww.push( word.word.substr( i, 2 ) );
			word.word = ww;
		} else word.word = word.word.split( '' );
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
			scene.begin.play();
			// move icon to right corner
			wordIcon.moveTo( 300 - wordIcon.render.originalWidth * 0.5, wordIcon.y, 0.5, Ease.Out ).finished = function (){
				// wait a bit
				wordIcon.async( function () {
					// scale down and move to default pos
					wordIcon.scaleTo( Math.min( 32 / wordIcon.render.originalHeight, 32 / wordIcon.render.originalWidth ),
					                  0.7, Ease.InOut );
					wordIcon.moveTo( 18, -32, 0.5, Ease.In ).finished = function () {
						wordIcon.moveTo( 18, 20, 0.5, Ease.Out, Ease.Bounce );
						collisionEnabled = true;
					};
					// add letter cards on bottom
					for ( var i = 0; i < word.word.length; i++ ) {
						var letter = wordContainer.addChild( {
							name: word.word[ i ],
							index: i,
							y: 12,
							render: new RenderText( {
								font: 'expressway', // 'blogger-sans-bold',
								text: word.word[ i ],
								outlineColor: 0x0,
								outlineRadius: 2,
								outlineOffsetY: 1,
								size: 20,
								pivotY: 0.2,
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
						letter.x = 300 + ( 48 + i * 32 ) + ( 6 - letter.cover.x );
						// animate each letters in sequence
						letter.async( function () {
							this.moveBy( -300, 0, 1, Ease.Out, Ease.Bounce );
							if ( this.index == 0 ) {
								acceptingEnabled = true;
								// force one correct letter spawn
								spawnLetter( -1 );
							}
							scene.letter.async( scene.letter.play, 0.5 );
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
			letter.missed = false;
			letter.useful = false;
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
		game.player = player;
		player.debounce( 'checkCollision', checkCollision );

	}

	// process input - move player
	function processInput( name, val, controller ) {

		if ( !controlEnabled ) return;

		function KeepSwimming( v ) {
			if ( typeof( v ) === 'undefined' ) v = controller.get( 'vertical' );
			if ( v ) {
				player.swimVertical( Math.round( player.y / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
				scene.debounce( 'KeepSwimming', KeepSwimming, 0.45 );
			} else {
				v = controller.get( 'horizontal' );
				if ( v ) {
					player.swimVertical( Math.round( player.y / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
					scene.debounce( 'KeepSwimming', KeepSwimming, 0.45 );
				}
			}
		}

		//function KeepSwimmingVertically( v ) {
		//	if ( typeof( v ) === 'undefined' ) v = controller.get( 'vertical' );
		//	if ( v ) {
		//		player.swimVertical( Math.round( player.y / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
		//		scene.debounce( 'keepSwimmingVertically', KeepSwimmingVertically, 0.45 );
		//	}
		//}
		//function KeepSwimmingHorizontally( v ) {
		//	if ( typeof( v ) === 'undefined' ) v = controller.get( 'horizontal' );
		//	if ( v ) {
		//		player.swimHorizontal( Math.round( player.x / scene.gridSize ) * scene.gridSize + scene.gridSize * v );
		//		scene.debounce( 'KeepSwimmingHorizontally', KeepSwimmingHorizontally, 0.3 );
		//	}
		//}

		if ( name == 'vertical' || name == 'horizontal' ) {

			if ( val ) {
				scene.debounce( 'KeepSwimming', KeepSwimming, 0.45 );
			} else {
				scene.cancelDebouncer( 'KeepSwimming' );
			}
			KeepSwimming( val );

		} if ( name == 'accept' || name == 'cancel' ) {

			if ( val ) {
				player.swimVertical( Math.round( player.y / scene.gridSize ) * scene.gridSize + scene.gridSize * ( name == 'accept' ? -1 : 1 ) );
			}

		} else if ( name == 'start' && val ) {

			// pause
			if ( App.timeScale ) {
				game.parent.render.stipple = 0.5;
				App.scene.music.pause();
				App.timeScale = 0;
				score.render.text = "^6PAUSED";
			} else {
				game.parent.render.stipple = 0;
				App.scene.music.play();
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
