
#include "stdh.h"

#include "WorldSystem/IEntitySystem.h"

#include "GuiData_Water.h"

//////////////////////////////////////////////////////////////////////////
IWaterEditor * GuiData_Water::GetWaterEditor()
{
	IWaterEditor * editor = NULL;
	if(pES)
		return (IWaterEditor *)pES->FindObjMapEditor(OBJMAP_TYPE_WATER);

	return editor;
}


