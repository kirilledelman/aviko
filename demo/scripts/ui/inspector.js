/*

	Inspector UI for debugging and developing for Aviko.
	To activate, long right-click anywhere on scene,
	or call inspector.show( [optional object] );

	usage:
		var inspector = include( 'ui/inspector' );

 */

include( './ui' );
new (function( params ){

	var logBuffer = "";
	var window, console, output, input, propertyList;
	var target2;
	var history = [];
	var historyPos = 0;

	// properties
	var mappedProps = {

		// (Object) currently inspected target
		'$0': {
			get: function (){ return propertyList.target; },
			set: function ( t ) {
				if ( t != propertyList.target ) {
					propertyList.target = t;
					sceneExplorer.selectNode( t );
					// target
					if ( t && typeof( t ) === 'object' ) {
						log( "$0 =", t );
					}
				}
			}, serialized: false
		},

		// (Object) secondary target
		'$1': {
			get: function (){ return target2; },
			set: function ( t ) {
				if ( t != target2 ) {
					// target
					if ( target2 && typeof( target2 ) === 'object' ) {
						log( "$1 =", t );
					}
					target2 = t;
				}
			}, serialized: false
		},

	};
	UI.base.mapProperties( global, mappedProps );

	// construct UI
	this.window = window = new GameObject( './window', {
		title: "Inspector",
		draggable: true,
		resizable: true,
		minHeight: 120,
		x: 0, y: App.windowHeight * 0.5,
		width: App.windowWidth, height: App.windowHeight * 0.5,
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Stretch,
		spacingX: 1,
		fixedPosition: true,
		ignoreCamera: true,
		active: false,
	} );

	// scene
	var sceneExplorer = this.sceneExplorer = window.addChild( './scrollable', {
		minWidth: 150,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		scrollbars: 'auto',
		pad: 2,
		layout: function ( w, h ) {
			if ( this.verticalScrollbar && this.verticalScrollbar.active ) {
				this.marginRight = this.verticalScrollbar.width + this.verticalScrollbar.marginLeft;
			} else {
				this.marginRight = 0;
			}
			this.scrollWidth = this.width;
			this.container.render.resize( w, h );

		},
		selectNode: function ( n ) {
			var row = this.getChild( 0 );
			if ( row ) {
				if ( this.currentNode ) {
					var prow = row.findRowForNode( this.currentNode );
					if ( prow ) {
						prow.button.label.addColor = prow.button.image.addColor = 0x0;
					}
				}
				row = row.findRowForNode( n );
				if ( row ) row.button.label.addColor = row.button.image.addColor = 0x0066a5;
			}
			this.currentNode = n;
		},
		findRowForNode: function ( n ) {
			var row = this.getChild( 0 );
			if ( row ) return row.findRowForNode( n );
			return null;
		}
	} );
	sceneExplorer.container.render = new RenderShape( { shape: Shape.Rectangle, filled: true, centered: false, color: 0xFFFFFF } );

	// resizer scene <-> console
	var resizer = window.addChild( './resizer', {
		minSize: 100,
		maxSize: 300,
		collapsible: true,
		target: sceneExplorer
	} );

	// console container
	console = window.addChild( './panel', {
		flex: 1,
		fitChildren: false,
		minWidth: 100,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start
	} );

	// console output
	output = console.addChild( './textfield', {
		disabled: true,
		multiLine: true,
		wrap: true,
		flex: 1,
		bold: false,
		lineSpacing: -2,
		size: 12,
		formatting: true,
		marginBottom: 4,
		alwaysShowSelection: true,
		newLinesResetFormatting: true,
		focusGroup: 'inspector',
		states: {
			off: { background: 0xFFFFFF },
			disabled: { background: 0xFFFFFF },
			scrolling: { background: 0xFFFFFF },
			focused: { background: 0xFFFFFF },
		},
		canScrollUnfocused: true
	} );

	// console input
	input = console.addChild( './textfield', {
		name: "ConsoleInput",
		autoGrow: true,
		newLinesRequireShift: true,
		tabEnabled: true,
		lineSpacing: -2,
		autocomplete: UI.base.autocompleteObjectProperty,
		size: 12,
		focusGroup: 'inspector'
	} );
	input.on( 'keyDown', function ( code, shift, ctrl, alt, meta ) {
		// history
		if ( ( ctrl || meta ) && history.length ) {
			if ( code == Key.Up ) {
				historyPos = ( historyPos == history.length ? ( history.length - 1 ) : ( ( historyPos > 0 ? historyPos : history.length ) - 1 ) );
			} else if ( code == Key.Down ) {
				historyPos = ( historyPos + 1 ) % history.length;
			} else return;
			input.text = history[ historyPos ];
			input.caretPosition = input.text.positionLength();
		// accept
		} else if ( code == Key.Enter && !( shift || meta || ctrl || alt ) && input.text.length ) {
			log ( "> " + input.text ); //.replace( /\^/g, '^^' )
			history.push( input.text );
			var r = eval( input.text );
			input.text = "";
			input.editing = true;
			historyPos = history.length;
			if ( r !== null && typeof ( r ) === 'object' && r.constructor.name.indexOf( 'Error' ) > 0 ) throw r;
			log( r );
		}
	} );

	// resizer console <-> properties
	resizer = window.addChild( './resizer', {
		minSize: 280,
		maxSize: 350,
		collapsible: true,
	} );

	// property editor
	resizer.target = this.propertyList = propertyList = window.addChild( './property-list', {
		minWidth: 280,
		focusGroup: 'inspector',
		render: new RenderShape( Shape.Rectangle, { color: 0xFFFFFF } ),
		change: function ( t ) {
			// update selected item in scene browser, if visible
			var row = sceneExplorer.findRowForNode( t );
			if ( row ) row.nodeUpdated();
		},
		targetChanged: function ( t ) {
			// update selected item in scene browser, if visible
			var row = sceneExplorer.findRowForNode( t );
			if ( row ) row.nodeUpdated();
		},
	} );

	// update window minWidth
	window.layout = function ( w, h ) {
		window.minWidth = 20 + console.minWidth +
			Number( sceneExplorer.active ) * sceneExplorer.minWidth +
			Number( propertyList.active ) * propertyList.minWidth;
	}

	// callbacks

	function onLog( s ) {
		// if ( !window.active ) return;
		logBuffer += (logBuffer.length ? "\n" : "") + s;
		output.text = logBuffer;
		output.async( output.scrollToBottom, 0.1 );
	}
	App.on( 'log', onLog );

	function onError( message, filename, line, column, linebuf, flags ) {
		var m = "^2^BERROR: " + message;
		if ( filename ) {
			m += "\nin " + filename + ":" + line + "|" + column + ":\n" + linebuf + "\n";
			for ( var i = 0; i < column; i ++ ) { m += " "; }
			m += '^';
		}
		log( m );
	}
	App.on( 'error', onError );

	// open inspector on long right click, set target to object under mouse
	var mouseDown = function ( btn, x, y ) {
		Input.on( 'mouseUp', function() { cancelDebouncer( 'showInspector' ); }, true );
		if ( btn != 3 ) return;
		// show
		debounce( 'showInspector', function () {
			// if shift isn't pressed
			if ( !Input.get( Key.LeftShift ) ) {
				// set target to object under cursor
				var context = App.scene.query( x, y, 1, 1, true );
				if ( context.length ) $0 = context[ context.length - 1 ];
			}
			// show window
			App.overlay.addChild( window );
			window.active = true;
			input.focus();
		}.bind( this ), 2.0 );
	}.bind( this );
	Input.on( 'mouseDown', mouseDown );

	// update scene on window open
	window.activeChanged = function () {
		if ( window.active ) reloadSceneExplorer();
		else sceneExplorer.removeAllChildren();
	}

	// scene explorer hooks
	App.on( 'sceneChanged', sceneChanged );
	sceneChanged( null, App.scene );

	// watches the scene change
	function sceneChanged( oldScene, newScene ) {
		if ( oldScene ) oldScene.off( 'sceneChanged', sceneChanged );
		if ( newScene ) newScene.on( 'sceneChanged', sceneChanged );
		if ( window.active ) reloadSceneExplorer();
	}

	// repopulates scene explorer
	function reloadSceneExplorer () {
		sceneExplorer.removeAllChildren();
		var sceneRow = SceneExplorerRow( App.scene );
		sceneExplorer.addChild( sceneRow );
	}

	// returns a collapsed row
	function SceneExplorerRow ( node ) {
		// row wrapper
		var go = new GameObject( './panel', {
			node: node,
			layoutType: Layout.Vertical,
			layoutAlignX: LayoutAlign.Stretch,
			layoutAlignY: LayoutAlign.Start,
		} );
		// main button
		go.button = go.addChild( './button', {
			focusable: false,
			fitChildren: false,
			minHeight: 20,
			pad: 2,
			focusRect:false,
			spacing: 4,
			icon: UI.style.propertyList.values.object.icon, // down arrow
			style: {
				label: { size: 12, color: 0x0, align: TextAlign.Left, bold: false, flex: 1 },
				states: {
					off: { label: { color: 0x0 }, background: false },
					over: { background: 0xd9e2e7 },
					down: { background: 0xd9e2e7 },
				},
				image: { angle: -90, ui: { offsetX: 4, offsetY: 9 } },
			},

			click: function ( btn, x, y ) {
				// left click
				if ( btn == 1 ) {
					// if clicked on icon
					if ( x < 20 ) {
						// toggle expand
						go.toggleExpand();
					} else {
						// select in property inspector
						$0 = node;
					}
				// right click
				} else if ( btn == 3 ){
					// popup menu
					// TODO
					// Remove from parent
					// Copy
					// Save object
				}
			}
		});

		// selected
		if ( node == propertyList.target ) {
			go.button.label.addColor = go.button.image.addColor = 0x0066a5;
		}

		// called to update row
		go.nodeUpdated = function () {
			go.button.text = (node.numChildren ? ('(' + node.numChildren + ') ^B') : '^B' ) +
							( node.name.length ? node.name :
								( node.script ? ( '<' + node.script + '>' ) : node.constructor.name ) );
			go.opacity = node.active ? 1 : 0.5;
			go.button.image.opacity = node.numChildren ? 1 : 0.5;
		}
		go.nodeUpdated();

		// expand
		go.toggleExpand = function () {
			// collapsed
			if ( !go.expanded ) {
				if ( !go.container ) {
					// container for child rows
					go.container = go.addChild( {
						ui: new UI( {
							padLeft: 16,
							layoutType: Layout.Vertical,
							layoutAlignX: LayoutAlign.Stretch,
							layoutAlignY: LayoutAlign.Start,
							fitChildren: true,
						} )
					} );
					// repopulate container
					var ch = node.children;
					for ( var i = 0, nc = ch.length; i < nc; i++ ) {
						go.container.addChild( SceneExplorerRow( ch[ i ] ) );
					}
				} else {
					// just update
					for ( var i = 0; i < go.container.numChildren; i++ ) {
						go.container.getChild( i ).nodeUpdated();
					}
				}
				go.button.image.angle = 0;
				go.button.image.ui.offsetX = 0;
				go.button.image.ui.offsetY = 0;
				go.container.active = true;
			} else {
				// collapse
				if ( go.container ) go.container.active = false;
				go.button.image.angle = -90;
				go.button.image.ui.offsetX = 4;
				go.button.image.ui.offsetY = 9;
			}
			// flag
			go.expanded = !go.expanded;
		}

		// this row was removed
		go.removedFromScene = function () {
			node.off( 'removed', go.nodeRemoved );
			node.off( 'childRemoved', go.nodeChildRemoved );
			node.off( 'childAdded', go.nodeChildAdded );
		}

		go.nodeRemoved = function () { go.parent = null; }

		// child row removed
		go.nodeChildRemoved = function ( ch ) {
			go.nodeUpdated();
			if ( go.container ) {
				for ( var i = 0; i < go.container.numChildren; i++ ) {
					if ( go.container.getChild( i ).node == ch ){
						go.container.removeChild( i );
						break;
					}
				}
			}
		}

		// node gained a child
		go.nodeChildAdded = function ( ch, i ) {
			go.nodeUpdated();
			if ( go.container ) {
				var newRow = SceneExplorerRow( ch );
				go.container.addChild( newRow, i );
			}
		}

		// find descendent row, if it exists
		go.findRowForNode = function ( n ) {
			if ( n == node ) return go;
			if ( go.container ) {
				for ( var i = 0; i < go.container.numChildren; i++ ) {
					var r = go.container.getChild( i ).findRowForNode( n );
					if ( r ) return r;
				}
			}
			return null;
		}

		// listeners
		node.on( 'removed', go.nodeRemoved, true );
		node.on( 'childRemoved', go.nodeChildRemoved );
		node.on( 'childAdded', go.nodeChildAdded );

		return go;
	}


})( this );

