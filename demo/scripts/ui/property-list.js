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
	var scrollable, shouldScroll = true;
	var target = null;
	var showAll = true;
	var properties = {};
	var labelWidth = 150;
	var disabled = false;
	var topPropertyList = go;
	var groups = [];
	var allFields = [];
	var updateInterval = 0;

	var showAll = false;
	go.serializeMask = { 'ui':1, 'target':1, 'children': 1 };

	// API properties
	var mappedProps = [

		// (Boolean) - if true, all enumerable properties of object will be displayed,
		// if false, only ones in .properties
		[ 'showAll',  function (){ return showAll; }, function ( v ){
			showAll = v;
			go.fireLate( 'refresh' );
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

		// (Number) width of label field (input field stretches to fill the rest of space)
		[ 'labelWidth',  function (){ return labelWidth; }, function ( v ) {
			labelWidth = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) automatically refresh displayed properties every updateInterval seconds
		[ 'updateInterval',  function (){ return updateInterval; }, function ( v ) {
			updateInterval = v;
			if ( updateInterval > 0 ) {
				go.debounce( 'reload', go.reload, updateInterval );
			} else {
				go.clearDebouncer( 'reload' );
			}
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
		[ 'pad',  function (){ return (scrollable || ui).pad; }, function ( v ){ ui.pad = 0; (scrollable || ui).pad = v; } ],

		// (Number) inner padding top
		[ 'padTop',  function (){ return (scrollable || ui).padTop; }, function ( v ){ ui.padTop = 0; (scrollable || ui).padTop = v; }, true ],

		// (Number) inner padding right
		[ 'padRight',  function (){ return (scrollable || ui).padRight; }, function ( v ){ ui.padRight = 0; (scrollable || ui).padRight = v; }, true ],

		// (Number) inner padding bottom
		[ 'padBottom',  function (){ return (scrollable || ui).padBottom; }, function ( v ){ ui.padBottom = 0; (scrollable || ui).padBottom = v; }, true ],

		// (Number) inner padding left
		[ 'padLeft',  function (){ return (scrollable || ui).padLeft; }, function ( v ){ ui.padLeft = 0; (scrollable || ui).padLeft = v; }, true ],

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		[ 'spacing',  function (){ return (scrollable || ui).spacing; }, function ( v ){ (scrollable || ui).spacing = v; }, true ],

		// (Number) spacing between children when layoutType is Vertical
		[ 'spacingX',  function (){ return (scrollable || ui).spacingX; }, function ( v ){ (scrollable || ui).spacingX = v; } ],

		// (Number) spacing between children when layoutType is Horizontal
		[ 'spacingY',  function (){ return (scrollable || ui).spacingY; }, function ( v ){ (scrollable || ui).spacingY = v; } ],


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
		}
		if ( go.render ) go.render.resize( w, h );
		go.fire( 'layout' );
	}

	// recreates controls
	go.refresh = function () {
		// set up
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
					fitChildren: true
				} );
			}
			cont = scrollable;
			ui.layoutType = Layout.Anchors;
		} else {
			if ( scrollable ) scrollable = null;
			ui.layoutType = Layout.Horizontal;
			ui.layoutAlignX = LayoutAlign.Start;
			ui.layoutAlignY = LayoutAlign.Start;
			ui.wrapEnabled = true;
			ui.wrapAfter = 2;
			ui.fitChildren = true;
		}

		// remove previous
		cont.removeAllChildren();
		cont.ui.height = cont.ui.minHeight = 0;
		allFields.length = 0;
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
				if ( ( target[ pname ] !== undefined && // target has property
					( ( showAll && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
					( !showAll && pdef !== undefined && pdef !== false ) ) ) || // showing select properties, and prop is included
					( pdef && pdef.target ) ) { // or has overridden target
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
		var field = null;
		var numRows = 0, numGroups = 0;
		for ( var i = 0, ng = groups.length; i <= ng; i++ ) {
			var props = i < ng ? regroup[ groups[ i ].name ] : regroup[ ' ' ];
			if ( props === undefined || !props.length ) continue;
			numGroups++;
			if ( i < ng || numGroups > 1 ) {
				// add group title
				var groupTitle = cont.addChild( './text', {
					forceWrap: true,
					minWidth: labelWidth,
					text: (i < ng ? groups[ i ].name : ''),
					style: UI.style.propertyList.group,
				} );
				// clear top margin if first
				if ( i == 0 ) groupTitle.marginTop = 0;
			}
			// for each property
			for ( var j = 0, np = props.length; j < np; j++ ) {
				var pname = props[ j ];
				var pdef = properties[ pname ];

				// add label
				var label = cont.addChild( './text', {
					text: ( ( pdef && pdef.description ) ? pdef.description : pname ),
					minWidth: labelWidth,
					style: UI.style.propertyList.label
				} );

				// value/type
				var curTarget = ( pdef && pdef.target ? pdef.target : target );
				var fieldValue = curTarget[ pname ];
				var fieldType = typeof( fieldValue );

				// check for enumeration
				if ( pdef && pdef.enum !== undefined ) {
					fieldType = 'enum';
				}

				// create appropriate control
				switch ( fieldType ) {
					case 'number':
						field = cont.addChild( './textfield', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							numeric: true,
							flex: 1,
							integer: ( pdef && pdef.integer !== undefined ) ? pdef.integer : false,
							min: ( pdef && pdef.min !== undefined ) ? pdef.min : -Infinity,
							max: ( pdef && pdef.max !== undefined ) ? pdef.max : Infinity,
							step: ( pdef && pdef.step !== undefined ) ? pdef.step : 1,
							value: fieldValue,
							disabled: disabled,
							style: UI.style.propertyList.values.any
						} );
						field.style = UI.style.propertyList.values.number;
						break;
					case 'enum':
						field = cont.addChild( './dropdown', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							flex: 1,
							value: fieldValue,
							items: pdef.enum,
							disabled: disabled,
							style: UI.style.propertyList.values.any
						} );
						field.style = UI.style.propertyList.values.enum;
						break;
					case 'boolean':
						field = cont.addChild( './checkbox', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							checked: fieldValue,
							disabled: disabled,
							text: fieldValue ? "True" : "False",
							flex: 1,
							style: UI.style.propertyList.values.any
						} );
						field.style = UI.style.propertyList.values.boolean;
						break;
					case 'object':
						field = cont.addChild( './button', {
							text: String( fieldValue.constructor ? fieldValue.constructor.name : fieldValue ) + '...',
							disabled: disabled,
							style: UI.style.propertyList.values.any,
							wrapEnabled: false,
							flex: 1
						} );
						field.style = UI.style.propertyList.values.object;
						// inline property list
						if ( pdef && fieldValue && typeof( fieldValue ) == 'object' ){
							if ( pdef.inline || pdef.properties ) {
								var sub = cont.addChild( './property-list', {
									flex: 1,
									showAll: true,
									scrollable: false,
									forceWrap: true,
									style: UI.style.propertyList.values.any,
									active: !!pdef.expanded,
								} );
								sub.style = UI.style.propertyList.values.inline;
								if ( pdef.showAll !== undefined ) sub.showAll = pdef.showAll;
								if ( pdef.properties !== undefined ) sub.properties = pdef.properties;
								if ( pdef.groups !== undefined ) sub.groups = pdef.groups;
								sub.target = fieldValue;
								field.propList = sub;
								field.image.flipY = sub.active;
								function togglePropList() {
									this.image.flipY = this.propList.active = !this.propList.active;
								}
								field.click = togglePropList;
								field = sub;
							}
						} else {
							// TODO - add button handler to push into object, or disable button
						}
						break;
					default:
						field = cont.addChild( './text', {
							name: pname,
							target: curTarget,
							change: go.fieldChanged,
							flex: 1,
							text: fieldValue ? fieldValue.toString() : '',
							style: UI.style.propertyList.values.any
						} );
						break;

				}
				// common properties
				if ( field ) {
					allFields.push( field );
					if ( pdef ) {
						// disabled
						if ( pdef.disabled ) field.disabled = true;
					}
				}
				numRows++;

			}

		}

		// placeholder
		if ( numRows == 0 ) {
			var nope = cont.addChild( './text', {
				selfAlign: LayoutAlign.Stretch,
				text: "no editable properties",
				style: UI.style.propertyList.group,
			} );
			nope.align = TextAlign.Center;
		}

		// scroll to top
		if ( scrollable ) scrollable.scrollLeft = scrollable.scrollTop = 0;

	}

	go.fieldChanged = function ( val ) {

		// update label
		if ( typeof ( this.target[ this.name ] ) == 'boolean' ) {
			this.text = ( val ? "True" : "False" );
		}

		// apply
		var oldVal = this.target[ this.name ];
		this.target[ this.name ] = val;

		// fire changed
		go.fire( 'change', this.target, this.name, val, oldVal );
		go.debounce( 'reload', go.reload );

	}

	// refreshes properties values in rows from target
	go.reload = function () {

		if ( !target || !go.scene ) return;

		// update fields
		for ( var i in allFields ) {
			var field = allFields[ i ];
			if ( field.focused ) continue;
			var val = target[ field.name ];
			var tp = typeof( val ) ;
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
		if ( updateInterval > 0 ) go.debounce( 'reload', go.reload, updateInterval );

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
		if ( updateInterval > 0 ) go.debounce( 'reload', go.reload, updateInterval );
	} );

	// apply defaults
	UI.base.applyProperties( go, UI.style.propertyList );
	go.values = go.values || { };
})(this);
