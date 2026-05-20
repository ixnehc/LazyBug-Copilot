#pragma once

#include "GuiEditor.h"
#include "GuiData.h"

class CGuiAgent_TerrainOp :public CGuiAgent
{
public:
	CGuiAgent_TerrainOp(void);
	~CGuiAgent_TerrainOp(void);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
};



