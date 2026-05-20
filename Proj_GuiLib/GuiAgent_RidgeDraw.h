
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "RenderSystem/IRenderSystem.h"


struct RidgeNCache;
class GuiLib_Api CGuiAgent_RidgeDraw :public CGuiAgent
{
public:
	CGuiAgent_RidgeDraw();
	~CGuiAgent_RidgeDraw();
	virtual BOOL OnDraw();
protected:
	void _Update();					 //每帧更新一部分
	BOOL _UpdateBuffer();			 //更新
	void _EnumNode();				 //枚举节点到缓存
	BOOL _UpdateNode(RidgeNCache * node);
};




