
#include "toothBrushCursor.h"

using namespace chai3d;

toothBrushCursor::toothBrushCursor(cWorld* a_parentWorld):cGenericTool(a_parentWorld)
{
	// create a single point contact
	m_hapticPoint = new cHapticPoint(this);
	m_hapticPoint2 = new cHapticPoint(this);

	// add point to list
	m_hapticPoints.push_back(m_hapticPoint);
	m_hapticPoints.push_back(m_hapticPoint2);

	// show proxy spheres only
	setShowContactPoints(true, false);
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
		globalForce += m_hapticPoint->computeInteractionForces(m_deviceGlobalPos,
			m_deviceGlobalRot,
			m_deviceGlobalLinVel,
			m_deviceGlobalAngVel);
		
	}
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
