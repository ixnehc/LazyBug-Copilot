#pragma once

#include "GuiAgent_MapObj.h"

class CGuiAgent_TreeMove :public CGuiAgent_MapObjTransform
{
public:
	virtual  void*_GetSelBuf();
	virtual DWORD *_GetVer();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual IObjMapEditor *_GetEditor();
	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
private:
	i_math::matrix43f _matWork;
};