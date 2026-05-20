
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/ITools.h"


#include "GuiAgent_ETProbeCreate.h"

#include "GuiData_ETProbe.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/ITrrn.h"

#include "AgentCmdID.h"

#include "ModBlockBack.h"

#include "GuiData.h"

BOOL CGuiAgent_ETProbeCreate::OnLButtonDown(int x,int y,DWORD flag)
{
	IETProbeEditor * editor = NULL;
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(data)
		editor = data->GetEditor();
	
	ITrrnMapEditor * editorTrrn = _GetTrrnEditor();
	if(!editor||!editorTrrn||!data->bOnCreate)
		return TRUE;

	IRenderPort * rp = GetRP();
	HitProbe hitProbe;
	rp->CalcHitProbe(x,y,hitProbe);
	
	//清除选中的对象
	data->hObjSels.clear();

	i_math::vector3df posIntersec;
	if(editorTrrn->GetHitPos(hitProbe,TRUE,posIntersec)){
		ETProbeInfo info;
		posIntersec.y += 5.0f;
		info.pos = posIntersec;
		HMapObj hObj = editor->AddETProbe(info);
		data->hObjSels.push_back(hObj);

		i_math::pos2di ptBlk;
		if(editor->GetMapFileBlk(hObj,ptBlk)){
			CModManager * mgr = _GetModMgr();
			if(mgr){
				CModBlockBack * mod = new CModBlockBack(GetView());
				mod->BackupBlocks(&ptBlk,1);
				editor->Save();
				Mod_New(mgr,(CModBase*&)mod);
			}
			else
				editor->Save();
		}
	}

	return FALSE;
}

BOOL CGuiAgent_ETProbeCreate::OnCommand(DWORD idCmd)
{
	GuiData_ETProbe * data = (GuiData_ETProbe * )FindData("etprobe");
	if(!data)
		return TRUE;

	if(idCmd==ID_AGENT_ETPROBE_CREATE)
		data->bOnCreate = TRUE;
	
	return TRUE;
}

BOOL CGuiAgent_ETProbeCreate::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_ETProbe * data = (GuiData_ETProbe *)FindData("etprobe");
	if(!data)
		return TRUE;
	
	if(data->bOnCreate){
		data->bOnCreate = FALSE;
		return FALSE;
	}
	else{
		_AddMenu("新建",ID_AGENT_ETPROBE_CREATE);
	}

	return TRUE;
}

ITrrnMapEditor * CGuiAgent_ETProbeCreate::_GetTrrnEditor()
{
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if(dataTrrn)
		return dataTrrn->GetTrrnMapEditor();
	return NULL;
}




