#pragma once
#include "stdh.h"

#include "GuiData_El.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IObjMap.h"

IEnvLight * GuiData_El::GetIEnvLight()
{
	IEnvLight * editor = pES->FindEnvLight();
	return editor;
}
IProbeCubeMapEditor * GuiData_El::GetEditor()
{
	IProbeCubeMapEditor * editor = NULL;
	
	IEnvLight * pEL = GetIEnvLight();
	if(pEL)
		editor = pEL->GetEditor();
    
	return editor;
}