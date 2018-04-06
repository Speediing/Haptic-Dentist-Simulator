#ifndef TOOTH_BRUSH_CURSOR_H
#define TOOTH_BRUSH_CURSOR_H

#include "chai3d.h"
using namespace chai3d;
//#include "tools/CGenericTool.h"

class toothBrushCursor : public cGenericTool
{
	//--------------------------------------------------------------------------
	// CONSTRUCTOR & DESTRUCTOR:
	//--------------------------------------------------------------------------

public:

	//! Constructor of cToolCursor.
	toothBrushCursor(cWorld* a_parentWorld);

	//! Destructor of cToolCursor.
	virtual ~toothBrushCursor();


	//--------------------------------------------------------------------------
	// PUBLIC MEMBERS
	//--------------------------------------------------------------------------

public:

	// Single haptic point representing a cursor.
	cHapticPoint * m_hapticPoint;
	cHapticPoint * m_hapticPoint2;


	//--------------------------------------------------------------------------
	// PUBLIC METHODS
	//--------------------------------------------------------------------------

public:

	//! This method computes the interaction forces between the haptic point and the environment.
	virtual void computeInteractionForces();

	//! This method renders the tools using OpenGL.
	virtual void render(cRenderOptions& a_options);

	//! This method updates the position and orientation of the tool image.
	virtual void updateToolImagePosition();
};

//------------------------------------------------------------------------------



#endif
