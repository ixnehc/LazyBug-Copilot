
#include "stdh.h"

#include "GuiAgent_ProbeOp.h"

#include "GuiActor_El.h"

#include "GuiData_El.h"

#include "ModBlockBack.h"

#include "RenderSystem/IRenderSystem.h"

#include "WorldSystem/IAssetRenderer.h"

#include "AgentCmdID.h"

BOOL CGuiAgent_ProbeOp::OnLButtonDown(int x,int y,DWORD flag)
{
	CGuiPanel_El * pActor = (CGuiPanel_El *)_GetActor();
	assert(pActor);

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return TRUE;


	//创建过程优先于选择操作
	if(!data->bOnAdd){
		_curOp=Click_Reset;	
		return CGuiAgent_3DNodeOperate::OnLButtonDown(x,y,flag);
	}
	
	GuiData_Trrn * dataTrrn = (GuiData_Trrn *)FindData("terrain");
	if(!dataTrrn)
		return TRUE;	

	IProbeCubeMapEditor * editor = data->GetEditor();
	if(!editor)
		return TRUE;

	//创建过程开始  得到第一个点
	if(_curOp==Click_Reset){
		IRenderPort * rp = GetRP();
		i_math::vector3df pos;
		if(!dataTrrn->GetHitPos(x,y,rp,pos))
			return TRUE;
		
		pos.y += 0.5f;

		_p0 = pos;
		_p1 = pos;
		_curOp = ClickDown_Once;
	}
	
	//创建过程结束
	if(_curOp==ClickUp_Once){
				
		float h0 = min(_p0.y,_p2.y);
		float h1 = max(_p0.y,_p2.y);
		i_math::aabbox3df abb;
		abb.MinEdge.set(_p0.x,h0,_p0.z);
		abb.MaxEdge.set(_p1.x,h1,_p1.z);

		_curOp = Click_Reset;

		i_math::vector3df diag = abb.MaxEdge - abb.MinEdge;
		if(diag.x<0.5f||diag.y<0.5f||diag.z<0.5f)
			return TRUE;

		HMapObj hObj = editor->AddProbeCube(abb,3.0f);

		//--  Undo/Redo support
		CModManager * _mgr = _GetModMgr();
		CModBlockBack * mod = NULL;
		if(_mgr){
			mod = new CModBlockBack(GetView());
			
			i_math::pos2di ptBlk;
			if(editor->GetMapFileBlk(hObj,ptBlk))
				mod->BackupBlocks(&ptBlk,1);
			else
				SAFE_DELETE(mod);
		}
		
		editor->Save();
		
		if(mod){
			mod->SetCallBack<CGuiPanel_El>(pActor,&CGuiPanel_El::OnBackUp,&CGuiPanel_El::OnRestore);
			Mod_New(_mgr,(CModBase *&)(mod));
		}
	}
	
	return FALSE;
}

BOOL CGuiAgent_ProbeOp::OnCommand(DWORD idCmd)
{
	if(FALSE==CGuiAgent_3DNodeOperate::OnCommand(idCmd))
		return FALSE;
	
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return TRUE;

	if(idCmd==ID_AGENT_EL_CREATENEW)
		data->bOnAdd = TRUE;
	
	return TRUE;
}

BOOL CGuiAgent_ProbeOp::OnLButtonUp(int x,int y,DWORD flag)
{
	if(_curOp==Click_Reset&&FALSE==CGuiAgent_3DNodeOperate::OnLButtonUp(x,y,flag))
		return FALSE;

	GuiData_El * data = (GuiData_El *)FindData("envlight");

	if(!data->bOnAdd){
		_curOp=Click_Reset;	
		return TRUE;
	}

	if(_curOp==ClickDown_Once){
		i_math::vector3df p0,p1;
		float h = _p0.y;
		p0.set(min(_p0.x,_p1.x),h,min(_p0.z,_p1.z));
		p1.set(max(_p0.x,_p1.x),h,max(_p0.z,_p1.z));
		_p0 = p0;
		_p1 = p1;
		_p2 = _p1;
		_curOp = ClickUp_Once;
		return FALSE;
	}
	else
		_curOp = Click_Reset;

	return TRUE;
}

BOOL CGuiAgent_ProbeOp::OnMouseMove(int x,int y,DWORD flag)
{
	if(_curOp==Click_Reset&&FALSE==CGuiAgent_3DNodeOperate::OnMouseMove(x,y,flag))
		return FALSE;

	IRenderPort * rp = GetRP();

	HitProbe lineHit;
	rp->CalcHitProbe(x,y,lineHit);
	i_math::plane3df p;

	if(_curOp==ClickDown_Once){ //得到第二点
		
		p.setPlane(_p0,i_math::vector3df(0,1.0f,0));
		p.getIntersectionWithLine(lineHit.start,lineHit.end-lineHit.start,_p1);
		
		//climp to range
		i_math::vector3df vec = _p1 - _p0;
		if(abs(vec.x)>LIMIT_LEN_MAX)
			vec.x = (vec.x>0)?LIMIT_LEN_MAX:-LIMIT_LEN_MAX;
		if(abs(vec.z)>LIMIT_LEN_MAX)
			vec.z = (vec.z>0)?LIMIT_LEN_MAX:-LIMIT_LEN_MAX;
		_p1 = _p0 + vec;
	}
	else if(_curOp==ClickUp_Once){//得到第三点
		ICamera * cam = rp->GetCamera();
		i_math::vector3df eyeDir,vec;
		cam->GetEyeDir(eyeDir);
		
		if(abs(eyeDir.x)>abs(eyeDir.z))
			vec.x = (eyeDir.x>0)?-1.0f:1.0f;	
		else
			vec.z = (eyeDir.z>0)?-1.0f:1.0f;

		p.setPlane(_p1,vec);		
		p.getIntersectionWithLine(lineHit.start,lineHit.end-lineHit.start,_p2);
	
		//climp to range
		float h = _p2.y - _p0.y;
		if(abs(h)>LIMIT_LEN_MAX)
			h = (h>0)?LIMIT_LEN_MAX:-LIMIT_LEN_MAX;
		_p2.y = _p0.y + h;
	}
	
	return FALSE;
}
BOOL CGuiAgent_ProbeOp::OnDraw()
{
	if(FALSE==CGuiAgent_3DNodeOperate::OnDraw())
		return FALSE;

	if(_curOp==Click_Reset)
		return TRUE;

	std::vector<i_math::vector3df> lines;

#define FILL_LINE_PropOp(p0,p1){	\
	lines.push_back(p0);			\
	lines.push_back(p1);			\
}

#define FILL_LINE_PropQuad(p0,p1,p2,p3){	\
	FILL_LINE_PropOp(p0,p1)					\
	FILL_LINE_PropOp(p1,p2)					\
	FILL_LINE_PropOp(p2,p3)					\
	FILL_LINE_PropOp(p3,p0)					\
}

	if(_curOp==ClickDown_Once){
		i_math::vector3df p0,p1,p2,p3;
		float h = _p0.y;
		p0.set(min(_p0.x,_p1.x),h,min(_p0.z,_p1.z));
		p1.set(max(_p0.x,_p1.x),h,max(_p0.z,_p1.z));
		p2.set(p0.x,h,p1.z);
		p3.set(p1.x,h,p0.z);
		FILL_LINE_PropQuad(p0,p2,p1,p3);
	}
	else if(_curOp==ClickUp_Once){
		float h0 = min(_p0.y,_p2.y);
		float h1 = max(_p0.y,_p2.y);
		i_math::aabbox3df abb;
		abb.MinEdge.set(_p0.x,h0,_p0.z);
		abb.MaxEdge.set(_p1.x,h1,_p1.z);
		i_math::vector3df c[8];
		abb.getCorners(c);
		FILL_LINE_PropQuad(c[0],c[1],c[3],c[2]);
		FILL_LINE_PropQuad(c[4],c[5],c[7],c[6]);

		FILL_LINE_PropQuad(c[2],c[3],c[7],c[6]);
		FILL_LINE_PropQuad(c[0],c[1],c[5],c[4]);


		FILL_LINE_PropQuad(c[0],c[2],c[6],c[4]);
		FILL_LINE_PropQuad(c[1],c[3],c[7],c[5]);
	}
	
	IRenderPort * rp = GetRP();
	assert(rp);

	if(lines.size())
		rp->Lines(lines.data(),lines.size()/2,0xffff0000);	

	return TRUE;
}
BOOL CGuiAgent_ProbeOp::OnKeyDown(char c,DWORD flag)
{
	if(_curOp==Click_Reset&&FALSE==CGuiAgent_3DNodeOperate::OnKeyDown(c,flag))
		return FALSE;

	if(c!=VK_DELETE)
		return TRUE;

	IProbeCubeMapEditor * editor = NULL;

	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();

	int nSel = data->hObjSels.size();

	if(!editor||(0==nSel))
		return TRUE;

	std::vector<i_math::pos2di> ptBlks(nSel);

	for(int i = 0;i<data->hObjSels.size();i++){
		HMapObj & hObj = data->hObjSels[i];
		editor->GetMapFileBlk(hObj,ptBlks[i]);
	}
	
	// -- Undo/Redo support
	CModManager * mgr = _GetModMgr();
	CModBlockBack * mod = NULL;
	if(mgr){
		mod = new CModBlockBack(GetView());
		mod->BackupBlocks(ptBlks.data(),nSel);
	}

	for(int i = 0;i<data->hObjSels.size();i++){
		HMapObj & hObj = data->hObjSels[i];
		editor->Delete(hObj);
	}

	//commit
	if(mod){
		CGuiPanel_El * pActor = (CGuiPanel_El *)_GetActor();
		assert(pActor);
		mod->SetCallBack<CGuiPanel_El>(pActor,&CGuiPanel_El::OnBackUp,&CGuiPanel_El::OnRestore);
		Mod_New(mgr,(CModBase *&)(mod));
	}

	//Update Selection state
	data->hObjSels.clear();
	CGuiPanel_El * pActor = (CGuiPanel_El *)_GetActor();
	assert(pActor);
	pActor->UpdateSel();
	
	return TRUE;
}
BOOL CGuiAgent_ProbeOp::OnRButtonClick(int x,int y,DWORD flag)
{	
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return TRUE;
	
	if(!data->bOnAdd)
		_AddMenu("新建",ID_AGENT_EL_CREATENEW);

	if(_curOp==Click_Reset&&FALSE==CGuiAgent_3DNodeOperate::OnRButtonClick(x,y,flag))
		return FALSE;

	//创建未成功过程结束
	if(data->bOnAdd)
	{
		_curOp = Click_Reset;
		data->bOnAdd = FALSE;
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void * CGuiAgent_ProbeOp::_GetSelBuf()
{
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(!data)
		return NULL;
	
	return &(data->hObjSels);
}

IObjMapEditor * CGuiAgent_ProbeOp::_GetEditor()
{
	IProbeCubeMapEditor * editor = NULL;
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		editor = data->GetEditor();
	
	return editor;
}

DWORD *CGuiAgent_ProbeOp::_GetVer()
{
	GuiData_El * data = (GuiData_El *)FindData("envlight");
	if(data)
		return &(data->ver);
	return NULL;
}

H3DNode CGuiAgent_ProbeOp::_Clone(H3DNode node)
{
	HMapObj hObj = INVALID_HMAPOBJ;

	IProbeCubeMapEditor * editor = (IProbeCubeMapEditor *)_GetEditor();
	if(editor){
		ProbeCubeInfo info;
		HMapObj hNode = HMapObj(node);
		if(editor->GetInfo(hNode,info))
			hObj = editor->AddProbeCube(info.aabb,info.density);
	}

	return H3DNode(hObj);
}

void CGuiAgent_ProbeOp::_CollectEnvelope(H3DNode *node,DWORD nNodes,Envelope &evlp)
{
	IProbeCubeMapEditor * editor = (IProbeCubeMapEditor *)_GetEditor();

	if(!node||nNodes==0)
		return;

#define Fill_Line(i0,i1){			\
	lines.push_back(corner[i0]);	\
	lines.push_back(corner[i1]);	\
}

	ProbeCubeInfo info;
	for(int i = 0;i<nNodes;i++){

		HMapObj hObj = HMapObj(node[i]);
		if(!editor->GetInfo(hObj,info))
			continue;

		i_math::vector3df corner[8];
		info.aabb.getCorners(corner);
		
		std::vector<i_math::vector3df> &lines = evlp.lines;
		Fill_Line(0,1)
		Fill_Line(0,4)
		Fill_Line(5,1)
		Fill_Line(5,4)

		Fill_Line(2,6)
		Fill_Line(2,3)
		Fill_Line(7,6)
		Fill_Line(7,3)

		Fill_Line(2,0)
		Fill_Line(3,1)
		Fill_Line(7,5)
		Fill_Line(6,4)
	}
}



