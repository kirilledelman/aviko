/*

	Inspector UI for debugging and developing for Aviko.
	To activate, long right-click anywhere on scene,
	or call inspector.show( [optional object] );

	usage:
		var inspector = include( 'ui/inspector' );

 */

if ( !UI.style ) include( './ui' );
new (function( params ){

	var logBuffer = "";
	var window, console, output, input, propertyList;
	var target2;
	var history = [];
	var historyPos = 0;

	// properties
	var mappedProps = [

		// (Object) currently inspected target
		[ '$0', function (){ return propertyList.target; }, function ( t ) {
			if ( t != propertyList.target ) {
				propertyList.target = t;
				// target
				if ( t && typeof( t ) === 'object' ) {
					log( "^I$0 =", t );
				}
			}
		}, true ],

		// (Object) secondary target
		[ '$1', function (){ return target2; }, function ( t ) {
			if ( t != target2 ) {
				// target
				if ( target2 && typeof( target2 ) === 'object' ) {
					log( "^I$1 =", t );
				}
				target2 = t;
			}
		}, true ],

	];
	UI.base.addMappedProperties( global, mappedProps );

	// construct UI
	this.window = window = new GameObject( './window', {
		title: "Inspector",
		draggable: true,
		resizable: true,
		x: 0, y: App.windowHeight * 0.5,
		width: App.windowWidth, height: App.windowHeight * 0.5,
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Start,
		layoutAlignY: LayoutAlign.Stretch,
		spacingX: 4,
		fixedPosition: true,
		ignoreCamera: true,
	} );

	// console container
	console = window.addChild( './panel', {
		flex: 1,
		fitChildren: false,
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
		code: global,
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
			log ( "^B> " + input.text );
			history.push( input.text );
			var r = eval( input.text );
			log( "^9" + r );
			input.text = "";
			input.editing = true;
			historyPos = history.length;
		}
	} );

	// property editor
	this.propertyList = propertyList = window.addChild( './property-list', {
		minWidth: 300,
		focusGroup: 'inspector',
		render: new RenderShape( Shape.Rectangle, { color: 0xFFFFFF } ),
	} );

	// events and callbacks

	function onLog( s ) {
		if ( !window.active ) return;
		logBuffer += (logBuffer.length ? "\n^n^c" : "") + s;
		output.text = logBuffer;
		output.async( output.scrollToBottom, 0.1 );
	}

	function onError( message, filename, line, column, linebuf, flags ) {
		var m = "^2^BERROR: " + message;
		if ( filename ) {
			m += "\nin " + filename + ":" + line + "|" + column + ":\n" + linebuf + "\n";
			for ( var i = 0; i < column; i ++ ) { m += " "; }
			m += '^';
		}
		log( m );
	}

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

	// register handlers
	App.on( 'log', onLog );
	App.on( 'error', onError );
	Input.on( 'mouseDown', mouseDown );

})( this );