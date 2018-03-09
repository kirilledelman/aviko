/*

	Control for editing object's properties

	Usage:

		var p = App.scene.addChild( 'ui/property-list' );
		p.target = myObject;

	look at mappedProps in source code below for additional properties
	also has shared layout properties from ui/ui.js

	Events:
		'change' - a property has changed - callback( targetObject, propertyName, newValue, oldValue )

*/

include( './ui' );
(function(go) {

	// internal props
	var ui = new UI();
	var target = null;
	var showAll = true;
	var properties = false;
	var valueWidth = 150;
	var disabled = false;
	var groups = [];

	var showAll = false;
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
		//          enum: [ { icon:"icon1", text:"text1", value:value1 }, { text:text2, value:value2 }, ... ] - shows dropdown
		//          description: "text displayed for property instead of property name",
		//
		[ 'properties',  function (){ return properties; }, function ( v ){
			properties = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Array) in form of [ { name: "Group name", properties: [ 'p1', 'p2', ... ] } ... ] to group and order properties
		[ 'groups',  function (){ return groups; }, function ( v ){
			groups = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : [];
			go.debounce( 'refresh', go.refresh );
		}],

		// (Object) target object whose properties are displayed in this property list
		[ 'target',  function (){ return target; }, function ( v ){
			target = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) width of value field (text stretches to fill the rest of space)
		[ 'valueWidth',  function (){ return excludeProperties; }, function ( v ) {
			excludeProperties = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Boolean) disable all fields
		[ 'disabled',  function (){ return disabled; }, function ( v ) {
			disabled = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Boolean) show add and remove property (and array items) buttons
		[ 'addRemoveProperties',  function (){ return 'TODO'; }, function ( v ){
			//TODO
		}],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "PropertyList";

	// UI
	ui.focusable = false;
	ui.layoutType = Layout.Horizontal;
	ui.layoutAlignX = LayoutAlign.Stretch;
	ui.layoutAlignY = LayoutAlign.Start;
	ui.wrapEnabled = true;
	ui.wrapAfter = 2;
	ui.fitChildren = true;
	go.ui = ui;

	ui.layout = function () {
		go.fire( 'layout' );
	}

	// recreates controls
	go.refresh = function () {
		// remove previous
		go.children = [];
		ui.height = ui.minHeight = 0;
		if ( !target ) return;

		// sort properties into groups
		var regroup = { ' ': [] }; // default (unsorted) group
		var mappedProps = {};

		// copy each specified group into regroup
		for ( var i in groups ) {
			var g = groups[ i ];
			var props = regroup[ g.name ] = [];
			// copy properties that exist in target and match showing criteria
			for( var i in g.properties ) {
				var pname = g.properties[ i ];
				var pdef = properties[ pname ];
				if ( target[ pname ] !== undefined && // target has property
					( ( showAll && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
					( !showAll && pdef !== undefined && pdef !== false ) ) ) { // showing select properties, and prop is included
						props.push( pname );
						mappedProps[ pname ] = g.name;
				}
 			}
		}
		// for each property name in target object
		for ( var p in target ) {
			var pdef = properties[ p ];
			if ( ( showAll && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
				( !showAll && pdef !== undefined && pdef !== false ) ) { // showing select properties, and prop is included
				// property not in any groups
				if ( mappedProps[ p ] === undefined ) {
					// put in default group
					regroup[ ' ' ].push( p );
				}
			}
		}
		// sort default group by name
		regroup[ ' ' ].sort();

		// for each group
		for ( var i = 0, ng = groups.length; i <= ng; i++ ) {
			var props = i < ng ? regroup[ groups[ i ].name ] : regroup[ ' ' ];
			if ( props === undefined || !props.length ) continue;

			// not default group
			if ( i < ng ) {

				// add group title
				var groupTitle = go.addChild( 'ui/text', {
					forceWrap: true,
					text: groups[ i ].name,
					style: UI.style.propertyList.group
				} );
				// clear top margin if first
				if ( i == 0 ) groupTitle.marginTop = 0;

			}

			// for each property
			for ( var j = 0, np = props.length; j < np; j++ ) {
				var pname = props[ j ];
				var pdef = properties[ pname ];

				// add label
				var label = go.addChild( 'ui/text', {
					text: ( ( pdef && pdef.description ) ? pdef.description : pname ),
					flex: 1,
					style: UI.style.propertyList.label
				} );

				// value/type
				var fieldValue = target[ pname ];
				var fieldType = typeof( fieldValue );

				// check for enumeration
				if ( pdef && pdef.enum !== undefined ) {
					fieldType = 'enum';
				}

				// create appropriate control
				var field;
				switch ( fieldType ) {
					case 'number':
						field = go.addChild( 'ui/textfield', {
							numeric: true,
							width: valueWidth,
							integer: ( pdef && pdef.integer !== undefined ) ? pdef.integer : false,
							min: ( pdef && pdef.min !== undefined ) ? pdef.min : -Infinity,
							max: ( pdef && pdef.max !== undefined ) ? pdef.max : Infinity,
							step: ( pdef && pdef.step !== undefined ) ? pdef.step : 1,
							value: fieldValue,
							style: UI.style.propertyList.value.any
						} );
						field.style = UI.style.propertyList.value.number;
						break;
					case 'enum':
						field = go.addChild( 'ui/dropdown', {
							value: fieldValue,
							items: pdef.enum,
							style: UI.style.propertyList.value.any
						} );
						field.style = UI.style.propertyList.value.number;
						break;
					default:
						field = go.addChild( 'ui/textfield', {
							width: valueWidth,
							disabled: true,
							value: fieldValue.toString(),
							style: UI.style.propertyList.value.any
						} );
						break;

				}


			}


		}


		// log ( Object.getOwnPropertyNames( target ) );

		// add labels and values

	}

	// refreshes properties values in rows from target
	go.reload = function () {

		if ( !target ) return;

		// for each property name in target
		// var propNamesArray = [];
		// var propNameObject = {};




	}

	// apply defaults
	UI.base.applyProperties( go, UI.style.propertyList );

})(this);
