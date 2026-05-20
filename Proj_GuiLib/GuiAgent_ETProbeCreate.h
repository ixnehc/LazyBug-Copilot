
#pragma once

#include "GuiEditor.h"

class ITrrnMapEditor;
class CGuiAgent_ETProbeCreate :public CGuiAgent
{
public:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
protected:
	ITrrnMapEditor * _GetTrrnEditor();
};