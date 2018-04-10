
#include "toothBrushCursor.h"
#include "iostream"
using namespace std;
using namespace chai3d;

toothBrushCursor::toothBrushCursor(cWorld* a_parentWorld):cGenericTool(a_parentWorld)
{
	// create a single point contact
	m_hapticPoint = new cHapticPoint(this);
	
	
	// add point to list
	//m_hapticPoints.push_back(m_hapticPoint);
	

	// add the offset for calculating position
	double xoff = 0.0;
	for (int i = 0; i < 1; i++) {
		double yoff = 0.0;
		for (int j = 0; j < 8; j++) {
			cHapticPoint * m_hapticPoint2;
			m_hapticPoint2 = new cHapticPoint(this);
			delete m_hapticPoint2->m_algorithmFingerProxy;
			m_hapticPoint2->m_algorithmFingerProxy = proxyAlgorithm;
			m_hapticPoint2->m_sphereProxy->m_material->setWhite();

			m_hapticPoints.push_back(m_hapticPoint);
			hapticPointOffset.push_back(cVector3d(xoff, yoff, 0.0));
			yoff -= 0.005;
			
		}
		//xoff -= 0.005;
	}
	//hapticPointOffset.push_back(cVector3d(0.0, 0.0, 0.0));
	//hapticPointOffset.push_back(cVector3d(0.0, -0.005, 0.0));
	
	// show proxy spheres only
	setShowContactPoints(false, false);
}


//==============================================================================
/*!
Destructor of cToolCursor.
*/
//==============================================================================
toothBrushCursor::~toothBrushCursor()
{
	delete m_hapticPoint;
	//delete m_hapticPoint2;
}


//==============================================================================
/*!
This method updates the position and orientation of the tool image.
*/
//==============================================================================
void toothBrushCursor::updateToolImagePosition()
{
	// set the position and orientation of the tool image to be equal to the 
	// one of the haptic point proxy.
	cVector3d pos = m_hapticPoint->getLocalPosProxy();
	m_image->setLocalPos(pos);
	m_image->setLocalRot(m_deviceLocalRot);
}


//==============================================================================
/*!
This method computes the interaction forces between the haptic point and
the virtual environment.
*/
//==============================================================================
void toothBrushCursor::computeInteractionForces()
{
	// compute interaction forces at haptic point in global coordinates
	int numContactPoint = (int)(m_hapticPoints.size());
	cVector3d globalForce = cVector3d(0.0, 0.0, 0.0);
	for (int i = 0; i < numContactPoint; i++) {
		cVector3d tempForce = m_hapticPoint->computeInteractionForces(m_deviceGlobalPos + hapticPointOffset.at(i),
			m_deviceGlobalRot,
			m_deviceGlobalLinVel,
			m_deviceGlobalAngVel);
		/*if (tempForce.length() != 0)
		{
			globalForce += tempForce;
			break;
		}*/
		//cout << tempForce << endl;
		globalForce += tempForce;
		
	}
	globalForce = globalForce / ((double) 2.0 * numContactPoint);
	cVector3d globalTorque(0.0, 0.0, 0.0);
	// update computed forces to tool
	setDeviceGlobalForce(globalForce);
	setDeviceGlobalTorque(globalTorque);
	setGripperForce(0.0);
}


//==============================================================================
/*!
This method renders the current tool using OpenGL.

\param  a_options  Rendering options.
*/
//==============================================================================
void toothBrushCursor::render(cRenderOptions& a_options)
{
	///////////////////////////////////////////////////////////////////////
	// render haptic points
	///////////////////////////////////////////////////////////////////////
	int numContactPoint = (int)(m_hapticPoints.size());
	for (int i = 0; i<numContactPoint; i++)
	{
		// get next haptic point
		cHapticPoint* nextContactPoint = m_hapticPoints[i];

		// render tool
		nextContactPoint->render(a_options);
	}

	///////////////////////////////////////////////////////////////////////
	// render mesh image
	///////////////////////////////////////////////////////////////////////
	if (m_image != NULL)
	{
		m_image->renderSceneGraph(a_options);
	}
}
