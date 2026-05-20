#include "stdh.h"
#include "commondefines\general_stl.h"
#include ".\guiagent_terrainview.h"
#include "WorldSystem/ITrrn.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IWorldSystem.h"
#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/ITexture.h"

#include "WorldSystem/IAssetRenderer.h"

#include "FileSystem/IMapFile.h"

#include "AgentCmdID.h"

#include "maptooldefines.h"

#include "GuiData.h"

#include "GuiActor_Trrn.h"

#include "resdata/TexData.h"

BOOL CGuiAgent_TerrainView::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;
	BOOL bWireframe=data->pES->GetAS()->GetSS()->ratomsTrrn->GetWireframe();
	BOOL bDrawHole=data->pES->GetAS()->GetSS()->ratomsTrrn->GetDrawHole();
	ITexture *texDraft=data->pES->GetAS()->GetSS()->ratomsTrrn->GetDraft();

	_AddMenuSep();

	CGuiView * view = GetGuiView();
	assert(view);

	_PushMenu("地表绘制方式");
	
		if (bWireframe)
			_AddMenu("线框模式(地表)",ID_AGENT_ViewTrrnWireframe,MF_ENABLED|MF_STRING|MF_CHECKED);
		else
			_AddMenu("线框模式(地表)",ID_AGENT_ViewTrrnWireframe);

		if (bDrawHole)
			_AddMenu("显示地表上的Hole",ID_AGENT_ViewTrrnHole,MF_ENABLED|MF_STRING|MF_CHECKED);
		else
			_AddMenu("显示地表上的Hole",ID_AGENT_ViewTrrnHole);

		if (texDraft)
			_AddMenu("显示地表草图",ID_AGENT_ViewTrrnDraft,MF_ENABLED|MF_STRING|MF_CHECKED);
		else
			_AddMenu("显示地表草图",ID_AGENT_ViewTrrnDraft);

	_PopMenu();

	_AddMenuSep();

	return TRUE;

}

BOOL CGuiAgent_TerrainView::OnCommand(DWORD idCmd)
{
	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;

	BOOL bWireframe=data->pES->GetAS()->GetSS()->ratomsTrrn->GetWireframe();
	BOOL bDrawHole=data->pES->GetAS()->GetSS()->ratomsTrrn->GetDrawHole();
	ITexture *texDraft=data->pES->GetAS()->GetSS()->ratomsTrrn->GetDraft();
	switch(idCmd)
	{
		case ID_AGENT_ViewTrrnWireframe:
		{
			data->pES->GetAS()->GetSS()->ratomsTrrn->SetWireframe(!bWireframe);
			_Redraw(FALSE);
			return FALSE;
		}
		case ID_AGENT_ViewTrrnHole:
		{
			data->pES->GetAS()->GetSS()->ratomsTrrn->SetDrawHole(!bDrawHole);
			_Redraw(FALSE);
			return FALSE;
		}
		case ID_AGENT_ViewTrrnDraft:
		{
			if (texDraft)
				data->pES->GetAS()->GetSS()->ratomsTrrn->SetDraft(NULL);
			else
			{
				IMapFile *mf=data->pES->GetMapFile();
				if (mf)
				{
					BYTE  * pData = NULL;
					DWORD szData = 0;
					if (mf->LoadUnique(TRRNDRAFT_DATA,pData,szData))
					{
						TexData td;
						td.textype=*(TexData::TexDataType*)pData;

						//略过前面的类型
						pData+=sizeof(DWORD);
						szData-=sizeof(DWORD);

						VEC_SET_BUFFER(td.data,pData,szData);

						ITexture *tex=data->pES->GetWS()->GetRS()->GetWTexMgr2()->Create(td);
						if (tex)
							data->pES->GetAS()->GetSS()->ratomsTrrn->SetDraft(tex);
						SAFE_RELEASE(tex);
					}
				}
			}
			_Redraw(FALSE);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CGuiAgent_TerrainView::OnMouseMove(int x,int y,DWORD flag)
{
	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;
// 	data->GetTrrnMapEditor()->SetTestValue(((float)x)/30.0f,((float)y)/100.0f);
	return TRUE;
}
