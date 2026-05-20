#pragma once

#include "GuiLib.h"

#include "GuiData.h"

#include "GuiEditor.h"

#include "GuiAgent_MapObj.h"

// 选择/绘制/删除/添加
class IProbeCubeMapEditor;
class CGuiAgent_ProbeOp :public CGuiAgent_MapObjOP
{

public:
	enum Op
	{
		ClickDown_Once,
		ClickUp_Once,
		Click_Reset,
	};

	CGuiAgent_ProbeOp(){_curOp = Click_Reset;}
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnLButtonUp(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnKeyDown(char c,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnCommand(DWORD idCmd);

	// 3DNodeOperate
	virtual  void*_GetSelBuf();
	virtual BOOL _NeedClone(){ return TRUE;}
	virtual H3DNode _Clone(H3DNode node);
	virtual void _CollectEnvelope(H3DNode *node,DWORD nNodes,Envelope &evlp);
	IObjMapEditor * _GetEditor();
	virtual DWORD *_GetVer();
private:
	Op _curOp;
	i_math::vector3df _p0,_p1,_p2;
};


