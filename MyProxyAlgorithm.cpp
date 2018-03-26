//==============================================================================
/*
    CPSC 599.86 / 601.86 - Computer Haptics
    Winter 2018, University of Calgary

    This class extends the cAlgorithmFingerProxy class in CHAI3D that
    implements the god-object/finger-proxy haptic rendering algorithm.
    It allows us to modify or recompute the force that is ultimately sent
    to the haptic device.

    Your job for this assignment is to implement the updateForce() method
    in this class to support for two new effects: force shading and haptic
    textures. Methods for both are described in Ho et al. 1999.
*/
//==============================================================================

#include "MyProxyAlgorithm.h"
#include "MyMaterial.h"
#include <iostream>
using std::cout;
using std::endl;

using namespace chai3d;

//==============================================================================
/*!
    This method uses the information computed earlier in
    computeNextBestProxyPosition() to calculate the force to be rendered.
    It first calls cAlgorithmFingerProxy::updateForce() to compute the base
    force from contact geometry and the constrained proxy position. That
    force can then be modified or recomputed in this function.

    Your implementation of haptic texture mapping will likely end up in this
    function. When this function is called, collision detection has already
    been performed, and the proxy point has already been updated based on the
    constraints found. Your job is to compute a force with all that information
    available to you.

    Useful variables to read:
        m_deviceGlobalPos   - current position of haptic device
        m_proxyGlboalPos    - computed position of the constrained proxy
        m_numCollisionEvents- the number of surfaces constraining the proxy
        m_collisionRecorderConstraint0,1,2
                            - up to three cCollisionRecorder structures with
                              cCollisionEvents that contain very useful
                              information about each contact

    Variables that this function should set/reset:
        m_normalForce       - computed force applied in the normal direction
        m_tangentialForce   - computed force along the tangent of the surface
        m_lastGlobalForce   - this is what the operator ultimately feels!!!
*/
//==============================================================================

void MyProxyAlgorithm::updateForce()
{
    // get the base class to do basic force computation first
    cAlgorithmFingerProxy::updateForce();
	cVector3d force = m_lastGlobalForce;
    // TODO: compute force shading and texture forces here


	m_debugInteger = m_numCollisionEvents;

    if (m_numCollisionEvents > 0)
    {
        // this is how you access collision information from the first constraint
        cCollisionEvent* c0 = &m_collisionRecorderConstraint0.m_nearestCollision;

		//m_debugVector = c0->m_globalNormal;
		int triIndex = c0->m_index;
		int vertexIndex0 = c0->m_triangles->getVertexIndex0(c0->m_index);
		
		

		if (MyMaterialPtr material = std::dynamic_pointer_cast<MyMaterial>(c0->m_object->m_material))
		{
			// you can access your custom material properties here
			int bin = material->binNum;

			cVector3d collisionGlobPos = c0->m_object->getGlobalPos();

			cVector3d collisionLocalPos = c0->m_localPos;
			cVector3d tCoordRemapped = c0->m_triangles->getTexCoordAtPosition(triIndex, collisionLocalPos);
			if (tCoordRemapped.x() < 0.0) {
				tCoordRemapped.set(tCoordRemapped.x() + 1.0, tCoordRemapped.y(), tCoordRemapped.z());
			}
			else if (tCoordRemapped.x() > 1.0) {
				tCoordRemapped.set(tCoordRemapped.x() - 1.0, tCoordRemapped.y(), tCoordRemapped.z());
			}
			if (tCoordRemapped.y() < 0.0) {
				tCoordRemapped.set(tCoordRemapped.x(), tCoordRemapped.y() + 1.0, tCoordRemapped.z());
			}
			else if (tCoordRemapped.y() > 1.0) {
				tCoordRemapped.set(tCoordRemapped.x(), tCoordRemapped.y() - 1.0, tCoordRemapped.z());
			}

			//Bumps bin
			if (bin == 4) {
				//cout << "In bumps!" << endl;
				//Force tex coords to be between 0 and 1;
				unsigned short bumpCount = 10;
				double dhdx = cos(2.0 * M_PI * bumpCount * tCoordRemapped.x());
				//cVector3d n = c0->m_localNormal + dhdx * cNormalize(m_proxyGlobalPos);
				cVector3d n = computeShadedSurfaceNormal(c0) + dhdx * cNormalize(m_proxyGlobalPos);
				n.normalize();
				force = m_lastGlobalForce.length() * n;
				//force.clamp(5.0);
			}
			//Friction bin
			else if (bin == 6) {
				//cout << "In friction!" << endl;
				double bumpCount = 2.5;
				double dhdy = pow(cos(2.0 * M_PI * bumpCount * (tCoordRemapped.y() + 0.15)), 4);

				c0->m_object->setFriction(abs(dhdy) * 1.0, abs(dhdy) * 0.6);
				

			}
			//Every other bin
			else {
				//cout << "In Other!" << endl;
				cVector3d normalMapVect, roughnessMapVect, heightMapVect;
				cColorf normalMapColour, roughnessMapColour, heightMapColour;
				cImagePtr normalImg, roughnessImg, heightImg;
					
				normalImg = material->normalMap->m_image;
				roughnessImg = material->roughnessMap->m_image;
				heightImg = material->heightMap->m_image;

				normalImg->getPixelColorInterpolated(tCoordRemapped.x() * normalImg->getWidth(), tCoordRemapped.y() * normalImg->getHeight(), normalMapColour);
				normalMapVect.set(normalMapColour.getG() - 0.5, normalMapColour.getR() - 0.5, normalMapColour.getB() - 0.5);

				//cVector3d n = c0->m_localNormal + normalMapVect;
				cVector3d n = computeShadedSurfaceNormal(c0) + normalMapVect;
				//n.normalize();
				//cout << m_debugVector.x() << endl;
				n = (cNormalize(m_normalForce) + n)/2.0;

				roughnessImg->getPixelColorInterpolated(tCoordRemapped.x() * roughnessImg->getWidth(), tCoordRemapped.y() * roughnessImg->getHeight(), roughnessMapColour);
				roughnessMapVect.set(roughnessMapColour.getG(), roughnessMapColour.getR(), roughnessMapColour.getB());
					
				double roughnessCo = (roughnessMapVect.x() + roughnessMapVect.y() + roughnessMapVect.z()) / 3.0;
				c0->m_object->setFriction(1.5 * roughnessCo, 1.0 * roughnessCo);
				m_debugVector.set(1.5 * roughnessCo, 1.0 * roughnessCo, roughnessCo);
					

				heightImg->getPixelColorInterpolated(tCoordRemapped.x() * heightImg->getWidth(), tCoordRemapped.y() * heightImg->getHeight(), heightMapColour);
				heightMapVect.set(heightMapColour.getG(), heightMapColour.getR(), heightMapColour.getB());
					
					
				//* (1 + heightVect.length())
				force = m_lastGlobalForce.length() * (n * (1.0 + heightMapVect.length()));
			
			}

		}

		m_lastGlobalForce = force;
		//This will force the haptic device to the right.
		//m_lastGlobalForce = cVector3d(0.0, 3.0, 0.0);
    }
}


//==============================================================================
/*!
    This method attempts to move the proxy, subject to friction constraints.
    This is called from computeNextBestProxyPosition() when the proxy is
    ready to move along a known surface.

    Your implementation of friction mapping will likely need to modify or
    replace the CHAI3D implementation in cAlgorithmFingerProxy. You may
    either copy the implementation from the base class and modify it to take
    into account a friction map, or use your own friction rendering from your
    previous assignment.

    The most important thing to do in this method is to write the desired
    proxy position into the m_nextBestProxyGlobalPos member variable.

    The input parameters to this function are as follows, all provided in the
    world (global) coordinate frame:

    \param  a_goal    The location to which we'd like to move the proxy.
    \param  a_proxy   The current position of the proxy.
    \param  a_normal  The surface normal at the obstructing surface.
    \param  a_parent  The surface along which we're moving.
*/
//==============================================================================
void MyProxyAlgorithm::testFrictionAndMoveProxy(const cVector3d& a_goal,
                                                const cVector3d& a_proxy,
                                                cVector3d &a_normal,
                                                cGenericObject* a_parent)
{
    cAlgorithmFingerProxy::testFrictionAndMoveProxy(a_goal, a_proxy, a_normal, a_parent);
}
