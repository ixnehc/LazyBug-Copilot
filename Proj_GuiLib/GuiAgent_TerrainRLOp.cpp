#include "stdh.h"

#include ".\guiagent_terrainrlop.h"

#include "RenderSystem/IRenderPort.h"


#include "WorldSystem/ITrrn.h"

#include "WorldSystem/IAssetSystemDefines.h"

#include "RenderSystem/IRenderSystem.h"

#include "ModBlockBack.h"

#include "BrushUtil.h"

#include "WorldSystem/IAssetEventer.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/IAssetSystem.h"

#include "GuiActor_Trrn.h"

#include "BrGeomRaiseLower.h"

#include "pin/pin.h"

GuiData_Trrn * GetTrrnData(CGuiView * view)
{
	assert(view);
	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)view->FindData("terrain");
	return dataTrrn;
}

ITrrnMapEditor * GetTrrnEditor(CGuiView * view)
{
	assert(view);
	GuiData_Trrn *dataTrrn=(GuiData_Trrn *)view->FindData("terrain");
	if(!dataTrrn)
		return NULL;
	ITrrnMapEditor *editor=dataTrrn->GetTrrnMapEditor();
	
	return editor;
}
BOOL CGuiAgent_TerrainRLOp::PrepareSeedMap(CGuiView * view,int x,int y)
{
	if(!view)
		return FALSE;
	ITrrnMapEditor * editor = GetTrrnEditor(view);
	if(!editor)
		return FALSE;
	
	GuiData_Trrn *  data = GetTrrnData(view);
	assert(data);

	i_math::vector3df vHit;

	if(TRUE)
	{
		IRenderPort * rp = view->GetRP();
		HitProbe hitProb;
		if(FALSE == rp->CalcHitProbe(x,y,hitProb,2000.0f))
			return FALSE;
		if(FALSE == editor->GetHitPos(hitProb,TRUE,vHit))
			return FALSE;
	}

	CBrushUtil * pTool = ((CBrushUtil *)_tool);
	if(!pTool)
		return FALSE;

	TrrnSeedMapArg::Purpose purpose = pTool->_purpose;

	pTool->_arg.vCenter = vHit;
	pTool->_arg.purpose = purpose;
	pTool->_arg.idBr = pTool->_idBr;

	editor->CalcSeedMap(pTool->_seedMap,pTool->_arg);

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
CGuiAgent_TerrainRLOp::CGuiAgent_TerrainRLOp(void)
{
	_bOp = FALSE;
	_tool = NULL;
	_clock=0;
	_tickLast=0;

}
CGuiAgent_TerrainRLOp::~CGuiAgent_TerrainRLOp(void)
{
}
void CGuiAgent_TerrainRLOp::SetTool(CToolBase * tool)
{
	_tool = tool;
}
BOOL CGuiAgent_TerrainRLOp::OnMouseMove(int x,int y,DWORD flag)
{
	CGuiView * view = GetGuiView();
	assert(view);

	PrepareSeedMap(view,x,y);

	_x = x;
	_y = y;

	if (_bOp)
	{
		_clock=0;
		if (_UpdateClock())
			UpdateTerrain();
	}

	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::OnMouseWheel(int delta,DWORD flag)
{
	CGuiView * view = GetGuiView();
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(!data||!data->bPaint)
		return TRUE;
	
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	if(flag&CtrlOpFlag_CtrlDown)
	{
		float r0 = pTool->_lnk_radius0->GetFVal();
		float r1 = pTool->_lnk_radius1->GetFVal();
		
		float minV,maxV;
		pTool->_lnk_radius0->GetLimits(minV,maxV);
		float d = (delta/WHEEL_DELTA)*(maxV - minV)*0.01f;
		
		d = (d>r0)?r0:d;

		r0 -= d;
		r1 -= d;
		
		pTool->_lnk_radius0->SetValue(r0,TRUE);
		pTool->_lnk_radius1->SetValue(r1,TRUE);

		 PrepareSeedMap(view,_x,_y);
		_Redraw();
        
		return FALSE;
	}

	return TRUE;
}

BOOL CGuiAgent_TerrainRLOp::OnLButtonDown(int x,int y,DWORD flag)
{
	CGuiView * view = GetGuiView();
	assert(view);
	PrepareSeedMap(view,x,y);
	
	GuiData_Trrn * data = (GuiData_Trrn *)GetTrrnData(view);
	
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	
	if(pTool&&data)
	{
		m_center = pTool->_arg.vCenter;
		ITrrnMapEditor * editor = data->GetTrrnMapEditor();
		if(editor)
			editor->BeginModify();
	}

	_bOp = TRUE;
	OccupyFocus(OpType_Mouse);

	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::OnLButtonUp(int x,int y,DWORD flag)
{
	_bOp = FALSE;
	
	DiscardFocus(OpType_Mouse);

	CGuiView * view = GetGuiView();
	assert(view);
	GuiData_Trrn * data = (GuiData_Trrn *)GetTrrnData(view);
	if(data)
	{
		ITrrnMapEditor * editor = data->GetTrrnMapEditor();
		if(editor)
			editor->EndModify();
	}
	
	ITrrnMap * map = NULL;
	if(data)
		map = data->GetTrrnMap();
	if(!map)
		return TRUE;
	
	CGeActor *actor = _GetActor();
	assert(actor);

	SaveToFile();


	return TRUE;
}
void CGuiAgent_TerrainRLOp::NotifyTrrnChangeHeight(BOOL bSave,std::vector<i_math::pos2di> &blks)
{
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	ITrrnMapEditor * editor = NULL;

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(NULL==pTool)
		return;

	switch(pTool->m_mode)
	{
	// 0,1:raise lower,2: flatten,3:smooth ,only this operation can change terrain height.
	case 0:	case 1: 
	case 2:	case 3:
		break;
	default:
		return;
	}

	if(data)
		editor = data->GetTrrnMapEditor();
	if(NULL==editor)
		return;

	HkChangeTrrnHeight event;
	event.editor = editor;
	event.bSave = bSave;
	event.blks.assign(blks.begin(),blks.end());

	assert(data->pES);
	IAssetSystem * pAS = data->pES->GetAS();
	assert(pAS);
	IAssetEventer * eventer = pAS->GetEventer();
	assert(eventer);
	eventer->SendHook(event);
}

BOOL CGuiAgent_TerrainRLOp::OnSetCursor(int x,int y,DWORD flag)
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	if(pTool->m_mode<2)
	{
		// raise lower
	}
	else if(pTool->m_mode == 2)
	{
		// flatten
	}
	else if(pTool->m_mode == 3)
	{
		// smooth
	}
	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::UpdateTerrain()
{
	CGuiView * view = GetGuiView();
	assert(view);

	GuiData_Trrn * data = (GuiData_Trrn *)GetTrrnData(view);
	//检测绘制状态
	if(!data->bPaint)
		return TRUE;

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	POINT cursorPoint;
	GetCursorPos(&cursorPoint);
	CWnd * pWnd = view->GetWnd();
	CRect rc;
	pWnd->GetWindowRect(&rc);
	
	if(!rc.PtInRect(cursorPoint))
		return TRUE;

	ITrrnMapEditor * edit = GetTrrnEditor(view);

	PrepareSeedMap(view,_x,_y);
	
	if(pTool->_seedMap.points.size()>0)
	{
		if(pTool->m_mode<2)
			RaiseLower(data);
		else if(pTool->m_mode == 2)
			Flatten(data);
		else if(pTool->m_mode == 3)
			Smooth(data);
	}
	
	if(pTool->m_mode>3)
		Hole(data);

	InvalidateView();

	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::Hole(GuiData_Trrn * data)
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	ITrrnMapEditor *editor = data->GetTrrnMapEditor();

	assert(pTool->m_mode==4||pTool->m_mode==5);
	
	BOOL bRemove = (pTool->m_mode==5);

	if(!editor||FALSE == editor->ModHole(pTool->_seedMap,bRemove))
		return FALSE;

	RecordModify();

	return TRUE;
}
void CGuiAgent_TerrainRLOp::RecordModify()
{

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return;
	
	CGuiView * view = GetGuiView();
	assert(view);
	GuiData_Trrn * data = GetTrrnData(view);
	
	for(int i = 0;i<pTool->_seedMap.blocksAffected.size();i++)
	{
		i_math::pos2di & block = pTool->_seedMap.blocksAffected[i];
		
		__int64 key = *(__int64*)(&block);

		m_srcBlocks.insert(key);
	}
}

void CGuiAgent_TerrainRLOp::NotifyTrrnHeightChange()
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return;
	NotifyTrrnChangeHeight(FALSE,pTool->_seedMap.blocksAffected);
}

BOOL CGuiAgent_TerrainRLOp::RaiseLower(GuiData_Trrn * data)
{	

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return FALSE;

	ITrrnMapEditor * edit = data->GetTrrnMapEditor();

	float hardness = pTool->_hardness*0.1f;
	float height = pTool->_speed*0.01f; /* 0.1f --- 10.1f*/
	float e = 2.71828183f;
	float innerRadius = pTool->_arg.radius;
	float outterRadius = pTool->_arg.radius2;
	float rg = outterRadius - innerRadius;
	i_math::vector3df center = pTool->_arg.vCenter;

	if(rg<0.0001f) 
		rg = 0.0001f;


	for(int i = 0;i<pTool->_seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = pTool->_seedMap.points[i];
		float w =sqrtf((center.x - point.v.x)*(center.x - point.v.x) + (center.z - point.v.z)*(center.z - point.v.z));
		w = (w<innerRadius)?1.0f:(1.0f-(w-innerRadius)/rg);
		point.wt = w*pow(e,hardness*(w - 1.0f));
	}


	height = (pTool->m_mode==1)?-height:height;
		
	if(FALSE ==edit->AddHeight(pTool->_seedMap,TRUE,height,0.0f,pTool->_bAccurate))
		return FALSE;
	
	RecordModify();
	NotifyTrrnHeightChange();

	return TRUE;
}
void CGuiAgent_TerrainRLOp::SaveToFile()
{
	CGuiView * view = GetGuiView();
	assert(view);
	
	CGeActor * actor = _GetActor();
	CModManager * modmgr = actor->GetModMgr();
	
	GuiData_Trrn * data = GetTrrnData(view);
	ITrrnMap * map = data->GetTrrnMap();
	
	if(!map||m_srcBlocks.size()==0)
	{
		m_srcBlocks.clear();
		return;
	}

	if(modmgr)
	{
		CModBlockBack * mod = new CModBlockBack(view);
		std::vector<i_math::pos2di> blocks;
		
		std::set<__int64>::iterator it;
		for(it = m_srcBlocks.begin();it!=m_srcBlocks.end();it++)
		{
			i_math::pos2di b = *(i_math::pos2di*)(&(*it));
			blocks.push_back(b);
		}
		mod->BackupBlocks(blocks.data(),blocks.size());
		
		map->SaveModified();
		blocks.clear();
		NotifyTrrnChangeHeight(TRUE,blocks);
		Mod_New(modmgr,(CModBase *&)mod);
	}

	m_srcBlocks.clear();
}
BOOL CGuiAgent_TerrainRLOp::Flatten(GuiData_Trrn * data)
{
	CBrGeomRaiseLower * pTool = (CBrGeomRaiseLower *)(_tool);
	if(!pTool)
		return FALSE;

	ITrrnMapEditor * edit = data->GetTrrnMapEditor();

	float speed = pTool->_speed;

	BOOL bSecondaryHeight = pTool->_check_height2.GetCheck();

	float low,high;

	if (!bSecondaryHeight)
	{
		float distHeight = 0;

		BOOL bSet = pTool->_check_height.GetCheck();
		if(bSet)
			distHeight = pTool->_height;
		else
			distHeight = m_center.y;

		low = high = pTool->_seedMap.points[0].v.y;

		for(int i = 0;i<pTool->_seedMap.points.size();i++)
		{
			TrrnSeedMap::SeedPoint & point = (pTool->_seedMap.points[i]);
			low = min(low,point.v.y);
			high = max(high,point.v.y);
		}

		if(low == high){ 
			if(low==distHeight)
				return TRUE;
			else{
				high = distHeight + 20000.0f;
				low  = distHeight - 20000.0f;
			}
		}

		float step = speed/20.0f;

		for(int i = 0;i<pTool->_seedMap.points.size();i++)
		{
			TrrnSeedMap::SeedPoint & point = (pTool->_seedMap.points[i]);

			float r  = distHeight - point.v.y;
			float h = 0;
	// 		if(!bSet){
			float v = step*point.wt;
			r = (abs(r)<v)?r:(r>0)?v:(-v);
			h = point.v.y + r;
	// 		}
	// 		else
	// 		{
	// 			//设置了高度 将一步调整到指定的高度
	// 			h = distHeight; 
	// 		}
			point.wt = (h - low)/(high - low);
		}
		if(FALSE ==edit->AddHeight(pTool->_seedMap,FALSE,high,low,pTool->_bAccurate))
			return FALSE;
	}
	else
	{
		low = pTool->_height;
		high = pTool->_height2;

		float hardness = pTool->_hardness*0.1f;
		float e = 2.71828183f;
		float innerRadius = pTool->_arg.radius;
		float outterRadius = pTool->_arg.radius2;
		float rg = outterRadius - innerRadius;
		i_math::vector3df center = pTool->_arg.vCenter;

		if(rg<0.0001f) 
			rg = 0.0001f;

		for(int i = 0;i<pTool->_seedMap.points.size();i++)
		{
			TrrnSeedMap::SeedPoint & point = pTool->_seedMap.points[i];
			float w =sqrtf((center.x - point.v.x)*(center.x - point.v.x) + (center.z - point.v.z)*(center.z - point.v.z));
			w = (w<innerRadius)?1.0f:(1.0f-(w-innerRadius)/rg);
			point.wt = w*pow(e,hardness*(w - 1.0f));
		}
		if(FALSE ==edit->AddHeight(pTool->_seedMap,FALSE,high,low,pTool->_bAccurate,TRUE))
			return FALSE;
	}

	
	RecordModify();
	NotifyTrrnHeightChange();    

	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::Smooth(GuiData_Trrn * data)
{

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return FALSE;

	ITrrnMapEditor * edit = data->GetTrrnMapEditor();

	float hardness = pTool->_hardness*0.1f;
	float speed = pTool->_speed;

	int minX,minY,maxX,maxY;
	float low,high;
	low = high = pTool->_seedMap.points[0].v.y;
	minX = maxX = pTool->_seedMap.points[0].x;
	minY = maxY = pTool->_seedMap.points[0].y;

	for(int i = 0;i<pTool->_seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = pTool->_seedMap.points[i];
		
		minX = min(point.x,minX);
		minY = min(point.y,minY);

		maxX = max(point.x,maxX);
		maxY = max(point.y,maxY);
		
		low = min(low,point.v.y);
		high = max(high,point.v.y);
	}
	
	if(high == low)
		return TRUE;

	int w = maxX - minX + 1;
	int h = maxY - minY + 1;

	std::vector<float> p;
	p.assign(h*w,-10000.0f);
	
	int size = p.size();
	for(int i = 0;i<pTool->_seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = pTool->_seedMap.points[i];
		p[(point.y - minY)*w + point.x - minX] = point.v.y;
	}

	int x,y;
	int mx[8];
	int my[8];
	
	mx[0] = 1;  my[0] = 1;   // 0
	mx[1] = 1;  my[1] = 0;   // 1
	mx[2] = 1;  my[2] = -1;  // 2
	mx[3] = 0;  my[3] = -1;  // 3
	mx[4] = -1; my[4] = -1;  // 4
	mx[5] = -1; my[5] = 0;	 // 5
	mx[6] = -1; my[6] = 1;	 // 6
	mx[7] = 0;  my[7] = 1;	 // 7 
	
	float rg = high - low;

	float step = speed/100.0f;
	for(int i = 0;i<pTool->_seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = pTool->_seedMap.points[i];
		x = point.x - minX;
		y = point.y - minY;

		int c = 1;
		float sum = point.v.y;
		float arg = point.v.y;
		for(int m = 0;m<8;m++)
		{
			int nx = x + mx[m];
			int ny = y + my[m];
			if(nx<0||nx>=w||ny<0||ny>=h)
				continue;

			float v = p[ny*w + nx];
			if(v!= -10000.0f)
			{
				sum += v;
				c++;
			}
		}

		arg = sum/c;
		
		float r = arg - point.v.y;
		r = (r<0.01f)?r:r*step;
		point.wt = (point.v.y + r - low)/rg;
		
	}

	if(FALSE ==edit->AddHeight(pTool->_seedMap,FALSE,high,low,pTool->_bAccurate))
		return FALSE;
	
	RecordModify();
	NotifyTrrnHeightChange();

	return TRUE;	
}
BOOL CGuiAgent_TerrainRLOp::OnTimer(int dt,DWORD flag)
{
	int code = GetAsyncKeyState(VK_CONTROL);
	if(_bOp&&code==0)
	{
		if (_UpdateClock())
			UpdateTerrain();
	}

	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::OnRButtonClick(int x,int y,DWORD flag)
{
	CGuiView * view = GetGuiView();
	assert(view);
	
	GuiData_Trrn * data = GetTrrnData(view);
	if(data){
		BOOL oldState = data->bPaint;
		data->bPaint = FALSE;
		CGuiPanel_Trrn * actor = (CGuiPanel_Trrn *)_GetActor();
		actor->UpdatePaintState();
		
		//如果状态发生改变  不再处理多余的消息 使这次改变更明显的显现出来
		if(TRUE==oldState)
			return FALSE;
	}
	
	return TRUE;
}
BOOL CGuiAgent_TerrainRLOp::OnDraw()
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	CGuiView * view = GetGuiView();

	assert(view);
	GuiData_Trrn * data = GetTrrnData(view);
	//检测绘制状态
	if(!data||!data->bPaint)
		return TRUE;

	if(!data||pTool->_seedMap.IsEmpty())
		return TRUE;

	IRenderPort * rp = GetRP();
	TrrnSeedMap & seedMap = pTool->_seedMap;

	std::vector<i_math::vector3df> b0,b1;
	b0.resize(seedMap.boundary.size());
	b1.resize(seedMap.boundary2.size());

	if(TRUE)
	{
		for(int i = 0;i<b0.size();i++)
		{
			b0[i] = seedMap.boundary[i];
			b0[i].y += 0.2f;
		}
		for(int i = 0;i<b1.size();i++)
		{
			b1[i] = seedMap.boundary2[i];
			b1[i].y += 0.2f;
		}
	}
	
	std::vector<i_math::vector3df> invalidPoints;
	for(int i = 0;i<seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = seedMap.points[i];
		if(point.flag==TrrnSeedMap::SeedPointF_ForbiddenBrush)
		{
			i_math::vector3df v[4];
			v[0] = v[1] = v[2] = v[3] = point.v;
			
			v[0].y += 0.3f;
			v[1].z += 0.3f;
			v[2].x += 0.3f;
			v[3].x -= 0.3f;
			

			invalidPoints.push_back(v[2]);
			invalidPoints.push_back(v[0]);
			invalidPoints.push_back(v[3]);
			
			invalidPoints.push_back(v[1]);
			invalidPoints.push_back(v[3]);
			invalidPoints.push_back(v[0]);

			invalidPoints.push_back(v[1]);
			invalidPoints.push_back(v[0]);
			invalidPoints.push_back(v[2]);
		}
	}
	

	rp->Lines(b1.data(),b1.size()/2,ColorAlpha(0xffff00,0xff));
	rp->Lines(b0.data(),b0.size()/2,ColorAlpha(0x00ff00,0xff));
	
	rp->Triangles(invalidPoints.data(),invalidPoints.size()/3,ColorAlpha(0xff0000,0xff));

	return TRUE;
}

#define UPDATE_CYCLE 100
BOOL CGuiAgent_TerrainRLOp::_UpdateClock()
{
	DWORD t=GetTickCount();
	DWORD dt=t-_tickLast;
	_tickLast=t;
	_clock-=(int)dt;
	if (_clock<=0)
	{
		_clock=UPDATE_CYCLE;
		return TRUE;
	}
	return FALSE;
}
