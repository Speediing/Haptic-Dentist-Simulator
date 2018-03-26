//==============================================================================
/*
    CPSC 599.86 / 601.86 - Computer Haptics
    Winter 2018, University of Calgary

    This class extends the cMaterial class in CHAI3D to include additional
    object material properties. You may find it useful to store properties
    you need when creating surface textures for your assignment.
*/
//==============================================================================

#ifndef MYMATERIAL_H
#define MYMATERIAL_H

#include "chai3d.h"

//------------------------------------------------------------------------------
struct MyMaterial;
typedef std::shared_ptr<MyMaterial> MyMaterialPtr;
//------------------------------------------------------------------------------

struct MyMaterial : public chai3d::cMaterial
{
public:

    //! Constructor of cMaterial.
    MyMaterial();

    //! Shared MyMaterial allocator.
    static MyMaterialPtr create() { return (std::make_shared<MyMaterial>()); }


    //--------------------------------------------------------------------------
    // [CPSC.86] CUSTOM MATERIAL PROPERTIES
    //--------------------------------------------------------------------------
	chai3d::cTexture2dPtr normalMap;
	chai3d::cTexture2dPtr heightMap;
	chai3d::cTexture2dPtr roughnessMap;

	int binNum;
    double m_myMaterialProperty;
};

//------------------------------------------------------------------------------
#endif
