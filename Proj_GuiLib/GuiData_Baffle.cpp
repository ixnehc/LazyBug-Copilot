
#include "stdh.h"

#include "GuiData_Baffle.h"

#include "WorldSystem/IEntitySystem.h"

IBaffleEditor * GuiData_Baffle::GetEditor()
{
	IBaffleEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_BAFFLE);
	if(base)
		editor  = (IBaffleEditor *)(base);

	return editor;  
}


