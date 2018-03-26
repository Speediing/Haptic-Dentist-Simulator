//==============================================================================
/*
    CPSC 599.86 / 601.86 - Computer Haptics
    Winter 2018, University of Calgary

    This class extends the cMaterial class in CHAI3D to include additional
    object material properties. You may find it useful to store properties
    you need when creating surface textures for your assignment.
*/
//==============================================================================

#include "MyMaterial.h"

using namespace chai3d;

//==============================================================================
/*!
    Constructor of MyMaterial.
*/
//==============================================================================
MyMaterial::MyMaterial()
{
	//Insert Material properties here
    m_myMaterialProperty = 1.0;
}
