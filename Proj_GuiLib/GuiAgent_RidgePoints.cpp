
#include "stdh.h"

#include "GuiAgent_RidgePoints.h"

#include "GuiData_Ridge.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

#include "WorldSystem/ITrrn.h"

#include "GuiData.h"

HMapObj * CGuiAgent_RidgePoints::_GetSelObj()
{
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)	
		return &(data->hObjSel);
	return NULL;
}

void CGuiAgent_RidgePoints::_GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps)
{
	IRidgeEditor * editor = NULL;
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)	
		editor = data->GetEditor();

	if(editor&&cps){
		ICtrlPointPack * pack = editor->GetRidge(hObj);
		if(pack){
			cps->Clone(pack);
			for(int i = 0;i<pack->GetNumberOfCP();i++){
				i_math::vector3df p = cps->At(i)->pos;
				editor->GetGroundPos(p.x,p.z,p);
				p.y += 2.0f;
				cps->At(i)->pos = p;
			}
		}
	}
}

HMapObj CGuiAgent_RidgePoints::_SetKeyPos(const HMapObj &hObj,ICtrlPointPack * cps)
{
	IRidgeEditor * editor = NULL;
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)	
		editor = data->GetEditor();

	ITrrnMapEditor * editorTrrn = NULL;
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if(dataTrrn)
		editorTrrn = dataTrrn->GetTrrnMapEditor();

	if(editor&&editorTrrn)
		return editor->SetCtrlPointPack(hObj,editorTrrn,cps);

	return hObj;
}

DWORD *CGuiAgent_RidgePoints::_GetVer()
{
	IRidgeEditor * editor = NULL;
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)	
		return &(data->ver);
	return NULL;
}

IObjMapEditor * CGuiAgent_RidgePoints::_GetEditor()
{
	IRidgeEditor * editor = NULL;
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)
		return data->GetEditor();
	return NULL;
}

HMapObj CGuiAgent_RidgePoints::_NewObj(IObjMapEditor * editor,ICtrlPointPack * cp)
{
	if(!editor||!cp||cp->GetNumberOfCP()<2)
		return INVALID_HMAPOBJ;

	ITrrnMapEditor * editorTrrn = NULL;
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if(dataTrrn)
		editorTrrn = dataTrrn->GetTrrnMapEditor();

	return ((IRidgeEditor*)editor)->AddRidge(editorTrrn,cp);
}

BOOL CGuiAgent_RidgePoints::_HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos)
{
	IRidgeEditor *editor = (IRidgeEditor*)_GetEditor();
	if(editor)
		return editor->GetGroundPos(rayHit,pos);

	return FALSE;
}

void * CGuiAgent_RidgePoints::_GetSelPointsBuf()
{
	GuiData_Ridge *data = (GuiData_Ridge *)FindData("ridge");
	if(data)
		return &(data->selKeys);
	else
		return NULL;
}

ICtrlPointPack * CGuiAgent_RidgePoints::_NewCtrlPointPack()
{
	IRidgeEditor * editor = (IRidgeEditor *)_GetEditor();
	if(editor)
		return editor->NewCtrlPointPack();
	return NULL;
}

BOOL CGuiAgent_RidgePoints::_DrawOnCreate(ICtrlPointPack *pack,i_math::vector3df &posMove,BOOL cCon2Tail)
{
	if(!pack)
		return FALSE;

	IRenderPort * rp = GetRP();	
	std::vector<i_math::vector3df> lines;

	if(pack->GetNumberOfCP()>1){
		for(int i = 0;i<pack->GetNumberOfCP() - 1;i++){
			lines.push_back(pack->At(i)->pos);
			lines.push_back(pack->At(i+1)->pos);
		}
	}

	if(!pack->IsEmpty()){
		if(cCon2Tail){
			lines.push_back(pack->Back()->pos);
			lines.push_back(posMove);
		}
		else{
			lines.push_back(pack->At(0)->pos);
			lines.push_back(posMove);
		}
	}
	
	i_math::vector3df p;
	for(int i = 0;i<pack->GetNumberOfCP();i++){
		p = pack->At(i)->pos;
		lines.push_back(p);
		p.y = -9999.0f;
		lines.push_back(p);
	}

	if(TRUE){
		p = posMove;
		lines.push_back(p);
		p.y = -9999.0f;
		lines.push_back(p);
	}

	if(!lines.empty())
		rp->Lines(lines.data(),lines.size()/2,0xffff0000);

	return FALSE;
}

HMapObj CGuiAgent_RidgePoints::_HitTest(i_math::line3df &ray,i_math::vector3df * intersec)
{
	GuiData_Ridge * data = (GuiData_Ridge *) FindData("ridge");
	if(!data)
		return TRUE;

	int sel = -1;
	float mind = 999999.0f;
	HMapObj hObjSel = INVALID_HMAPOBJ;

	//找到选中的节点对象
	std::list<RidgeNCache*> &cache = data->nodeCache;
	std::list<RidgeNCache*>::iterator it;
	for(it=cache.begin();it!=cache.end();it++){
		RidgeNCache * n = *it;
		i_math::triangle3df tri;
		i_math::vector3df * p = &(n->tris[0]);
		i_math::vector3df out;
		for(int i = 0;i<n->tris.size();i+=3){
			tri.set(p[0],p[1],p[2]);
			if(tri.getIntersectionWithLimitedLine(ray,out)){
				float d =(float) (out - ray.start).getLengthSQ();
				if(d<mind){
					hObjSel = n->hObj;
					sel = i;
					mind = d;
					if(intersec)
						*intersec = out;
				}
			}
			p += 3;
		}
	}
	return hObjSel;
}

void CGuiAgent_RidgePoints::_BeginCreate()
{
	GuiData_Ridge * data = (GuiData_Ridge *) FindData("ridge");
	if(!data)
		return;
	data->stateCreate = GuiData_Ridge::Creating;
}

void CGuiAgent_RidgePoints::_EndCreate()
{
	GuiData_Ridge * data = (GuiData_Ridge *) FindData("ridge");
	if(!data)
		return;
	data->stateCreate = GuiData_Ridge::Idle;
}