
#include "stdh.h"

#include "GuiData_Shore.h"

#include "WorldSystem/IEntitySystem.h"

IShoreEditor * GuiData_Shore::GetEditor()
{
	IShoreEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_SHORE);
	if(base)
		editor  = (IShoreEditor *)(base);

	return editor;  
}


