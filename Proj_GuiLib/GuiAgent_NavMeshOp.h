#pragma once

#include "GuiEditor.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/INavMesh.h"
#include "nav/navdata.h"
#include "nav/navservice.h"

class CGuiAgent_NavMeshOp :public CGuiAgent
{
public:
	CGuiAgent_NavMeshOp(void);
	virtual ~CGuiAgent_NavMeshOp(void);

public:

	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnCommand(DWORD idCmd);
	
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual	void OnDetachView(CGeView *view,DWORD iLevel);

protected:
	void _GetRcBlock(INavMeshEditor * editor,i_math::recti &rc);

private:
	std::string						_lastmsg;
};


