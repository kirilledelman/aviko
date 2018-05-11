/*

	Control for editing object's properties as a list

	Usage:

		var p = App.scene.addChild( 'ui/property-list' );
		p.target = myObject;

	This component is configured mainly via .properties, .groups, and .showAll properties

	When target is set, the properties displayed are a combination of:
		target.constructor.__propertyListConfig object, if exists, combined+overridden with
		target.__propertyListConfig object, if exists, combined+overridden with
		propertyList's own .properties, .groups, and .showAll values
		where __propertyListConfig is object with all optional fields {
			properties: (Object)
			groups: (Array),
			showAll: (Boolean),
			inspector: (Function) - function that returns a new GameObject with UI to use as custom
			            inspector for this object. Function's params are
			            (thisPropertyList, target, properties, groups, showAll)
			}


	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'change' - a property has changed - callback( targetObject, propertyName, newValue, oldValue )
		TODO - add "final" param to 'change' event to facilitate undo system

*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI();
	var scrollable, shouldScroll = true;
	var target = null;
	var showAll = undefined;
	var properties = {};
	var valueWidth = 130;
	var disabled = false;
	var topPropertyList = go;
	var groups = [];
	var allFields = [];
	// var updateInterval = 0;
	var constructing = true;
	var pad = [ 0, 0, 0, 0 ];
	var spacingX = 0, spacingY = 0;
	var customInspector, inspector;
	go.serializeMask = { 'ui':1, 'target':1, 'children': 1 };

	// API properties
	var mappedProps = [

		// (Boolean) - if true, all enumerable properties of object will be displayed,
		// if false, only ones in .properties
		[ 'showAll',  function (){ return showAll; }, function ( v ){
			showAll = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Object) in form of { 'propertyName': PROPERTY_DEF, 'propertyName2': PROPERTY_DEF ... }
		//      PROPERTY_DEF is either
		//      (Boolean)   true - show with auto settings
		//                  false - do not show
		//      or
		//      (Object) with (all optional) properties:
		//          min: (Number) mininim numeric value,
		//          max: (Number) maximim numeric value,
		//          step: (Number) step for numeric text box,
		//          integer: (Boolean) if numeric - only allows integers
		//          multiLine: (Boolean) if string - allow multi-line text input
		//          readOnly: (Boolean) force read only for this field
		//          enum: [ { icon:"icon1", text:"text1", value:value1 }, { text:text2, value:value2 }, ... ] - shows select dropdown menu
		//          label: (String) "text displayed for property instead of property name", or (Function) to transform propname to text
		//          style: (Object) - apply properties in this object to resulting input field
		//          hidden: (Function) - return true to conditionally hide this field. Function's only param is target
		//
		//      for inline object editing, when an embedded property-list will be displayed, can override defaults with:
		//          properties: (Object) - apply properties to sub-property-list
		//          groups: (Object) - apply groups to sub-property-list
		//          showAll: (Boolean) - apply showAll param to sub-property-list
		//
		[ 'properties',  function (){ return properties; }, function ( v ){
			properties = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Array) in form of [ { name: "Group name", properties: [ 'p1', 'p2', ... ] } ... ] to group and order properties
		//      Properties not listed in groups will appear in an automatically generated group after listed groups, alphabetically
		//      To override alphabetical order of default group, supply group without name: param
		[ 'groups',  function (){ return groups; }, function ( v ){
			groups = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : [];
			go.debounce( 'refresh', go.refresh );
		}],

		// (Object) target object whose properties are displayed in this property list
		[ 'target',  function (){ return target; }, function ( v ){
			target = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) width of value fields
		[ 'valueWidth',  function (){ return valueWidth; }, function ( v ) {
			valueWidth = Math.max( 45, v );
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) automatically refresh displayed properties every updateInterval seconds
		/* [ 'updateInterval',  function (){ return updateInterval; }, function ( v ) {
			updateInterval = v;
			if ( updateInterval > 0 ) {
				go.debounce( 'reload', go.reload, updateInterval );
			} else {
				go.clearDebouncer( 'reload' );
			}
		}],*/

		// (Boolean) disable all fields
		[ 'disabled',  function (){ return disabled; }, function ( v ) {
			ui.disabled = disabled = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Boolean) show add and remove property (and array items) buttons
		[ 'addRemoveProperties',  function (){ return 'TODO'; }, function ( v ){
			//TODO
		}],

		// (Boolean) should this property list be scrollable
		[ 'scrollable',  function (){ return shouldScroll; }, function ( v ){
			shouldScroll = v;
			go.debounce( 'refresh', go.refresh );
		} ],

		// (GameObject) container to which all fields are added
		[ 'container',  function (){ return (scrollable || go); } ],

		// (GameObject) when displaying nested property list editors, this holds reference to the topmost one
		[ 'topPropertyList',  function (){ return topPropertyList; }, function ( v ){ topPropertyList = v; } ],

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		[ 'pad',  function (){ return pad; }, function ( v ){ pad = v; ui.requestLayout( 'pad' ); } ],

		// (Number) inner padding top
		[ 'padTop', function (){ return pad[ 0 ]; }, function ( v ){ pad[ 0 ] = v; ui.requestLayout( 'padTop' ); }, true ],

		// (Number) inner padding right
		[ 'padRight', function (){ return pad[ 1 ]; }, function ( v ){ pad[ 1 ] = v; ui.requestLayout( 'padRight' ); }, true ],

		// (Number) inner padding bottom
		[ 'padBottom', function (){ return pad[ 2 ]; }, function ( v ){ pad[ 2 ] = v; ui.requestLayout( 'padBottom' ); }, true ],

		// (Number) inner padding left
		[ 'padLeft', function (){ return pad[ 3 ]; }, function ( v ){ pad[ 3 ] = v; ui.requestLayout( 'padLeft' ); }, true ],

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		[ 'spacing',  function (){ return Math.max( spacingX, spacingY ); }, function ( v ){ spacingX = spacingY = v; ui.requestLayout( 'spacing' ); }, true ],

		// (Number) spacing between label and value
		[ 'spacingX',  function (){ return spacingX; }, function ( v ){ spacingX = v; ui.requestLayout( 'spacingX' ); } ],

		// (Number) spacing between rows
		[ 'spacingY',  function (){ return spacingY; }, function ( v ){ spacingY = v; ui.requestLayout( 'spacingY' ); } ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "PropertyList";

	// UI
	ui.focusable = false;
	go.ui = ui;

	ui.layout = function ( w, h ) {
		if ( scrollable ) {
			// make scrollable and its scrollbar fit in width
			var vsb = scrollable.verticalScrollbar;
			if ( vsb && vsb.active ) {
				scrollable.marginRight =  vsb.width + vsb.marginLeft;
			} else scrollable.marginRight = 0;
			scrollable.scrollWidth = w - scrollable.marginRight;
			scrollable.resize( scrollable.scrollWidth, h );
			scrollable.spacingX = spacingX;
			scrollable.spacingY = spacingY;
			scrollable.pad = pad;
			ui.pad = 0;
		} else {
			ui.spacingX = spacingX;
			ui.spacingY = spacingY;
			ui.pad = pad;
		}
		if ( go.render ) go.render.resize( w, h );
		go.fire( 'layout' );
	}

	// recreates controls
	go.refresh = function () {
		log( "refresh" );
		// set up container
		var cont = go;
		if ( shouldScroll ) {
			if ( !scrollable ) {
				scrollable = go.addChild( './scrollable', {
					layoutType: Layout.Horizontal,
					layoutAlignX: LayoutAlign.Start,
					layoutAlignY: LayoutAlign.Start,
					marginRight: 0,
					wrapEnabled: true,
					wrapAfter: 2,
					acceptToCycle: true,
					fitChildren: true,
				} );
			}
			cont = scrollable;
			ui.layoutType = Layout.Anchors;
			scrollable.scrollbars = false;

		} else {
			if ( scrollable ) scrollable = null;
			ui.layoutType = Layout.Horizontal;
			ui.layoutAlignX = LayoutAlign.Start;
			ui.layoutAlignY = LayoutAlign.Start;
			ui.wrapEnabled = true;
			ui.wrapAfter = 2;
			ui.fitChildren = true;
		}

		// remove previous elements
		cont.removeAllChildren();
		cont.ui.height = cont.ui.minHeight = 0;
		for ( var i in allFields ) {
			var trg = allFields[ i ].target;
			if ( trg ) trg.unwatch( allFields[ i ].name );
		}
		allFields.length = 0;

		// configure / merge properties with config in
		// target.constructor.__propertyListConfig and target.__propertyListConfig
		var _properties = {};
		var _showAll = undefined;
		var _groups = [];
		var _customInspector = null;
		function mergeConfig( other ) {
			// merge properties
			if ( typeof( other.properties ) === 'object' ) {
				for ( var a in other.properties ) {
					if ( _properties[ a ] === undefined || (_properties[ a ] !== undefined && other.properties[ a ] !== true ) ) {
						_properties[ a ] = other.properties[ a ];
					}
				}
			}
			// merge groups
			if ( typeof( other.groups ) === 'object' ) {
				for ( var i = 0; i < other.groups.length; i++ ) {
					var g = other.groups[ i ];
					var found = false;
					for ( var j = 0; j < _groups.length; j++ ) {
						if ( _groups[ j ].name === g.name ) {
							_groups[ j ] = g;
							found = true;
							break;
						}
					}
					if ( !found ) _groups.push( g );
				}
			}
			// showAll
			if ( typeof ( other.showAll ) === 'boolean' ) _showAll = other.showAll;
			// custom inspector
			if ( typeof ( other.inspector ) === 'function' ) _customInspector = other.inspector;
		}

		if ( target ) {
			// merge constructor's __propertyListConfig
			if ( target.constructor && target.constructor.__propertyListConfig ) {
				mergeConfig( target.constructor.__propertyListConfig );
			}
			// merge object's __propertyListConfig
			if ( target.__propertyListConfig ) {
				mergeConfig( target.constructor.__propertyListConfig );
			}
			// merge current properties etc
			mergeConfig( { properties: properties, groups: groups, showAll: showAll, inspector: customInspector } );

			// if properties are empty, set showall to true
			if ( !Object.keys( _properties ).length && _showAll === undefined ) _showAll = true;
		}

		// if displaying a custom inspector
		if ( _customInspector ) {
			// initialize it
			inspector = _customInspector( go, target, _properties, _groups, _showAll );
			cont.addChild( inspector );
			return;
		}

		// displaying standard inspector

		// empty target
		if ( !target ) {
			cont.addChild( './text', {
				selfAlign: LayoutAlign.Stretch,
				text: "(null)",
				style: go.baseStyle.empty,
			} );
			return;
		}

		// sort properties into groups
		var regroup = { ' ': [] }; // default (unsorted) group
		var unsortedGroup = null; // if unnamed group was supplied it will be put here
		var mappedProps = {};

		// copy each specified group into regroup
		var _gs = [];
		for ( var i in _groups ) {
			var g = _groups[ i ];
			if ( g.name ) {
				_gs.push( g );
			} else {
				unsortedGroup = g;
				continue;
			}
			var props = regroup[ g.name ] = [];
			// copy properties that exist in target and match showing criteria
			for( var i in g.properties ) {
				var pname = g.properties[ i ];
				var pdef = properties[ pname ];
				if ( ( target[ pname ] !== undefined && // target has property
					( ( _showAll === true && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
					( _showAll !== true && pdef !== undefined && pdef !== false ) ) ) || // showing select properties, and prop is included
					( pdef && pdef.target ) ) { // or has overridden target
						props.push( pname );
						mappedProps[ pname ] = g.name;
				}
 			}
		}
		// for each property name in target object
		for ( var p in target ) {
			var pdef = _properties[ p ];
			if ( ( _showAll === true && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
				( _showAll !== true && pdef !== undefined && pdef !== false ) ) { // showing select properties, and prop is included
				// property not in any groups
				if ( mappedProps[ p ] === undefined ) {
					// put in default group
					regroup[ ' ' ].push( p );
				}
			}
		}

		// if order of ungrouped properties isn't provided, sort default group by name
		if ( !unsortedGroup ) {
			regroup[ ' ' ].sort();
		}

		// for each group
		var field = null;
		var numRows = 0, numGroups = 0;
		for ( var i = 0, ng = _gs.length; i <= ng; i++ ) {
			var props = i < ng ? regroup[ _gs[ i ].name ] : regroup[ ' ' ];
			if ( props === undefined || !props.length ) continue;
			numGroups++;
			if ( i < ng || numGroups > 1 ) {
				// add group title
				var groupTitle = cont.addChild( './text', {
					forceWrap: true,
					flex: 1,
					text: (i < ng ? _gs[ i ].name : ''),
					style: go.baseStyle.group,
				} );
				// clear top margin if first
				if ( i == 0 ) groupTitle.marginTop = 0;
			}
			// for each property
			var propertyInfo;
			for ( var j = 0, np = props.length; j < np; j++ ) {
				var pname = props[ j ];
				var pdef = _properties[ pname ] || {};

				// add label
				var labelText = pname;
				if ( typeof( pdef.label ) === 'function' ) labelText = pdef.label( pname );
				else if ( typeof ( pdef.label ) === 'string' ) labelText = pdef.label;
				var label = cont.addChild( './text', {
					text: labelText,
					flex: 1,
					wrap: true,
					style: go.baseStyle.label
				} );

				// value/type
				var curTarget = ( pdef.target || target );
				var fieldValue = curTarget[ pname ];
				var fieldType = typeof( fieldValue );
				var button = null;

				// check for enumeration
				if ( pdef.enum !== undefined ) fieldType = 'enum';

				// get property info
				propertyInfo = Object.getOwnPropertyDescriptor( curTarget, pname );

				// create appropriate control
				switch ( fieldType ) {

					// input fields:

					case 'number':
						field = cont.addChild( './textfield', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							numeric: true,
							minWidth: valueWidth,
							integer: ( pdef && pdef.integer !== undefined ) ? pdef.integer : false,
							min: ( pdef && pdef.min !== undefined ) ? pdef.min : -Infinity,
							max: ( pdef && pdef.max !== undefined ) ? pdef.max : Infinity,
							step: ( pdef && pdef.step !== undefined ) ? pdef.step : 1,
							value: fieldValue,
							style: go.baseStyle.values.any
						} );
						field.style = go.baseStyle.values.number;
						break;

					case 'string':
						field = cont.addChild( './textfield', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							minWidth: valueWidth,
							value: fieldValue,
							style: go.baseStyle.values.any
						} );
						field.style = go.baseStyle.values.string;
						break;

					// dropdown:

					case 'enum':
						field = cont.addChild( './select', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							minWidth: valueWidth,
							value: fieldValue,
							items: pdef.enum,
							style: go.baseStyle.values.any
						} );
						field.style = go.baseStyle.values.enum;
						break;

					// check box:

					case 'boolean':
						field = cont.addChild( './checkbox', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							checked: fieldValue,
							text: fieldValue ? "True" : "False",
							minWidth: valueWidth,
							style: go.baseStyle.values.any
						} );
						field.style = go.baseStyle.values.boolean;
						break;

					// inspector:

					case 'object':

						// button to show inspector
						function togglePropList() {
							this.toggleState = this.propList.active = !this.propList.active;
							this.image.imageObject.angle = (this.propList.active ? 0 : -90);
						}
						function pushToTarget() {
							// TODO - push pop
							go.target = this.fieldValue;
						}
						var inline = ( pdef && pdef.inline );
						button = cont.addChild( './button', {
							fieldValue: fieldValue,
							name: pname,
							wrapEnabled: false,
							minWidth: valueWidth,
							disabled: (disabled || ( pdef && pdef.disabled )),
							target: curTarget,
							style: go.baseStyle.values.any,
							toggleState: !!pdef.expanded
						} );
						button.style = go.baseStyle.values.object;
						if ( inline ) {
							button.click = togglePropList;
							button.text = String( (fieldValue && fieldValue.constructor) ? fieldValue.constructor.name : String(fieldValue) ) + '...',
							// embedded inspector
							field = cont.addChild( './property-list', {
								name: pname,
								flex: 1,
								scrollable: false,
								forceWrap: true,
								active: !!pdef.expanded,
								style: go.baseStyle.values.any,
								fieldButton: button,
							} );
							field.style = go.baseStyle.values.inline;
							field.valueWidth = valueWidth - field.marginLeft; // indent
							if ( pdef.showAll !== undefined ) field.showAll = pdef.showAll;
							if ( pdef.properties !== undefined ) field.properties = pdef.properties;
							if ( pdef.groups !== undefined ) field.groups = pdef.groups;
							field.target = fieldValue;
							button.image.imageObject.angle = (field.active ? 0 : -90);
							button.propList = field;
						} else {
							button.text = String( fieldValue );
							button.click = pushToTarget;
							button.image.imageObject.angle = -90;
							button.label.flex = 1;
							button.reversed = true;
							field = button;
						}

						// button

						break;
					default:
						field = cont.addChild( './text', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							minWidth: valueWidth,
							wrap: true,
							text: fieldValue ? fieldValue.toString() : '',
							style: go.baseStyle.values.any
						} );
						break;

				}
				// common properties
				if ( field ) {
					field.type = typeof( fieldValue );
					field.pdef = pdef;
					field.fieldLabel = label;
					if ( disabled || ( pdef && pdef.disabled ) || ( propertyInfo && !propertyInfo.writable ) ) field.disabled = true;
					if ( typeof( pdef.style ) === 'object' ) {
						UI.base.applyProperties( field, pdef.style );
					}
					if ( typeof( pdef.hidden ) === 'function' ) {
						var shown = !pdef.hidden( curTarget );
						field.fieldLabel.active = shown;
						if ( field.fieldButton ) {
							field.fieldButton.active = shown;
							field.active = (shown && field.fieldButton.toggleState );
						} else {
							field.active = shown;
						}
					}
					allFields.push( field );
					curTarget.watch( field.name, debounceReload );
				}

				numRows++;

			}

		}

		// placeholder
		if ( numRows == 0 ) {
			cont.addChild( './text', {
				selfAlign: LayoutAlign.Stretch,
				text: "no editable properties",
				style: go.baseStyle.empty,
			} );
		}

		// scroll to top, enable scrollbars
		if ( scrollable ) {
			scrollable.scrollLeft = scrollable.scrollTop = 0;
			function _showScrollbars() { this.scrollbars = 'auto'; }
			scrollable.debounce( 'showScrollbars', _showScrollbars, 0.1 );
		}

	}

	function debounceReload( p, ov, v ) {
		go.debounce( 'reload', go.reload, 0.5 );
		return v;
	}

	go.fieldChanged = function ( val ) {

		log( "field changed", this.name, val );

		// type changed
		if ( this.type != typeof( val ) ) {
			log( "field changed, type changed: ", this.type, typeof( val ) );
			go.async( go.refresh );
			return;
		}

		// update label
		if ( typeof ( this.target[ this.name ] ) === 'boolean' ) {
			this.text = ( val ? "True" : "False" );
		}

		// apply
		var oldVal = this.target[ this.name ];
		this.target[ this.name ] = val;

		// fire changed
		go.fire( 'change', this.target, this.name, val, oldVal );
		// go.debounce( 'reload', go.reload );

	}

	// refreshes properties values in rows from target
	go.reload = function () {

		if ( !target ) return;

		// update fields
		for ( var i in allFields ) {
			var field = allFields[ i ];
			var pdef = field.pdef;
			var val = field.target ? field.target[ field.name ] : null;
			var tp = typeof( val ) ;
			// type changed? refresh
			if ( field.type !== tp ) {
				log( "reload, type changed: ", field.type, '!=', tp, "/", field.name );
				go.async( go.refresh );
				return;
			}
			// have def
			if ( pdef !== undefined ) {
				// have conditional hiding - apply
				if ( typeof( pdef.hidden ) === 'function' ) {
					if ( typeof( field.fieldButton ) !== 'undefined' ){
						field.fieldLabel.active = field.fieldButton.active = !pdef.hidden( field.fieldButton.target );
						field.active = (field.fieldButton.active && field.fieldButton.toggleState );
					} else {
						field.fieldLabel.active = field.active = !pdef.hidden( field.target );
					}
				}
			}
			// skip focused and hidden fields
			if ( field.focused || !field.active ) continue;
			if ( tp == 'object' && field.reload ) {
				field.reload();
			} else if ( tp == 'boolean' ) {
				field.checked = val;
				field.text = ( val ? "True" : "False" );
			} else {
				field.value = val;
			}
		}

		// schedule update
		// if ( updateInterval > 0 ) go.debounce( 'reload', go.reload, updateInterval );

	}

	go.push = function ( newTarget ) {
		/// TODO
	}

	go.pop = function () {
		/// TODO
	}

	// restart auto refresh on adding to scene
	go.on( 'addedToScene', function() {
		// schedule update
		// if ( updateInterval > 0 ) go.debounce( 'reload', go.reload, updateInterval );
	} );

	// apply defaults
	go.baseStyle = Object.create( UI.style.propertyList );
	UI.base.applyProperties( go, go.baseStyle );
	go.values = go.values || { };
	go.state = 'auto';
	constructing = false;

})(this);

/*

Default inspector parameters for property list

*/

Color.__propertyListConfig = Color.__propertyListConfig ? Color.__propertyListConfig : {
	showAll: false,
	properties: {
		'r': { min: 0, max: 1, step: 0.1 },
		'g': { min: 0, max: 1, step: 0.1 },
		'b': { min: 0, max: 1, step: 0.1 },
		'a': { min: 0, max: 1, step: 0.1 },
		'hex': { style: { selectAllOnFocus: true, pattern: /^[0-9a-f]{0,8}$/i } },
	},
	groups: [ { properties: [ 'r', 'g', 'b', 'a', 'hex' ] } ]
}