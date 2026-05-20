
#pragma once

#include "GuiAgent_3dnodeedit.h"

#include "WorldSystem/IObjMap.h"

#include "WorldSystem/ICtrlPoint.h"

#include "class/class.h"

class CGuiAgent_KeySelDraw;
class CGuiAgent_KeyPointOp;
class CGuiAgent_KeyPointCreate;

class CGuiAgent_KeyPoints :public CGuiAgent_3DNodeMatEdit
{
public:
	CGuiAgent_KeyPoints(void);
	virtual ~CGuiAgent_KeyPoints(void);
protected:
	virtual  void * _GetSelBuf();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::pos2di *_GetBlock(H3DNode node);
	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual BOOL Respond(CtrlOp &co);
protected:
	// override function
	virtual HMapObj _GetSelObj() = 0;
	virtual void _GetKeyPos(const HMapObj & hObj,ICtrlPointPack * _cpPack) = 0;
	virtual HMapObj _SetKeyPos(const HMapObj &hObj,ICtrlPointPack * _cpPack) = 0;
	virtual HMapObj _Move(const HMapObj & hObj,DWORD idxKey,i_math::matrix43f &mat) = 0;
	virtual DWORD *_GetVer() = 0;
	virtual IObjMapEditor * _GetEditor() = 0;
	virtual HMapObj _NewObj(IObjMapEditor * editor,ICtrlPointPack * _cpPack) = 0;	
	virtual void _OnNewCP(CtrlPoint * cp) = 0; //新建一个节点子类设置用户数据
	virtual BOOL _HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos) = 0;	//rayHit 检测线 pos:返回的交点
	virtual void * _GetSelPointsBuf() = 0;										//返回当前选中对象的缓存
	virtual ICtrlPointPack * _NewCtrlPointPack() = 0;

	HMapObj _HitTest(i_math::line3df &ray);
	int _HitSel(int x,int y);
	void _UpdateBufCP(void);
	CtrlPoint * _New(i_math::vector3df &pos);
	void _Del(CtrlPoint * p);
	BOOL _GetObjBlock(const HMapObj &hObj,i_math::pos2di &ptBlk);
	void _RefreshSelBuf();

	friend class CGuiAgent_KeySelDraw;
	friend class CGuiAgent_KeyPointOp;
	friend class CGuiAgent_KeyPointCreate;

private:
	std::vector<H3DNode> _selCPs;
	ICtrlPointPack * _cpPack;
	i_math::matrix43f _matTemp;
	i_math::pos2di _ptBlk;
	HMapObj _hObjEdit;
};



