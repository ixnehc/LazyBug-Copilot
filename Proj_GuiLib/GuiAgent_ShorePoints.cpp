
#include "stdh.h"

#include "GuiAgent_ShorePoints.h"

#include "GuiData_Shore.h"

#include "GuiData.h"

#include "WorldSystem/ITrrn.h"

#include "GuiActor_Shore.h"

#include "GuiData_Water.h"

HMapObj * CGuiAgent_ShorePoints::_GetSelObj()
{
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		return &(data->hObjSel);
	return NULL;
}

void CGuiAgent_ShorePoints::_GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps)
{
	IShoreEditor * editor = NULL;
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();
	
	if(editor){
		cps->Clean();
		ICtrlPointPack * pack = editor->GetCtrlPointPack(hObj);
		if(pack){
			cps->Clone(pack);
		}
	}
}

DWORD *CGuiAgent_ShorePoints::_GetVer()
{
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		return &(data->ver);
	else 
		return NULL;
}

HMapObj CGuiAgent_ShorePoints::_SetKeyPos(const HMapObj &hObj,ICtrlPointPack * cps)
{
	IShoreEditor * editor = (IShoreEditor *)_GetEditor();
	HMapObj hObjNew = hObj;
	if(editor){
		hObjNew = editor->SetCtrlPointPack(hObj,cps);
	}
	return hObjNew;
}

IObjMapEditor * CGuiAgent_ShorePoints::_GetEditor()
{
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		return data->GetEditor();
	return NULL;
}

HMapObj CGuiAgent_ShorePoints::_NewObj(IObjMapEditor * editor,ICtrlPointPack * cps)
{
	HMapObj hObj = INVALID_HMAPOBJ;

	IShoreEditor * editorShore = NULL;
	GuiData_Shore *data = (GuiData_Shore *)FindData("shore");
	if(data)
		editorShore = data->GetEditor();

	if(editorShore)
		hObj = editorShore->AddShore(data->info,cps,data->brID);
	
	return hObj;
}

BOOL CGuiAgent_ShorePoints::_HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos)
{	
	IWaterEditor * editorWater = NULL;
	GuiData_Water *dataWater = (GuiData_Water *)FindData("water");
	if(dataWater)
		editorWater = dataWater->GetWaterEditor();

	ITrrnMapEditor * trrnEditor = NULL;
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(data)
		trrnEditor = data->GetTrrnMapEditor();
	
	if(!trrnEditor)
		return FALSE;

	i_math::vector3df p0,p1;
	if(!trrnEditor->GetHitPos(rayHit,TRUE,p0))
		return FALSE;
	
	pos = p0;
	if(editorWater){
		HMapObj hObj = editorWater->HitTest(rayHit,&p1);
		if(hObj!=INVALID_HMAPOBJ){
			float d0 = (float)(p0 - rayHit.start).getLengthSQ();
			float d1 = (float)(p1 - rayHit.start).getLengthSQ();
			pos = (d0<d1)?p0:p1;
		}
	}

	return TRUE;
}

void * CGuiAgent_ShorePoints::_GetSelPointsBuf()
{
	GuiData_Shore * data = (GuiData_Shore *)FindData("shore");
	if(data)
		return &(data->idxSelCPs);
	return NULL;
}

void CGuiAgent_ShorePoints::_OnNewCP(CtrlPoint * cp)
{
	CtrlPt_Shore * sp = (CtrlPt_Shore *)(cp);
	sp->alpha = 1.0f;
}

ICtrlPointPack * CGuiAgent_ShorePoints::_NewCtrlPointPack()
{
	IShoreEditor * editor = NULL;
	GuiData_Shore *data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();
	if(editor)
		return editor->NewCtrlPointPack();
	return NULL;
}

void CGuiAgent_ShorePoints::OnSelected(const HMapObj &hObj)
{
	IShoreEditor * editor = NULL;
	GuiData_Shore *data = (GuiData_Shore *)FindData("shore");
	if(data)
		editor = data->GetEditor();
	
	if(editor){
		const ShoreInfo * info = editor->GetShoreInfo(hObj);
		if(info){
			data->info = *info;
			CGuiPanel_Shore * actor = (CGuiPanel_Shore *)_GetActor();
			data->brID = editor->GetBrush(hObj);
			actor->OnBrChange();
		}
	}
}



