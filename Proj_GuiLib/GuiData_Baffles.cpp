
#include "stdh.h"

#include "GuiData_Baffles.h"

#include "WorldSystem/IEntitySystem.h"

IBafflesEditor * GuiData_Baffles::GetEditor()
{
	IBafflesEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_BAFFLE);
	if(base)
		editor  = (IBafflesEditor *)(base);

	return editor;  
}


