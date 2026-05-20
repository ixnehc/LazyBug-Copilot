#pragma once

#include "GuiEditor.h"

#include "GuiAgent_3dnodeedit.h"

class CGuiAgent_ETProbeOp :public CGuiAgent_3DNodeOperate
{
protected:
	virtual  void*_GetSelBuf();
	virtual DWORD *_GetVer();
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual H3DNode _HitTest(i_math::line3df &ray);
	virtual BOOL _Remove(H3DNode node);

private:
	i_math::pos2di _ptBlk;
};

