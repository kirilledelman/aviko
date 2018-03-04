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
	var groups = [];
	var valueWidth = 50;

	var showAll = false;
	go.serializeMask = { 'ui':1 };

	// API properties
	var mappedProps = [

		// (Boolean) - if true, all enumerable properties of object will be displayed,
		// if false, only ones in .properties
		[ 'showAll',  function (){ return showAll; }, function ( v ){
			showAll = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Object) in form of { 'propertyName': PROPDEF, 'propertyName2': PROPDEF ... }
		//      PROPDEF is either
		//      (Boolean)   true - show with auto settings
		//                  false - do not show
		//      or
		//      (Object) with optional properties:
		//          min: mininim numeric value,
		//          max: maximim numeric value,
		//          step: step for numeric text box,
		//          type("int"|"float"|"string"|"object")
		//          description: help text
		[ 'properties',  function (){ return properties; }, function ( v ){
			properties = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Array) of objects in form { name: "group name", properties: [ 'p1', 'p2'... ] }
		// properties will be grouped into named sections, in order specified
		// if group without properties is specified, it's used for all other properties not in a group
		[ 'groups',  function (){ return target; }, function ( v ){
			target = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Object) target object whose properties are displayed in this property list
		[ 'target',  function (){ return target; }, function ( v ){
			target = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) width of value field
		[ 'valueWidth',  function (){ return excludeProperties; }, function ( v ) {
			excludeProperties = v;
			go.debounce( 'update', go.update );
		}],

		// (Number) spacing between rows
		[ 'spacingY',  function (){ return ui.spacingY; }, function ( v ) {
			ui.spacingY = v;
			go.debounce( 'update', go.update );
		}],

		// (Boolean) read-only mode
		[ 'readOnly',  function (){ return 'TODO'; }, function ( v ){
			//TODO
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
	ui.layoutAlign = LayoutAlign.Stretch;
	ui.fitChildren = true;
	go.ui = ui;

	// vertical container for all labels
	var labelsContainer = go.addChild( {
		ui: new UI( {
			layoutType: Layout.Vertical,
			spacingY: ui.spacingY
		} )
	} );

	// vertical container for all value inputs
	var valuesContainer = go.addChild( {
		ui: new UI( {
			layoutType: Layout.Vertical,
			spacingY: ui.spacingY
		} )
	} );

	// recreates controls
	go.refresh = function () {
		// remove previous
		labelsContainer.children = [];
		valuesContainer.children = [];
		if ( !target ) return;

		// for each property name in target
		var propNamesArray = [];
		var propNameObject = {};

		for ( var p in target ) {
			// if displaying all properties
			var pp = properties[ p ];
			if ( ( showAll && pp !== false ) || (!showAll && pp !== undefined && pp !== false ) ) {
				// add to list
				propNamesArray.push( p );
				propNamesObject[ p ] = true;
			}
		}





		// log ( Object.getOwnPropertyNames( target ) );

		// add labels and values

	}

	// update style
	go.update = function () {


	}

	// apply defaults
	UI.base.applyDefaults( go, UI.style.propertyList );

})(this);
