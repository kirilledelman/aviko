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
	var header, backButton, moreButton;
	var target = null;
	var targetStack = [];
	var showAll = undefined;
	var properties = {};
	var valueWidth = 130;
	var disabled = false;
	var topPropertyList = go;
	var groups = [];
	var allFields = [];
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
			targetStack.length = 0;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Number) width of value fields
		[ 'valueWidth',  function (){ return valueWidth; }, function ( v ) {
			valueWidth = Math.max( 45, v );
			go.debounce( 'refresh', go.refresh );
		}],

		// (Boolean) disable all fields
		[ 'disabled',  function (){ return disabled; }, function ( v ) {
			ui.disabled = disabled = v;
			go.debounce( 'refresh', go.refresh );
		}],

		// (Boolean) show add and remove property (and array items) buttons
		[ 'showHeader',  function (){ return header.active; }, function ( v ){ header.active = v; }],

		// (ui/panel) reference to header (where object / navigation / extras are displayed)
		[ 'header',  function (){ return header; } ],

		// (ui/button) reference to back button
		[ 'backButton',  function (){ return backButton; } ],

		// (ui/button) reference to more button
		[ 'moreButton',  function (){ return moreButton; } ],

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

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		[ 'focusGroup',  function (){ return ui.focusGroup; }, function ( f ){
			backButton.focusGroup = moreButton.focusGroup = ui.focusGroup = f;
			for ( var i in allFields ) {
				allFields[ i ].focusGroup = f;
			}
		} ],

	];
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.addMappedProperties( go, mappedProps );

	// create components

	// set name
	if ( !go.name ) go.name = "PropertyList";

	// UI
	ui.focusable = false;
	go.ui = ui;

	// header
	header = new GameObject( './panel', {
		layoutType: Layout.Horizontal,
		layoutAlignY: LayoutAlign.End,
		layoutAlignX: LayoutAlign.Start,
		fitChildren: true,
	} );

	// back
	backButton = header.addChild( './button', {
		text: "Nothing selected",
		focusGroup: ui.focusGroup,
		flex: 1,
		click: function () {
			// pop to previously selected object
			if ( targetStack.length ) {
				target = targetStack[ targetStack.length - 1 ].target;
				targetStack.pop();
				go.refresh();
				if ( targetStack.length == 0 ) backButton.blur();
			}
		}
	} );
	backButton.style = UI.style.propertyList.backButton;

	// editing options
	moreButton = header.addChild( './button', {
		text: "...",
		focusGroup: ui.focusGroup,
		click: function (){
			var items = [
				{ text: "Reload object", action: function ( obj ){ go.refresh(); } },
				{ text: "Add property", action: function ( obj ){ log("^4TODO"); } },
				{ text: "Remove property", action: function ( obj ){ log("^4TODO"); } },
			];
			var popup = new GameObject( './popup-menu', {
				target: this,
				items: items,
				selectedIndex: 0,
				selected: function ( s ) { s.action( target ); }
			} );
		}
	} );
	moreButton.style = UI.style.propertyList.moreButton;

	// auto fit scrollbar
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
			header.maxWidth = scrollable.width + scrollable.marginRight;
		} else {
			ui.spacingX = spacingX;
			ui.spacingY = spacingY;
			ui.pad = pad;
		}
		if ( go.render ) go.render.resize( w, h );
		go.fire( 'layout' );
	}

	// creates or replaces a field for property
	go.makeField = function ( curTarget, pname, pdef, label, cont ) {

		var fieldValue, fieldType, replaceField = null, replaceIndex = -1, insertChildIndex = -1;

		// replace field mode?
		if ( arguments.length == 2 ) {
			replaceIndex = arguments[ 1 ];
			replaceField = arguments[ 0 ];
			curTarget = replaceField.target;
			pdef = replaceField.pdef;
			pname = replaceField.name;
			label = replaceField.fieldLabel;
			cont = replaceField.parent;
		}

		// common
		fieldValue = curTarget[ pname ];
		fieldType = typeof( fieldValue );
		if ( pdef.enum !== undefined ) fieldType = 'enum';

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
				}, insertChildIndex );
				field.style = go.baseStyle.values.number;
				break;

			case 'string':
				field = cont.addChild( './textfield', {
					name: pname,
					target: curTarget,
					editEnd: go.fieldChanged,
					minWidth: valueWidth,
					value: fieldValue,
					style: go.baseStyle.values.any
				}, insertChildIndex );
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
				}, insertChildIndex );
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
				}, insertChildIndex );
				field.style = go.baseStyle.values.boolean;
				break;

			// inspector:

			case 'object':

				// button to show inspector
				function togglePropList() {
					// embedded inspector created on demand
					if ( !this.propList ) {
						var myPos = this.parent.children.indexOf( this );
						this.propList = new GameObject( './property-list', {
							name: pname,
							flex: 1,
							scrollable: false,
							forceWrap: true,
							active: !!pdef.expanded,
							style: go.baseStyle.values.any,
							fieldButton: this,
							showHeader: false,
							type: 'object',
						} );
						this.parent.addChild( this.propList, myPos + 1 );
						this.propList.style = go.baseStyle.values.inline;
						this.propList.valueWidth = valueWidth - this.propList.marginLeft; // indent
						if ( this.pdef.showAll !== undefined ) this.propList.showAll = this.pdef.showAll;
						if ( this.pdef.properties !== undefined ) this.propList.properties = this.pdef.properties;
						if ( this.pdef.groups !== undefined ) this.propList.groups = this.pdef.groups;
						this.propList.target = this.fieldValue;
					}
					// toggle display
					this.toggleState = this.propList.active = !this.propList.active;
					this.image.imageObject.angle = (this.propList.active ? 0 : -90);
				}

				// button to go into an object
				function pushToTarget() {
					targetStack.push( {
						target: target,
						name: (target.constructor ? target.constructor.name : String( target ) ) + "." + this.name
					} );
					target = this.fieldValue;
					go.refresh();
				}

				// inline option
				var inline = ( pdef && pdef.inline );

				// create field button
				field = cont.addChild( './button', {
					target: curTarget,
					name: pname,
					fieldValue: fieldValue,
					text: String( (fieldValue && fieldValue.constructor) ? fieldValue.constructor.name : String(fieldValue) ),
					wrapEnabled: false,
					minWidth: valueWidth,
					disabled: (disabled || ( pdef && pdef.disabled )),
					style: go.baseStyle.values.any,
					toggleState: !!pdef.expanded
				}, insertChildIndex );
				field.style = go.baseStyle.values.object;
				if ( inline ) {
					field.click = togglePropList;
					field.image.imageObject.angle = (field.toggleState ? 0 : -90);
				} else {
					field.click = pushToTarget;
					field.image.imageObject.angle = -90;
					field.label.flex = 1;
					field.reversed = true;
				}

				break;

			default:
				field = cont.addChild( './textfield', {
					name: pname,
					minWidth: valueWidth,
					value: String(fieldValue),
					style: go.baseStyle.values.any,
					disabled: true,
				}, insertChildIndex );
				field.style = go.baseStyle.values.string;
				break;

		}

		// common properties
		if ( field ) {
			field.type = typeof( fieldValue );
			field.pdef = pdef;
			field.focusGroup = ui.focusGroup;
			field.fieldLabel = label;
			if ( disabled || pdef.disabled ) field.disabled = true;
			if ( typeof( pdef.style ) === 'object' ) {
				UI.base.applyProperties( field, pdef.style );
			}
			if ( typeof( pdef.hidden ) === 'function' ) {
				var shown = !pdef.hidden( curTarget );
				field.fieldLabel.active = shown;
				if ( field.propList ) {
					field.propList.active = shown;
					field.active = (shown && field.toggleState );
				} else {
					field.active = shown;
				}
			}
			if ( replaceField ) {
				allFields.splice( replaceIndex, 1, field );
				replaceField.parent = null;
			} else {
				allFields.push( field );
				curTarget.watch( field.name, go.watchCallback );
			}
		}

		return field;
	}

	// recreates controls
	go.refresh = function () {
		// set up container
		var cont = go;
		if ( shouldScroll ) {
			if ( !scrollable ) {
				scrollable = go.addChild( './scrollable', {
					layoutType: Layout.Horizontal,
					layoutAlignX: LayoutAlign.Start,
					layoutAlignY: LayoutAlign.Start,
					marginRight: 0,
					focusGroup: ui.focusGroup,
					wrapEnabled: true,
					wrapAfter: 2,
					acceptToCycle: true,
					fitChildren: true,
					scrollbars: false,
					flex: 1
				} );
			}
			cont = scrollable;
			ui.layoutType = Layout.Vertical;
			ui.layoutAlignX = LayoutAlign.Stretch;
			ui.layoutAlignY = LayoutAlign.Stretch;
			ui.fitChildren = false;
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

		// add header as first child
		go.addChild( header, 0 );

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

		// set header button name
		var bn = [];
		for ( var i = 0; i < targetStack.length; i++ ) {
			bn.push( targetStack[ i ].name );
		}
		bn.push( ( target && target.constructor ) ? target.constructor.name : "(null)" );
		backButton.text = bn.join( ' ^B->^n ' );
		backButton.disabled = ( targetStack.length == 0 );
		if ( targetStack.length ) {
			backButton.icon = UI.style.propertyList.values.object.icon;
		} else {
			backButton.icon = "";
		}

		// empty target
		if ( !target ) {
			moreButton.disabled = true;
			return;
		} else {
			moreButton.disabled = false;
		}

		// if displaying a custom inspector
		if ( _customInspector ) {
			// initialize it
			inspector = _customInspector( go, target, _properties, _groups, _showAll );
			cont.addChild( inspector );
			return;
		}

		// displaying standard inspector

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
			for ( var j = 0, np = props.length; j < np; j++ ) {
				var pname = props[ j ];
				var pdef = _properties[ pname ] || {};

				// add label
				if ( typeof( pdef.label ) === 'function' ) labelText = pdef.label( pname );
				else if ( typeof ( pdef.label ) === 'string' ) labelText = pdef.label;
				var label = cont.addChild( './text', {
					text: pname,
					flex: 1,
					wrap: true,
					style: go.baseStyle.label
				} );

				// add field
				go.makeField( ( pdef.target || target ) , pname, pdef, label, cont );
				numRows++;

			}

		}

		// placeholder
		if ( numRows == 0 ) {
			var sv = String( target );
			cont.addChild( './text', {
				selfAlign: LayoutAlign.Stretch,
				text: ( sv.length ? ("^B(" + sv + ")^b: ") : "" ) + "no editable properties",
				style: go.baseStyle.empty,
			} );
		}

		// scroll to top, enable scrollbars
		if ( scrollable ) {
			scrollable.scrollLeft = scrollable.scrollTop = 0;
			function _showScrollbars() { this.scrollbars = 'auto'; }
			scrollable.debounce( 'showScrollbars', _showScrollbars, 0.1 );
		}

		// clean up
		gc();

	}

	// single field changed, update field
	go.watchCallback = function ( p, ov, v ) {
		go.async( function() { go.reload( p, v ); } );
		return v;
	}

	go.fieldChanged = function ( val ) {

		// if field is boolean
		if ( this.type === 'boolean' ) {
			// update label
			this.text = ( val ? "True" : "False" );
		}

		// apply
		var oldVal = this.target[ this.name ];
		this.target[ this.name ] = val;

		// fire changed
		go.fire( 'change', this.target, this.name, val, oldVal );

		// callback
		if ( this.pdef.change ) this.pdef.change( this.target, this.name, val, oldVal );

		// full reload option
		if ( this.pdef.reloadOnChange ) go.reload();

	}

	// refreshes properties values in rows from target
	go.reload = function ( propName, valOverride ) {

		if ( !target ) return;

		// update fields
		var field, val, tp;
		for ( var i in allFields ) {
			field = allFields[ i ];
			pdef = field.pdef;
	        val = field.target ? field.target[ field.name ] : undefined;
			// if reload is called with field name
			if ( typeof( propName ) !== 'undefined' ) {
				// skip all others
				if ( field.name !== propName ) continue;
				// override value, if given
				if ( arguments.length == 2 ) val = valOverride;
			}
			tp = typeof( val );

			// if field type has changed
			if ( field.type !== tp && field.type !== 'enum' ) {
				log( "reload type changed for \"" + field.name + "\" field.type:", field.type, ', current value:', val, ' of type: ', tp );
				// re-create just this field
				field = go.makeField( field, i );
			}

			// have conditional hiding - apply
			if ( typeof( pdef.hidden ) === 'function' ) {
				if ( typeof( field.fieldButton ) !== 'undefined' ){
					field.fieldLabel.active = field.fieldButton.active = !pdef.hidden( field.fieldButton.target );
					field.active = (field.fieldButton.active && field.fieldButton.toggleState );
				} else {
					field.fieldLabel.active = field.active = !pdef.hidden( field.target );
				}
			}

			// skip focused and hidden fields
			if ( field.focused || !field.active ) continue;

			// field is object with inline property list
			if ( tp == 'object' && field.propList ) {
				field.propList.reload();

			// field is boolean
			} else if ( tp == 'boolean' ) {
				field.checked = val;
				field.text = ( val ? "True" : "False" );

			// other
			} else {
				field.value = val;
			}

			// if reload is called with field name, exit after updating
			if ( typeof( propName ) !== 'undefined' && field.name === propName ) break;
		}

	}

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
		'r': { min: 0, max: 1, step: 0.1, reloadOnChange: true },
		'g': { min: 0, max: 1, step: 0.1, reloadOnChange: true },
		'b': { min: 0, max: 1, step: 0.1, reloadOnChange: true },
		'a': { min: 0, max: 1, step: 0.1, reloadOnChange: true },
		'hex': { style: { selectAllOnFocus: true, pattern: /^[0-9a-f]{0,8}$/i }, reloadOnChange: true },
	},
	groups: [ { properties: [ 'r', 'g', 'b', 'a', 'hex' ] } ]
}