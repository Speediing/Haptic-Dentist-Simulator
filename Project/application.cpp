//==============================================================================
/*
    \author    Your Name
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "MyProxyAlgorithm.h"
#include "MyMaterial.h"
#include "toothBrushCursor.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled 
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------
cVector3d debug_vect = cVector3d(0.0, 0.0, 0.0);
cShapeSphere * sphere0;

// a world that contains all objects of the virtual environment
cWorld* world;
//cShapeBox* menu;
// a camera to render the world in the window display
cCamera* camera;
cVector3d eye = cVector3d(1.0, 0.0, 0.5);
cVector3d lookat = cVector3d(0.0, 0.0, 0.0);

// a light source to illuminate the objects in the world
cSpotLight* light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a label to display the rates [Hz] at which the simulation is running
cLabel* labelRates;
cLabel* timeLabel;
cPrecisionClock* timer;

// a small sphere (cursor) representing the haptic device 
toothBrushCursor* tool;
double degrees;
double degrees2;
// a pointer to the custom proxy rendering algorithm inside the tool
MyProxyAlgorithm* proxyAlgorithm;

// nine objects with different surface textures that we want to render
cMultiMesh* scope;
cMultiMesh *objects[32];
cMesh* yes[32];
cMesh* gummesh;
cMesh* topgums;
cMesh* bottomgums;
cMultiMesh* gums;
bool start = true;
int highscore = 0;
// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = false;
cFontPtr plzWork;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width  = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);


//==============================================================================
/*
    TEMPLATE:    application.cpp

    Description of your application.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;
    cout << endl << endl;


    //--------------------------------------------------------------------------
    // OPENGL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();
    // set the background color of the environment
	world->m_backgroundColor.setBlueDarkTurquoise();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set( eye					,    // camera position (eye)
                 lookat					,    // look at position (target)
                 cVector3d (0.0, 1.0, 0.0));   // direction of the (up) vector

    // set the near and far clipping planes of the camera
    camera->setClippingPlanes(0.01, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.01);
    camera->setStereoFocalLength(0.5);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // create a directional light source
    light = new cSpotLight(world);

    // insert light source inside world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // position the light source
    light->setLocalPos(0.7, 0.3, 1.0);

    // define the direction of the light beam
    light->setDir(-0.5,-0.2,-0.8);

    // enable this light source to generate shadows
    light->setShadowMapEnabled(true);

    // set the resolution of the shadow map
    light->m_shadowMap->setQualityHigh();

    // set light cone half angle
    light->setCutOffAngleDeg(180);

    // use a point avatar for this scene
    double toolRadius = 0.001;
    //--------------------------------------------------------------------------
    // [CPSC.86] TEXTURED OBJECTS
    //--------------------------------------------------------------------------

	//How to load all the maps
	cImagePtr image = cImage::create();
	image->loadFromFile("images/unknown.png");
	//Has these funcitons
	//image->getPixelColorInterpolated()


    const double objectSpacing = 0.09;


	
	const std::string textureFile = "uv_teeth";

	for (int i = 0; i < 35; i++) {
		cMultiMesh* object = new cMultiMesh();

		object->loadFromFile("data/"+cStr(i+1)+".obj");
		object->createAABBCollisionDetector(toolRadius);
		object->computeBTN();
		yes[i] = object->getMesh(0);
		yes[i]->m_material = MyMaterial::create();
		yes[i]->m_material->setUseHapticShading(true);
		MyMaterialPtr material = dynamic_pointer_cast<MyMaterial>(yes[i]->m_material);
		object->setStiffness(1000.0, true);
		cTexture2dPtr albedoMap = cTexture2d::create();
		albedoMap->loadFromFile("data/sandpaper_colour.jpg");
		albedoMap->setWrapModeS(GL_REPEAT);
		albedoMap->setWrapModeT(GL_REPEAT);
		albedoMap->setUseMipmaps(true);
		cTexture2dPtr normalMap = cTexture2d::create();
		normalMap->loadFromFile("data/SandPaperNormalMap.png");
		normalMap->setWrapModeS(GL_REPEAT);
		normalMap->setWrapModeT(GL_REPEAT);
		normalMap->setUseMipmaps(true);
		material->normalMap = normalMap;
		material->obj = "teeth";
		yes[i]->m_texture = albedoMap;
		yes[i]->setUseTexture(true);
		object->setLocalPos(0.0, 0.0);
		world->addChild(object);
	}
	//menu = new cShapeBox();
	gums = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	gums->loadFromFile("data/gumsbottom.obj");
	gums->createAABBCollisionDetector(toolRadius);
	gums->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	gummesh = gums->getMesh(0);
	// replace the object's material with a custom one
	gummesh->m_material = MyMaterial::create();
	//gummesh->m_material->setRed();
	gummesh->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material2 = dynamic_pointer_cast<MyMaterial>(gummesh->m_material);
	gummesh->setStiffness(2000.0, true);


	cTexture2dPtr albedoMap2 = cTexture2d::create();
	// create a colour texture map for this mesh object

	albedoMap2->loadFromFile("data/gumcolor.jpg");
	albedoMap2->setWrapModeS(GL_REPEAT);
	albedoMap2->setWrapModeT(GL_REPEAT);
	albedoMap2->setUseMipmaps(true);

	cTexture2dPtr normalMap2 = cTexture2d::create();
	normalMap2->loadFromFile("data/bumppy.png");
	normalMap2->setWrapModeS(GL_REPEAT);
	normalMap2->setWrapModeT(GL_REPEAT);
	normalMap2->setUseMipmaps(true);


	material2->normalMap = normalMap2;
	material2->obj = "gums";
	material2->mesh = gummesh;

	gummesh->m_texture = albedoMap2;
	gummesh->setUseTexture(true);
	gums->setLocalPos(0.0, 0.0);

	world->addChild(gums);

	cMultiMesh* gums2 = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	gums2->loadFromFile("data/gumstop.obj");
	gums2->createAABBCollisionDetector(toolRadius);
	gums2->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	topgums = gums2->getMesh(0);
	// replace the object's material with a custom one
	topgums->m_material = MyMaterial::create();
	//gummesh->m_material->setRed();
	topgums->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material = dynamic_pointer_cast<MyMaterial>(topgums->m_material);
	topgums->setStiffness(2000.0, true);


	cTexture2dPtr albedoMap = cTexture2d::create();
	// create a colour texture map for this mesh object
	double highscore = 0;
	albedoMap->loadFromFile("data/gumcolor.jpg");
	albedoMap->setWrapModeS(GL_REPEAT);
	albedoMap->setWrapModeT(GL_REPEAT);
	albedoMap->setUseMipmaps(true);

	cTexture2dPtr normalMap = cTexture2d::create();
	normalMap->loadFromFile("data/bumppy.png");
	normalMap->setWrapModeS(GL_REPEAT);
	normalMap->setWrapModeT(GL_REPEAT);
	normalMap->setUseMipmaps(true);


	material->normalMap = normalMap2;
	material->obj = "gums";
	material->mesh = gummesh;

	topgums->m_texture = albedoMap2;
	topgums->setUseTexture(true);
	gums2->setLocalPos(0.0, 0.0);

	world->addChild(gums2);
	cMultiMesh* gums3 = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	gums3->loadFromFile("data/36.obj");
	gums3->createAABBCollisionDetector(toolRadius);
	gums3->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	bottomgums = gums3->getMesh(0);
	// replace the object's material with a custom one
	bottomgums->m_material = MyMaterial::create();
	bottomgums->m_material->setGreen();
	bottomgums->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material3 = dynamic_pointer_cast<MyMaterial>(bottomgums->m_material);
	bottomgums->setStiffness(2000.0, true);

	material3->obj = "gums";
	material3->mesh = bottomgums;
	gums3->setLocalPos(0.0, 0.0);

	//world->addChild(gums3);
	//gums3->rotateExtrinsicEulerAnglesDeg(M_PI / 2, 0, 0, C_EULER_ORDER_XYZ);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICE
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get a handle to the first haptic device
    handler->getDevice(hapticDevice, 0);

    // if the device has a gripper, enable the gripper to simulate a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    tool = new toothBrushCursor(world);
    world->addChild(tool);
	
	//Move to bumps bin
	//tool->setLocalPos(0.0, -objectSpacing, 0.0);

    // [CPSC.86] replace the tool's proxy rendering algorithm with our own
    proxyAlgorithm = new MyProxyAlgorithm;
    delete tool->m_hapticPoint->m_algorithmFingerProxy;
	
    tool->m_hapticPoint->m_algorithmFingerProxy = proxyAlgorithm;
    tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    tool->setRadius(0.001, toolRadius);

    tool->setHapticDevice(hapticDevice);

    tool->setWaitForSmallForce(true);
	scope = new cMultiMesh();

	// attach scope to tool
	tool->m_image->addChild(scope);
	tool->setLocalPos(0.5, 0.0, 0.25);
	// load an object file
	scope->loadFromFile("data/toothbrush.3ds");
	//tool->setShowContactPoints(false, false);
	scope->rotateExtrinsicEulerAnglesDeg(M_PI, M_PI, M_PI, C_EULER_ORDER_XYZ);
	scope->setUseCulling(false);
	scope->createAABBCollisionDetector(toolRadius);
	tool->updateToolImagePosition();

	// use display list for faster rendering
	scope->setUseDisplayList(true);
	scope->setLocalPos(0.0, 0.09, 0.02);
	tool->start();

	camera->set(eye,   // camera position (eye)
		lookat,   // look at position (target)
		cVector3d(0.0, 0.0, 1.0)	// direction of the (up) vector
	);
    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI144();
	cFontPtr maybe = cFont::create();
	if (maybe->loadFromFile("data/Blonde.ttf") == false) {
		cout << "yeah no good" << endl;
	}
    // create a label to display the haptic and graphic rates of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setWhite();
    camera->m_frontLayer->addChild(labelRates);
	timeLabel = new cLabel(font);
	timeLabel->m_fontColor.setWhite();
	camera->m_frontLayer->addChild(timeLabel);
	timer = new cPrecisionClock();
	timer->start();
    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width  = a_width;
    height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------
void reset(void) {
	int totalPoints = 0;
	for (int i = 0; i < 35; i++) {
		totalPoints += dynamic_pointer_cast<MyMaterial>(yes[i]->m_material)->points;
		dynamic_pointer_cast<MyMaterial>(yes[i]->m_material)->points = 0;
	}
	int Gumpoints = 4 * (dynamic_pointer_cast<MyMaterial>(gummesh->m_material)->points + dynamic_pointer_cast<MyMaterial>(topgums->m_material)->points);
	dynamic_pointer_cast<MyMaterial>(gummesh->m_material)->points = 0;
	dynamic_pointer_cast<MyMaterial>(topgums->m_material)->points = 0;
	int point = totalPoints - Gumpoints;
	if (point > highscore) {
		highscore = point;
	}
	for (int i = 0; i < 35; i++) {
		world->removeChild(yes[i]);
		yes[i] = NULL;
		cMultiMesh* object = new cMultiMesh();

		object->loadFromFile("data/" + cStr(i + 1) + ".obj");
		object->createAABBCollisionDetector(0.01);
		object->computeBTN();
		yes[i] = object->getMesh(0);
		yes[i]->m_material = MyMaterial::create();
		yes[i]->m_material->setUseHapticShading(true);
		MyMaterialPtr material = dynamic_pointer_cast<MyMaterial>(yes[i]->m_material);
		object->setStiffness(1000.0, true);
		cTexture2dPtr albedoMap = cTexture2d::create();
		albedoMap->loadFromFile("data/sandpaper_colour.jpg");
		albedoMap->setWrapModeS(GL_REPEAT);
		albedoMap->setWrapModeT(GL_REPEAT);
		albedoMap->setUseMipmaps(true);
		cTexture2dPtr normalMap = cTexture2d::create();
		normalMap->loadFromFile("data/SandPaperNormalMap.png");
		normalMap->setWrapModeS(GL_REPEAT);
		normalMap->setWrapModeT(GL_REPEAT);
		normalMap->setUseMipmaps(true);
		material->normalMap = normalMap;
		material->obj = "teeth";
		yes[i]->m_texture = albedoMap;
		yes[i]->setUseTexture(true);
		object->setLocalPos(0.0, 0.0);
		world->addChild(object);
	}
	world->removeChild(topgums);
	cMultiMesh* gums2 = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	gums2->loadFromFile("data/gumstop.obj");
	gums2->createAABBCollisionDetector(0.01);
	gums2->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	topgums = gums2->getMesh(0);
	// replace the object's material with a custom one
	topgums->m_material = MyMaterial::create();
	//gummesh->m_material->setRed();
	topgums->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material = dynamic_pointer_cast<MyMaterial>(topgums->m_material);
	topgums->setStiffness(2000.0, true);


	cTexture2dPtr albedoMap = cTexture2d::create();
	// create a colour texture map for this mesh object
	albedoMap->loadFromFile("data/gumcolor.jpg");
	albedoMap->setWrapModeS(GL_REPEAT);
	albedoMap->setWrapModeT(GL_REPEAT);
	albedoMap->setUseMipmaps(true);

	cTexture2dPtr normalMap = cTexture2d::create();
	normalMap->loadFromFile("data/bumppy.png");
	normalMap->setWrapModeS(GL_REPEAT);
	normalMap->setWrapModeT(GL_REPEAT);
	normalMap->setUseMipmaps(true);


	material->normalMap = normalMap;
	material->obj = "gums";
	material->mesh = gummesh;

	topgums->m_texture = albedoMap;
	topgums->setUseTexture(true);
	gums2->setLocalPos(0.0, 0.0);

	world->addChild(gums2);
	//world->removeChild(gummesh);
	//gummesh = NULL;
	//cMultiMesh* gums = new cMultiMesh();

	//// load geometry from file and compute additional properties
	////object->loadFromFile("data/simpleteeth.obj");
	//gums->loadFromFile("data/gumsbottom.obj");
	//gums->createAABBCollisionDetector(0.01);
	//gums->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	gummesh = gums->getMesh(0);
	// replace the object's material with a custom one
	gummesh->m_material = MyMaterial::create();
	//gummesh->m_material->setRed();
	gummesh->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material2 = dynamic_pointer_cast<MyMaterial>(gummesh->m_material);
	gummesh->setStiffness(2000.0, true);


	cTexture2dPtr albedoMap2 = cTexture2d::create();
	// create a colour texture map for this mesh object

	albedoMap2->loadFromFile("data/gumcolor.jpg");
	albedoMap2->setWrapModeS(GL_REPEAT);
	albedoMap2->setWrapModeT(GL_REPEAT);
	albedoMap2->setUseMipmaps(true);

	cTexture2dPtr normalMap2 = cTexture2d::create();
	normalMap2->loadFromFile("data/bumppy.png");
	normalMap2->setWrapModeS(GL_REPEAT);
	normalMap2->setWrapModeT(GL_REPEAT);
	normalMap2->setUseMipmaps(true);


	material2->normalMap = normalMap2;
	material2->obj = "gums";
	material2->mesh = gummesh;

	gummesh->m_texture = albedoMap2;
	gummesh->setUseTexture(true);
	gums->setLocalPos(0.0, 0.0);

	world->addChild(gums);
	timer->reset();
	timer->start();
}
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if (a_action != GLFW_PRESS)
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
	else if (a_key == GLFW_KEY_R)
	{
		reset();
	}
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    hapticDevice->close();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------
void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

	//std::string debugString = cStr(proxyAlgorithm->m_debugInteger) + " " + debug_vect.str();
	std::string debugString = cStr(proxyAlgorithm->m_debugInteger) + " " + proxyAlgorithm->m_debugVector.str();

    // update haptic and graphic rate data
	int totalPoints = 0;
	for (int i = 0; i < 35; i++) {
		totalPoints += dynamic_pointer_cast<MyMaterial>(yes[i]->m_material)->points;
	}
	int Gumpoints = 4 * (dynamic_pointer_cast<MyMaterial>(gummesh->m_material)->points + dynamic_pointer_cast<MyMaterial>(topgums->m_material)->points);
	int point = totalPoints - Gumpoints;
	if (60 - timer->getCurrentTimeSeconds() 
		< 0) {
		reset();
	}
	timeLabel->setText(cStr(60 - timer->getCurrentTimeSeconds()) + " Score: " + cStr(point) + " High-Score: " + cStr(highscore));


    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);
	timeLabel->setLocalPos((int)(0.5 * (width - timeLabel->getWidth())), 900);
    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error:  %s\n" << gluErrorString(err);
}

//------------------------------------------------------------------------------

bool checkMovement(cVector3d position) {
	bool ret = false;
	//Sphere
	double R = 0.02;
	
	cVector3d sphereOffset(0.01, 0.0, 0.0);
	cVector3d pNew = cVector3d(position.x(), position.y(), position.z());
	cVector3d c = pNew - sphereOffset;

	if (c.length() > R && proxyAlgorithm->m_debugInteger == 0) {
		double factor = (c.length() - R)/R;
		double x = pNew.x() * 0.005 * factor;
		double y = pNew.y() * 0.005 * factor;
		double z = pNew.z() * 0.005 * factor;
		debug_vect.set(x, y, 0.0);

		eye.set(eye.x() + x, eye.y() + y, eye.z() + z);
		lookat.set(lookat.x() + x, lookat.y() + y, lookat.z() + z);
		cVector3d tPos = tool->getLocalPos();
		tool->setLocalPos(tPos.x() + x, tPos.y() + y, tPos.z() + z);

		camera->set(eye						,   // camera position (eye)
					lookat					,   // look at position (target)
					cVector3d(0.0, 0.0, 1.0)	// direction of the (up) vector
					);   
		
		ret = true;
	}
	else {
		ret = false;
	}

	return ret;
}

void updateHaptics(void)
{
    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;
    // main haptic simulation loop
    while(simulationRunning)
    {
		bool button = false;
		hapticDevice->getUserSwitch(0, button);
        /////////////////////////////////////////////////////////////////////
        // READ HAPTIC DEVICE
        /////////////////////////////////////////////////////////////////////
		//tool->m_image->setLocalPos(cVector3d(tool->getLocalPos().x(), tool->getLocalPos().y(), tool->getLocalPos().z()));
		//tool->updateToolImagePosition();
        // read position 
        cVector3d position;
        hapticDevice->getPosition(position);

        // read orientation 

        // read user-switch status (button 0)
        world->computeGlobalPositions();


        /////////////////////////////////////////////////////////////////////
        // UPDATE 3D CURSOR MODEL
        /////////////////////////////////////////////////////////////////////

        tool->updateFromDevice();

		if (button == true) {
			degrees += .1;
			//tool->rotateAboutLocalAxisDeg(cVector3d(0, 1, 0), 0.1);
		}
		bool button2 = false;
		hapticDevice->getUserSwitch(1, button2);
		if (button2 == true) {
			degrees2 += .1;
			//tool->rotateAboutLocalAxisDeg(cVector3d(0, 0, 1), 0.1);
		}
		bool button3 = false;
		hapticDevice->getUserSwitch(2, button3);
		if (button3 == true) {
			degrees -= .1;
			//tool->rotateAboutLocalAxisDeg(cVector3d(0, 1, 0), -0.1);
		}
		bool button4 = false;
		hapticDevice->getUserSwitch(3, button4);
		if (button4 == true) {
			degrees2 -= .1;
			//tool->rotateAboutLocalAxisDeg(cVector3d(0, 0, 1), -.1);
		}
        /////////////////////////////////////////////////////////////////////
        // COMPUTE FORCES
        /////////////////////////////////////////////////////////////////////

        tool->computeInteractionForces();

        cVector3d force(0, 0, 0);
        cVector3d torque(0, 0, 0);
        double gripperForce = 0.0;

		bool flag = checkMovement(position);
		if (flag) {

		}
        /////////////////////////////////////////////////////////////////////
        // APPLY FORCES
        /////////////////////////////////////////////////////////////////////

        tool->applyToDevice();

        // signal frequency counter
        freqCounterHaptics.signal(1);
    }
    
    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
