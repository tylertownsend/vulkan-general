"use strict"; // good practice - see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Strict_mode
////////////////////////////////////////////////////////////////////////////////
// Staircase exercise
// Your task is to complete the model for simple stairs
// Using the provided sizes and colors, complete the staircase
// and reach the Gold Cup!
////////////////////////////////////////////////////////////////////////////////
/*global THREE, Coordinates, $, document, window, dat*/

var camera, scene, renderer;
var cameraControls, effectController;
var clock = new THREE.Clock();
var gridX = false;
var gridY = false;
var gridZ = false;
var axes = false;
var ground = true;

function createStepProperties() {
	var stepWidth = 500;
	var stepSize = 200;
	var stepThickness = 50;
	// height from top of one step to bottom of next step up
	var verticalStepHeight = stepSize;
	var horizontalStepDepth = stepSize*2;

	var stepHalfThickness = stepThickness/2;
	return {
		stepWidth,
		stepSize,
		stepThickness,
		verticalStepHeight,
		horizontalStepDepth,
		stepHalfThickness
	};
}


function createVerticalStepPart(scene, stepProperties, position) {
	var stepMaterialVertical = new THREE.MeshLambertMaterial( {
		color: 0xA85F35
	} );

	var stepVertical = new THREE.CubeGeometry(stepProperties.stepWidth, stepProperties.verticalStepHeight, stepProperties.stepThickness);

	var stepMesh = new THREE.Mesh( stepVertical, stepMaterialVertical );
	stepMesh.position.x = position.x;
	stepMesh.position.y = stepProperties.verticalStepHeight/2 + position.y;	// half of height: put it above ground plane
	stepMesh.position.z = position.z;
	scene.add( stepMesh );
}

function createHorizontalStepPart(scene, stepProperties, position) {
	var stepMaterialHorizontal = new THREE.MeshLambertMaterial( {
		color: 0xBC7349
	} );


	var stepHorizontal = new THREE.CubeGeometry(stepProperties.stepWidth, stepProperties.stepThickness, stepProperties.horizontalStepDepth);

	var stepMesh = new THREE.Mesh( stepHorizontal, stepMaterialHorizontal );
	stepMesh.position.x = position.x;
	stepMesh.position.y = position.y + stepProperties.stepThickness/2 + stepProperties.verticalStepHeight;
	stepMesh.position.z = position.z + stepProperties.horizontalStepDepth/2 - stepProperties.stepHalfThickness;
	scene.add( stepMesh );
}

/**
 * We are getting the 
 * @param {*} scene 
 * @param {*} stepProperties 
 * @param {{x, y, z}} position The initial position for the axis of the current step.
 * @returns The new intial position for the following step to start.
 */
function createStep(scene, stepProperties, position) {
	// +Y direction is up
	createVerticalStepPart(scene, stepProperties, position);
	createHorizontalStepPart(scene, stepProperties, position);
}

function createStepStartingPosition(i, stepProperties) {
	var xPos = 0;
	var yPos = i * (stepProperties.stepThickness + stepProperties.verticalStepHeight);
	var zPos = i * (stepProperties.horizontalStepDepth - stepProperties.stepThickness);
	return { x: xPos, y: yPos, z:zPos };
}

function createStairs(cup) {
	var cupBasePosition = {x: cup.position.x, y: cup.position.y, z: cup.position.z };

	var stepProperties = createStepProperties();
	// debugger;
	var i = 0;
	var position = createStepStartingPosition(i, stepProperties);
	while (cupBasePosition.x >= position.x &&
				 cupBasePosition.y >= position.y &&
				 cupBasePosition.z >= position.z) {
		createStep(scene, stepProperties, position );
		position = createStepStartingPosition(++i, stepProperties);
	}
}

function createCup() {
	var cupMaterial = new THREE.MeshLambertMaterial( { color: 0xFDD017});
	// THREE.CylinderGeometry takes (radiusTop, radiusBottom, height, segmentsRadius)
	var cupGeo = new THREE.CylinderGeometry( 200, 50, 400, 32 );
	var cup = new THREE.Mesh( cupGeo, cupMaterial );
	cup.position.x = 0;
	cup.position.y = 1725;
	cup.position.z = 1925;
	scene.add( cup );

	var cupBaseGeo = new THREE.CylinderGeometry( 100, 100, 50, 32 );
	var cupBase = new THREE.Mesh( cupBaseGeo, cupMaterial );
	cupBase.position.x = 0;
	cupBase.position.y = 1525;
	cupBase.position.z = 1925;
	scene.add( cupBase );
	return cupBase;
}

function init() {
	var canvasWidth = 846;
	var canvasHeight = 494;
	// For grading the window is fixed in size; here's general code:
	//var canvasWidth = window.innerWidth;
	//var canvasHeight = window.innerHeight;
	var canvasRatio = canvasWidth / canvasHeight;

	// RENDERER
	renderer = new THREE.WebGLRenderer( { antialias: true } );
	renderer.gammaInput = true;
	renderer.gammaOutput = true;
	renderer.setSize(canvasWidth, canvasHeight);
	renderer.setClearColorHex( 0xAAAAAA, 1.0 );

	// CAMERA
	camera = new THREE.PerspectiveCamera( 45, canvasRatio, 1, 40000 );
	camera.position.set( -700, 500, -1600 );
	// CONTROLS
	cameraControls = new THREE.OrbitAndPanControls(camera, renderer.domElement);
	cameraControls.target.set(0,600,0);

	// Camera(2) for testing has following values:
	// camera.position.set( 1225, 2113, 1814 );
	// cameraControls.target.set(-1800,180,630);

	fillScene();
}
function addToDOM() {
	var container = document.getElementById('container');
	var canvas = container.getElementsByTagName('canvas');
	if (canvas.length>0) {
		container.removeChild(canvas[0]);
	}
	container.appendChild( renderer.domElement );
}
function fillScene() {
	// SCENE
	scene = new THREE.Scene();
	scene.fog = new THREE.Fog( 0x808080, 3000, 6000 );
	// LIGHTS
	var ambientLight = new THREE.AmbientLight( 0x222222 );
	var light = new THREE.DirectionalLight( 0xFFFFFF, 1.0 );
	light.position.set( 200, 400, 500 );

	var light2 = new THREE.DirectionalLight( 0xFFFFFF, 1.0 );
	light2.position.set( -400, 200, -300 );

	scene.add(ambientLight);
	scene.add(light);
	scene.add(light2);

	if (ground) {
		Coordinates.drawGround({size:1000});
	}
	if (gridX) {
		Coordinates.drawGrid({size:1000,scale:0.01});
	}
	if (gridY) {
		Coordinates.drawGrid({size:1000,scale:0.01, orientation:"y"});
	}
	if (gridZ) {
		Coordinates.drawGrid({size:1000,scale:0.01, orientation:"z"});
	}
	if (axes) {
		Coordinates.drawAllAxes({axisLength:300,axisRadius:2,axisTess:50});
	}
	var cup = createCup();
	var stairs = createStairs(cup);
	scene.add(stairs);
}
//

function animate() {
	window.requestAnimationFrame(animate);
	render();
}

function render() {
	var delta = clock.getDelta();
	cameraControls.update(delta);
	if ( effectController.newGridX !== gridX || effectController.newGridY !== gridY || effectController.newGridZ !== gridZ || effectController.newGround !== ground || effectController.newAxes !== axes)
	{
		gridX = effectController.newGridX;
		gridY = effectController.newGridY;
		gridZ = effectController.newGridZ;
		ground = effectController.newGround;
		axes = effectController.newAxes;

		fillScene();
	}
	renderer.render(scene, camera);
}

function setupGui() {

	effectController = {

		newGridX: gridX,
		newGridY: gridY,
		newGridZ: gridZ,
		newGround: ground,
		newAxes: axes
	};

	var gui = new dat.GUI();
	gui.add(effectController, "newGridX").name("Show XZ grid");
	gui.add( effectController, "newGridY" ).name("Show YZ grid");
	gui.add( effectController, "newGridZ" ).name("Show XY grid");
	gui.add( effectController, "newGround" ).name("Show ground");
	gui.add( effectController, "newAxes" ).name("Show axes");
}



try {
	init();
	setupGui();
	addToDOM();
	animate();
} catch(e) {
	var errorReport = "Your program encountered an unrecoverable error, can not draw on canvas. Error was:<br/><br/>";
	$('#container').append(errorReport+e);
}
