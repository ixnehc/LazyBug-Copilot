
#pragma  once

#include "GuiAgent_CtrlPoints.h"

class CGuiAgent_RoadPoints :public CGuiAgent_CtrlPoints
{
protected:
	virtual HMapObj * _GetSelObj();
	virtual void _GetKeyPos(const HMapObj & hObj,ICtrlPointPack * cps);
	virtual HMapObj _SetKeyPos(const HMapObj &hObj,ICtrlPointPack * cps);
	virtual DWORD *_GetVer();
	virtual IObjMapEditor * _GetEditor();
	virtual HMapObj _NewObj(IObjMapEditor * editor,ICtrlPointPack * cps);	
	virtual BOOL _HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos);	//rayHit 检测线 pos:返回的交点
	virtual void * _GetSelPointsBuf();										//返回当前选中对象的缓存
	virtual ICtrlPointPack * _NewCtrlPointPack();
	virtual BOOL _DrawOnCreate(ICtrlPointPack *pack,i_math::vector3df &posMove,BOOL cCon2Tail);
	virtual HMapObj _HitTest(i_math::line3df &ray,i_math::vector3df * intersec);

	virtual void _BeginCreate();
	virtual void _EndCreate();
	virtual std::vector<H3DNode> &_GetSelCPs()
	{
		static std::vector<H3DNode> sel;
		return sel;
	}

};


