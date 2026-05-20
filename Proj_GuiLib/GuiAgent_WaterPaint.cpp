
#include "stdh.h"

#include "RenderSystem/IRenderPort.h"
#include "RenderSystem/IFont.h"


#include "GuiAgent_WaterPaint.h"

#include "GuiData_Water.h"

#include "WorldSystem/IWorldSystem.h"

#include "RenderSystem/IRenderSystem.h"

#include "ModBlockBack.h"

#include "GuiActor_Water.h"

#include "stringparser/stringparser.h"

#define ID_AGENT_WaterPaint  30023
#define ID_AGENT_WaterClear  30024

void CGuiAgent_WaterPaint::_CalQuad(int x,int y,DWORD flag)
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return;

	IWaterEditor * editor = dataWater->GetWaterEditor();
	if(!editor)
		return;

	IBrushLib * pLib = editor->GetBrushLib();
	WBrushInfo * brInfo = (WBrushInfo *)pLib->GetInfo(dataWater->br);
	if(!brInfo)
		return;

	IRenderPort * rp = GetRP();
	if(!rp)
		return;

	HitProbe probe;
	rp->CalcHitProbe(x,y,probe);
	i_math::vector3df vintersec;
	i_math::plane3df plane;
		
	float height =  brInfo->height;
	plane.setPlane(i_math::vector3df(0,height,0),i_math::vector3df(0,1.0f,0));

	if(plane.getIntersectionWithLine(probe.start,probe.end-probe.start,_vintersec)){

		float h = brInfo->height + 0.1f;

		float t = dataWater->brsize/2.0f;

		i_math::vector3df vborder[4];

		i_math::pos2df p0,p1;
		p0.x = _vintersec.x + t + 2.0f; // x0
		p1.x = _vintersec.x - t + 2.0f; // x1
		p0.y = _vintersec.z + t + 2.0f; // y0
		p1.y = _vintersec.z - t + 2.0f; // y1

		p0.scale_signed(BLOCK_LENGTH);
		p1.scale_signed(BLOCK_LENGTH);
		p0 *= BLOCK_LENGTH;
		p1 *= BLOCK_LENGTH;

		_vborder[0].set(p0.x,h,p0.y);
		_vborder[1].set(p1.x,h,p0.y);
		_vborder[2].set(p1.x,h,p1.y);
		_vborder[3].set(p0.x,h,p1.y);
	}
}

void CGuiAgent_WaterPaint::_Paint()
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return ;

	IWaterEditor * editor = dataWater->GetWaterEditor();
	if(!editor)
		return ;

	i_math::pos2di sb,eb;

	sb.x = int(_vborder[2].x);
	sb.y = int(_vborder[2].z);

	eb.x = int(_vborder[0].x);
	eb.y = int(_vborder[0].z);

	int len = (int)editor->GetMapBlockLen();

	CGuiPanel_Water * actor = (CGuiPanel_Water *)_GetActor();
	if(!actor)
		return;

	CModManager * mgr = actor->GetModMgr();
	if(mgr){
		for(int i = sb.x;i<eb.x;i++){
			for(int j = sb.y;j<eb.y;j++){
				i_math::pos2di blk(i,j);
				blk.scale_signed(len);
				blk *= len/BLOCK_LENGTH;
				int k = _blks.size()-1;
				for(;k>=0&&blk!=_blks[k];k--);

				if(k<0)
					_blks.push_back(blk);
			}
		}

		//传入单位为米
		editor->Paint(dataWater->br,sb,eb,dataWater->op);
	}		
}

void CGuiAgent_WaterPaint::_NotifyPaintFinish()
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return ;

	IWaterEditor * editor = dataWater->GetWaterEditor();
	if(!editor)
		return ;

	i_math::pos2di sb,eb;

	sb.x = int(_vborder[2].x);
	sb.y = int(_vborder[2].z);

	eb.x = int(_vborder[0].x);
	eb.y = int(_vborder[0].z);

	sb.scale_signed(4);
	eb.scale_signed(4);

	editor->Paint(dataWater->br,sb,eb,WPaintOp_Complete);
}

BOOL CGuiAgent_WaterPaint::OnBeginDrag(int x,int y,DWORD flag)
{
	_blks.clear();
	OnDrag(x,y,flag);
	return TRUE;
}

void CGuiAgent_WaterPaint::OnDrag(int x,int y,DWORD flag)
{
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(dataWater->op!=WPaintOP_Idle){
		_CalQuad(x,y,flag);
		_Paint();
	}	
}

void CGuiAgent_WaterPaint::OnEndDrag(int x,int y,DWORD flag)
{
	IWaterEditor * editor = NULL;
	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(dataWater)
		editor = dataWater->GetWaterEditor();

	if(!editor)
		return;

	_NotifyPaintFinish();

	CGuiPanel_Water * actor = (CGuiPanel_Water *)_GetActor();
	assert(actor);
	CModManager * mgr = actor->GetModMgr();
	if(mgr)
	{
		CModBlockBack * mod = new CModBlockBack(GetView());
		mod->BackupBlocks(_blks.data(),_blks.size());
		editor->Save();
		Mod_New(mgr,(CModBase *&)mod);
	}	
}

BOOL CGuiAgent_WaterPaint::OnRButtonClick(int x,int y,DWORD flag)
{
	GuiData_Water * data = (GuiData_Water *)FindData("water");
	if(data&&data->op != WPaintOP_Idle){
		data->op = WPaintOP_Idle;
		return FALSE;
	}
	else{
		_AddMenu("Paint",ID_AGENT_WaterPaint);
		_AddMenu("Clear",ID_AGENT_WaterClear);
	}
	
	return TRUE;
}

BOOL CGuiAgent_WaterPaint::OnCommand(DWORD idCmd)
{
	GuiData_Water * data = (GuiData_Water *)FindData("water");
	switch(idCmd){
		case ID_AGENT_WaterPaint:
			data->op = WPaintOP_Add; // paint
			break;
		case ID_AGENT_WaterClear:
			data->op = WPaintOP_Clear;  //clear
			break;
		default:
			break;
	}

	return TRUE;
}

BOOL CGuiAgent_WaterPaint::OnDraw()
{
	IRenderPort * rp = GetRP();
	if(!rp)
		return TRUE;

	GuiData_Water * dataWater = (GuiData_Water *)FindData("water");
	if(!dataWater)
		return TRUE;

	IWaterEditor * editor = dataWater->GetWaterEditor();
	if(!editor)
		return TRUE;


	HMapObj hObj = INVALID_HMAPOBJ;
	Eye2WaterState eyeState= editor->GetEyeState(rp->GetCamera(),hObj);	

	if(TRUE){
		DrawFontArg argDraw;
		argDraw.m_ptLoc.set(200,0);
		std::string msgDump;
		switch(eyeState){
			case EWState_No:
				msgDump = "没有水"; break;
			case EWState_Below:
				msgDump = "在水下"; break;
			case EWState_Above:
				msgDump = "在水上"; break;
			case EWState_Intersec:
				msgDump = "在临界位置"; break;
			default : break;
		}
		rp->DrawText(msgDump.c_str(),argDraw);
	}

	i_math::pos2di pt;
	if(dataWater->op!=WPaintOP_Idle){
		_GetCursorPos(pt);
		_CalQuad(pt.x,pt.y,0);
		i_math::vector3df tris[] ={_vborder[1],_vborder[0],_vborder[2],
			_vborder[2],_vborder[0],_vborder[3]
		};
		rp->Triangles(tris,2,ColorAlpha(0xffff,0xff));
	}

	return TRUE;
}



