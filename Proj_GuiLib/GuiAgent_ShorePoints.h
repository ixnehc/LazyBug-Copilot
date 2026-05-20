
#pragma once

#include "GuiAgent_CtrlPoints.h"

class CGuiAgent_ShorePoints : public CGuiAgent_CtrlPoints
{
protected:
	virtual HMapObj *_GetSelObj();
	virtual void _GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps);
	virtual HMapObj _SetKeyPos(const HMapObj &hObj,ICtrlPointPack *cps);
	virtual DWORD *_GetVer();
	virtual IObjMapEditor * _GetEditor();
	virtual HMapObj _NewObj(IObjMapEditor * editor,ICtrlPointPack * cps);
	virtual BOOL _HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos);
	virtual void _OnNewCP(CtrlPoint * cp);
	virtual void * _GetSelPointsBuf();
	virtual ICtrlPointPack * _NewCtrlPointPack();
	virtual void OnSelected(const HMapObj &hObj);
	virtual std::vector<H3DNode> &_GetSelCPs()
	{
		static std::vector<H3DNode> sel;
		return sel;
	}
};





