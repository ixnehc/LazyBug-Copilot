
#include "stdh.h"

#include "GuiAgent_BafflesPoints.h"

#include "GuiData_Baffles.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

HMapObj * CGuiAgent_BafflesPoints::_GetSelObj()
{
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)	
		return &(data->hObjSel);
	return NULL;
}

void CGuiAgent_BafflesPoints::_GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps)
{
	IBafflesEditor * editor = NULL;
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)	
		editor = data->GetEditor();
	if(editor&&cps){
		ICtrlPointPack * pack = editor->GetCtrlPointPack(hObj);
		if(pack){
			cps->Clone(pack);
			for(int i = 0;i<pack->GetNumberOfCP();i++){
				i_math::vector3df p = cps->At(i)->pos;
				editor->GetGroundPos(p.x,p.z,p);
				cps->At(i)->pos = p;
			}
		}
	}
}

HMapObj CGuiAgent_BafflesPoints::_SetKeyPos(const HMapObj &hObj,ICtrlPointPack * cps)
{
	IBafflesEditor * editor = NULL;
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)	
		editor = data->GetEditor();
	
	if(editor)
		return editor->SetCtrlPointPack(hObj,cps);
	
	return hObj;
}

DWORD *CGuiAgent_BafflesPoints::_GetVer()
{
	IBafflesEditor * editor = NULL;
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)	
		return &(data->ver);
	return NULL;
}

IObjMapEditor * CGuiAgent_BafflesPoints::_GetEditor()
{
	IBafflesEditor * editor = NULL;
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)
		return data->GetEditor();
	return NULL;
}

HMapObj CGuiAgent_BafflesPoints::_NewObj(IObjMapEditor * editor,ICtrlPointPack * cp)
{
	if(editor&&cp)
		return ((IBafflesEditor * )editor)->AddBaffles(cp);
	else
		return INVALID_HMAPOBJ;
}

BOOL CGuiAgent_BafflesPoints::_HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos)
{
	IBafflesEditor * editor = (IBafflesEditor *)_GetEditor();
	if(editor)
		return editor->GetGroundPos(rayHit,pos);
	
	return FALSE;
}

void * CGuiAgent_BafflesPoints::_GetSelPointsBuf()
{
	GuiData_Baffles *data = (GuiData_Baffles *)FindData("baffle");
	if(data)
		return &(data->selKeys);
	else
		return NULL;
}

ICtrlPointPack * CGuiAgent_BafflesPoints::_NewCtrlPointPack()
{
	IBafflesEditor * editor = (IBafflesEditor *)_GetEditor();
	if(editor)
		return editor->NewCtrlPointPack();
	return NULL;
}

BOOL CGuiAgent_BafflesPoints::_DrawOnCreate(ICtrlPointPack *pack,i_math::vector3df &posMove,BOOL cCon2Tail)
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

HMapObj CGuiAgent_BafflesPoints::_HitTest(i_math::line3df &ray,i_math::vector3df * intersec)
{
	GuiData_Baffles * data = (GuiData_Baffles *) FindData("baffle");
	if(!data)
		return TRUE;

	int sel = -1;
	float mind = 999999.0f;
	HMapObj hObjSel = INVALID_HMAPOBJ;

	//找到选中的节点对象
	std::list<BafflesNCache*> &cache = data->nodeCache;
	std::list<BafflesNCache*>::iterator it;
	for(it=cache.begin();it!=cache.end();it++){
		BafflesNCache * n = *it;
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

void CGuiAgent_BafflesPoints::_BeginCreate()
{
	GuiData_Baffles * data = (GuiData_Baffles *) FindData("baffle");
	if(!data)
		return;
	data->stateWork = GuiData_Baffles::Creating;
}

void CGuiAgent_BafflesPoints::_EndCreate()
{
	GuiData_Baffles * data = (GuiData_Baffles *) FindData("baffle");
	if(!data)
		return;
	data->stateWork = GuiData_Baffles::Idle;	
}