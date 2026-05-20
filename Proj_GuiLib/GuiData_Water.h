#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/IWater.h"

#include "WorldSystem/IBrushLib.h"

class IWaterEditor;
struct GuiLib_Api GuiData_Water:public GeData
{
	GuiData_Water(){pES = NULL; br = INVALID_BRUID;op = WPaintOP_Idle ;brsize = 4.0f;}
	virtual const char * GetName() {return "water";}

	IWaterEditor * GetWaterEditor();
	IEntitySystem * pES;
	BRUID br;
	float brsize;
	WPaintOP op; 
};
