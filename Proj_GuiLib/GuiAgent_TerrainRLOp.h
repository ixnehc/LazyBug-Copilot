#pragma once

#include "GuiEditor.h"
#include "GuiData.h"
#include <set>
#include "ToolBase.h"

class CGuiAgent_TerrainRLOp :public CGuiAgent
{
public:
	CGuiAgent_TerrainRLOp(void);
	~CGuiAgent_TerrainRLOp(void);

	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);

	void  SetTool(CToolBase * tool);
protected:
	virtual BOOL UpdateTerrain();
	BOOL RaiseLower(GuiData_Trrn * data);
	BOOL Flatten(GuiData_Trrn * data);
	BOOL Smooth(GuiData_Trrn * data);
	BOOL Hole(GuiData_Trrn * data);

	BOOL  PrepareSeedMap(CGuiView * view,int x,int y);

	void NotifyTrrnHeightChange();
	void RecordModify();
	void SaveToFile();
	void NotifyTrrnChangeHeight(BOOL bSave,std::vector<i_math::pos2di> &blks);

protected:
	BOOL _UpdateClock();
	BOOL _bOp;
	int _x,_y;
	i_math::vector3df m_center;

	std::set<__int64> m_srcBlocks;

	int _clock;//用来调UpdateTerrain()的计时器,in tick
	DWORD _tickLast;
	
	CToolBase * _tool;
};



