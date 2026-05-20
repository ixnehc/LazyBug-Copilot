#pragma once

#include "GuiEditor.h"

#include "GuiAgent_MapObj.h"

//添加 删除 绘制
class ISpt;
struct TreeInfo;
class CGuiAgent_treeOperate :public CGuiAgent_MapObjOP ////目前支持3D Node的选择/绘制/删除
{
protected:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual BOOL OnDraw();

	// 3DNodeOperate
	virtual  void*_GetSelBuf();
	virtual BOOL _NeedClone();
	virtual H3DNode _Clone(H3DNode node);
	virtual void _CollectEnvelope(H3DNode *node,DWORD nNodes,Envelope &evlp);
	virtual DWORD *_GetVer();
	virtual IObjMapEditor * _GetEditor();
protected:
	void _DrawCapsule(ISpt * pSpt,const TreeInfo * info);
};


