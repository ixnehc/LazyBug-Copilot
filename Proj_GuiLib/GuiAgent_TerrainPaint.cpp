
#include "stdh.h"

#include ".\guiagent_terrainpaint.h"

#include "WorldSystem/ITrrn.h"

#include "pin/pin.h"

#include "BrushUtil.h"

#include "timer/profiler.h"

CGuiAgent_TerrainPaint::CGuiAgent_TerrainPaint(void)
{
}
CGuiAgent_TerrainPaint::~CGuiAgent_TerrainPaint(void)
{
}
BOOL CGuiAgent_TerrainPaint::UpdateTerrain()
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return FALSE;

	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;
	
	if(TRUE)
	{
		ProfilerStart(PaintBrush);
		PaintBrush();
		ProfilerEnd();
	}

	return TRUE;
}
void CGuiAgent_TerrainPaint::PaintBase()
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return;

	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	ITrrnMapEditor * editor = NULL;
	if(data)
		editor = data->GetTrrnMapEditor();
	if(!editor)	return;

	assert(FALSE);
// 	editor->AddBaseBrush(pTool->_seedMap,pTool->_idBr);

	RecordModify();
}

void CGuiAgent_TerrainPaint::PaintBrush()
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return;

	GuiData_Trrn * data =  (GuiData_Trrn *)FindData("terrain");
	
	//检测状态
	if(!data->bPaint)
		return;

	ITrrnMapEditor * editor = NULL;
	if(data)
		editor = data->GetTrrnMapEditor();
	
	if(!editor)
		return;
	
	float innerRadius = pTool->_arg.radius;
	float outterRadius = pTool->_arg.radius2;
	float rg = outterRadius - innerRadius;
	float hardness = pTool->_hardness;
	float weight = pTool->_speed/100.0f;

	i_math::vector3df center = pTool->_arg.vCenter;
	const float e = 2.71828183f;
	TrrnSeedMap & seedMap = pTool->_seedMap;

	for(int i = 0;i<seedMap.points.size();i++)
	{
		TrrnSeedMap::SeedPoint & point = seedMap.points[i];
		float w =sqrtf((center.x - point.v.x)*(center.x - point.v.x) + (center.z - point.v.z)*(center.z - point.v.z));
		w = (w<innerRadius)?1.0f:(1.0f-(w-innerRadius)/rg);
		point.wt = weight*w;
	}

	if (pTool->m_mode==0)
	{
		BrushID idBr = pTool->_idBr;
		editor->AddBrush(seedMap,idBr);
	}
	if (pTool->m_mode==1)
	{
		editor->ModOpaque(seedMap,TRUE);
	}
	if (pTool->m_mode==2)
	{
		editor->ModOpaque(seedMap,FALSE);
	}

	RecordModify();

	_Redraw(FALSE);
}

BOOL CGuiAgent_TerrainPaint::OnMouseWheel(int delta,DWORD flag)
{
	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	CGuiView * view = GetGuiView();
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(!data)
		return TRUE;

	if(flag&CtrlOpFlag_CtrlDown)
	{
		float r0 = pTool->_lnk_radius0->GetFVal();

		float minV,maxV;
		pTool->_lnk_radius0->GetLimits(minV,maxV);
		float d = (delta/WHEEL_DELTA)*(maxV - minV)*0.01f;

		d = (d>r0)?r0:d;

		r0 -= d;

		pTool->_lnk_radius0->SetValue(r0,TRUE);

		PrepareSeedMap(view,_x,_y);
		_Redraw();

		return FALSE;
	}

	return TRUE;
}

BOOL CGuiAgent_TerrainPaint::OnSetCursor(int x,int y,DWORD flag)
{
	CGuiView * view = GetGuiView();
	GuiData_Trrn * data = (GuiData_Trrn *)FindData("terrain");
	if(!data||!data->bPaint)
		return TRUE;

	CBrushUtil * pTool = (CBrushUtil *)(_tool);
	if(!pTool)
		return TRUE;

	if(pTool->m_mode==0)
	{
		_SetCursor(IDC_CURSORMOVE);
		// Paint Paint
	}
	else if(pTool->m_mode == 1)
	{
		// Paint Base
	}

	return TRUE;
}



