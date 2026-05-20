#pragma once
#include "GuiAgent_TerrainRLOp.h"

class CGuiAgent_TerrainPaint :public CGuiAgent_TerrainRLOp
{
public:
	CGuiAgent_TerrainPaint(void);
	~CGuiAgent_TerrainPaint(void);
public:
	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
protected:
	virtual BOOL UpdateTerrain();
	void PaintBrush();
	void PaintBase();
};
