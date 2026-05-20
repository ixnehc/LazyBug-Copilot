
#pragma  once

#include "GuiEditor.h"

#include "WorldSystem/IETProbe.h"

#include "RenderSystem/IRenderSystem.h"

class CGuiAgent_ETProbeBake :public CGuiAgent
{
public:
	BOOL OnRButtonClick(int x,int y,DWORD flag);
	BOOL OnCommand(DWORD idCmd);
};

void BakeEnvTex(CGeActor * actor,const HMapObj &hObj,IRenderPort * rp);

