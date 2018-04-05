//==============================================================================
/*
    \author    Your Name
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "MyProxyAlgorithm.h"
#include "MyMaterial.h"
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

// a camera to render the world in the window display
cCamera* camera;
cVector3d eye = cVector3d(0.5, 0.0, 0.0);
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
cToolCursor* tool;

// a pointer to the custom proxy rendering algorithm inside the tool
MyProxyAlgorithm* proxyAlgorithm;

// nine objects with different surface textures that we want to render
cMultiMesh* scope;
cMultiMesh *objects[3][3];
cMesh* mesh;
cMesh* gummesh;

// flag to indicate if the haptic simulation currently running
bool simulationRunning = false;

// flag to indicate if the haptic simulation has terminated
bool simulationFinished = false;

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
                 cVector3d (0.0, 0.0, 1.0));   // direction of the (up) vector

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
    double toolRadius = 0.01;
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

	cMultiMesh* object = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	object->loadFromFile("data/teethmagic.obj");
	object->createAABBCollisionDetector(toolRadius);
	object->computeBTN();

	//object->setWireMode(true);

	// obtain the first (and only) mesh from the object
	mesh = object->getMesh(0);
	// replace the object's material with a custom one
	mesh->m_material = MyMaterial::create();
	//mesh->m_material->setWhite();
	mesh->m_material->setUseHapticShading(true);
	//MyMaterialPtr m = mesh->m_material;
	MyMaterialPtr material = dynamic_pointer_cast<MyMaterial>(mesh->m_material);
	object->setStiffness(2000.0, true);

	
	cTexture2dPtr albedoMap = cTexture2d::create();
		// create a colour texture map for this mesh object

	albedoMap->loadFromFile("data/yellow.jpg");
	albedoMap->setWrapModeS(GL_REPEAT);
	albedoMap->setWrapModeT(GL_REPEAT);
	albedoMap->setUseMipmaps(true);

	cTexture2dPtr normalMap = cTexture2d::create();
	normalMap->loadFromFile("data/bumppy.png");
	normalMap->setWrapModeS(GL_REPEAT);
	normalMap->setWrapModeT(GL_REPEAT);
	normalMap->setUseMipmaps(true);

	/*cTexture2dPtr heightMap = cTexture2d::create();
	heightMap->loadFromFile("images/" + textureFiles[i][j] + "_height.jpg");
	heightMap->setWrapModeS(GL_REPEAT);
	heightMap->setWrapModeT(GL_REPEAT);
	heightMap->setUseMipmaps(true);

	cTexture2dPtr roughMap = cTexture2d::create();
	roughMap->loadFromFile("images/" + textureFiles[i][j] + "_roughness.jpg");
	roughMap->setWrapModeS(GL_REPEAT);
	roughMap->setWrapModeT(GL_REPEAT);
	roughMap->setUseMipmaps(true);*/

	material->normalMap = normalMap;
	material->obj = "teeth";
	//material->heightMap = heightMap;
	//material->roughnessMap = roughMap;


	// assign textures to the mesh
	
	mesh->m_texture = albedoMap;
	mesh->setUseTexture(true);
			
	
	// set the position of this object
	object->setLocalPos(0.0, 0.0);

	world->addChild(object);
    
	cMultiMesh* gums = new cMultiMesh();

	// load geometry from file and compute additional properties
	//object->loadFromFile("data/simpleteeth.obj");
	gums->loadFromFile("data/gums.3ds");
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

	/*cTexture2dPtr heightMap = cTexture2d::create();
	heightMap->loadFromFile("images/" + textureFiles[i][j] + "_height.jpg");
	heightMap->setWrapModeS(GL_REPEAT);
	heightMap->setWrapModeT(GL_REPEAT);
	heightMap->setUseMipmaps(true);

	cTexture2dPtr roughMap = cTexture2d::create();
	roughMap->loadFromFile("images/" + textureFiles[i][j] + "_roughness.jpg");
	roughMap->setWrapModeS(GL_REPEAT);
	roughMap->setWrapModeT(GL_REPEAT);
	roughMap->setUseMipmaps(true);*/

	material2->normalMap = normalMap2;
	material2->obj = "gums";
	material2->mesh = gummesh;
	//material->heightMap = heightMap;
	//material->roughnessMap = roughMap;


	// assign textures to the mesh

	gummesh->m_texture = albedoMap2;
	gummesh->setUseTexture(true);


	// set the position of this object
	gums->setLocalPos(0.0, 0.0);

	world->addChild(gums);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICE
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get a handle to the first haptic device
    handler->getDevice(hapticDevice, 0);

    // if the device has a gripper, enable the gripper to simulate a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    tool = new cToolCursor(world);
    world->addChild(tool);

	//Move to bumps bin
	//tool->setLocalPos(0.0, -objectSpacing, 0.0);

    // [CPSC.86] replace the tool's proxy rendering algorithm with our own
    proxyAlgorithm = new MyProxyAlgorithm;
    delete tool->m_hapticPoint->m_algorithmFingerProxy;
    tool->m_hapticPoint->m_algorithmFingerProxy = proxyAlgorithm;

    tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();

    tool->setRadius(0.01, toolRadius);

    tool->setHapticDevice(hapticDevice);

    tool->setWaitForSmallForce(true);
	scope = new cMultiMesh();

	// attach scope to tool
	tool->m_image = scope;
	tool->m_image->setLocalPos(cVector3d(tool->getLocalPos().x(), tool->getLocalPos().y() - 1, tool->getLocalPos().z()-.07));
	// load an object file
	scope->loadFromFile("data/toothbrush.3ds");
	//tool->setShowContactPoints(false, false);
	scope->rotateExtrinsicEulerAnglesDeg(M_PI, M_PI, M_PI, C_EULER_ORDER_XYZ);
	scope->setUseCulling(false);
	scope->createAABBCollisionDetector(toolRadius);
	tool->updateToolImagePosition();

	// use display list for faster rendering
	scope->setUseDisplayList(true);
	tool->start();
    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    cFontPtr font = NEW_CFONTCALIBRI20();
    
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
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz " + debugString);
	timeLabel->setText(cStr(60 - timer->getCurrentTimeSeconds()) + " " + cStr(dynamic_pointer_cast<MyMaterial>(mesh->m_material)->points));


    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);
	timeLabel->setLocalPos((int)(0.5 * (width - timeLabel->getWidth())), 550);
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
	
	cVector3d sphereOffset(0.0, 0.0, 0.0);
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
        /////////////////////////////////////////////////////////////////////
        // READ HAPTIC DEVICE
        /////////////////////////////////////////////////////////////////////
		tool->m_image->setLocalPos(cVector3d(tool->getLocalPos().x(), tool->getLocalPos().y() - 1, tool->getLocalPos().z() - .07));
		tool->updateToolImagePosition();
        // read position 
        cVector3d position;
        hapticDevice->getPosition(position);

        // read orientation 
        cMatrix3d rotation;
        hapticDevice->getRotation(rotation);

        // read user-switch status (button 0)
        bool button = false;
        hapticDevice->getUserSwitch(0, button);


        world->computeGlobalPositions();


        /////////////////////////////////////////////////////////////////////
        // UPDATE 3D CURSOR MODEL
        /////////////////////////////////////////////////////////////////////

        tool->updateFromDevice();


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
