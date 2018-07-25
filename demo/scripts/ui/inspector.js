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
				sceneExplorer.async( function (){ this.selectNode( t ); }, 0.5 );
				if ( t != propertyList.target ) {
					propertyList.target = t;
					// target
					if ( t && typeof( t ) === 'object' ) {
						log( "$0 =", t );
					}
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

	// scene explorer
	var sceneExplorer = this.sceneExplorer = window.addChild( './scrollable', {
		minWidth: 150,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
		scrollbars: 'auto',
		pad: 2,

		// scene explorer layout
		layout: function ( w, h ) {
			// auto fit scroll bar
			if ( this.verticalScrollbar && this.verticalScrollbar.active ) {
				this.marginRight = this.verticalScrollbar.width + this.verticalScrollbar.marginLeft;
			} else {
				this.marginRight = 0;
			}
			// auto resize
			this.scrollWidth = this.width;
			this.container.render.resize( w, Math.max( h, this.scrollHeight ) );

		},

		// select node in tree
		selectNode: function selectNode ( node ) {
			// clear selection
			var row = this.getChild( 0 );
			if ( !row || node == this.currentNode ) return;
			row.traverse( function( prow ) {
				prow.button.label.color = 0x0;
				prow.button.render.active = false;
			} );
			this.currentNode = null;

			// make sure it's gameobject
			if ( !node || !(node.constructor == GameObject || node.constructor == Scene ) ) return;

			// highlight
			highlightGameObject( node );

			// find the chain of node parents
			var chain = [], n = node;
			while ( n ) { chain.push( n ); n = n.parent; }

			// expand up to this level
			for ( var i = chain.length - 1; i > 0; i-- ){
				// expand
				if ( !row.expanded ) row.toggleExpand();
				// find child row
				var curNode = chain[ i ];
				var curSub = chain[ i - 1 ];
				var ni = curNode.children.indexOf( curSub );
				if ( ni < 0 ) { reloadSceneExplorer(); return; } // fail
				row = row.container.getChild( ni );
				if ( !row || row.node != curSub ) { reloadSceneExplorer(); return; } // fail
			}

			// highlight
			if ( row ) {
				row.button.label.color = 0x0066a5;
				row.button.render.active = true;
				row.button.async( function() { UI.base._scrollIntoView.apply( row.button ); }, 0.5 );
			}
			this.currentNode = node;
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
		focusGroup: 'inspector',
		keyDown: function ( code, shift, ctrl, alt, meta ) {
			// history
			if ( ( ctrl || meta ) && history.length ) {
				if ( code == Key.Up ) {
					historyPos = ( historyPos == history.length ? ( history.length - 1 ) : ( ( historyPos > 0 ? historyPos : history.length ) - 1 ) );
				} else if ( code == Key.Down ) {
					historyPos = ( historyPos + 1 ) % history.length;
				} else return;
				input.text = history[ historyPos ];
				input.async( function(){ this.caretPosition = this.text.positionLength() }, 0.1 );
			// accept
			} else if ( code == Key.Enter && !( shift || meta || ctrl || alt ) && input.text.length ) {
				log ( "> " + input.text );
				history.push( input.text );
				var r = eval( input.text );
				input.text = "";
				input.editing = true;
				historyPos = history.length;
				if ( r !== null && typeof ( r ) === 'object' && r.constructor.name.indexOf( 'Error' ) > 0 ) throw r;
				log( r );
			}
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
			sceneExplorer.selectNode( t );
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
		logBuffer += (logBuffer.length ? "\n^c^n" : "") + s;
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
			// show window
			App.overlay.addChild( window );
			window.active = true;
			input.focus();

			// check objects under cursor
			var context = App.scene.query( x, y, 1, 1, true );
			if ( context.length ) {
				if ( context.length == 1 ) {
					$0 = context[ context.length - 1 ];
				} else {
					var items = [];
					for ( var i = context.length - 1; i >= 0; i-- ) {
						var node = context[ i ];
						var scriptName = '';
						var name = node.name ? node.name : node.constructor.name;
						if ( node.script ){
							scriptName = node.script.split( '/' );
							scriptName = ' ^b<' + scriptName[ scriptName.length - 1 ].replace( /^(.+)\.([^\.]+)$/, "$1" ) + '>';
						}
						items.push( { text: name + scriptName, node: node } );
					}
					var popup = new GameObject( './popup-menu', {
						x: x, y: y,
						items: items,
						selected: function ( s ) { $0 = s.node; },
					} );

				}
			}

		}.bind( this ), 1.0 );

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
		sceneExplorer.currentNode = null;
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
		go.button = go.addChild( {
			render: new RenderShape( {
				shape: Shape.Rectangle,
				centered: false,
				filled: true,
				color: 0xd9e2e7,
				active: false
			} )
		} );
		go.button.ui = new UI( {
			fitChildren: false,
			minHeight: 20,
			pad: 2,
			spacing: 4,
			layoutType: Layout.Horizontal,
			layoutAlignX: LayoutAlign.Stretch,
			layoutAlignY: LayoutAlign.Center,
			click: rowButtonClick.bind( go.button )
		});
		go.button.image = go.button.addChild( './image', {
			texture: UI.style.propertyList.values.object.icon,
			rotation: -90,
		} );
		go.button.label = go.button.addChild( './text', {
			size: 12, color: 0x0, align: TextAlign.Left, bold: false, flex: 1,
			autoSize: true,
		} );

		// selected
		if ( node == propertyList.target ) {
			go.button.label.color = 0x0066a5;
			go.button.render.active = true;
		}

		// called to update row
		go.nodeUpdated = function () {
			var numKids = (node.numChildren ? ('(' + node.numChildren + ') ^B') : '^B' );
			var scriptName = '';
			var name = node.name ? node.name : node.constructor.name;
			if ( node.script ){
				var scriptName = node.script.split( '/' );
				scriptName = ' ^b<' + scriptName[ scriptName.length - 1 ].replace( /^(.+)\.([^\.]+)$/, "$1" ) + '>';
			}

			go.button.label.text = numKids + name + scriptName;
			go.opacity = node.active ? 1 : 0.5;
			go.button.image.opacity = node.numChildren ? 1 : 0.5;
		}
		go.nodeUpdated();

		// expand
		go.toggleExpand = function () {
			// collapsed
			if ( !go.expanded ) {
				var highlight = UI.highlight;
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
						if ( ch[ i ] !== highlight ) go.container.addChild( SceneExplorerRow( ch[ i ] ) );
					}
				} else {
					// just update
					for ( var i = 0; i < go.container.numChildren; i++ ) {
						go.container.getChild( i ).nodeUpdated();
					}
				}
				go.button.image.rotation = 0;
				go.container.active = true;
			} else {
				// collapse
				if ( go.container ) go.container.active = false;
				go.button.image.rotation = -90;
			}
			// flag
			go.expanded = !go.expanded;
		}

		// this row was removed
		go.removedFromScene = function () {
			node.off( 'removed', go.nodeRemoved );
			node.off( 'childRemoved', go.nodeChildRemoved );
			node.off( 'childAdded', go.nodeChildAdded );
			node.off( 'activeChanged', go.nodeUpdated );
			node.off( 'nameChanged', go.nodeUpdated );
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
			if ( ch === UI.highlight ) return;
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
		
		// call fe( row ) on each row/subrow
		go.traverse = function ( fe ) {
			fe( go );
			if ( go.container ) {
				for ( var i = 0, nc = go.container.numChildren; i < nc; i++ ) {
					go.container.getChild( i ).traverse( fe );
				}
			}
		}

		// listeners
		node.on( 'removed', go.nodeRemoved, true );
		node.on( 'childRemoved', go.nodeChildRemoved );
		node.on( 'childAdded', go.nodeChildAdded );
		node.on( 'activeChanged', go.nodeUpdated );
		node.on( 'nameChanged', go.nodeUpdated );
		return go;
	}

	function rowButtonClick( btn, x, y ) {
		// left click
		if ( btn == 1 ) {
			// if clicked on icon
			if ( x < 20 ) {
				// toggle expand
				this.parent.toggleExpand();
			} else {
				// select in property inspector
				$0 = this.parent.node;
			}
		// right click
		} else if ( btn == 3 ) {
			var items = [
				{ text: "Copy", action: function () { UI.copiedValue = { type: 'object', value: this.node }; } },
			];
			// gameobject in copiedValue
			if ( UI.copiedValue && UI.copiedValue.type === 'object' && UI.copiedValue.value.constructor === GameObject && UI.copiedValue.value != this.parent.node ) {
				// if not already a child of this node
				if ( UI.copiedValue.value.parent != this.node )
					items.push( { text: "Paste child", action: function () { this.node.addChild( UI.copiedValue.value ); } } );
				// can paste copy
				items.push( { text: "Paste clone", action: function () {
					var cp = clone( UI.copiedValue.value );
					if ( cp ) this.node.addChild( cp );
				} } );
			}
			// not scene
			if ( this.parent.node.parent ) {
				items.push( null );
				items.push( {
					text: "Duplicate", action: function () {
						var cp = clone( this.node );
						this.node.parent.addChild( cp );
					}
				} );
				items.push( { text: "Unparent", action: function () { this.node.parent = null; } } );
				items.push( null );
			}
			items.push( { text: "Add child", action: function () {
				this.node.addChild( new GameObject() );
				if ( !this.expanded ) this.toggleExpand();
			} } );
			// popup menu
			var popup = new GameObject( './popup-menu', {
				target: this,
				items: items,
				selectedIndex: -1,
				selected: function ( s ) { s.action.call( this ); }.bind( this.parent ),
			} );
		}
	}

	// creates an effect that highlights go's location
	function highlightGameObject( go ) {
		if ( !go || go.constructor != GameObject || !go.parent ) return;
		// attach highlight to scene
		var highlight = new GameObject( {
			x: go.worldX, y: go.worldY,
			angle: go.worldAngle,
			scaleX: go.worldScaleX, scaleY: go.worldScaleY } );

		// show location/UI bounds
		var bounds = highlight.addChild(
			new GameObject( {
				render: new RenderShape( {
					shape: Shape.Rectangle,
					filled: false,
					centered: true,
					color: 0x1a92dc,
					lineThickness: 2,
					width: 2, height: 2,
				} )
			} ) );
		if ( go.ui ) {
			bounds.render.resize( Math.max( 2, go.ui.width ), Math.max( 2, go.ui.height ) );
			bounds.setTransform( bounds.render.width * 0.5, bounds.render.height * 0.5 );
		}
		bounds.scaleX = ( bounds.render.width + 40 ) / bounds.render.width;
		bounds.scaleY = ( bounds.render.height + 40 ) / bounds.render.height;
		bounds.scaleTo( 1, 0.5, Ease.Out );

		// flash render
		if ( go.render ) {
			var ac = go.render.addColor.hex;
			go.render.addColor = 'FF000000';
			go.render.addColor.hexTo( ac, 1 );
		}

		// remove after 2 sec
		highlight.async( function () { highlight.parent = null; }, 2 );

		// show
		App.scene.addChild( highlight, -1 );
	}

})( this );

