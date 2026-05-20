
#include "stdh.h"

#include "GuiData_Ridge.h"

#include "WorldSystem/IEntitySystem.h"

IRidgeEditor * GuiData_Ridge::GetEditor()
{
	IRidgeEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_RIDGE);
	if(base)
		editor  = (IRidgeEditor *)(base);

	return editor;  
}


