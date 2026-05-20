
#include "stdh.h"

#include "GuiData_ETProbe.h"

#include "WorldSystem/IEntitySystem.h"

IETProbeEditor * GuiData_ETProbe::GetEditor()
{
	IETProbeEditor * editor = NULL;

	IObjMapEditor *base = pES->FindObjMapEditor(OBJMAP_TYPE_ETPROBE);
	if(base)
		editor  = (IETProbeEditor *)(base);

	return editor;  
}


