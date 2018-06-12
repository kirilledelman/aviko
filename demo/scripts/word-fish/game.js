(function(){

	var inOrderMode = true;

	// words - todo - move to an external json file
	var curWord = -1;
	var curLetter = 0;
	var position = 10;
	var word = null;
	var controlEnabled = false;
	var allWords = [
		{ icon: 'poop', word: 'POOP' },
		{ icon: 'clown', word: 'CLOWN' }
	];

	// scene
	var scene = new Scene( {
		name: "Word Fish",
		backgroundColor: 0x001133,
		gridSize: 32,
	} );

	// add all the static components
	var container, wordContainer, game, score, player, wordIcon;
	addComponents();


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
					if ( tween.target != wordIcon ) {
						log( "unparented", tween.target );
						tween.target.parent = null;
					}
				}
			}
		}
		// new word
		curWord = ( curWord + 1 ) % allWords.length;
		word = allWords[ curWord ];

		// when old word is gone
		wordIcon.async( function () {
			// set new word icon
			wordIcon.render.texture = word.icon;
			wordIcon.scale = 1;
			wordIcon.angle = 0;
			wordIcon.x = 320 + wordIcon.render.originalWidth * 0.5;
			wordIcon.y = -wordIcon.render.originalHeight * 0.5;
			// move icon to center of screen
			wordIcon.moveTo( 152, wordIcon.y, 0.5, Ease.Out ).finished = function (){
				// wait a bit
				wordIcon.async( function () {
					// scale down and move to default pos
					wordIcon.scaleTo( 32 / wordIcon.render.originalHeight, 0.7, Ease.InOut );
					wordIcon.moveTo( 32, -32, 0.5, Ease.In ).finished = function () {
						wordIcon.moveTo( 32, 20, 0.5, Ease.Out, Ease.Bounce );
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
						}, 0.4 * i );
					} // end for

					// animation to highlight letter as current
					function makeCurrent() {
						var coverCopy = clone( this.cover );
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
						if ( val ) {
							// TODO - make next letter current
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
						} else {
							// TODO
						}
					}

					// if in order mode, highlight letter
					if ( inOrderMode ) {
						curLetter = 0;
						wordContainer.async( function() {
							letter = wordContainer.getChild( 1 );
							letter.makeCurrent();
						}, 3 );
					}

				}, 1 );
			};
		}, 1.25 );

	}
	scene.nextWord = nextWord;

	// START sequence - player enter, show word
	player.swimHorizontal( 32 );
	player.swimVertical( 64 );
	scene.async( function () {
		nextWord();
		controlEnabled = true;
	}, 1 );

	// adds static components - background, water, sky etc.
	function addComponents() {

		// outer container scaled to fit/center in window
		container = new GameObject( { name: "Container" } );

		// main game container - sprites etc are added here
		game = new GameObject( { name: "Game" } );

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
				text: '00000',
				align: TextAlign.Right,
				font: 'UpheavalPro',
				autoResize: true,
				size: 16,
				width: 60, height: 20,
			}),
			x: 258, y: 12,
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
		} );

		// water
		var water = new GameObject( {
			name: "Water",
			script: './water',
		} );

		// player
		player = new GameObject( {
			script: './player',
			name: "Player",
			x: -64, y: 96,
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

	}

	// process input - move player
	scene.controllerInput = function ( name, val, controller ) {

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
					if ( i < wordContainer.numChildren - 1 ) {
						wordContainer.getChild( i + 1 ).async( function(){ this.makeCurrent(); }, 1 );
					}
					break;
				}
				else h++;
			}

			if ( h >= wordContainer.numChildren - 2 ) {
				scene.async( nextWord, 1 );
			}
		}
	}

	// scale/recenter
	scene.scaleScene = function( w, h ) {
		var sh = w / 640;
		var sv = h / 480;
		container.scale = 2 * Math.min( sh, sv );
		container.x = 0.5 * ( w - 320 * container.scale );
		container.y = 0.5 * ( h - 240 * container.scale );
	}
	return scene;
})();