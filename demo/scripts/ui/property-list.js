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
		'targetChanged'

*/

include( './ui' );
(function(go) {

	// API properties
	UI.base.propListPrototype = UI.base.propListPrototype || {

		__proto__: UI.base.componentPrototype,

		// (Boolean) - if true, all enumerable properties of object will be displayed,
		// if false, only ones in .properties
		get showAll(){ return this.__showAll; }, set showAll( v ){ this.__showAll = v; this.debounce( 'refresh', this.refresh ); },

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
		//          reloadOnChange: (Array), (String) - reload this/these properties on change, or (Boolean) true to reload all, or string "refresh" to refresh entire object
		//          liveUpdate: (Boolean) for numeric and string field force update as you type if true, or after pressing Enter if false
		//          validate: (Function) function called when field changes with new value as param, return (modified) value to accept, undefined to reject
		//          autocomplete: (String) autocomplete type for text box. 'file' is supported
		//          autocompleteParam: (String) autocomplete parameter - e.g. 'textures;png,jpg'
		//          formatting: (Boolean) for string fields - whether field should allow ^code formatting
		//
		//      for inline object editing, when an embedded property-list will be displayed, can override defaults with:
		//          properties: (Object) - apply properties to sub-property-list
		//          groups: (Object) - apply groups to sub-property-list
		//          showAll: (Boolean) - apply showAll param to sub-property-list
		//
		get properties(){ return this.__properties; }, set properties( v ){ this.__properties = v; this.debounce( 'refresh', this.refresh ); },

		// (Array) in form of [ { name: "Group name", properties: [ 'p1', 'p2', ... ] } ... ] to group and order properties
		//      Properties not listed in groups will appear in an automatically generated group after listed groups, alphabetically
		//      To override alphabetical order of default group, supply group without name: param
		get groups(){ return this.__groups; }, set groups( v ){ this.__groups = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : []; this.debounce( 'refresh', this.refresh ); },

		// (Array) in form of [ { text, button, targetUpdated, hidden, disabled, action } ... ]
		//      These will be added as additional actions to context menu, or as action buttons on top, if buttons = true
		//      params:
		//          text: (String) - action text
		//          button: (Boolean) - if true, will be added as button to the header, instead of context menu
		//          targetUpdated: (Function) - if button, this function will be called whenever any property is updated, 'this' is propertyList, param is button
		//          hidden: (Function) - if context menu, function called when constructing popup menu, return true to hide action, 'this' is propertyList
		//          disabled: (Function) - if context menu, function called when constructing popup menu, return true to disable action, 'this' is propertyList
		//          action: (Function) - function called to execute action. 'this' is propertyList
		get actions(){ return this.__actions; }, set actions( v ){ this.__actions = (v && typeof( v ) == 'object' && v.constructor == Array) ? v : []; },

		// (Object) target object whose properties are displayed in this property list
		get target(){ return this.__target; },
		set target( v ){
			this.__target = v;
			this.__targetStack.length = 0;
			this.fire( 'targetChanged', v );
			this.debounce( 'refresh', this.refresh );
		},

		// (Number) width of value fields
		get valueWidth(){ return this.__valueWidth; }, set valueWidth( v ) {
			this.__valueWidth = Math.max( 45, v );
			this.debounce( 'refresh', this.refresh );
		},

		// (Boolean) disable all fields
		get disabled(){ return this.__disabled; },
		set disabled( v ) {
			if ( this.__disabled != v ) {
				this.__disabled = v;
				this.debounce( 'refresh', this.refresh );
			}
		},

		// (Boolean) disable all fields except objects - can follow deeper into objects
		get readOnly(){ return this.__readOnly; },
		set readOnly( v ) {
			if ( this.__readOnly != v ) {
				this.__readOnly = v;
				this.debounce( 'refresh', this.refresh );
			}
		},
		
		// (Boolean) show context right click menu on fields
		get showContextMenu(){ return this.__showContextMenu; }, set showContextMenu( v ){ this.__showContextMenu = v; },

		// (Boolean) show [...] button
		get showMoreButton(){ return this.__moreButton.active; }, set showMoreButton( v ){ this.__moreButton.active = v; },

		// (ui/panel) reference to header (where object / navigation / extras are displayed)
		get header (){ return this.__header; },

		// (ui/button) reference to back button
		get backButton (){ return this.__backButton; },

		// (ui/button) reference to more button
		get moreButton (){ return this.__moreButton; },

		// (Boolean) should this property list be scrollable
		get scrollable(){ return this.__shouldScroll; },
		set scrollable( v ){
			if ( this.__shouldScroll != v ) {
				this.__shouldScroll = v;
				this.debounce( 'refresh', this.refresh );
			}
		},

		// (GameObject) container to which all fields are added
		get container (){ return ( this.__scrollable || this ); },

		// (GameObject) when displaying nested property list editors, this holds reference to the topmost one
		get topPropertyList(){ return this.__topPropertyList; }, set topPropertyList( v ){ this.__topPropertyList = v; },

		// (Number) or (Array[4] of Number [ top, right, bottom, left ] ) - inner padding
		get pad(){ return this.__pad; }, set pad( v ){ this.__pad = v; this.ui.requestLayout( 'pad' ); },

		// (Number) inner padding top
		get padTop(){ return this.__pad[ 0 ]; }, set padTop( v ){ this.__pad[ 0 ] = v; this.ui.requestLayout( 'padTop' ); },

		// (Number) inner padding right
		get padRight(){ return this.__pad[ 1 ]; }, set padRight( v ){ this.__pad[ 1 ] = v; this.ui.requestLayout( 'padRight' ); },

		// (Number) inner padding bottom
		get padBottom(){ return this.__pad[ 2 ]; }, set padBottom( v ){ this.__pad[ 2 ] = v; this.ui.requestLayout( 'padBottom' ); },

		// (Number) inner padding left
		get padLeft(){ return this.__pad[ 3 ]; }, set padLeft( v ){ this.__pad[ 3 ] = v; this.ui.requestLayout( 'padLeft' ); },

		// (Number) spacing between children when layoutType is Grid, Horizontal or Vertical
		get spacing(){ return Math.max( this.__spacingX, this.__spacingY ); }, set spacing( v ){ this.__spacingX = this.__spacingY = v; this.ui.requestLayout( 'spacing' ); },

		// (Number) spacing between label and value
		get spacingX(){ return this.__spacingX; }, set spacingX( v ){ this.__spacingX = v; this.ui.requestLayout( 'spacingX' ); },

		// (Number) spacing between rows
		get spacingY(){ return this.__spacingY; }, set spacingY( v ){ this.__spacingY = v; this.ui.requestLayout( 'spacingY' ); },

		// (String) - when moving focus with Tab or arrows/controller, will only consider control with same focusGroup
		get focusGroup(){ return this.ui.focusGroup; },
		set focusGroup( f ){
			this.__backButton.focusGroup = this.__moreButton.focusGroup = this.ui.focusGroup = f;
			for ( var i = 0, nf = this.__allFields.length; i < nf; i++ ) {
				this.__allFields[ i ].focusGroup = f;
			}
		},

		__layout: function ( w, h ) {
			var go = this.gameObject;
			var ui = go.ui;
			var scrollable = go.__scrollable;
			if ( scrollable ) {
				// make scrollable and its scrollbar fit in width
				var vsb = scrollable.verticalScrollbar;
				if ( vsb && vsb.active ) {
					scrollable.marginRight =  vsb.width + vsb.marginLeft;
				} else scrollable.marginRight = 0;
				scrollable.scrollWidth = w - scrollable.marginRight;
				scrollable.resize( scrollable.scrollWidth, h - ( go.__header.active ? go.__header.height : 0 ) );
				scrollable.spacingX = go.__spacingX;
				scrollable.spacingY = go.__spacingY;
				scrollable.pad = go.__pad;
				ui.pad = 0;
				go.__header.maxWidth = scrollable.width + scrollable.marginRight;
			} else {
				ui.spacingX = go.__spacingX;
				ui.spacingY = go.__spacingY;
				ui.pad = go.__pad;
			}
			// position more button in upper right
			if ( go.__moreButton.active ) {
				go.__header.marginRight = go.__header.padRight + go.__moreButton.width + go.__header.spacingX;
				go.__moreButton.setTransform( go.__header.width + go.__header.spacingX, go.__header.padTop );
				go.__header.minHeight = Math.max( go.__header.minHeight, go.__moreButton.height );
			} else {
				go.__header.marginRight = go.__moreButton.active ? (go.__header.padRight + go.__moreButton.width + go.__header.spacingX) : 0;
			}
			// update background
			if ( go.render ) go.render.resize( w, h );
		},
		
		__nameObject: function ( obj ) {
			if ( obj === null ) return '(null)';
			if ( obj === undefined ) return '(undefined)';
			if ( typeof( obj ) === 'object' ) {
				if ( obj.constructor === Array ) return 'Array[' + obj.length +']';
				if ( obj.constructor === Vector ) return 'Vector[' + obj.length +']';
				if ( obj.constructor === Color ) return '#' + obj.hex;
				if ( obj.constructor === Image ) return 'Image(' + obj.width + 'x' + obj.height + ')';
				if ( obj.constructor === GameObject || obj.constructor === Scene  ) {
					return ( obj.active ? '' : '^9' ) + ( obj.name.length > 0 ? ( obj.name ) : obj.constructor.name );
				}
				if ( obj.constructor === Body || obj.constructor === UI || obj.constructor === RenderText || obj.constructor === RenderSprite || obj.constructor === RenderShape ) {
					return ( obj.active ? '' : '^9' ) + obj.constructor.name;
				}
				return obj.constructor.name;
			}
			return String( obj );
		},
	
		// button callback to show inspector
		__togglePropList: function ( btn ) {
			if ( btn != 1 ) return; // ignore right click
			var top = this.ownPropertyList;
			// embedded inspector created on demand
			if ( !this.propList ) {
				var myPos = this.parent.children.indexOf( this );
				this.propList = top.__getFieldOfType( 'object', './property-list', {
					name: this.name,
					flex: 1,
					readOnly: !!this.pdef.readOnly,
					scrollable: false,
					forceWrap: true,
					focusGroup: this.focusGroup,
					active: !this.pdef.expanded, // toggle
					style: top.__baseStyle.values.any,
					fieldButton: this,
					showContextMenu: top.__showContextMenu,
					showMoreButton: false,
					topPropertyList: top.__topPropertyList ? top.__topPropertyList : top,
					ownPropertyList: top,
					type: 'object',
					change: function () { this.fieldButton.text = top.__nameObject( this.target ); }
				} );
				this.parent.addChild( this.propList, myPos + 1 );
				this.propList.style = top.__baseStyle.values.inline;
				this.propList.valueWidth = top.__valueWidth - this.propList.marginLeft; // indent
				if ( this.pdef.showAll !== undefined ) this.propList.showAll = this.pdef.showAll;
				if ( this.pdef.properties !== undefined ) this.propList.properties = this.pdef.properties;
				if ( this.pdef.groups !== undefined ) this.propList.groups = this.pdef.groups;
				this.propList.target = this.fieldValue;
			} else {
				this.propList.active = !this.propList.active;
			}
			// toggle display
			this.toggleState = this.pdef.expanded = this.propList.active;
			this.image.angle = (this.propList.active ? 0 : -90);
			this.image.ui.offsetY = (this.propList.active ? 0 : 9);
		},
	
		__editFunctionBody: function ( btn ) {
			if ( btn != 1 ) return; // ignore right click
			var top = this.ownPropertyList;
			// force "edit value"
			top.__addProperty( this.name, 'Function', this.fieldValue, 'edit' );
		},
	
		// creates or replaces a field for property
		__makeField: function ( curTarget, pname, pdef, label, cont ) {
	
			var fieldValue, fieldType, replaceField = null, replaceAllFieldsIndex = -1, insertChildIndex = -1;
	
			// replace field mode?
			if ( arguments.length == 1 ) {
				replaceField = arguments[ 0 ];
				cont = replaceField.parent;
				replaceAllFieldsIndex = this.__allFields.indexOf( replaceField );
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
					if ( typeof( pdef.liveUpdate ) === 'undefined' ) pdef.liveUpdate = true;
					field = cont.addChild( this.__getFieldOfType( fieldType, './textfield', {
						name: pname,
						target: curTarget,
						change: pdef.liveUpdate === false ? undefined : this.__fieldChanged,
						editEnd: pdef.liveUpdate === false ? this.__fieldChanged : undefined,
						numeric: true,
						minWidth: valueWidth,
						integer: ( pdef && pdef.integer !== undefined ) ? pdef.integer : false,
						min: ( pdef && pdef.min !== undefined ) ? pdef.min : -Infinity,
						max: ( pdef && pdef.max !== undefined ) ? pdef.max : Infinity,
						step: ( pdef && pdef.step !== undefined ) ? pdef.step : 1,
						value: fieldValue,
						style: this.__baseStyle.values.any
					} ), insertChildIndex );
					field.style = this.__baseStyle.values.number;
					break;
	
				case 'string':
					if ( typeof( pdef.liveUpdate ) === 'undefined' ) pdef.liveUpdate = false;
					field = cont.addChild( this.__getFieldOfType( fieldType, './textfield', {
						name: pname,
						target: curTarget,
						change: pdef.liveUpdate === false ? undefined : this.__fieldChanged,
						editEnd: pdef.liveUpdate === false ? this.__fieldChanged : undefined,
						minWidth: valueWidth,
						value: fieldValue,
						autoGrow: true,
						formatting: !!pdef.formatting,
						bold: !pdef.formatting,
						newLinesRequireShift: true,
						style: this.__baseStyle.values.any
					} ), insertChildIndex );
					field.style = this.__baseStyle.values.string;
					// supported autocomplete
					if ( pdef.autocomplete ) {
						switch( pdef.autocomplete ){
							case 'file':
							field.autocomplete = UI.base.autocompleteFilePath;
							field.autocompleteParam = pdef.autocompleteParam;
							break;
							case 'texture':
							field.autocomplete = UI.base.autocompleteTexturePath;
							field.autocompleteParam = (pdef.autocompleteParam || 'textures;png,jpg,jpeg');
							break;
						}
					}
					break;
	
				// dropdown:
	
				case 'enum':
					field = cont.addChild( this.__getFieldOfType( 'enum', './select', {
						name: pname,
						target: curTarget,
						change: this.__fieldChanged,
						minWidth: valueWidth,
						value: fieldValue,
						items: pdef.enum,
						style: this.__baseStyle.values.any
					} ), insertChildIndex );
					field.style = this.__baseStyle.values.enum;
					break;
	
				// check box:
	
				case 'boolean':
					field = cont.addChild( this.__getFieldOfType( 'bool', './checkbox', {
						name: pname,
						target: curTarget,
						change: this.__fieldChanged,
						checked: fieldValue,
						text: fieldValue ? "True" : "False",
						minWidth: valueWidth,
						style: this.__baseStyle.values.any
					} ), insertChildIndex );
					field.style = this.__baseStyle.values.boolean;
					break;
	
				case 'function':
					// if function is native, display as read-only
					field = cont.addChild( this.__getFieldOfType( 'button', './button', {
						name: pname,
						target: curTarget,
						fieldValue: fieldValue,
						text: "function " + fieldValue.name + "()",
						wrapEnabled: false,
						minWidth: valueWidth,
						disabled: fieldValue.toString().match( /^function.+\(\) \{\s+\[native code\]\s+\}$/g ),
						style: this.__baseStyle.values.any,
						click: this.__editFunctionBody,
					} ), insertChildIndex );
					field.style = this.__baseStyle.values.func;
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
						field = cont.addChild( this.__getFieldOfType( 'enum', './select', {
							name: pname,
							target: curTarget,
							change: function ( v, sel ) { if ( sel.action ) sel.action.call( go ); },
							minWidth: valueWidth,
							value: fieldValue,
							items: dropdownItems,
							style: this.__baseStyle.values.any
						} ), insertChildIndex );
						field.style = this.__baseStyle.values.enum;
	
					} else {
	
						// inline option - if function, invoke w value
						var inline = ( typeof( pdef.inline ) === 'function' ? pdef.inline( fieldValue ) : !!pdef.inline );
	
						// create field button
						field = cont.addChild( this.__getFieldOfType( 'button', './button', {
							target: curTarget,
							name: pname,
							fieldValue: fieldValue,
							text: this.__nameObject( fieldValue ),
							wrapEnabled: false,
							minWidth: valueWidth,
							style: this.__baseStyle.values.any,
							toggleState: !!pdef.expanded,
							reversed: !inline,
						} ), insertChildIndex );
						field.style = this.__baseStyle.values.object;
						
						if ( inline ) {
							field.click = this.__togglePropList;
							field.image.angle = (field.toggleState ? 0 : -90);
							field.image.ui.offsetY = (field.toggleState ? 0 : 9);
							if ( pdef.expanded ) field.async( function () { this.ownPropertyList.__togglePropList.call( this, 1 ); } );
						} else {
							field.click = this.__pushToTargetClicked;
							field.image.angle = -90;
							field.image.ui.offsetY = 9;
							field.label.flex = 1;
						}
					}
					break;
	
				default:
					field = cont.addChild( './textfield', {
						name: pname,
						minWidth: valueWidth,
						value: '(' + String(fieldValue) + ')',
						style: this.__baseStyle.values.any,
						disabled: true,
					}, insertChildIndex );
					field.style = this.__baseStyle.values.string;
					break;
			}
	
			// common properties
			if ( field ) {
				field.ownPropertyList = this;
				field.type = fieldType;
				field.pdef = pdef;
				field.focusGroup = this.ui.focusGroup;
				field.fieldLabel = label;
				field.disabled = ( fieldType == 'object' && pdef.disabled ) ||
					( fieldType != 'object' && ( pdef.readOnly || this.__readOnly || this.__disabled ) );
				label.tooltip = field.tooltip = ( pdef.tooltip ? pdef.tooltip : null );
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
					if ( replaceField.propList ) {
						replaceField.propList.parent = null;
					}
					this.__allFields.splice( replaceAllFieldsIndex, 1, field );
					replaceField.parent = null;
				} else {
					this.__allFields.push( field );
					curTarget.watch( field.name, this.__watchCallback );
				}
				label.field = field;
			}
	
			return field;
		},
	
		// recreates controls
		refresh: function ( restoreScrollPos, autoFocus ) {
			// set up container
			var cont = this;
			if ( this.__shouldScroll ) {
				if ( !this.__scrollable ) {
					this.__scrollable = this.addChild( './scrollable', {
						layoutType: Layout.Horizontal,
						layoutAlignX: LayoutAlign.Start,
						layoutAlignY: LayoutAlign.Start,
						marginRight: 0,
						focusGroup: this.ui.focusGroup,
						wrapEnabled: true,
						wrapAfter: 2,
						acceptToCycle: true,
						fitChildren: true,
						scrollbars: false,
						flex: 1
					} );
				}
				cont = this.__scrollable;
				this.ui.layoutType = Layout.Vertical;
				this.ui.layoutAlignX = LayoutAlign.Stretch;
				this.ui.layoutAlignY = LayoutAlign.Stretch;
				this.ui.fitChildren = false;
				this.__scrollable.scrollbars = 'auto';
	
			} else {
				if ( this.__scrollable ) this.__scrollable = null;
				this.ui.layoutType = Layout.Horizontal;
				this.ui.layoutAlignX = LayoutAlign.Start;
				this.ui.layoutAlignY = LayoutAlign.Start;
				this.ui.wrapEnabled = true;
				this.ui.wrapAfter = 2;
				this.ui.fitChildren = true;
			}
	
			// remove previous elements
			cont.removeAllChildren();
			cont.ui.height = cont.ui.minHeight = 0;
			for ( var i = 0, nf = this.__allFields.length; i < nf; i++ ) {
				var trg = this.__allFields[ i ].target;
				if ( trg ) trg.unwatch( this.__allFields[ i ].name );
			}
			this.__allFields.length = 0;
	
			// add header as first child
			this.addChild( this.__header, 0 );
	
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
							// same name? replace
							if ( _groups[ j ].name === g.name ) {
								_groups[ j ] = g;
								found = true;
								break;
							}
						}
						// new group
						if ( !found ) {
							// insert or push
							if ( g.pos !== undefined ) _groups.splice( g.pos, 0, g );
							else _groups.push( g );
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
	
			var target = this.__target;
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
				mergeConfig( { properties: this.__properties, groups: this.__groups, showAll: this.__showAll, inspector: this.__customInspector, actions: this.__actions } );
	
				// if properties are empty, set showall to true
				if ( !Object.keys( _properties ).length && _showAll === undefined ) _showAll = true;
			}
	
			// set header button name
			var bn = [];
			var spaces = '';
			for ( var i = 0; i < this.__targetStack.length; i++ ) {
				bn.push( spaces + this.__targetStack[ i ].name );
				spaces += ' ';
			}
			bn.push( ( target && target.constructor ) ?
				         ( spaces + "^B" + target.constructor.name + "^b" ) :
				         ( target === undefined ? "Nothing selected" : "(null)" ) );
			this.__backButton.text = bn.join( '\n' );
			this.__backButton.disabled = ( this.__targetStack.length == 0 );
			this.__backButton.active = ( this.topPropertyList == this );
			if ( this.__targetStack.length ) {
				// < icon from \/ image
				this.__backButton.icon = UI.style.propertyList.values.object.icon;
				this.__backButton.image.angle = 90;
				this.__backButton.image.ui.offsetY = -2;
				this.__backButton.image.ui.offsetX = 8;
			} else {
				this.__backButton.icon = '';
			}
	
			// remove extra buttons from header
			while ( this.__header.numChildren > 2 ) this.__header.removeChild( this.__header.numChildren - 1 );
	
			// process actions .hidden and .disabled conditionals
			if ( _actions.length ) {
				this.__moreButton.actions = [];
				for ( var i = 0; i < _actions.length; i++ ) {
					var a = _actions[ i ];
					if ( a.button ) {
						this.__header.addChild ( './button', {
							text: a.text,
							icon: a.icon,
							targetUpdated: a.targetUpdated,
							action: a.action,
							click: function () { this.action.call( this ); }.bind( this ),
							style: this.__baseStyle.moreButton,
						} );
					} else {
						if ( typeof( a.disabled ) === 'function' ) {
							this.__moreButton.actions.push( {
								text: a.text,
								disabled: a.disabled.call( this ),
								action: a.action
							} );
						} else if ( typeof( a.hidden ) === 'function' ) {
							if ( !a.hidden.call( go ) ) this.__moreButton.actions.push( { text: a.text, action: a.action } );
						} else this.__moreButton.actions.push( a );
					}
				}
			}
			
			this.__moreButton.click = function () {
				// popup menu items
				var items = [];
				if ( this.__target ) {
	
					items.push( { text: "Reload object", action: function () { this.refresh(); } } );
	
					// can add properties
					if ( !this.__readOnly && this.__showAll !== false ) {
						items.push( {
							text: "Add property", action: function () { this.__addProperty(); }
						} );
					}
	
					// load/save
					items.push( null );
				}
	
				items.push( {
					text: "Load object",
					action: function() {
						this.__browsePath(
							"Load Object",
	                        "Enter a path to a ^B.json^b file to instantiate a new object from, and load into ^B$0^b global variable.",
	                        true,
							function ( path ) {
								var obj = null;
								try {
									obj = load( path, true );
								} catch ( e ) {
									return "Error parsing JSON file.";
								}
								if ( obj ) {
									$0 = unserialize( obj );
									return true;
								} else {
									log( "Couldn't load file." );
									return "Couldn't load file.";
								}
							}
						);
					}
				} );
	
				// save only if serializable
				if ( this.__target && this.__target.serializeable !== false ) {
					items.push( {
						text: "Save object",
						action: function() {
							this.__browsePath(
								"Save Object",
		                        "Enter a path to a ^B.json^b file to serialize object currently in ^B$0^b global variable.",
		                        false,
								function ( path ) {
									if ( save( this.__target, path, true ) ) {
										log( "Saved ^B$0^b to ^I" + path + "^i" );
										return true;
									} else {
										log( "Couldn't save to file." );
										return false;
									}
								}
							);
						}
					} );
				}
	
				// add object specific actions from .actions section of __propertyListConfig
				if ( this.__target && _actions.length ) {
					items.push( null );
					items = items.concat( _actions );
				}
				// show popup
				var popup = new GameObject( './popup-menu', {
					target: this.__moreButton,
					items: items,
					selectedIndex: 0,
					selected: function ( s ) { s.action.call( this ); }.bind( this ),
				} );
			}.bind( this );
	
			// hide header if it's empty
			this.__header.active = this.__header.numChildren > 2 || this.__moreButton.active || this.__backButton.active;
	
			// if displaying a custom inspector
			if ( _customInspector ) {
				// initialize it
				this.__inspector = _customInspector( this, this.__target, _properties, _groups, _showAll );
				cont.addChild( this.__inspector );
				return;
			}
	
			// no target/stop here
			if ( !this.__target ) return;
	
			// displaying standard inspector
	
			// sort properties into groups
			var regroup = { ' ': [] }; // default (unsorted) group
			var unsortedGroup = null; // if unnamed group was supplied it will be put here
			var mappedProps = {};
	
			// copy each specified group into regroup
			var _gs = [];
			for ( var i in _groups ) {
				var g = _groups[ i ];
				// if properties is a string, split by ','
				if ( typeof ( g.properties ) === 'string' ) {
					g.properties = g.properties.split( ',' );
				}
				// group has name - normal group
				if ( typeof( g.name ) == 'string' ) {
					_gs.push( g );
					
				// group with no name is used to specify order of properties in Miscellaneous group
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
			var allProps = Object.getProperties( this.__target, true, true, true, true );
			for ( var i = 0, np = allProps.length; i < np; i++ ) {
				var p = allProps[ i ];
				var pdef = _properties[ p ];
				if ( ( _showAll === true && pdef !== false ) || // if displaying all properties and prop isn't excluded, or
					( _showAll !== true && pdef !== undefined && pdef !== false ) ) { // showing select properties, and prop is included
					// property not in any groups
					if ( mappedProps[ p ] === undefined && p.substr( 0, 1 ) != '_' ) {
						// put in default group
						regroup[ ' ' ].push( p );
					}
				}
			}
	
			// if restoreScrollPos is string, we'll be looking for this field to scroll into view
			var scrollToField = null;
			var numeric = /^\d+$/;
	
			// if order of ungrouped properties isn't provided, sort default group by name
			if ( !unsortedGroup ) {
				regroup[ ' ' ].sort( function ( a, b ) {
					if ( numeric.test( a ) && numeric.test( b ) ){
						a = parseInt( a );
						b = parseInt( b );
					}
					return a < b ? -1 : 1;
				});
			} else {
				regroup[ ' ' ] = unsortedGroup.properties;
			}
	
			// for each group
			var numRows = 0, numGroups = 0;
			var firstField = null;
			for ( var i = 0, ng = _gs.length; i <= ng; i++ ) {
				var props = i < ng ? regroup[ _gs[ i ].name ] : regroup[ ' ' ];
				if ( props === undefined || !props.length ) continue;
				numGroups++;
				var groupTitle = null;
				if ( ( i < ng || numGroups > 1 ) && ( i >= ng || _gs[ i ].name.length ) ) {
					// add group title
					groupTitle = cont.addChild( './text', {
						forceWrap: true,
						flex: 1,
						text: (i < ng ? _gs[ i ].name : 'Miscellaneous'),
						style: this.__baseStyle.group,
					} );
					// clear top margin if first
					if ( i == 0 ) groupTitle.marginTop = 0;
				}
	
				// for each property
				var numAdded = 0;
				for ( var j = 0, np = props.length; j < np; j++ ) {
					var pname = props[ j ];
					var pdef;
					
					// purely numeric? special # property
					if ( numeric.test( pname ) ) {
						pdef = _properties[ '#' ] || { deletable: true };
					} else pdef = _properties[ pname ] || { deletable: true };
	
					// skip disabled properties
					if ( pdef === false ) continue;
					
					// skip native functions
					if ( typeof ( this.__target[ pname ] ) === 'function' && this.__target[ pname ].toString().match( /^function.+\(\) \{\s+\[native code\]\s+\}$/g ) ) {
						continue;
					}
	
					// add label
					if ( typeof( pdef.label ) === 'function' ) labelText = pdef.label( pname );
					else if ( typeof ( pdef.label ) === 'string' ) labelText = pdef.label;
					var label = cont.addChild( './text', {
						text: pname,
						flex: 1,
						wrap: true,
						style: this.__baseStyle.label
					} );
	
					// add field
					var field = this.__makeField( ( pdef.target || this.__target ), pname, pdef, label, cont );
	
					// autofocus
					if ( !firstField ) firstField = field;
					else if ( autoFocus === pname ) firstField = field;
					
					// if looking to scroll to a field
					if ( restoreScrollPos === pname ) {
						// we found it
						scrollToField = field;
					}
	
					numRows++;
					numAdded++;
	
				}
	
				// if nothing added to group, remove header
				if ( numAdded === 0 && groupTitle ) {
					groupTitle.parent = null;
				}
	
			}
	
			// placeholder
			if ( numRows == 0 ) {
				var sv = String( this.__target );
				cont.addChild( './text', {
					selfAlign: LayoutAlign.Stretch,
					text: ( sv.length ? ("^B(" + sv + ")^b: ") : "" ) + "no editable properties",
					style: this.__baseStyle.empty,
				} );
			}
	
			// scroll to top, enable scrollbars
			if ( this.__scrollable ) {
				this.__scrollable.scrollLeft = this.__scrollable.scrollTop = 0;
				function _showScrollbars() {
					if ( scrollToField ) {
						scrollToField.scrollIntoView();
					} else this.scrollTop = (typeof( restoreScrollPos ) === 'number' ? restoreScrollPos : 0);
				}
				this.__scrollable.debounce( 'showScrollbars', _showScrollbars, 0.5 );
			}
			
			// autofocus
			this.async( function() {
				if ( scrollToField ) scrollToField.focus();
				else if ( autoFocus && firstField ) firstField.focus();
				this.requestLayout();
			}, 0.1 );
	
			// if this is an inline propList, update our button
			if ( this.fieldButton ) this.fieldButton.text = this.__nameObject( this.__target );
	
			// call update on header extra buttons
			this.__updateHeaderActionsButtons();
			
			// clean up
			gc();
	
		},
	
		__updateHeaderActionsButtons: function () {
			for ( var i = 2; i < this.__header.numChildren; i++ ) {
				var btn = this.__header.getChild( i );
				if ( typeof( btn[ 'targetUpdated' ] ) === 'function' ) btn[ 'targetUpdated' ].call( this, btn );
			}
		},
	
		// single field changed, update field
		__watchCallback: function ( p, ov, v ) {
			this.async( function() { this.reload( p ); } );
			this.fire( 'change', this.__target, p, ov, v );
			return v;
		},
	
		__fieldChanged: function ( val ) {
	
			// validate can modify value
			if ( typeof( this.pdef.validate ) == 'function' ) {
				val = this.pdef.validate( val );
				if ( val === undefined ) return;
			}
	
			// if field is boolean
			if ( this.type === 'boolean' ) {
				// update label
				this.text = ( val ? "True" : "False" );
			}
	
			// apply
			var oldVal = this.target[ this.name ];
			this.target[ this.name ] = val;
	
			// fire changed
			this.ownPropertyList.fire( 'change', this.target, this.name, val, oldVal );
	
			// callback
			if ( this.pdef.change ) this.pdef.change( this.target, this.name, val, oldVal );
	
			// reload other fields option
			var reloads = this.pdef.reloadOnChange;
			this.ownPropertyList.debounce( 'reload', function() {
				// refresh
				if ( reloads === 'refresh' ) {
					this.refresh();
				// true = reload all
				} if ( reloads === true ) {
					this.reload();
				// single other field
				} else if ( typeof( reloads ) === 'string' ) {
					this.reload( reloads );
				// array
				} else if ( reloads && typeof( reloads ) === 'object' && reloads.length ) {
					for ( var i in reloads ) this.reload( reloads[ i ] );
				}
			}, 0.1 );
		},
	
		// refreshes properties values in rows from target
		reload: function ( propName, valOverride ) {
	
			if ( !this.__target ) return;
	
			// update fields
			for ( var i = 0, nf = this.__allFields.length; i < nf; i++ ) {
				var field = this.__allFields[ i ];
				var pdef = field.pdef;
				var fieldTarget = field.target;
				var fieldName = field.name;
		        var val = fieldTarget ? fieldTarget[ fieldName ] : undefined;
	
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
					field = this.__makeField( field );
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
					field.text = this.__nameObject( val );
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
			if ( this.fieldButton ) this.fieldButton.text = this.__nameObject( this.__target );
	
			// update actions buttons
			this.__updateHeaderActionsButtons();
	
		},
	
		__pushToTarget: function( newTarget, propDesc, propName ) {
			var obj = {
				target: this.__target,
				name: ((this.__target && this.__target.constructor) ? ( this.__target.constructor.name + "." ) : '' ) + propDesc,
				scrollTop: this.container.scrollTop,
				propName: propName
			}
			this.__targetStack.push( obj );
			this.__target = newTarget;
			this.fire( 'targetChanged', newTarget );
			this.refresh( 0, true );
			
		},
	
		
		// button callback to push into an object
		__pushToTargetClicked: function ( btn ) {
			if ( btn != 1 ) return; // ignore right click
			
			// get full path
			var propName = this.name;
			var popName = propName; // field on which to focus when popping
			var top = this.ownPropertyList.__topPropertyList;
			var r = this.ownPropertyList;
			
			// nested property list
			while ( r && r !== top ) {
				popName = r.name;
				if ( r.target.constructor == Array ) {
					propName = r.name + '[' + propName + ']';
				} else {
					propName = r.name + '.' + propName;
				}
				r = r.ownPropertyList;
			}
			
			// push to target
			this.ownPropertyList.__topPropertyList.__pushToTarget( this.target[ this.name ], propName, popName );
		},
		
		// right click menu
		__click: function ( btn, x, y, wx, wy ) {
			var go = this.gameObject;
			if ( btn == 3 && !go.__disabled && go.__showContextMenu ) {
	
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
					if ( !go.__readOnly ) {
						// field can be deleted
						var isArray = ( go.__target.constructor == Array || go.__target.constructor == Vector );
						if ( field.pdef.deletable === true ) {
							items.push( {
								text: "Delete ^B" + field.name + "^b " + ( isArray ? "element" : "property"), action: function () {
									// array - use splice
									if ( /^\d+$/.test( field.name ) && isArray ) {
										this.__target.splice( parseInt( field.name ), 1 );
										this.refresh( this.container.scrollTop );
									// otherwise delete
									} else {
										delete this.__target[ field.name ];
										if ( typeof( this.__target[ field.name ] ) === 'undefined' ) {
											log( "Deleted ^B" + field.name + "^b successfully." );
											this.refresh( this.container.scrollTop );
										} else {
											log( "Unable to delete ^B" + field.name + "^b property." );
										}
									}
								}
							} );
							if ( field.type !== 'object' ) {
								items.push( {
									text: "Edit ^B" + field.name + "^b", action: function () {
										this.__addProperty( field.name, field.type, field.target[ field.name ], 'edit' );
									}
								} );
							}
						}
						// field is nullable
						if ( field.type === 'object' && field.fieldValue && field.pdef.nullable && !field.disabled ) {
							items.push( {
								text: "Set ^B" + field.name + "^b = ^Inull", action: function () {
									this.__target[ field.name ] = null;
								}
							} );
						}
						// copy, paste
						items.push( null );
						items.push( {
							text: "Copy", action: function () {
								UI.copiedValue = { value: field.value || field.fieldValue, type: field.type };
							}
						} );
						// if can paste
						if ( UI.copiedValue &&
							(UI.copiedValue.type == field.type || ( UI.copiedValue.type == 'object' && field.type == 'null' ) ) &&
							!readOnly && !field.disabled ) {
							var pasteName = go.__nameObject( UI.copiedValue.value );
							items.push( {
								text: "Paste (" + pasteName + ")", action: function () {
									field.target[ field.name ] = UI.copiedValue.value;
									this.reload( field.name );
								}
							} );
							if ( UI.copiedValue.type === 'object' ) {
								items.push( {
									text: "Paste clone", action: function () {
										field.target[ field.name ] = clone( UI.copiedValue.value );
										this.reload( field.name );
									}
								} );
							}
						}
					}
				}
				// field or no field
				if ( !go.__readOnly ) {
					// can add properties
					if ( showAll !== false ) {
						items.push( {
							text: "Add property", action: function () {
								this.__addProperty();
							}
						} );
					}
				}
				// common actions
				if ( go.__moreButton.actions.length ) {
					items.push( null );
					items = items.concat( go.__moreButton.actions );
				}
				// show popup menu at cursor
				if ( items.length > 0 ) {
					var popup = new GameObject( './popup-menu', {
						x: wx, y: wy,
						items: items,
						selectedIndex: 0,
						selected: function ( s ) { s.action.call( go ); }
					} );
				}
			}
		},
	
		// Load/Save filename dialog
		__browsePath: function ( title, prompt, mustExist, finished ) {
	
			// Add modal window
			var win = App.overlay.addChild( './window', {
				modal: true,
				minWidth: 300,
				pad: 8,
				title: title,
				layoutType: Layout.Vertical,
				layoutAlignX: LayoutAlign.Stretch,
				layoutAlignY: LayoutAlign.Start,
				fitChildren: true,
			} );
	
			// instructions
			win.addChild( './text', {
				pad: 8,
				text: prompt,
				color: 0x0,
				wrap: true,
				bold: false,
			} );
	
			// field
			var fileName = win.addChild( './textfield', {
				focusGroup: 'browsePath',
				text: this.__browseFileName || "",
				autocomplete: UI.base.autocompleteFilePath,
				autocompleteParam: 'json',
				change: function() { this.debounce( 'validate', validate, 1 ); }
			} );
			var status = win.addChild( './text', {
				pad: 4,
				text: "Enter path to a file.",
				color: 0x0,
				wrap: true,
				bold: false,
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
				focusGroup: 'browsePath',
				flex: 2,
				click: win.close.bind( win )
			} );
			var btnOk = btns.addChild( './button', {
				text: "Accept",
				focusGroup: 'browsePath',
				disabled: true,
				flex:3,
				click: function() {
					// call done with path
					var r = finished.call( this, fileName.text );
					// if done returned error, place it into status, keep editing
					if ( r !== true ) {
						status.text = r;
						btnOk.disabled = true;
						fileName.focus();
					} else {
						// accepted, close
						win.close();
						this.__browseFileName = fileName.text;
					}
				}.bind( this )
			} );
	
			// checks path, enables OK
			function validate() {
				// check if file exists
				var pathOk = ( fileName.text.replace( '.', '' ).split( '/' ).join( '' ).length > 0 );
				if ( pathOk ) {
					var exists = fileExists( fileName.text );
					if ( mustExist ) {
						if ( exists == 'directory' ) {
							status.text = "Directory OK, add file name.";
							btnOk.disabled = true;
						} else {
							status.text = exists ? "File found at path." : "Enter path to a file.";
							btnOk.disabled = ( !exists && mustExist );
						}
					} else {
						status.text = exists ? "File already exists. Will overwrite." : "Path OK";
						btnOk.disabled = false;
					}
				} else {
					status.text = "Enter path to a file.";
					btnOk.disabled = true;
				}
			}
	
			// initial focus
			fileName.focus();
		},
	
		// shows dialog that lets create a new property
		__addProperty: function ( forceName, forceType, forceValue, mode ) {
			if ( typeof ( mode ) === 'undefined' ) mode = 'add';
			var forceNameIsUndefined = ( typeof( forceName ) === 'undefined' );
			// Add modal window
			var win = App.overlay.addChild( './window', {
				modal: true,
				minWidth: 300,
				pad: 8,
				title: ( mode == 'add' ? "Add Property" : "Edit Property" ),
				layoutType: Layout.Vertical,
				layoutAlignX: LayoutAlign.Stretch,
				layoutAlignY: LayoutAlign.Start,
				fitChildren: true,
			} );
			// instructions
			win.addChild( './text', {
				pad: 8,
				text: ( mode == 'add' ? "Enter new property name, type and value." : "Edit property value." ),
				color: 0x0,
				wrap: true,
				bold: true,
			} );
			// name
			var nameLabel = win.addChild( './text', {
				pad: 8,
				text: "Name:",
				color: 0x0,
				bold: true,
			} );
			var targetIsArray = ( this.__target.constructor.name === 'Array' || this.__target.constructor.name === 'Vector' );
			var propName = win.addChild( './textfield', {
				focusGroup: 'addProperty',
				editEnd: validate,
				text: forceNameIsUndefined ? "" : forceName.toString(),
				disabled: !forceNameIsUndefined,
				numeric: targetIsArray && !forceNameIsUndefined,
				integer: true,
				min: 0, step: 1,
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
				value: forceType ? forceType : 'Number',
				focusGroup: 'addProperty',
				items: [
					{ text: "Null", value: "null" },
					{ text: "Boolean", value: "Boolean" },
					{ text: "Number", value: "Number" },
					{ text: "String", value: "String" },
					{ text: "Array", value: "Array" },
					{ text: "Function", value: "Function" },
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
				click: win.close.bind( win )
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
					var pname = /^\d+$/.test( propName.text ) ? parseInt( propName.text ) : propName.text;
					this.__target[ pname ] = pendingValue;
					if ( this.__target[ pname ] === pendingValue ) {
						log( "Set property ^B" + propName.text + "^b successfully" );
					} else {
						log( "Failed to write property ^B" + propName.text + "^b." );
					}
					this.refresh( pname ); // scroll to prop name
				}.bind( this )
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
					case 'Boolean':
						showDropdown = true;
						propValueDropdown.items = [
							{ text: "true", value: true },
							{ text: "false", value: false },
						];
						propValueDropdown.value = true;
						break;
					case 'Number':
						numeric = true;
						val = '0';
						break;
					case 'String':
						autoGrow = true;
						tabs = true;
						break;
					case 'Array':
						lbl = "Value: ^b(e.g.[1, 2, 3])";
						val = "[ ]";
						break;
					case 'Vector':
						lbl = "Value: ^b(e.g.[1, 2, 3])";
						val = "new Vector([ ])";
						break;
					case 'Function':
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
					propValue.autocompleteParam = target;
					propValue.autocomplete = UI.base.autocompleteObjectProperty;
				} else {
					propValue.autocomplete = false;
				}
				validate();
			}
	
			// returns value
			var target = this.__target;
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
					case 'Array':
						if ( !pval.match( /^\[[^]*\]$/ ) ) err = "Bad array syntax";
						else value = eval( pval );
						break;
					case 'Vector':
						if ( !pval.match( /^new Vector\((.+,)?\s*\[[^]*\]\s*\)$/ ) ) err = "Bad vector constructor syntax";
						else if ( final === 'final' ) value = eval( pval );
						break;
					case 'Function':
						if ( !pval.match( /^function(\s+[a-zA-Z0-9_$]+)?\s*\([a-zA-Z0-9_$,\s]*\)\s*\{([^]*)\}$/ ) ) err = "Bad function syntax";
						else if ( final === 'final' ) {
							value = eval( "(" + pval + ");" ); // to allow nameless functions
						}
						break;
					case 'Number':
						if ( final === 'final' ) value = parseFloat( pval );
						break;
					case 'Boolean':
						if ( final === 'final' ) value = propValueDropdown.value;
						break;
					case 'String':
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
		},
	
		// returns reused or new field with script=type
		__getFieldOfType: function ( pool, script, initObj ) {
			var fld;
			if ( this.__cachedFields[ pool ] && this.__cachedFields[ pool ].length ) {
				var fld = this.__cachedFields[ pool ].pop();
				delete initObj[ 'style' ];
				for ( var p in initObj ) { fld[ p ] = initObj[ p ]; }
				fld.active = true;
			} else {
				fld = new GameObject( script, initObj );
			}
			fld.pool = pool;
			fld.removed = this.__returnFieldToCached;
			return fld;
		},
	
		// set as .removed to fields
		__returnFieldToCached: function () {
			var pool = this.pool;
			var top = this.ownPropertyList;
			delete this.propList;
			if ( typeof( top.__cachedFields[ pool ] ) === 'undefined' ) top.__cachedFields[ pool ] = [ this ];
			else top.__cachedFields[ pool ].push( this );
		},
	
		// automatically clear target when removing from scene
		removed: function () { this.target = null; }
	};
	
	// initialize
	go.name = "PropertyList";
	go.ui = new UI( {
		focusable: false,
		layoutType: Layout.Vertical,
		layout: UI.base.propListPrototype.__layout,
		click: UI.base.propListPrototype.__click,
	} );
	go.__scrollable = null;
	go.__shouldScroll = true;
	go.__header = new GameObject( './panel', {
		layoutType: Layout.Horizontal,
		layoutAlignY: LayoutAlign.End,
		layoutAlignX: LayoutAlign.Start,
		wrapEnabled: true,
		forceWrap: true,
		fitChildren: true,
	} );
	go.__backButton = go.__header.addChild( './button', {
		text: "Nothing selected",
		focusGroup: go.ui.focusGroup,
		flex: 1,
		click: function () {
			// pop to previously selected object
			if ( this.__targetStack.length ) {
				var pop = this.__targetStack[ this.__targetStack.length - 1 ];
				this.__target = pop.target;
				this.fire( 'targetChanged', this.__target );
				this.__targetStack.pop();
				this.refresh( pop.scrollTop, pop.propName );
			}
		}.bind( go ),
	} );
	go.__backButton.style = UI.style.propertyList.backButton;
	go.__moreButton = go.__header.addChild( './button', {
		text: "...",
		focusGroup: go.ui.focusGroup,
		actions: [],
		fixedPosition: true
	} );
	go.__moreButton.style = UI.style.propertyList.moreButton;
	go.__target = undefined;
	go.__targetStack = [];
	go.__showAll = undefined;
	go.__properties = {};
	go.__valueWidth = 130;
	go.__disabled = false;
	go.__readOnly = false;
	go.__showContextMenu = true;
	go.__topPropertyList = go;
	go.__groups = [];
	go.__allFields = [];
	go.__actions = [];
	go.__pad = [ 0, 0, 0, 0 ];
	go.__spacingX = 0;
	go.__spacingY = 0;
	go.__customInspector = null;
	go.__inspector = null;
	go.__cachedFields = {};
	go.__proto__ = UI.base.propListPrototype;
	go.__watchCallback = go.__watchCallback.bind( go );
	go.__init();
	go.serializeMask.push( 'target', 'children', 'removed' );
	
	// apply defaults
	go.__baseStyle = UI.base.mergeStyle( {}, UI.style.propertyList );
	UI.base.applyProperties( go, go.__baseStyle );
	go.values = go.values || { }; // ???????

})(this);

/*

Default inspector parameters for various built in classes in property list component

*/

App.__propertyListConfig = App.__propertyListConfig ||
{
	showAll: false,
	properties: {
		'scene': { nullable: true, actions: [ { text:"new Scene", action: function() { this.__pushToTarget( this.target.scene = new Scene(), 'scene' ); } } ]},
	},
	groups: [
		{ name: "", properties: [ 'scene' ] }
	],
}

Color.__propertyListConfig = Color.__propertyListConfig ||
{
	showAll: false,
	properties: {
		'r': { min: 0, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Red." },
		'g': { min: 0, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Green." },
		'b': { min: 0, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Blue." },
		'a': { min: 0, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Alpha (opacity)." },
		'hex': { style: { selectAllOnFocus: true, pattern: /^[0-9a-f]{0,8}$/i }, reloadOnChange: true, tooltip: "Color as a hexadecimal RGBA value." },
	},
	groups: [ { properties: [ 'r', 'g', 'b', 'a', 'hex' ] } ]
}

GameObject.__propertyListConfig = GameObject.__propertyListConfig ||
{
	showAll: true,
	actions: [
		{ text: "Add child", action: function() { this.__pushToTarget( this.target.addChild(), this.target.numChildren - 1 ); } },
	],
	properties: {
		'name': { tooltip: "Object name. It does not have to be unique." },
		'script': { reloadOnChange: 'refresh', tooltip: "Path to .js file defining this GameObject's functionality.", autocomplete: 'file', autocompleteParam: 'scripts;js' },
		'x': { step: 1, tooltip: "Horizontal position relative to parent." },
		'y': { step: 1, tooltip: "Vertical position relative to parent." },
		'z': { step: 1, tooltip: "Depth offset relative to parent." },
		'angle': { min: -360, max: 360, step: 1, tooltip: "Rotation in degrees." },
		'scaleX': { step: 0.1, tooltip: "Horizontal scale." },
		'scaleY': { step: 0.1, tooltip: "Vertical scale." },
		'skewX': { min: -90, max: 90, step: 1, tooltip: "Horizontal shear in degrees." },
		'skewY': { min: -90, max: 90, step: 1, tooltip: "Vertical shear in degrees." },
		'scene': { tooltip: "Reference to the scene containing this GameObject." },
		'parent': { nullable: true, reloadOnChange: 'scene', tooltip: "Reference to this GameObject's parent." },
		'children': { inline: true, readOnly: true, tooltip: "Children array. Use ^BaddChild^b, and ^BremoveChild^b, or ^Bparent^b property to manage children. Modifying this array's elements will have no effect. Setting ^Bchildren^b property to another array, however will overwrite children." },
		'active': { tooltip: "Controls GameObject's visibility and whether input and scene events are dispatched." },
		'eventMask': { inline: true, tooltip: "Event names added to this array will not be processed by GameObject or its descendents.",
			showAll: true,
			properties: {
				type: { disabled: true, readOnly: true },
			}
		},
		'ignoreCamera': { tooltip: "Disregard Scene's ^Bcamera...^b transforms when rendering." },
		'serializeable': { tooltip: "If property exists and is set to false, this object will not be serialized when calling ^Bserialize^b or ^Bclone^b." },
		'serializeMask': { tooltip: "If property exists, its elements define which other properties will not be serialized by ^Bserialize^b or ^Bclone^b calls." },
		'renderAfterChildren': { tooltip: "Draw this object ^B*after*^b its children, not before." },
		'render': { nullable: true, tooltip: "Instance of ^BRender____^b class, defining how this object is rendered.",
			actions: [
			{ text:"new RenderSprite", action: function() { this.__pushToTarget( this.target.render = new RenderSprite(), 'render' ); } },
			{ text:"new RenderShape", action: function() { this.__pushToTarget( this.target.render = new RenderShape(), 'render' ); } },
			{ text:"new RenderText", action: function() { this.__pushToTarget( this.target.render = new RenderText(), 'render' ); } },
		] },
		'opacity': { min: 0, max: 1, step: 0.1, tooltip: "Object's own opacity. Affects children.", },
		'body': { nullable: true, tooltip: "Instance of ^BBody^b class, allowing object to participate in physics simulation and collision detection.",
			actions: [
			{ text:"new Body", action: function() { this.__pushToTarget( this.target.body = new Body(), 'body' ); } }
		]},
		'ui': { nullable: true, tooltip: "Instance of ^BUI^b class allowing object to participate in layout, and receive user interface events.",
			actions: [
			{ text:"new UI", action: function() { this.__pushToTarget( this.target.ui = new UI(), 'ui' ); } }
		] },
		'worldX': false, 'worldY': false, 'worldScale': false, 'worldScaleX': false, 'worldScaleY': false,
		'worldAngle': false, 'scale': false, 'numChildren': false,
		'__propertyListConfig': false,

	},
	groups: [
		{ name: "", properties: [ 'active', 'name', 'script' ] },
		{ name: "Hierarchy", properties: [ 'scene', 'parent', 'children' ] },
		{ name: "Transform", properties: [ 'x', 'y', 'z', 'angle', 'scaleX', 'scaleY', 'skewX', 'skewY' ] },
		{ name: "Display", properties: [ 'render', 'opacity', 'ignoreCamera', 'renderAfterChildren', ] },
		{ name: "Physics", properties: [ 'body' ] },
		{ name: "UI", properties: [ 'ui' ] },
	]
}

Vector.__propertyListConfig = Vector.__propertyListConfig ||
{
	showAll: true,
	actions: [
		{ text: "Add element", action: function () { this.__addProperty( this.target.length, this.target.type ); } },
	],
	properties: {
		type: {
			enum: [
			{ text: "Boolean", value: "Boolean" },
			{ text: "Byte", value: "Byte" },
			{ text: "Integer", value: "Integer" },
			{ text: "Float", value: "Float" },
			{ text: "Number", value: "Number" },
			{ text: "String", value: "String" },
			{ text: "Array", value: "Array" },
			{ text: "Object", value: "Object" },
			{ text: "GameObject", value: "GameObject" },
			], style: { autoAddValue: true },
			reloadOnChange: 'refresh',
			tooltip: "Vector elements type.",
		},
		length: { min: 0, step: 1, liveUpdate: false, integer: true, reloadOnChange: 'refresh', tooltip: "Number of elements in Vector. Set to 0 to truncate." },
		array: { readOnly: true, reloadOnChange: true, inline: true, tooltip: "Vector values as array. Modifying this array's elements will have no effect. Setting ^Barray^b property to another Array, however will overwrite the values." },
		'#': { reloadOnChange: 'array', liveUpdate: false, deletable: true, inline: function (v) { return ( v && typeof(v) == 'object' && v.constructor == Color ); } },
		'__propertyListConfig': false,
	}
}

RenderShape.__propertyListConfig = RenderShape.__propertyListConfig ||
{
	showAll: true,
	properties: {
		active: { tooltip: "Render component is enabled." },

		shape: {
			enum: [
			{ text: "None", value: Shape.None },
			{ text: "Arc", value: Shape.Arc },
			{ text: "Circle", value: Shape.Circle },
			{ text: "Ellipse", value: Shape.Ellipse },
			{ text: "Line", value: Shape.Line },
			{ text: "Polygon", value: Shape.Polygon },
			{ text: "Rectangle", value: Shape.Rectangle },
			{ text: "Rounded Rectangle", value: Shape.RoundedRectangle },
			{ text: "Sector", value: Shape.Sector },
			{ text: "Triangle", value: Shape.Triangle },
			{ text: "Chain", value: Shape.Chain },
			], reloadOnChange: true,
			tooltip: "Shape type.",
		},

		blendMode: {
			enum: [
			{ text: "Normal", value: BlendMode.Normal },
			{ text: "Add", value: BlendMode.Add },
			{ text: "Subtract", value: BlendMode.Subtract },
			{ text: "Multiply", value: BlendMode.Multiply },
			{ text: "Screen", value: BlendMode.Screen },
			{ text: "Burn", value: BlendMode.Burn },
			{ text: "Dodge", value: BlendMode.Dodge },
			{ text: "Invert", value: BlendMode.Invert },
			{ text: "Color", value: BlendMode.Color },
			{ text: "Hue", value: BlendMode.Hue },
			{ text: "Saturation", value: BlendMode.Saturation },
			{ text: "Luminosity", value: BlendMode.Luminosity },
			{ text: "Refract", value: BlendMode.Refract },
			{ text: "Cut", value: BlendMode.Cut },
			], tooltip: "Blending operation with the background."
		},

		color: { inline: true, tooltip: "Base color." },
		addColor: { inline: true,
			properties: {
				'r': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional red." },
				'g': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional green." },
				'b': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional blue." },
				'a': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional alpha." },
				'hex': false,
			},
			tooltip: "Additive color."
		},
		outlineColor: { inline: true,
			hidden: function (t){ return t.lineThickness == 0 || ( t.shape == Shape.Polygon && !t.filled ) || t.shape == Shape.Chain; },
			tooltip: "The color of shape outline." },

		stipple: { min: 0, max: 1, step: 0.1, tooltip: "Stippling trasparency effect amount." },
		stippleAlpha: { tooltip: "Determines whether stippling is applied to alpha transparency." },

		lineThickness: { min: 0, step: 1, reloadOnChange: 'outlineColor',
			hidden: function (t){ return t.shape == Shape.Chain; }
		},
		filled: {
			reloadOnChange: [ 'lineThickness', 'outlineColor' ],
			hidden: function (t){ return t.shape == Shape.Chain || t.shape == Shape.Line; }
		},
		centered: { hidden: function (t){ return !( t.shape == Shape.Circle || t.shape == Shape.Ellipse || t.shape == Shape.Rectangle || t.shape == Shape.RoundedRectangle ); } },

		width: { hidden: function (t){ return !( t.shape == Shape.Rectangle || t.shape == Shape.RoundedRectangle || t.shape == Shape.Ellipse ); } },
		height: { hidden: function (t){ return !( t.shape == Shape.Rectangle || t.shape == Shape.RoundedRectangle || t.shape == Shape.Ellipse ); } },

		radius: { hidden: function (t){ return !( t.shape == Shape.Circle || t.shape == Shape.Sector || t.shape == Shape.RoundedRectangle ); } },
		innerRadius: { hidden: function (t){ return !( t.shape == Shape.Sector ); } },

		startAngle: { hidden: function (t){ return !( t.shape == Shape.Sector || t.shape == Shape.Arc ); } },
		endAngle: { hidden: function (t){ return !( t.shape == Shape.Sector || t.shape == Shape.Arc ); } },

		x: { hidden: function (t){ return !( t.shape == Shape.Line || t.shape == Shape.Arc || t.shape == Shape.Sector || t.shape == Shape.Triangle ); } },
		y: { hidden: function (t){ return !( t.shape == Shape.Line || t.shape == Shape.Arc || t.shape == Shape.Sector || t.shape == Shape.Triangle ); } },
		x1: { hidden: function (t){ return !( t.shape == Shape.Triangle ); } },
		x2: { hidden: function (t){ return !( t.shape == Shape.Triangle ); } },
		y1: { hidden: function (t){ return !( t.shape == Shape.Triangle ); } },
		y2: { hidden: function (t){ return !( t.shape == Shape.Triangle ); } },

		points: {
			inline: true,
			properties: {
				array: false,
				type: false,
				'#': { liveUpdate: true, deletable: true }
			},
			hidden: function (t){ return !( t.shape == Shape.Polygon || t.shape == Shape.Chain ); },
			tooltip: "Sequence of x, y coordinates for Polygon and Chain shapes.",
		},
		'__propertyListConfig': false,
	},
	groups: [
		{ name: "", properties:
			[ 'active', 'shape', 'centered', 'filled', 'width', 'height', 'radius', 'innerRadius', 'startAngle', 'endAngle',
				'x', 'y', 'x1', 'y1', 'x2', 'y2',
				'points', 'lineThickness', ] },
		{ name: "Blending", properties: [ 'blendMode', 'color', 'addColor', 'outlineColor', 'stipple', 'stippleAlpha' ] },
	]
}

RenderSprite.__propertyListConfig = RenderSprite.__propertyListConfig ||
{
	showAll: true,
	properties: {

		active: { tooltip: "Render component is enabled." },

		texture: {
			autocomplete: 'texture',
			tooltip: "Path to texture or texture frame."
		},

		width: { min: 0, step: 1, tooltip: "Texture rendered width." },
		height: { min: 0, step: 1, tooltip: "Texture rendered height." },
		originalWidth: { readOnly: true, tooltip: "Texture original width." },
		originalHeight: { readOnly: true, tooltip: "Texture original height." },

		image: { nullable: true, tooltip: "Set to an instance of ^BImage^b class to render a dynamic texture.",
			actions: [ { text:"new Image", action: function() { this.__pushToTarget( this.target.image = new Image(), 'image' ); } }
		] },

		pivotX: { step: 0.1, tooltip: "Sprite rotational pivot X." },
		pivotY: { step: 0.1, tooltip: "Sprite rotational pivot Y." },

		tileX: { step: 1, tooltip: "Number of times to tile texture tiling in X direction." },
		tileY: { step: 1, tooltip: "Number of times to tile texture tiling in Y direction." },
		autoTileX: { tooltip: "Width-based automatic tiling." },
		autoTileY: { tooltip: "Height-based automatic tiling." },

		flipX: { tooltip: "Flip texture horizontally." },
		flipY: { tooltip: "Flip texture vertically." },

        offsetX: { step: 1, tooltip: "Texture offset/scroll in X direction." },
        offsetY: { step: 1, tooltip: "Texture offset/scroll in Y direction." },

		slice: false,
		sliceLeft: { min: 0, step: 1, tooltip: "Left slicing boundary." },
		sliceTop: { min: 0, step: 1, tooltip: "Top slicing boundary." },
		sliceBottom: { min: 0, step: 1, tooltip: "Bottom slicing boundary." },
		sliceRight: { min: 0, step: 1, tooltip: "Right slicing boundary." },

		color: { inline: true, tooltip: "Multiplicative color." },
		addColor: { inline: true,
			properties: {
				'r': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional red." },
				'g': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional green." },
				'b': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional blue." },
				'a': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional alpha." },
				'hex': false,
			},
			tooltip: "Additive color."
		},

		stipple: { min: 0, max: 1, step: 0.1, tooltip: "Stippling trasparency effect amount." },
		stippleAlpha: { tooltip: "Determines whether stippling is applied to alpha transparency." },

		outlineColor: { inline: true, hidden: function( t ){ return (t.outlineRadius === 0 && t.outlineOffsetX === 0 && t.outlineOffsetY === 0 ); }, tooltip: "Color of sprite outline." },
		outlineOffsetX: { step: 1, reloadOnChange: 'outlineColor', tooltip: "Outline offset in X direction." },
		outlineOffsetY: { step: 1, reloadOnChange: 'outlineColor', tooltip: "Outline offset in Y direction." },
		outlineRadius: { min: -16, max: 16, step: 1, reloadOnChange: 'outlineColor', tooltip: "Sprite outline line thickness." },

		blendMode: {
			enum: [
			{ text: "Normal", value: BlendMode.Normal },
			{ text: "Add", value: BlendMode.Add },
			{ text: "Subtract", value: BlendMode.Subtract },
			{ text: "Multiply", value: BlendMode.Multiply },
			{ text: "Screen", value: BlendMode.Screen },
			{ text: "Burn", value: BlendMode.Burn },
			{ text: "Dodge", value: BlendMode.Dodge },
			{ text: "Invert", value: BlendMode.Invert },
			{ text: "Color", value: BlendMode.Color },
			{ text: "Hue", value: BlendMode.Hue },
			{ text: "Saturation", value: BlendMode.Saturation },
			{ text: "Luminosity", value: BlendMode.Luminosity },
			{ text: "Refract", value: BlendMode.Refract },
			{ text: "Cut", value: BlendMode.Cut },
			], tooltip: "Blending operation with the background."
		},
		'__propertyListConfig': false,
	},
	groups: [
		{ name: "", properties: [ 'active', 'texture', 'image', ] },
		{ name: "Transform", properties:
			[ 'width', 'height', 'originalWidth', 'originalHeight', 'pivotX', 'pivotY', 'flipX', 'flipY',
				'tileX', 'tileY', 'autoTileX', 'autoTileY', ] },
		{ name: "Outline", properties: [ 'outlineRadius', 'outlineOffsetX', 'outlineOffsetY', 'outlineColor' ] },
		{ name: "Blending", properties: [ 'blendMode', 'color', 'addColor', 'stipple', 'stippleAlpha' ] },
		{ name: "Slicing", properties: [ 'sliceLeft', 'sliceRight', 'sliceTop', 'sliceBottom' ] },
	]
}

RenderText.__propertyListConfigReloadWH = RenderText.__propertyListConfigReloadWH || [ 'width', 'height', 'scrollWidth', 'scrollHeight', 'numLines' ];
RenderText.__propertyListConfig = RenderText.__propertyListConfig ||
{
	showAll: true,
	properties: {
		
		active: { tooltip: "Render component is enabled." },
		dirty: false,
		text: { tooltip: "Text.", formatting: true, reloadOnChange: RenderText.__propertyListConfigReloadWH },
		size: { min: 4, max: 256, step: 1, tooltip: "Text size.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		
		textColor: { inline: true, tooltip: "Base text color." },
		backgroundColor: { inline: true, tooltip: "Background color." },
		
		font: { tooltip: "Base font" },
		boldFont: { tooltip: "Font used for bold characters (optional)." },
		italicFont: { tooltip: "Font used for italic characters (optional)." },
		boldItalicFont: { tooltip: "Font used for bold+italic characters (optional)." },
		bold: { tooltip: "Bold style.", reloadOnChange: [ 'width', 'height' ]},
		italic: { tooltip: "Italic style.", reloadOnChange: [ 'width', 'height' ]},
		outline: { tooltip: "Draw text as outline.", reloadOnChange: [ 'width', 'height' ]},
		antialias: { tooltip: "Smooth edges of characters." },
		lineSpacing: { tooltip: "Extra spacing between lines.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		characterSpacing: { tooltip: "Extra spacing between characters.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		
		align: {
			enum: [
			{ text: "Left", value: TextAlign.Left },
			{ text: "Center", value: TextAlign.Center },
			{ text: "Right", value: TextAlign.Right }
			], tooltip: "Text alignment." },
		multiLine: { tooltip: "Text drawn as multiple lines.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		wrap: { tooltip: "Auto-wrap at word boundaries when width is reached.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		autoSize: { tooltip: "Auto-resize to fit text.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		width: { min: 0, max: 4096, step: 1, integer: true, tooltip: "Text width.", reloadOnChange: [ 'height', 'scrollWidth', 'scrollHeight' ] },
		height: { min: 0, max: 4096, step: 1, integer: true, tooltip: "Text height." },
		pivotX: { min: 0, max: 1, step: 0.1, tooltip: "Transform origin X." },
		pivotY: { min: 0, max: 1, step: 0.1, tooltip: "Transform origin Y." },
		
		formatting: { tooltip: "Show inline formatting with ^^ codes.", reloadOnChange: RenderText.__propertyListConfigReloadWH },
		colors: { inline: true, tooltip: "Inline formatting ^^0-^^9 colors." },
		
		outlineColor: { inline: true, hidden: function( t ){ return (t.outlineRadius === 0 && t.outlineOffsetX === 0 && t.outlineOffsetY === 0 ); }, tooltip: "Color of outline." },
		outlineOffsetX: { step: 1, reloadOnChange: 'outlineColor', tooltip: "Outline offset in X direction." },
		outlineOffsetY: { step: 1, reloadOnChange: 'outlineColor', tooltip: "Outline offset in Y direction." },
		outlineRadius: { min: -16, max: 16, step: 1, reloadOnChange: 'outlineColor', tooltip: "Text outline line thickness." },

		showSelection: { tooltip: "Highlight selected text." },
		selectionStart: { min: 0, integer: true, tooltip: "First selected character." },
		selectionEnd: { min: 0, integer: true, tooltip: "Last selected character." },
		selectionTextColor: { inline: true, tooltip: "Text color for selected text." },
		selectionColor: { inline: true, tooltip: "Selection background color." },
		showCaret: { tooltip: "Show caret character." },
		caretPosition: { min: 0, step: 1, integer: true, tooltip: "Caret position in text." },
		
		scrollLeft: { tooltip: "Horizontal drawing offset." },
		scrollTop: { tooltip: "Horizontal drawing offset." },
		scrollWidth: { readOnly: true, tooltip: "Actual text width." },
		scrollHeight: { readOnly: true, tooltip: "Actual text height." },
		revealStart: { min: 0, integer: true, tooltip: "Number of characters to skip drawing from the beginning." },
		revealEnd: { min: 0, integer: true, tooltip: "Number of characters to skip drawing from the end." },
		numLines: { readOnly: true, tooltip: "Number of lines." },
		
		color: { inline: true, tooltip: "Multiplicative color." },
		addColor: { inline: true,
			properties: {
				'r': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional red." },
				'g': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional green." },
				'b': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional blue." },
				'a': { min: -1, max: 1, step: 0.1, reloadOnChange: true, tooltip: "Additional alpha." },
				'hex': false,
			},
			tooltip: "Additive color."
		},
		stipple: { min: 0, max: 1, step: 0.1, tooltip: "Stippling trasparency effect amount." },
		stippleAlpha: { tooltip: "Determines whether stippling is applied to alpha transparency." },
		blendMode: {
			enum: [
			{ text: "Normal", value: BlendMode.Normal },
			{ text: "Add", value: BlendMode.Add },
			{ text: "Subtract", value: BlendMode.Subtract },
			{ text: "Multiply", value: BlendMode.Multiply },
			{ text: "Screen", value: BlendMode.Screen },
			{ text: "Burn", value: BlendMode.Burn },
			{ text: "Dodge", value: BlendMode.Dodge },
			{ text: "Invert", value: BlendMode.Invert },
			{ text: "Color", value: BlendMode.Color },
			{ text: "Hue", value: BlendMode.Hue },
			{ text: "Saturation", value: BlendMode.Saturation },
			{ text: "Luminosity", value: BlendMode.Luminosity },
			{ text: "Refract", value: BlendMode.Refract },
			{ text: "Cut", value: BlendMode.Cut },
			], tooltip: "Blending operation with the background."
		},
		
	},
	groups: [
		{ name: "", properties: [ 'active', 'text', 'size', 'textColor', 'backgroundColor' ] },
		{ name: "Font", properties: [ 'font', 'boldFont', 'italicFont', 'boldItalicFont', 'bold', 'italic', 'antialias', 'outline', 'lineSpacing', 'characterSpacing' ] },
		{ name: "Alignment & Size", properties: [ 'align', 'multiLine', 'wrap', 'autoSize', 'width', 'height', 'pivotX', 'pivotY' ] },
		{ name: "Formatting", properties: [ 'formatting', 'colors' ] },
		{ name: "Selection & Caret", properties: [ 'showSelection', 'selectionStart', 'selectionEnd',
			'selectionTextColor', 'selectionColor', 'showCaret', 'caretPosition' ] },
		{ name: "Scrolling", properties: [ 'scrollLeft', 'scrollTop', 'scrollWidth', 'scrollHeight', 'revealStart', 'revealEnd', 'numLines' ] },
		{ name: "Outline", properties: [ 'outlineRadius', 'outlineOffsetX', 'outlineOffsetY', 'outlineColor' ] },
		{ name: "Blending", properties: [ 'blendMode', 'color', 'addColor', 'stipple', 'stippleAlpha' ] },
	]
}