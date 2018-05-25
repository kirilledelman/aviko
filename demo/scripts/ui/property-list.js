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
	var target = undefined;
	var targetStack = [];
	var showAll = undefined;
	var properties = {};
	var valueWidth = 130;
	var disabled = false;
	var readOnly = false;
	var showContextMenu = true;
	var topPropertyList = go;
	var groups = [];
	var allFields = [];
	var actions = [];
	var constructing = true;
	var pad = [ 0, 0, 0, 0 ];
	var spacingX = 0, spacingY = 0;
	var customInspector, inspector;
	go.serializeMask = [ 'ui', 'target', 'children' ];

	// API properties
	var mappedProps = {

		// (Boolean) - if true, all enumerable properties of object will be displayed,
		// if false, only ones in .properties
		'showAll': { get: function (){ return showAll; }, set: function( v ){
			showAll = v;
			go.debounce( 'refresh', go.refresh );
		} },

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
		//          readOnly: (Boolean) force read only for this field
		//          enum: [ { icon:"icon1", text:"text1", value:value1 }, { text:text2, value:value2 }, ... ] - shows select dropdown menu
		//          label: (String) "text displayed for property instead of property name", or (Function) to transform propname to text
		//          style: (Object) - apply properties in this object to resulting input field
		//          hidden: (Function) - return true to conditionally hide this field. Function's only param is target
		//          deletable: (Boolean) - show "Delete property" option in right-click menu
		//          actions: (Array) - context menu actions when right-clicking on property. Format same as .actions property, but no buttons
		//          reloadOnChange: (Array) - reload these other properties on change, or (Boolean) true to reload all
		//
		//      for inline object editing, when an embedded property-list will be displayed, can override defaults with:
		//          properties: (Object) - apply properties to sub-property-list
		//          groups: (Object) - apply groups to sub-property-list
		//          showAll: (Boolean) - apply showAll param to sub-property-list
		//
		'properties': { get: function (){ return properties; }, set: function( v ){
			properties = v;
			go.debounce( 'refresh', go.refresh );
		} },

		// (Array) in form of [ { name: "Group name", properties: [ 'p1', 'p2', ... ] } ... ] to group and order properties
		//      Properties not listed in groups will appear in an automatically generated group after listed groups, alphabetically
		//      To override alphabetical order of default group, supply group without name: param
		'groups': { get: function (){ return groups; }, set: function( v ){
			groups = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : [];
			go.debounce( 'refresh', go.refresh );
		} },

		// (Array) in form of [ { text, button, targetUpdated, hidden, disabled, action } ... ]
		//      These will be added as additional actions to context menu, or as action buttons on top, if buttons = true
		//      params:
		//          text: (String) - action text
		//          button: (Boolean) - if true, will be added as button to the header, instead of context menu
		//          targetUpdated: (Function) - if button, this function will be called whenever any property is updated, 'this' is propertyList, param is button
		//          hidden: (Function) - if context menu, function called when constructing popup menu, return true to hide action, 'this' is propertyList
		//          disabled: (Function) - if context menu, function called when constructing popup menu, return true to disable action, 'this' is propertyList
		//          action: (Function) - function called to execute action. 'this' is propertyList
		'actions': { get: function (){ return actions; }, set: function( v ){
			actions = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : [];
		} },

		// (Object) target object whose properties are displayed in this property list
		'target': { get: function (){ return target; }, set: function( v ){
			target = v;
			targetStack.length = 0;
			go.debounce( 'refresh', go.refresh );
		} },

		// (Number) width of value fields
		'valueWidth': { get: function (){ return valueWidth; }, set: function( v ) {
			valueWidth = Math.max( 45, v );
			go.debounce( 'refresh', go.refresh );
		} },

		// (Boolean) disable all fields
		'disabled': { get: function (){ return disabled; }, set: function( v ) {
			if ( disabled != v ) {
				disabled = v;
				go.debounce( 'refresh', go.refresh );
			}
		} },

		// (Boolean) disable all fields except objects - can follow deeper into objects
		'readOnly': { get: function (){ return readOnly; }, set: function( v ) {
			if ( readOnly != v ) {
				readOnly = v;
				go.debounce( 'refresh', go.refresh );
			}
		} },

		// (Boolean) show navigation button (used only in top level property list)
		'showBackButton': { get: function (){ return backButton.active; }, set: function( v ){ backButton.active = v; } },

		// (Boolean) show context right click menu on fields
		'showContextMenu': { get: function (){ return showContextMenu; }, set: function( v ){ showContextMenu = v; } },

		// (Boolean) show [...] button
		'showMoreButton': { get: function (){ return moreButton.active; }, set: function( v ){ moreButton.active = v; } },

		// (ui/panel) reference to header (where object / navigation / extras are displayed)
		'header': { get: function (){ return header; } },

		// (ui/button) reference to back button
		'backButton': { get: function (){ return backButton; } },

		// (ui/button) reference to more button
		'moreButton': { get: function (){ return moreButton; } },

		// (Boolean) should this property list be scrollable
		'scrollable': {
			get: function (){ return shouldScroll; },
			set: function( v ){
				if ( shouldScroll != v ) {
					shouldScroll = v;
					go.debounce( 'refresh', go.refresh );
				}
			}
		},

		// (GameObject) container to which all fields are added
		'container': { get: function (){ return (scrollable || go); } },

		// (GameObject) when displaying nested property list editors, this holds reference to the topmost one
		'topPropertyList': { get: function (){ return topPropertyList; }, set: function( v ){ topPropertyList = v; }  },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		'pad': { get: function (){ return pad; }, set: function( v ){ pad = v; ui.requestLayout( 'pad' ); }  },

		// (Number) inner padding top
		'padTop': { get: function (){ return pad[ 0 ]; }, set: function( v ){ pad[ 0 ] = v; ui.requestLayout( 'padTop' ); }, serialized: false  },

		// (Number) inner padding right
		'padRight': { get: function (){ return pad[ 1 ]; }, set: function( v ){ pad[ 1 ] = v; ui.requestLayout( 'padRight' ); }, serialized: false },

		// (Number) inner padding bottom
		'padBottom': { get: function (){ return pad[ 2 ]; }, set: function( v ){ pad[ 2 ] = v; ui.requestLayout( 'padBottom' ); }, serialized: false },

		// (Number) inner padding left
		'padLeft': { get: function (){ return pad[ 3 ]; }, set: function( v ){ pad[ 3 ] = v; ui.requestLayout( 'padLeft' ); }, serialized: false },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		'spacing': { get: function (){ return Math.max( spacingX, spacingY ); }, set: function( v ){ spacingX = spacingY = v; ui.requestLayout( 'spacing' ); }, serialized: false },

		// (Number) spacing between label and value
		'spacingX': { get: function (){ return spacingX; }, set: function( v ){ spacingX = v; ui.requestLayout( 'spacingX' ); } },

		// (Number) spacing between rows
		'spacingY': { get: function (){ return spacingY; }, set: function( v ){ spacingY = v; ui.requestLayout( 'spacingY' ); } },

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		'focusGroup': { get: function (){ return ui.focusGroup; }, set: function( f ){
			backButton.focusGroup = moreButton.focusGroup = ui.focusGroup = f;
			for ( var i in allFields ) {
				allFields[ i ].focusGroup = f;
			}
		}  },

	};
	UI.base.addSharedProperties( go, ui ); // add common UI properties (ui.js)
	UI.base.mapProperties( go, mappedProps );

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
		wrapEnabled: true,
		forceWrap: true,
		fitChildren: true,
	} );

	// back
	backButton = header.addChild( './button', {
		text: "Nothing selected",
		focusGroup: ui.focusGroup,
		flex: 1,
		forceWrap: true,
		click: function () {
			// pop to previously selected object
			if ( targetStack.length ) {
				var pop = targetStack[ targetStack.length - 1 ];
				target = pop.target;
				targetStack.pop();
				go.refresh( pop.scrollTop );
				if ( targetStack.length == 0 ) backButton.blur();
			}
		},
	} );
	backButton.style = UI.style.propertyList.backButton;

	// editing options
	moreButton = header.addChild( './button', {
		text: "...",
		focusGroup: ui.focusGroup,
		actions: [],
		fixedPosition: true
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
		// position more button in upper right
		if ( moreButton.active ) {
			header.marginRight = header.padRight + moreButton.width + header.spacingX;
			moreButton.setTransform( header.width + header.spacingX, header.padTop );
			header.minHeight = Math.max( header.minHeight, moreButton.height );
		} else {
			header.marginRight = moreButton.active ? (header.padRight + moreButton.width + header.spacingX) : 0;
		}
		// update background
		if ( go.render ) go.render.resize( w, h );
	}

	//
	function nameObject( obj ) {
		if ( obj === null ) return '(null)';
		if ( obj === undefined ) return '(undefined)';
		if ( typeof( obj ) === 'object' ) {
			if ( obj.constructor === Array ) return 'Array[' + obj.length +']';
			if ( obj.constructor === Vector ) return 'Vector[' + obj.length +']';
			if ( obj.constructor === Color ) return '#' + obj.hex;
			if ( obj.constructor === Image ) return 'Image(' + obj.width + 'x' + obj.height + ')';
			if ( obj.constructor === GameObject || obj.constructor === Scene  ) {
				return ( obj.active ? '' : '^9' ) + ( obj.name.length > 0 ? ( '<' + obj.name + '>' ) : obj.constructor.name );
			}
			if ( obj.constructor === Body || obj.constructor === UI || obj.constructor === RenderText || obj.constructor === RenderSprite || obj.constructor === RenderShape ) {
				return ( obj.active ? '' : '^9' ) + obj.constructor.name;
			}
			return obj.constructor.name;
		}
		return String( obj );
	}

	// button callback to show inspector
	function togglePropList() {
		// embedded inspector created on demand
		if ( !this.propList ) {
			var myPos = this.parent.children.indexOf( this );
			this.propList = new GameObject( './property-list', {
				name: this.name,
				flex: 1,
				readOnly: !!this.pdef.readOnly,
				scrollable: false,
				forceWrap: true,
				active: !!this.pdef.expanded,
				style: go.baseStyle.values.any,
				fieldButton: this,
				showBackButton: false,
				showContextMenu: showContextMenu,
				showMoreButton: moreButton.active,
				topPropertyList: topPropertyList ? topPropertyList : go,
				type: 'object',
				change: function () { this.fieldButton.text = nameObject( this.target ); }
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
		this.image.angle = (this.propList.active ? 0 : -90);
		this.image.ui.offsetY = (this.propList.active ? 0 : 9);
	}

	// button callback to push into an object
	function pushToTarget() {
		if ( topPropertyList ) {
			topPropertyList.pushToTarget( this.fieldValue, this.name );
		} else {
			go.pushToTarget( this.fieldValue, this.name );
		}
	}

	function editFunctionBody() {

		// force "edit value"
		go.addProperty( this.name, 'function', this.fieldValue );

	}

	// creates or replaces a field for property
	go.makeField = function ( curTarget, pname, pdef, label, cont ) {

		var fieldValue, fieldType, replaceField = null, replaceAllFieldsIndex = -1, insertChildIndex = -1;

		// replace field mode?
		if ( arguments.length == 1 ) {
			replaceField = arguments[ 0 ];
			cont = replaceField.parent;
			replaceAllFieldsIndex = allFields.indexOf( replaceField );
			insertChildIndex = cont.children.indexOf( replaceField );
			curTarget = replaceField.target;
			pdef = replaceField.pdef;
			pname = replaceField.name;
			label = replaceField.fieldLabel;
		}

		// common
		fieldValue = curTarget[ pname ];
		fieldType = typeof( fieldValue );
		if ( fieldValue === null ) fieldType = 'null';
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
					autoGrow: true,
					newLinesRequireShift: true,
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

			case 'function':
				// if function is native, display as read-only
				field = cont.addChild( './button', {
					name: pname,
					target: curTarget,
					fieldValue: fieldValue,
					text: "function " + fieldValue.name + "()",
					wrapEnabled: false,
					minWidth: valueWidth,
					disabled: fieldValue.toString().match( /^function.+\(\) \{\s+\[native code\]\s+\}$/g ),
					style: go.baseStyle.values.any,
					click: editFunctionBody,
				}, insertChildIndex );
				field.style = go.baseStyle.values.func;
				break;

			// inspector:

			case 'object':
			case 'null':

				// if object is null
				if ( fieldValue === null && !readOnly ) {
					var dropdownItems = [
						{ text: "(null)", value: null },
					];
					// actions to change value from null
					if ( pdef.actions ) {
						dropdownItems = dropdownItems.concat( pdef.actions );
					}
					// show dropdown
					field = cont.addChild( './select', {
						name: pname,
						target: curTarget,
						change: function ( v, sel ) { if ( sel.action ) sel.action.call( go ); },
						minWidth: valueWidth,
						value: fieldValue,
						items: dropdownItems,
						style: go.baseStyle.values.any
					}, insertChildIndex );
					field.style = go.baseStyle.values.enum;

				} else {

					// inline option
					var inline = ( pdef && pdef.inline );

					// create field button
					field = cont.addChild( './button', {
						target: curTarget,
						name: pname,
						fieldValue: fieldValue,
						text: nameObject( fieldValue ),
						wrapEnabled: false,
						minWidth: valueWidth,
						disabled: (disabled || ( pdef && pdef.disabled )),
						style: go.baseStyle.values.any,
						toggleState: !!pdef.expanded
					}, insertChildIndex );
					field.style = go.baseStyle.values.object;
					if ( inline ) {
						field.click = togglePropList;
						field.image.angle = (field.toggleState ? 0 : -90);
						field.image.ui.offsetY = (field.toggleState ? 0 : 9);
					} else {
						field.click = pushToTarget;
						field.image.angle = -90;
						field.image.ui.offsetY = 9;
						field.label.flex = 1;
						field.reversed = true;
					}
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
			field.type = fieldType;
			field.pdef = pdef;
			field.focusGroup = ui.focusGroup;
			field.fieldLabel = label;
			if ( fieldType != 'object' && readOnly ) field.disabled = true;
			if ( disabled || pdef.disabled ) field.disabled = true;
			if ( typeof( pdef.style ) === 'object' ) {
				UI.base.applyProperties( field, pdef.style );
			}
			if ( typeof( pdef.hidden ) === 'function' ) {
				var shown = !pdef.hidden( curTarget );
				field.fieldLabel.active = shown;
				field.active = shown;
				if ( field.propList ) {
					field.propList.active = (shown && field.toggleState );
				}
			}
			if ( replaceField ) {
				// log( "replacing ", replaceField, " with new field", field, "at ", replaceAllFieldsIndex, "child index:", insertChildIndex,"field.parent = ", field.parent );
				if ( replaceField.propList ) {
					replaceField.propList.parent = null;
				}
				allFields.splice( replaceAllFieldsIndex, 1, field );
				replaceField.parent = null;
			} else {
				allFields.push( field );
				curTarget.watch( field.name, go.watchCallback );
			}
			label.field = field;
		}

		return field;
	}

	// recreates controls
	go.refresh = function ( restoreScrollPos ) {
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
		var _actions = [];
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
					if ( !found ) {
						_groups.push( g );
					}
				}
			}
			// showAll
			if ( typeof ( other.showAll ) === 'boolean' ) _showAll = other.showAll;
			// merge actions
			if ( typeof ( other.actions ) === 'object' && other.actions.length ) _actions = _actions.concat( other.actions );
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
				mergeConfig( target.__propertyListConfig );
			}
			// merge current properties etc
			mergeConfig( { properties: properties, groups: groups, showAll: showAll, inspector: customInspector, actions: actions } );

			// if properties are empty, set showall to true
			if ( !Object.keys( _properties ).length && _showAll === undefined ) _showAll = true;
		}

		// set header button name
		var bn = [];
		for ( var i = 0; i < targetStack.length; i++ ) {
			bn.push( targetStack[ i ].name );
		}
		bn.push( ( target && target.constructor ) ? target.constructor.name : ( target === undefined ? "Nothing selected" : "(null)" ) );
		backButton.text = bn.join( ' ^B->^n ' );
		backButton.disabled = ( targetStack.length == 0 );
		if ( targetStack.length ) {
			// < icon from \/ image
			backButton.icon = UI.style.propertyList.values.object.icon;
			backButton.image.angle = 90;
			backButton.image.ui.offsetY = -2;
			backButton.image.ui.offsetX = 8;
		} else {
			backButton.icon = "";
		}

		// remove extra buttons from header
		while ( header.numChildren > 2 ) header.removeChild( header.numChildren - 1 );

		// empty target
		if ( !target ) {
			moreButton.disabled = true;
			return;
		} else {
			moreButton.disabled = false;
		}

		// process actions .hidden and .disabled conditionals
		if ( _actions.length ) {
			moreButton.actions = [];
			for ( var i = 0; i < _actions.length; i++ ) {
				var a = _actions[ i ];
				if ( a.button ) {
					header.addChild ( './button', {
						text: a.text,
						icon: a.icon,
						targetUpdated: a.targetUpdated,
						action: a.action,
						click: function () { this.action.call( go ); },
						style: go.baseStyle.moreButton,
					} );
				} else {
					if ( typeof( a.disabled ) === 'function' ) {
						moreButton.actions.push( {
							text: a.text,
							disabled: a.disabled.call( go ),
							action: a.action
						} );
					} else if ( typeof( a.hidden ) === 'function' ) {
						if ( !a.hidden.call( go ) ) moreButton.actions.push( { text: a.text, action: a.action } );
					} else moreButton.actions.push( a );
				}
			}
		}
		moreButton.click = function () {
			// popup menu items
			var items = [ { text: "Reload object", action: function (){ this.refresh(); } }, ];
			// can add properties
			if ( !readOnly && showAll !== false ) {
				items.push( {
					text: "Add property", action: function () {
						this.addProperty();
					}
				} );
			}
			// save only if serializable
			if ( target.serializeable !== false ) {
				items.push( {
					text: "Serialize to JSON",
					action: function() { log( stringify( this.target ) ); }
				} );
			}
			// add object specific actions from .actions section of __propertyListConfig
			if ( this.actions.length ) {
				items.push( null );
				items = items.concat( this.actions );
			}
			// show popup
			var popup = new GameObject( './popup-menu', {
				target: this,
				items: items,
				selectedIndex: 0,
				selected: function ( s ) { s.action.call( this ); }.bind( go )
			} );
		};

		// hide header if it's empty
		header.active = header.numChildren > 2 || moreButton.active || backButton.active;

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
				var pdef = _properties[ pname ];
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

		// if restoreScrollPos is string, we'll be looking for this field to scroll into view
		var scrollToField = null;

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
					text: (i < ng ? _gs[ i ].name : 'Miscellaneous'),
					style: go.baseStyle.group,
				} );
				// clear top margin if first
				if ( i == 0 ) groupTitle.marginTop = 0;
			}
			// for each property
			for ( var j = 0, np = props.length; j < np; j++ ) {
				var pname = props[ j ];
				var pdef = _properties[ pname ] || { deletable: true };

				// skip native functions
				if ( typeof ( target[ pname ] ) === 'function' && target[ pname ].toString().match( /^function.+\(\) \{\s+\[native code\]\s+\}$/g ) ) {
					continue;
				}

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
				var field = go.makeField( ( pdef.target || target ), pname, pdef, label, cont );

				// if looking to scroll to a field
				if ( restoreScrollPos === pname ) {
					// we found it
					scrollToField = field;
				}

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
			function _showScrollbars() {
				this.scrollbars = 'auto';
				this.async( function() {
					if ( scrollToField ) {
						scrollToField.scrollIntoView();
					} else scrollable.scrollTop = (typeof( restoreScrollPos ) === 'number' ? restoreScrollPos : 0);
				}, 0.5 );
			}
			scrollable.debounce( 'showScrollbars', _showScrollbars, 0.1 );
		}

		// if this is an inline propList, update our button
		if ( go.fieldButton ) go.fieldButton.text = nameObject( target );

		// call update on header extra buttons
		updateHeaderActionsButtons();

		// clean up
		gc();

	}

	function updateHeaderActionsButtons() {
		for ( var i = 2; i < header.numChildren; i++ ) {
			var btn = header.getChild( i );
			if ( typeof( btn[ 'targetUpdated' ] ) === 'function' ) btn[ 'targetUpdated' ].call( go, btn );
		}
	}

	// single field changed, update field
	go.watchCallback = function ( p, ov, v ) {
		go.async( function() { go.reload( p ); } );
		//go.reload( p, v );
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

		// reload other fields option
		var reloads = this.pdef.reloadOnChange;

		// true = reload all
		if ( reloads === true ) {
			go.reload();
		// single other field
		} else if ( typeof( reloads ) === 'string' ) {
			go.reload( reloads );
		// array
		} else if ( reloads && typeof( reloads ) === 'object' && reloads.length ) {
			for ( var i in reloads ) go.reload( reloads[ i ] );
		}

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
			if ( val === null ) tp = 'null';

			// if field type has changed
			if ( ( field.type !== tp ) && field.type !== 'enum' ) {
				// re-create just this field
				field = go.makeField( field );
			}

			// have conditional hiding - apply
			if ( typeof( pdef.hidden ) === 'function' ) {
				var shown = !pdef.hidden( field.target );
				field.fieldLabel.active = shown;
				field.active = shown;
				if ( field.propList ) {
					field.propList.active = (shown && field.toggleState );
				}
			}

			// skip focused and hidden fields
			if ( field.focused || !field.active ) continue;

			// field is object
			if ( tp === 'object' ) {
				field.text = nameObject( val );
				if ( field.propList ) {
					if ( field.propList.target == val ) field.propList.reload();
					else field.propList.target = val;
				}
				field.fieldValue = val;
			// field is boolean
			} else if ( tp === 'boolean' ) {
				field.checked = val;
				field.text = ( val ? "True" : "False" );

			// other
			} else {
				field.value = val;
			}

			// if reload is called with field name, exit after updating
			if ( typeof( propName ) !== 'undefined' && field.name === propName ) break;
		}

		// if this is an inline propList, update our button
		if ( go.fieldButton ) go.fieldButton.text = nameObject( target );

		// update actions buttons
		updateHeaderActionsButtons();

	}

	go.pushToTarget = function( newTarget, propName ) {
		targetStack.push( {
			target: target,
			name: ((target && target.constructor) ? ( target.constructor.name + "." ) : '' ) + propName,
			scrollTop: go.container.scrollTop
		} );
		target = newTarget;
		go.refresh();
	}

	// right click menu
	ui.click = function ( btn, x, y, wx, wy ) {
		if ( btn == 3 && !disabled && showContextMenu ) {

			stopAllEvents();

			// find field right-clicked on
			var objs = go.container.query( wx, wy, 1, 1, true );
			var field = null;
			for ( var i in objs ) {
				var obj = objs[ i ];
				field = obj.field || ( obj.pdef ? obj : null );
				if ( field ) break;
			}
			// construct a menu
			var items = [];
			if ( field ) {
				// field actions
				if ( field.pdef.actions ) {
					items = items.concat( field.pdef.actions );
					items.push( null );
				}
				// reload property
				items.push( { text: "Re-load ^B" + field.name + "^b property", action: function () {
					this.reload( field.name );
				} } );
				// if not readonly mode
				if ( !readOnly ) {
					// field can be deleted
					if ( field.pdef.deletable === true ) {
						items.push( {
							text: "Delete ^B" + field.name + "^b property", action: function () {
								delete this.target[ field.name ];
								if ( typeof( this.target[ field.name ] ) === 'undefined' ) {
									log( "Deleted ^B" + field.name + "^b successfully." );
									this.refresh( this.container.scrollTop );
								} else {
									log( "Unable to delete ^B" + field.name + "^b property." );
								}
							}
						} );
						if ( field.type !== 'object' ) {
							items.push( {
								text: "Edit ^B" + field.name + "^b", action: function () {
									addProperty( field.name, field.type, field.target[ field.name ] );
								}
							} );
						}
					}
					// field is nullable
					if ( field.type === 'object' && field.fieldValue && field.pdef.nullable && !field.disabled ) {
						items.push( {
							text: "Set ^B" + field.name + "^b = ^Inull", action: function () {
								this.target[ field.name ] = null;
							}
						} );
					}
				}
			}
			// field or no field
			if ( !readOnly ) {
				// can add properties
				if ( showAll !== false ) {
					items.push( {
						text: "Add property", action: function () {
							this.addProperty();
						}
					} );
				}
			}
			// copy, paste
			items.push( null );
			items.push( {
				text: "Copy value", action: function () {
					this.copiedValue = { value: field.value || field.fieldValue, type: field.type };
				}
			} );
			if ( go.copiedValue && go.copiedValue.type == field.type && !readOnly && !field.disabled ) {
				items.push( {
					text: "Paste value (" + go.copiedValue.type + ")", action: function () {
						field.target[ field.name ] = this.copiedValue.value;
						this.reload( field.name );
					}
				} );
			}

			// common actions
			items.push( null );
			items = items.concat( moreButton.actions );

			// show popup menu at cursor
			if ( !items.length ) return;
			var popup = new GameObject( './popup-menu', {
				x: wx, y: wy,
				items: items,
				selectedIndex: 0,
				selected: function ( s ) { s.action.call( go ); }
			} );

		}
	}

	// shows dialog that lets create a new property
	go.addProperty = function ( forceName, forceType, forceValue ) {
		// Add property window
		var win = App.overlay.addChild( './window', {
			modal: true,
			minWidth: 300,
			pad: 8,
			title: forceName ? "Edit Property" : "Add Property",
			layoutType: Layout.Vertical,
			layoutAlignX: LayoutAlign.Stretch,
			layoutAlignY: LayoutAlign.Start,
			fitChildren: true,
		} );
		// instructions
		win.addChild( './text', {
			pad: 8,
			text: forceName ? "Edit existing property value." : "Enter new property name, type and value.",
			color: 0x0,
			wrap: true,
			bold: false,
		} );
		// name
		var nameLabel = win.addChild( './text', {
			pad: 8,
			text: "Name:",
			color: 0x0,
			bold: true,
		} );
		var propName = win.addChild( './textfield', {
			focusGroup: 'addProperty',
			editEnd: validate,
			text: forceName || "",
			disabled: !!forceName,
			change: function() { this.debounce( 'validate', validate, 1 ); }
		} );
		// type
		win.addChild( './text', {
			pad: 8,
			text: "Type",
			color: 0x0,
			bold: true,
		} );
		var propType = win.addChild( './select', {
			value: 'number',
			focusGroup: 'addProperty',
			items: [
				{ text: "Null", value: "null" },
				{ text: "Boolean", value: "boolean" },
				{ text: "Number", value: "number" },
				{ text: "String", value: "string" },
				{ text: "Array", value: "array" },
				{ text: "Function", value: "function" },
				{ text: "Object", value: "Object" },
				{ text: "GameObject", value: "GameObject" },
				{ text: "Color", value: "Color" },
				{ text: "Sound", value: "Sound" },
				{ text: "Image", value: "Image" },
				{ text: "Vector", value: "Vector" },
			],
			change: typeChanged
		} );
		// value
		var valLabel = win.addChild( './text', {
			pad: 8,
			text: "Value:",
			color: 0x0,
			bold: true,
		} );
		var propValue = win.addChild( './textfield', {
			focusGroup: 'addProperty',
			editEnd: validate,
			newLinesRequireShift: false,
			autoGrow: true,
			maxHeight: 14 * 8,
		} );
		var propValueDropdown = win.addChild( './select', {
			focusGroup: 'addProperty',
			change: validate,
			active: false,
		} );

		// Cancel, OK
		var btns = win.addChild( './panel', {
			layoutType: Layout.Horizontal,
			layoutAlignX: LayoutAlign.Stretch,
			spacing: 4,
			marginTop: 16,
		} );
		btns.addChild( './button', {
			text: "Cancel",
			focusGroup: 'addProperty',
			flex: 2,
			click: win.close
		} );
		var btnOk = btns.addChild( './button', {
			text: "Accept",
			focusGroup: 'addProperty',
			disabled: true,
			flex:3,
			click: function() {
				// get value
				var pendingValue = validate( "final" );
				if ( pendingValue === undefined ) return;
				win.close();
				// create property
				target[ propName.text ] = pendingValue;
				if ( target[ propName.text ] === pendingValue ) {
					log( "Set property ^B" + propName.text + "^b successfully" );
				} else {
					log( "Failed to write property ^B" + propName.text + "^b." );
				}
				go.refresh( propName.text ); // scroll to prop name
			}
		} );
		// callback
		function typeChanged ( v ) {

			var lbl = "Value:";
			var showDropdown = false;
			var valueDisabled = false;
			var autoGrow = false;
			var tabs = false;
			var val = '';
			var numeric = false;
			var isCode = false;
			switch ( v ) {
				case 'null':
					valueDisabled = true;
					val = 'null';
					break;
				case 'boolean':
					showDropdown = true;
					propValueDropdown.items = [
						{ text: "true", value: true },
						{ text: "false", value: false },
					];
					propValueDropdown.value = true;
					break;
				case 'number':
					numeric = true;
					val = '0';
					break;
				case 'string':
					autoGrow = true;
					tabs = true;
					break;
				case 'array':
					lbl = "Value: ^b(e.g.[1, 2, 3])";
					val = "[ ]";
					break;
				case 'Vector':
					lbl = "Value: ^b(e.g.[1, 2, 3])";
					val = "new Vector([ ])";
					break;
				case 'function':
					lbl = "Function:";
					val = "function () {\n\t\n}";
					autoGrow = true;
					tabs = true;
					isCode = true;
					break;
				default:
					val = 'new ' + v + '()';
					break;
			}
			valLabel.text = valLabel.defaultText = lbl;
			propValueDropdown.active = showDropdown;
			propValue.active = !showDropdown;
			propValue.disabled = valueDisabled;
			propValue.numeric = numeric;
			propValue.multiLine = !autoGrow;
			propValue.autoGrow = autoGrow;
			propValue.text = val;
			propValue.tabEnabled = tabs;
			if ( isCode ) {
				propValue.target = target;
				propValue.autocomplete = UI.base.autocompleteObjectProperty;
			} else {
				propValue.autocomplete = false;
			}
			validate();
		}

		// returns value
		function validate( final ){
			btnOk.disabled = true;
			// prop name
			var pname = propName.text;
			nameLabel.text = "Name:";
			var exists = false;
			if ( pname.length && !forceName ) {
				if ( typeof( target[ pname ] ) !== 'undefined' ) {
					nameLabel.text = "^3Warning: property already exists";
					exists = true;
				}
			} else if ( !pname.length ) return;

			// prop val
			var ptype = propType.value;
			var pval = propValue.text;
			var err = null;
			var value = undefined;
			valLabel.text = valLabel.defaultText;
			switch( ptype ) {
				case 'array':
					if ( !pval.match( /^\[[^]*\]$/ ) ) err = "Bad array syntax";
					else value = eval( pval );
					break;
				case 'Vector':
					if ( !pval.match( /^new Vector\((.+,)?\s*\[[^]*\]\s*\)$/ ) ) err = "Bad vector constructor syntax";
					else if ( final === 'final' ) value = eval( pval );
					break;
				case 'function':
					if ( !pval.match( /^function(\s+[a-zA-Z0-9_$]+)?\s*\([a-zA-Z0-9_$,\s]*\)\s*\{([^]*)\}$/ ) ) err = "Bad function syntax";
					else if ( final === 'final' ) {
						value = eval( "var _temp_=" + pval + ";_temp_;" ); // to allow nameless functions
					}
					break;
				case 'number':
					if ( final === 'final' ) value = parseFloat( pval );
					break;
				case 'boolean':
					if ( final === 'final' ) value = propValueDropdown.value;
					break;
				case 'string':
					if ( final === 'final' ) value = pval;
					break;
				case 'null':
					if ( final === 'final' ) value = null;
					break;
				default:
					if ( !pval.match( new RegExp( '^new '+ptype+'\\([^]*\\)$' ) ) ) err = "Bad constructor syntax";
					else if ( final === 'final' ) value = eval( pval );
					break;
			}
			if ( err ) {
				valLabel.text = "^2" + err;
				return;
			} else if ( final === 'final' && ( typeof( value ) === 'object' && value !== null && value.constructor.name.indexOf( 'Error' ) >= 0 ) ) {
				valLabel.text = "^2" + value.constructor.name + ": " + value.toString();
				return;
			}
			// pass
			btnOk.disabled = false;
			return value;
		}
		// edit property mode
		if ( forceValue !== undefined ) {
			if ( forceType ) propType.value = forceType;
			propType.change( propType.value );
			if ( forceType == 'boolean' ) {
				propValueDropdown.value = forceValue;
				propValueDropdown.focus();
			} else {
				propValue.text = String(forceValue);
				propValue.focus();
			}
		// add prop mode
		} else {
			propName.focus();
			propType.change( propType.value );
		}
	}

	// automatically clear target when removing from scene
	go.removed = function () {
		go.target = null;
	}

	// apply defaults
	go.baseStyle = UI.base.mergeStyle( {}, UI.style.propertyList );
	UI.base.applyProperties( go, go.baseStyle );
	go.values = go.values || { };
	go.state = 'auto';
	constructing = false;

})(this);

/*

Default inspector parameters for various built in classes in property list component

*/

App.__propertyListConfig = App.__propertyListConfig ||
{
	showAll: false,
	properties: {
		'scene': { nullable: true, actions: [ { text:"new Scene", action: function() { this.pushToTarget( this.target.scene = new Scene(), 'scene' ); } } ]},
	},
	groups: [
		{ name: "Scene", properties: [ 'scene' ] }
	],
}

Color.__propertyListConfig = Color.__propertyListConfig ||
{
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

GameObject.__propertyListConfig = GameObject.__propertyListConfig ||
{
	showAll: true,
	actions: [
		{ text: "Add child", action: function() { this.pushToTarget( this.target.addChild(), this.target.numChildren - 1 ); } },
		// { text: "Test Button", button: true, action: function(){ log( this ); }, targetUpdated: function( pl ) { log( this, pl ); } }
	],
	properties: {
		'name': true,
		'script': { reloadOnChange: true },
		'x': { step: 1 },
		'y': { step: 1 },
		'z': { step: 1 },
		'angle': { min: -360, max: 360, step: 1 },
		'scaleX': { step: 0.1 },
		'scaleY': { step: 0.1 },
		'skewX': { min: -90, max: 90, step: 1 },
		'skewY': { min: -90, max: 90, step: 1 },
		'scene': true,
		'parent': { nullable: true, },
		'children': { inline: true, readOnly: true },
		'active': true,
		'ignoreCamera': true,
		'renderAfterChildren': true,
		'render': { nullable: true, actions: [
			{ text:"new RenderSprite", action: function() { this.pushToTarget( this.target.render = new RenderSprite(), 'render' ); } },
			{ text:"new RenderShape", action: function() { this.pushToTarget( this.target.render = new RenderShape(), 'render' ); } },
			{ text:"new RenderText", action: function() { this.pushToTarget( this.target.render = new RenderText(), 'render' ); } },
		] },
		'opacity': { min: 0, max: 1, step: 0.1 },
		'body': { nullable: true, actions: [
			{ text:"new Body", action: function() { this.pushToTarget( this.target.body = new Body(), 'body' ); } }
		]},
		'ui': { nullable: true, actions: [
			{ text:"new UI", action: function() { this.pushToTarget( this.target.ui = new UI(), 'ui' ); } }
		] },
		'scale': { reloadOnChange: [ 'scaleY', 'scaleX' ] },
		'__propertyListConfig': false,
		'worldX': false, 'worldY': false, 'worldScaleX': false, 'worldScaleY': false, 'worldX': false,
	},
	groups: [
		{ name: "GameObject", properties: [ 'active', 'name', 'script', 'serializeable' ] },
		{ name: "Hierarchy", properties: [ 'scene', 'parent', 'children' ] },
		{ name: "Transform", properties: [ 'x', 'y', 'z', 'angle', 'scaleX', 'scaleY', 'skewX', 'skewY' ] },
		{ name: "Display", properties: [ 'render', 'opacity', 'ignoreCamera', 'renderAfterChildren', ] },
		{ name: "Physics", properties: [ 'body' ] },
		{ name: "UI", properties: [ 'ui' ] },
	]
}