#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"


class CGuiAgent_TerrainView :public CGuiAgent
{
public:
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
};
