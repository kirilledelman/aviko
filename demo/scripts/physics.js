new (function (){

	// create scene
	var scene = new Scene( {
		name: "Physics",
		backgroundColor: Color.Background,
		cameraX: App.windowWidth,
		gravityY: 100,
	} );

	scene.ui = new UI( {
		layoutType: Layout.Horizontal,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Stretch,
		pad: 20,
		wrapEnabled: true,
		fitChildren: false,
		spacing: 10,
		width: App.windowWidth,
		height: App.windowHeight,
	} );

	var leftColumn = scene.addChild( 'ui/panel', {
		flex: 1,
		pad: 0,
		minWidth: 150,
		spacing: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	// title
	var title = leftColumn.addChild( 'ui/text', {
		name: "Title",
		size: 30,
		color: Color.Title,
		bold: true,
		align: TextAlign.Center,
		wrap: false,
		text: "Physics",
	} );

	// scrollable description
	var description = leftColumn.addChild( 'ui/textfield', {
		size: 14,
		disabled: true,
		bold: false,
		states:{
			off: { background: false, },
			scroll: { background: 0xF0F0F0 },
			focus: { background: false },
			disabled: { background: false },
		},
		cornerRadius: 2,
		cancelToBlur: false,
		pad: 5,
		color: Color.Text,
		wrap: true,
		flex: 1,
		multiLine: true,
		focusRect: true,
		canScrollUnfocused: true,
		formatting: true,
		text:
		"Aviko physics is based on venerable open-source ^BBox2D^b library developed by Erin Catto.\n\n" +
		"To make a ^1^BGameObject^c^b participate in physics, assign an instance of ^B^1Body^c^b to its ^B.body^b property.\n\n" +
		"Body shape for collision detection is set by adding one or more ^1^BBodyShape^c^b objects to body's ^B.shapes^b array.\n\n" +
		"Bodies can be static, dynamic, or kinematic, and be connected to each other by several different types " +
		"of ^1^BJoint^c^b.\n\nBodies' collisions generate events, which can be filtered per-shape using bitmasks.\n\n"
		
	} );

	// back to main menu button
	var backButton = leftColumn.addChild( 'ui/button', {
		text: "Back to main menu",
		click: function () {
			sceneBack();
		}
	} );

	var rightColumn = scene.addChild( 'ui/panel', {
		pad: 0,
		minWidth: 300,
		selfAlign: LayoutAlign.Stretch,
		spacingY: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );

	var rightColumn = scene.addChild( 'ui/panel', {
		pad: 0,
		minWidth: 300,
		selfAlign: LayoutAlign.Stretch,
		spacingY: 5,
		layoutType: Layout.Vertical,
		layoutAlignX: LayoutAlign.Stretch,
		layoutAlignY: LayoutAlign.Start,
	} );
	
	// sample sprite container
	var container = rightColumn.addChild( 'ui/scrollable', {
		minHeight: 200,
		minWidth: 300,
		layoutType: Layout.None,
		scrollbars: false,
		layout: function () {
			this.scrollWidth = this.width;
			this.scrollHeight = this.height;
		},
	} );
	
	// background for sprite
	container.addChild( 'ui/image', {
		width: 300,
		height: 150,
		mode: 'fit',
		texture: 'checker.png',
	} );
	
	var bs = { label: { size: 12, bold: false, }, pad: 4 };
	
	// creates box for example
	function makeBox() {
		container.container.numChildren = 1;
		container.addChild( new GameObject({
			name: "Wall",
			render: new RenderShape( { shape: Shape.Rectangle, centered: false, x: 10, y: 200, color: 0x333333 } ),
			body: new Body( {
				type: BodyType.Static,
				shape: new BodyShape( {
					type: Shape.Rectangle,
					width: 10,
					height: 200,
	            } ),
	        } ),
			x: 0, y: 0,
	    }));
		container.addChild( new GameObject({
			name: "Wall",
			render: new RenderShape( { shape: Shape.Rectangle, centered: false, x: 10, y: 200, color: 0x333333 } ),
			body: new Body( {
				type: BodyType.Static,
				shape: new BodyShape( {
					type: Shape.Rectangle,
					width: 10,
					height: 200,
	            } ),
	        } ),
			x: 290, y: 0,
	    }));
		container.addChild( new GameObject({
			name: "Floor",
			render: new RenderShape( { shape: Shape.Rectangle, centered: false, x: 300, y: 10, color: 0x333333 } ),
			body: new Body( {
				type: BodyType.Static,
				shape: new BodyShape( {
					type: Shape.Rectangle,
					width: 300,
					height: 10,
	            } ),
	        } ),
			x: 0, y: 190,
	    }));
		return container.addChild( new GameObject({
			name: "Ceiling",
			render: new RenderShape( { shape: Shape.Rectangle, centered: false, x: 300, y: 10, color: 0x333333 } ),
			body: new Body( {
				type: BodyType.Static,
				shape: new BodyShape( {
					type: Shape.Rectangle,
					width: 300,
					height: 10,
	            } ),
	        } ),
			x: 0, y: 0,
	    }));
	}
	
	// example picker
	var selector = rightColumn.addChild( 'ui/select', {
		items: [ "Dynamic bodies", "Kinematic bodies", "Joints", "Sensors", "Particles" ],
		margin: [ 8, 4, 8, 4 ]
	} );
	
	// prototype for UI for draggable / controllable physics object
	var draggableObjectUIProto = {
		__proto__: UI.prototype,
		mouseOver: function (){
			this.gameObject.render.addColor = 0x003300;
		},
		mouseOut: function () {
			this.gameObject.render.addColor = 0x0;
		},
		mouseDown: function ( btn, x, y, wx, wy ) {
			this.focused = true;
			this.mouseJoint = this.gameObject.body.addJoint( new Joint( {
                type: JointType.Mouse,
                anchorX: x, anchorY: y,
			    mouseX: wx, mouseY: wy } ) );
			this.mouseDrag = this.mouseDrag.bind( this );
			Input.on( 'mouseMove', this.mouseDrag );
		},
		mouseUp: function () {
			this.mouseUpOutside();
		},
		mouseUpOutside: function () {
			if ( !this.mouseJoint ) return;
			this.gameObject.body.removeJoint( this.mouseJoint );
			this.mouseJoint = null;
			Input.off( 'mouseMove', this.mouseDrag );
		},
		mouseDrag: function ( x, y ) {
			this.gameObject.body.joint.setMouseTarget( x, y );
		},
		
		navigation: function ( name, val ) {
			// ignore if dragging with mouse
			if ( this.mouseJoint ) return;
			// up/down
			if ( name == 'vertical' ) {
				this.gameObject.body.velocityY += val * 120;
			// left/right
			} else if ( name == 'horizontal' ) {
				this.gameObject.body.velocityX += val * 120;
			// escape/cancel
			} else if ( name == 'cancel' ) {
				selector.focus();
			// enter/accept
			} else if ( name == 'accept' ) {
				// find next object with body and ui to focus on
				var objs = this.gameObject.parent.children;
				var index = objs.indexOf( this.gameObject ), i = (index + 1) % objs.length, next = null;
				while ( index != i ) {
					if ( objs [ i ].body && objs [ i ].ui ) {
						next = objs[ i ]; break;
					}
					i = (i + 1) % objs.length;
				}
				if ( next ) next.ui.focus();
			}
			stopEvent();
		},
		
		focusChanged: function ( nf ) {
			if ( nf == this ) {
				this.gameObject.render.outlineColor = 0x00FF00;
			} else {
				this.gameObject.render.outlineColor = 0x0;
			}
		}
		
	}
	
	// example 1
	var e1 = rightColumn.addChild( 'ui/panel', {
		layoutType: Layout.Vertical,
		active: false,
		children: [
			new GameObject( 'ui/text', { color: 0x0, size: 12, multiLine: true,
				text:
				"^BBasic dynamic bodies example^b\n\n" +
				"Focus on objects, and use:\nLeft/Right/Up/Down to move\n" +
				"Accept/Enter to cycle object\nBack/Escape to exit\n" +
				"Use mouse to drag objects"
			} )
		],
		activeChanged: function () {
			if ( this.active ) {
				makeBox();
				
				// ball
				var b = container.addChild( new GameObject({
					name: "Ball",
					render: new RenderShape( { shape: Shape.Circle, color: 0x663333, radius: 20, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Circle,
							radius: 20,
							density: 10,
							bounce: 0.2,
			            } ),
			        } ),
					x: 150, y: 20, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
				
				// rectangle
				var b = container.addChild( new GameObject({
					name: "Box",
					render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x666633, x: 40, y: 20, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Rectangle,
							x: 40, y: 20,
							density: 20,
							bounce: 0.2,
			            } ),
			        } ),
					x: 20, y: 10, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
				
				// generate star -> points
				var pts = [];
				var r0 = 25, r1 = 12, th = Math.PI * 0.2;
				for ( var i = 0; i < 5; i++ ) {
					pts.push( r0 * Math.cos( th * i * 2 ), r0 * Math.sin( th * i * 2 ), r1 * Math.cos( th * ( 1 + i * 2 ) ), r1 * Math.sin( th * ( 1 + i * 2 ) ) );
				}
				var b = container.addChild( new GameObject({
					name: "Star",
					render: new RenderShape( { shape: Shape.Polygon, points: pts, color: 0x333366, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Polygon,
							points: pts,
							density: 20,
							bounce: 0.2,
			            } ),
			        } ),
					x: 250, y: 40, angle: 45, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
			}
		}
	} );
	
	// example 2
	var e2 = rightColumn.addChild( 'ui/panel', {
		layoutType: Layout.Vertical,
		active: false,
		children: [
			new GameObject( 'ui/text', { color: 0x0, size: 12, multiLine: true,
				text:
				"^BKinematic body example^b\n\n" +
				"Kinematic bodies move with set velocity, but aren't\naffected by forces or collisions. Dynamic objects\n" +
				"collide and are pushed around by kinematic bodies."
			} )
		],
		activeChanged: function () {
			if ( this.active ) {
				makeBox();
				
				// balls
				for( var i = 0; i < 50; i++ ) {
					var b = container.addChild( new GameObject({
						name: "Ball",
						render: new RenderShape( { shape: Shape.Circle, color: 0x336099, radius: 10, lineThickness: 2 } ),
						body: new Body( {
							shape: new BodyShape( {
								type: Shape.Circle,
								radius: 10,
								density: 10,
								bounce: 0.5,
				            } ),
				        } ),
						x: 20 + 25 * ( i % 10 ),
                        y: 20 + 40 * Math.floor( i / 10 ), ui: new UI( { focusable: true } ),
				    }));
					b.ui.__proto__ = draggableObjectUIProto;
				}
				
				// mixer
				var b = container.addChild( new GameObject({
					name: "Mixer",
					render: new RenderShape( { shape: Shape.Rectangle, centered: true, color: 0xD0D0D0, x: 100, y: 20, lineThickness: 2 } ),
					body: new Body( {
						type: BodyType.Kinematic,
						shape: new BodyShape( {
							type: Shape.Rectangle,
							centerX: 50, centerY: 10,
							x: 100, y: 20,
			            } ),
						angularVelocity: 20,
			        } ),
					x: 150, y: 150,
			    }));
			}
		}
	} );
	
	// example 3
	var e3 = rightColumn.addChild( 'ui/panel', {
		layoutType: Layout.Vertical,
		active: false,
		children: [
			new GameObject( 'ui/text', { color: 0x0, size: 12, multiLine: true,
				text:
				"^BJoints example^b\n\nJoints connect two bodies, constraining movement\n" +
				"and rotation in various ways. Joint types:\n\n" +
				"Ball with cap - Weld joint\nBridge - Revolute joint\nCar - Wheel joints\nSliding ball - Prismatic joint\n" +
				"Squares - Distance joint\nDragging objects with mouse - Mouse joint"
			} )
		],
		activeChanged: function () {
			if ( this.active ) {
				var ceiling = makeBox();
				var b, prev, c, pts;
				// Revolute joint
				for ( var i = 0; i < 8; i++ ) {
					b = container.addChild( new GameObject({
						name: "Box",
						render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x666633, x: 34, y: 10, lineThickness: 2 } ),
						body: new Body( {
							shape: new BodyShape( {
								type: Shape.Rectangle,
								x: 34, y: 10,
				            } ),
				        } ),
						x: 15 + i * 34, y: 60, ui: new UI( { focusable: true } ),
				    }));
					b.ui.__proto__ = draggableObjectUIProto;
					
					// first - anchor
					if ( i == 0 ) {
						b.body.addJoint( new Joint({
							type: JointType.Revolute,
							otherBody: ceiling.body,
							anchorX: 0, otherAnchorX: b.x,
							anchorY: 5, otherAnchorY: b.y + 5,
						} ) );
					// mid
					} else {
						prev.body.addJoint( new Joint({
							type: JointType.Revolute,
							otherBody: b.body,
							anchorX: 34, otherAnchorX: 0,
							anchorY: 5, otherAnchorY: 5,
						}));
					}
					// last - anchor
					if ( i == 7 ) {
						b.body.addJoint( new Joint({
							type: JointType.Revolute,
							otherBody: ceiling.body,
							anchorX: 34, otherAnchorX: b.x + 34,
							anchorY: 5, otherAnchorY: b.y + 5,
						} ) );
					}
					prev = b;
				}
				
				// Weld joint
				b = container.addChild( new GameObject({
					name: "Ball",
					render: new RenderShape( { shape: Shape.Circle, color: 0x333333, radius: 10, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Circle,
							radius: 10,
							density: 30,
							bounce: 0.2,
			            } ),
						bullet: true,
			        } ),
					x: 150, y: 20, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
				pts = [ 0, -5, 10, 0, 0, 5 ];
				c = container.addChild( new GameObject({
					name: "BallCap",
					render: new RenderShape( { shape: Shape.Polygon, color: 0x993333, points: pts, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Polygon,
							points: pts,
							density: 30,
							bounce: 0.2,
			            } ),
						joint: new Joint( JointType.Weld, b.body ),
			        } ),
					x: 160, y: 20, ui: new UI( { focusable: true } ),
			    }));
				c.ui.__proto__ = draggableObjectUIProto;
				
				// Prismatic joint
				b = container.addChild( new GameObject({
					name: "Rail",
					render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x663366, x: 60, y: 10, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Rectangle,
							x: 60, y: 10,
			            } ),
			        } ),
					x: 50, y: 160, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
				c = container.addChild( new GameObject({
					name: "End",
					render: new RenderShape( { shape: Shape.Circle, color: 'D0D03399', radius: 10, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Circle,
							radius: 10,
			            } ),
		                joint: new Joint( {
		                    type: JointType.Prismatic,
							lowerLimit: -50, upperLimit: 5, limit: true,
			                axisX: 1, axisY: 0,
			                otherAnchorY: 5,
							otherBody: b.body,
		                } ),
			        } ),
					x: 55, y: 165, ui: new UI( { focusable: true } ),
			    }));
				c.ui.__proto__ = draggableObjectUIProto;
				
				// Distance joint
				b = container.addChild( new GameObject({
					name: "A",
					render: new RenderShape( { shape: Shape.Rectangle, centered: true, color: 0x993366, x: 15, y: 15, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Rectangle,
							centerX: 7.5, centerY: 7.5,
							x: 15, y: 15,
			            } ),
			        } ),
					x: 150, y: 160, ui: new UI( { focusable: true } ),
			    }));
				b.ui.__proto__ = draggableObjectUIProto;
				c = container.addChild( new GameObject({
					name: "B",
					render: new RenderShape( { shape: Shape.Rectangle, centered: true, color: 0x993366, x: 10, y: 10, lineThickness: 2 } ),
					body: new Body( {
						shape: new BodyShape( {
							type: Shape.Rectangle,
							centerX: 5, centerY: 5,
							x: 10, y: 10,
			            } ),
						joint: new Joint( {
							type: JointType.Distance,
							otherBody: b.body,
			                distance: 25 } )
			        } ),
					x: 185, y: 160, ui: new UI( { focusable: true } ),
			    }));
				c.ui.__proto__ = draggableObjectUIProto;

				// subfloor
				pts = [ 0, 0, 25, 10, 50, -5, 75, 15, 100, 0, 125, -5, 150, 10, 175, 10, 200, -5, 250, 15 ];
                b = container.addChild( new GameObject({
                    name: "Subfloor",
                    render: new RenderShape( { shape: Shape.Chain, color: 0x663366, points: pts, lineThickness: 2 } ),
                    body: new Body( {
						type: BodyType.Static,
                        shape: new BodyShape( {
                            type: Shape.Chain,
                            points: pts
                        } ),
                    } ),
                    x: 10, y: 120,
                }));

                // Wheel joint
                b = container.addChild( new GameObject({
                    name: "Car",
                    render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x663366, x: 40, y: 10, lineThickness: 2 } ),
                    body: new Body( {
                        shape: new BodyShape( {
                            type: Shape.Rectangle,
                            x: 40, y: 10,
                        } ),
                    } ),
                    x: 10, y: 100, ui: new UI( { focusable: true } ),
                }));
                b.ui.__proto__ = draggableObjectUIProto;
                c = container.addChild( new GameObject({
                    name: "Wheel1",
                    render: new RenderShape( { shape: Shape.Circle, color: 'D0306399', radius: 5, lineThickness: 2 } ),
                    body: new Body( {
                        shape: new BodyShape( {
                            type: Shape.Circle,
                            radius: 5,
                        } )
                    } ),
					x: 15, y: 110
                }));
                var d = container.addChild( new GameObject({
                    name: "Wheel2",
                    render: new RenderShape( { shape: Shape.Circle, color: 'D0306399', radius: 5, lineThickness: 2 } ),
                    body: new Body( {
                        shape: new BodyShape( {
                            type: Shape.Circle,
                            radius: 5,
                        } )
                    } ),
                    x: 45, y: 110
                }));
				b.body.joints = [ new Joint( {
                    type: JointType.Wheel,
                    axisX: 0, axisY: 1,
					anchorX: 5, anchorY: 15,
					motorSpeed: 15,
					damping: 0.9,
                    otherBody: c.body,
                } ), new Joint( {
                    type: JointType.Wheel,
                    axisX: 0, axisY: 1,
                    anchorX: 35, anchorY: 15,
                    motorSpeed: 15,
                    damping: 0.9,
                    otherBody: d.body,
                } ) ];
			}
		}
	} );
	
	// example 4
	var e4 = rightColumn.addChild( 'ui/panel', {
		layoutType: Layout.Vertical,
		active: false,
		children: [
			new GameObject( 'ui/text', { color: 0x0, size: 12, multiLine: true,
				text:
				"^BSensors^b\n\n" +
				"Sensors are body shapes that don't collide, but \ngenerate `touch` and `untouch` events when\n" +
				"other bodies enter and leave their area."
			} )
		],
		activeChanged: function () {
			if ( this.active ) {
				makeBox();
				
				
				// rectangle
				var b = container.addChild( new GameObject({
					name: "Box",
					render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x333333, x: 20, y: 60, lineThickness: 2 } ),
					body: new Body( {
						fixedRotation: true,
						shapes: [
							new BodyShape( {
								type: Shape.Rectangle,
								x: 20, y: 60,
								density: 20,
								bounce: 0.2,
				            } ),
							new BodyShape( {
								type: Shape.Circle,
								centerX: 10, centerY: 0,
								radius: 50,
								sensor: true,
				            } ) ],
						touch: function ( sh, o ) {
							// log( "touch", o.body.gameObject );
							if ( sh.sensor && o.body.gameObject.name == 'X' ) {
								o.body.gameObject.render.bold = true;
								o.body.gameObject.render.color = 0x990000;
							}
						},
						untouch: function ( sh, o ) {
							// log( "untouch", o.body.gameObject );
							if ( sh.sensor && o.body.gameObject.name == 'X' ) {
								o.body.gameObject.render.bold = false;
								o.body.gameObject.render.color = 0x003399;
							}
						},
			        } ),
					x: 150, y: 100, ui: new UI( { focusable: true } ),
			    }));
				b.addChild( new GameObject({ x: 10, render: new RenderShape( {
					shape: Shape.Circle, radius: 50, color: 'FFFF0099'
	            } ) }) );
				b.ui.__proto__ = draggableObjectUIProto;
				
				// generate objects
				for ( var i = 0; i < 5; i++ ) {
					b = container.addChild( new GameObject({
						name: "X",
						render: new RenderText( { text: ( i + 1 ), pivot: 0.5, size: 15, color: 0x003399 } ),
						body: new Body( {
							type: BodyType.Static,
							shape: new BodyShape( {
								type: Shape.Rectangle,
								centerX: 5, centerY: 5,
								x: 10, y: 10, sensor: true
				            } ),
				        } ),
						x: 20 + i * 50, y: 60 + Math.sin ( i ) * 50,
			        }));
				}
			}
		}
	} );

    // example 5
    var e5 = rightColumn.addChild( 'ui/panel', {
        layoutType: Layout.Vertical,
        active: false,
        children: [
            new GameObject( 'ui/text', { color: 0x0, size: 12, multiLine: true,
                text:
                "^BParticles^b\n\nCan be used to simulate liquids."
            } )
        ],
        activeChanged: function () {
            if ( this.active ) {
                makeBox();

                // rectangle
                var b = container.addChild( new GameObject({
                    name: "Box",
                    render: new RenderShape( { shape: Shape.Rectangle, centered: false, color: 0x666633, x: 40, y: 20, lineThickness: 2 } ),
                    body: new Body( {
                        shape: new BodyShape( {
                            type: Shape.Rectangle,
                            x: 40, y: 20,
                            density: 20,
                            bounce: 0.2,
                        } ),
                    } ),
                    x: 150, y: 15, ui: new UI( { focusable: true } ),
                }));
                b.ui.__proto__ = draggableObjectUIProto;

                // particles
                c = container.addChild( new GameObject({
                    name: "Particles",
                    render: new RenderParticles(),
                    body: new Particles( {
                        shape: new BodyShape( {
                            type: Shape.Rectangle,
                            x: 280, y: 50
                        } ),
						color: 0x333399,
						rigid: false,
						solid: true,
						flags: ParticleFlags.Tensile | ParticleFlags.StaticPressure,
                    } ),
                    x: 10, y: 135,
                }));

            }
        }
    } );

    App.debugDraw = true;

    // example selector activates panel corresponding to example
	selector.change = function () {
		var examples = [ e1, e2, e3, e4, e5 ];
		for ( i in examples ) examples[ i ].active = ( i == this.selectedIndex );
	};
	
	// select first example after scene initializes
	scene.async( function() {
		selector.selectedIndex = 4;
		selector.fire( 'change' );
		selector.focus();
	}, 0.5 );
	
	// backButton.focus();
	return scene;
})();