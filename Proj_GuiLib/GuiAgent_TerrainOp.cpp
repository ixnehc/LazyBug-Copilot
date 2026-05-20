#include "stdh.h"

#include "GuiAgent_TerrainOp.h"

#include "WorldSystem/ITrrn.h"

#include "GuiActor_Trrn.h"

CGuiAgent_TerrainOp::CGuiAgent_TerrainOp(void)
{
}

CGuiAgent_TerrainOp::~CGuiAgent_TerrainOp(void)
{

}

BOOL CGuiAgent_TerrainOp::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(data){
		_AddMenuSep();
		DWORD flagState = data->bPaint?MF_CHECKED:MF_UNCHECKED;
		_AddMenu("修改地表",ID_AGENT_Trrn_Mod,flagState|MF_ENABLED|MF_STRING);
	}

	return TRUE;
}

BOOL CGuiAgent_TerrainOp::OnCommand(DWORD idCmd)
{
	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;

	switch(idCmd){
		case ID_AGENT_Trrn_Mod:
			{
				CGuiPanel_Trrn * actor = (CGuiPanel_Trrn *)_GetActor();
				if(actor){
					data->bPaint = (data->bPaint)?FALSE:TRUE;
					actor->UpdatePaintState();
				}
				return FALSE;
			}
		default: break;
	}

	return TRUE;
}



