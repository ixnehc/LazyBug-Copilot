
#include "stdh.h"

#include "GuiData_Road.h"

#include "WorldSystem/IEntitySystem.h"

IRoadEditor * GuiData_Road::GetEditor()
{
	IRoadEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_ROAD);
	if(base)
		editor  = (IRoadEditor *)(base);
	return editor;  
}


