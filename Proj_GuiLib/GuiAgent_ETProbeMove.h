#pragma once

#include "GuiEditor.h"

#include "GuiAgent_3dnodeedit.h"

class CGuiAgent_ETProbeMove :public CGuiAgent_3DNodeMatEdit
{
public:
	CGuiAgent_ETProbeMove(void);
protected:
	virtual  void*_GetSelBuf();
	virtual DWORD *_GetVer();
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
private:
	i_math::pos2di _ptBlk;
	i_math::matrix43f _mat;
};

