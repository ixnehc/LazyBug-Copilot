
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"


#include "GuiAgent_WaterSelect.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/IWater.h"

#include "GuiActor_Water.h"

#include "GuiData_Water.h"

BOOL CGuiAgent_WaterSelect::OnLButtonDown(int x,int y,DWORD flag)
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	
	//选择操作只有在闲时进行
	if(!dataWater||dataWater->op!=WPaintOP_Idle)
		return TRUE;
	
	IWaterEditor * editor = dataWater->GetWaterEditor();
	if(!editor)
		return TRUE;

	IRenderPort * rp = GetRP();
	HitProbe hitLine;
	if(rp->CalcHitProbe(x,y,hitLine)){
		int br = -1;
		HMapObj hObj = INVALID_HMAPOBJ;
		hObj = editor->HitTest(hitLine);
		const WaterInfo * info = editor->GetWaterInfo(hObj);
		if(info){
			dataWater->br = info->idBr;
			CGuiPanel_Water * actor = (CGuiPanel_Water *)_GetActor();
			actor->OnBrushSelChange(info->idBr);
		}
	}

	return TRUE;
}



