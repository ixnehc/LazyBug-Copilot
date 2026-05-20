#pragma once

#include "GraphPads.h"

#define DVBOX_WIDTH 8//调试控制条的拖拽方块的大小


//AnimTreePad
class CGraphAtp:public CGraphPad
{
public:
	DEFINE_CLASS(CGraphAtp);

	CGraphAtp()
	{
		Zero();
	}
	~CGraphAtp()
	{
		Clear();
	}

	void Zero();
	void Clear();

	virtual void RecalcLayout(GraphicsGraph *gg);
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight);
	virtual BOOL HitTest(int x,int y,GraphPadHit &hit);

	GraphPadItem *FindItem(const char *name);

protected:

	void _DrawTitle(GraphPadItem*item,BOOL bHilight,GraphicsGraph *gg);
	void _DrawOut(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawIn(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawDbg(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawTuner(GraphicsGraph *gg);
	void _DrawSep(GraphicsGraph *gg,int ySep);

	void _LocateOut(GraphPadItem *item,i_math::recti &rcItem);
	void _LocateIn(GraphPadItem *item,i_math::recti &rcItem);
	void _LocateDbg(GraphPadItem *item,i_math::recti &rcItem);

	BOOL _ExistInOutSeg();
	BOOL _ExistCtrlSeg();
	BOOL _NeedSep2();//signal/slot后面的seperator

	BOOL _ItemsHitTest(int x,int y,GraphPadItem**items,DWORD c,int part,GraphPadHit &hit);
	BOOL _ItemHitTest(int x,int y,GraphPadItem*item,int part,GraphPadHit &hit);

	std::vector<GraphPadItem*>_ins;
	std::vector<GraphPadItem*>_outs;
	GraphPadItem*_dbg;
	int  _dbgtype;//a CAnimTreePad::DbgType value

	int _ySep2;//in/out与ctrl之间的seperator

	StringID _idTuner;
	std::string _nmTuner;


	friend class CGraphATPads;
};

//Pad的动态信息
struct PadDyn
{
	PadDyn()
	{
		name=StringID_Invalid;
	}

	StringID name;
	i_math::vector2df dv;//二维调试控制数据
};


class IAnimTreeCtrl;
//记录所有proto的绘制信息
class GuiLib_Api CGraphATPads:public CGraphPads
{
public:
	CGraphATPads()
	{
		_ctrl=NULL;
	}
	void SyncDyn(CLinkPads *pads);

	PadDyn *FindPadDyn(PadID id);

	void Draw(GraphicsGraph *gg,PadID *sels,DWORD c,IAnimTreeCtrl *ctrl)
	{
		_ctrl=ctrl;
		CGraphPads::Draw(gg,sels,c);
		_ctrl=NULL;
	}
	IAnimTreeCtrl *GetCtrl()	{		return _ctrl;	}

protected:

	virtual CGraphPad *_LoadPad(CLinkPad *pad);
	virtual void _DrawPermConnect(GraphicsGraph *gg);

	std::unordered_map<PadID,PadDyn> _dyns;

	IAnimTreeCtrl *_ctrl;//这个指针只在Draw(..)中有效

};
