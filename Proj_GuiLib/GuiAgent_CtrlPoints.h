
#pragma once

#include "GuiAgent_3dnodeedit.h"

#include "WorldSystem/IObjMap.h"

#include "WorldSystem/ICtrlPoint.h"

#include "class/class.h"

class CGuiAgent_KeySelDraw;
class CGuiAgent_KeyPointOp;
class CGuiAgent_KeyPointCreate;
class CGuiAgent_KeyObjSel;
class CGuiAgent_KeyMove;
class CGuiAgent_CtrlPoints :public CGuiAgent
{
public:
	CGuiAgent_CtrlPoints(void);
	virtual ~CGuiAgent_CtrlPoints(void);

	virtual void Enable(BOOL bEnable); 
	virtual void UpdateBind();

	void BeginCreate();
	void EndCreate();

protected:
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	virtual BOOL Respond(CtrlOp &co);
protected:
	// override function
	virtual HMapObj *_GetSelObj() = 0;
	virtual void _GetKeyPos(const HMapObj & hObj,ICtrlPointPack * pack) = 0;
	virtual HMapObj _SetKeyPos(const HMapObj &hObj,ICtrlPointPack * pack) = 0;
	virtual DWORD *_GetVer() = 0;
	virtual IObjMapEditor * _GetEditor() = 0;
	virtual BOOL _HitGroundPos(HitProbe &rayHit,i_math::vector3df &pos) = 0;	//rayHit 检测线 pos:返回的交点
	virtual void * _GetSelPointsBuf() = 0;		//返回当前选中对象的缓存 std::vector<DWORD> 类型的指针
	virtual ICtrlPointPack * _NewCtrlPointPack() = 0;
	virtual HMapObj _NewObj(IObjMapEditor * editor,ICtrlPointPack * pack) = 0;
	
	virtual void OnSelected(const HMapObj &hObj) {}; //
	virtual HMapObj _HitTest(i_math::line3df &ray,i_math::vector3df *intersec);
	//创建过程中的绘制 ，返回FALSE 不绘制默认的内容
	virtual BOOL _DrawOnCreate(ICtrlPointPack *pack,i_math::vector3df &posMove,BOOL cCon2Tail){return TRUE;} 
	virtual void _OnNewCP(CtrlPoint * cp){}; //新建一个节点子类设置用户数据
	
	//创建过程通知
	virtual void _BeginCreate(){}
	virtual void _OnCreate() {}
	virtual void _EndCreate(){}

	virtual std::vector<H3DNode> &_GetSelCPs()=0;


	int _HitSel(int x,int y,HMapObj * hObjTempSel = NULL,i_math::vector3df *posIntersec = NULL);
	void _UpdateBufCP(void);
	CtrlPoint * _New(i_math::vector3df &pos);
	void _Del(CtrlPoint * p);
	BOOL _GetObjBlock(const HMapObj &hObj,i_math::pos2di &ptBlk);
	void _RefreshSelBuf();
	
	friend class CGuiAgent_KeySelDraw;
	friend class CGuiAgent_KeyPointOp;
	friend class CGuiAgent_KeyPointCreate;
	friend class CGuiAgent_KeyObjSel;
	friend class CGuiAgent_KeyMove;
private:
	ICtrlPointPack * _cpPack;
	HMapObj _hObjEdit;

	CGuiAgent_KeySelDraw	 * _agentDraw;
	CGuiAgent_KeyPointOp	 * _agentCtrlPointOp;
	CGuiAgent_KeyPointCreate * _agentCreate;
	CGuiAgent_KeyObjSel		 * _agentSel;
	CGuiAgent_KeyMove		 * _agentMove;
};



