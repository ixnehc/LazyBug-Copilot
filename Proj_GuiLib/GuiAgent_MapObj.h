
#pragma once 

#include "GuiAgent_3dnodeedit.h"

#include "WorldSystem/IObjMap.h"

class CGuiAgent_MapObjOP : public CGuiAgent_3DNodeOperate
{
protected:
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual H3DNode _HitTest(i_math::line3df &ray);
	virtual BOOL _Remove(H3DNode node);
protected:
 	virtual IObjMapEditor * _GetEditor() = 0;
private:
	i_math::pos2di _ptBlk;
};


class CGuiAgent_MapObjTransform :public CGuiAgent_3DNodeMatEdit
{
protected:
	virtual i_math::pos2di *_GetBlock(H3DNode node);

protected:
	virtual IObjMapEditor * _GetEditor() = 0;

private:
	i_math::pos2di _ptBlk;
};

