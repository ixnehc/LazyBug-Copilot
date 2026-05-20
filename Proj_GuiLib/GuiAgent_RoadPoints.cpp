
#include "stdh.h"

#include "GuiAgent_RoadPoints.h"

#include "GuiData_Road.h"

#include "AgentCmdID.h"

#include "MapObjUtil.h"

#include "WorldSystem/ITrrn.h"

#include "GuiData.h"

#include "spline/CubicSpline.h"

HMapObj * CGuiAgent_RoadPoints::_GetSelObj()
{
	GuiData_Road *data = (GuiData_Road *)FindData("road");
	if(data)	
		return &(data->hObjSel);
	return NULL;
}

void CGuiAgent_RoadPoints::_GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps)
{
	IRoadEditor * editor = NULL;
	GuiData_Road *data = (GuiData_Road*)FindData("road");
	if(data)	
		editor = data->GetEditor();

	if(editor&&cps){
		ICtrlPointPack * pack = editor->GetRoad(hObj);
		if(pack){
			cps->Clone(pack);
			for(int i = 0;i<pack->GetNumberOfCP();i++){
				i_math::vector3df p = cps->At(i)->pos;
				editor->GetGroundPos(p.x,p.z,p);
				p.y += ROAD_POSH2GROUD;
				cps->At(i)->pos = p;
			}
		}
	}
}

HMapObj CGuiAgent_RoadPoints::_SetKeyPos(const HMapObj &hObj,ICtrlPointPack * cps)
{
	IRoadEditor * editor = NULL;
	GuiData_Road *data = (GuiData_Road *)FindData("road");
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

DWORD *CGuiAgent_RoadPoints::_GetVer()
{
	IRoadEditor * editor = NULL;
	GuiData_Road *data = (GuiData_Road *)FindData("road");
	if(data)	
		return &(data->ver);
	return NULL;
}

IObjMapEditor * CGuiAgent_RoadPoints::_GetEditor()
{
	IRoadEditor * editor = NULL;
	GuiData_Road *data = (GuiData_Road *)FindData("road");
	if(data)
		return data->GetEditor();
	return NULL;
}

HMapObj CGuiAgent_RoadPoints::_NewObj(IObjMapEditor * editor,ICtrlPointPack * cp)
{
	GuiData_Road *data = (GuiData_Road *)FindData("road");

	if(!editor||!data||!cp||cp->GetNumberOfCP()<2)
		return INVALID_HMAPOBJ;

	ITrrnMapEditor * editorTrrn = NULL;
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if(dataTrrn)
		editorTrrn = dataTrrn->GetTrrnMapEditor();

	return ((IRoadEditor*)editor)->AddRoad(editorTrrn,cp,data->roadProp);
}

BOOL CGuiAgent_RoadPoints::_HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos)
{
	IRoadEditor *editor = (IRoadEditor*)_GetEditor();
	if(editor)
		return editor->GetGroundPos(rayHit,pos);

	return FALSE;
}

void * CGuiAgent_RoadPoints::_GetSelPointsBuf()
{
	GuiData_Road *data = (GuiData_Road *)FindData("road");
	if(data)
		return &(data->selKeys);
	else
		return NULL;
}

ICtrlPointPack * CGuiAgent_RoadPoints::_NewCtrlPointPack()
{
	IRoadEditor * editor = (IRoadEditor *)_GetEditor();
	if(editor)
		return editor->NewCtrlPointPack();
	return NULL;
}

BOOL CGuiAgent_RoadPoints::_DrawOnCreate(ICtrlPointPack *pack,i_math::vector3df &posMove,BOOL cCon2Tail)
{
	GuiData_Road * data = (GuiData_Road *)FindData("road");
	IRoadEditor * editor = (IRoadEditor *)_GetEditor();

	if(!pack||!data||!editor)
		return FALSE;

	std::vector<i_math::vector3df> lines;
	i_math::vector3df hitPos;

	IRenderPort * rp = GetRP();
	DWORD n = pack->GetNumberOfCP();

	editor->GetGroundPos(posMove.x,posMove.z,hitPos);
	lines.push_back(hitPos);
	hitPos.y += ROAD_POSH2GROUD;
	lines.push_back(hitPos);

	if(!pack->IsEmpty()){
		CCubicSpline spline;
		i_math::vector3df pos,vec,up(0,1.0f,0),basisVec;
		i_math::quatf q;

		RoadProp &prop = data->roadProp;

		for(int i = 0;i<n;++i){
			pos = pack->At(i)->pos;
			spline.AddNode(pos,q);
		}
		spline.AddNode(posMove,q);
		spline.BuildSNS();

		float step = 1.0f/float(n);
		float ftime = 0;

		std::vector<i_math::vector3df> segPos;
		for(int i = 0;i<n;i++){
			pos = pack->At(i)->pos;//spline.GetPosition(ftime);
			editor->GetGroundPos(pos.x,pos.z,pos);
			segPos.push_back(pos);
			ftime += step;
		}
		editor->GetGroundPos(posMove.x,posMove.z,posMove);
		segPos.push_back(posMove);

		size_t nSeg = segPos.size();
		std::vector<i_math::vector3df> basis(nSeg);
		i_math::vector3df p0,p1,p2;
		for(int i = 1;i<segPos.size();i++){
			p0 = segPos[i-1];
			p1 = segPos[i+0];
			vec = p1 - p0;
			basisVec = vec.crossProduct(up);
			basisVec.setLength(data->roadProp.w/2);
			if(i>1&&basisVec.dotProduct(basis[i-1])<0)
				basisVec = -basisVec;
			basis[i] = basisVec;
		}

		if(nSeg==2){
			vec = segPos[1] - segPos[0];
			basisVec = vec.crossProduct(up);
			basisVec.setLength(data->roadProp.w/2);
			basis[0] = basis[1] = basisVec;	
		}
		else if(nSeg>1){
			basis[0] = basis[1];
		}

		std::vector<i_math::vector3df> basis0(nSeg);
		std::vector<i_math::vector3df> basis1(nSeg);
		for(int i = 0;i<segPos.size();i++){
			basis0[i] = segPos[i] + basis[i];
			basis1[i] = segPos[i] - basis[i];
			editor->GetGroundPos(basis0[i].x,basis0[i].z,basis0[i]);
			editor->GetGroundPos(basis1[i].x,basis1[i].z,basis1[i]);
			basis0[i].y += ROAD_POSH2GROUD;
			basis1[i].y += ROAD_POSH2GROUD;
		}

#define FILL_TRI(x0,x1,x2){		\
	lines.push_back(x0);		\
	lines.push_back(x1);		\
	lines.push_back(x1);		\
	lines.push_back(x2);		\
	lines.push_back(x2);		\
	lines.push_back(x0);		\
		}
		for(int i = 0;i<segPos.size()-1;++i){
			FILL_TRI(basis0[i],basis0[i+1],segPos[i]);
			FILL_TRI(basis0[i+1],segPos[i+1],segPos[i]);
			FILL_TRI(segPos[i],segPos[i+1],basis1[i+1]);
			FILL_TRI(segPos[i],basis1[i+1],basis1[i]);
		}
#undef  FILL_TRI
	}

	rp->Lines(lines.data(),lines.size()/2,0xffff0000);

	return TRUE;
}

HMapObj CGuiAgent_RoadPoints::_HitTest(i_math::line3df &ray,i_math::vector3df * intersec)
{
	GuiData_Road * data = (GuiData_Road *) FindData("road");
	if(!data)
		return TRUE;

	HMapObj hObjSel = INVALID_HMAPOBJ;
	IRoadEditor * editor = data->GetEditor();
	if(editor)
		hObjSel = editor->HitTest(ray);

	return hObjSel;
}

void CGuiAgent_RoadPoints::_BeginCreate()
{
	GuiData_Road * data = (GuiData_Road *) FindData("road");
	if(!data)
		return;
	data->stateWork = GuiData_Road::Creating;	
}

void CGuiAgent_RoadPoints::_EndCreate()
{
	GuiData_Road * data = (GuiData_Road *) FindData("road");
	if(!data)
		return;
	data->stateWork = GuiData_Road::Idle;	
}





