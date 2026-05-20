#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

#include "class/class.h"
#include "gds/GObj.h"
#include "WorldSystem/IEntitySystemDefines.h"

#include <map>

class GraphicsGraph;

enum GraphNodeType
{
	Standard,
	ExposedStub,
	Comment,
};

struct GraphNodeDescs
{
	void Clear()
	{
		desc="";
		descStbs.clear();
	}
	std::string desc;
	std::unordered_map<std::string,std::string>descStbs;
};


class CProtoGraph;
struct GraphItem
{
	DECLARE_CLASS(GraphItem);
	GraphItem()
	{
		bShow=FALSE;
		bConnectable=FALSE;
		bPassive=FALSE;
		id=ProtoNodeID_Null;
		graph=NULL;
	}
	void SetShow(const char *s,GraphicsGraph *gg);
	void SetName(const char *s)	{		name=s;	}
	BOOL IsFocus();
	i_math::pos2di GetConnectSpot(BOOL bConnecting);//or connected
	ProtoNodeID id;//owner
	std::string name;
	i_math::size2di sz;
	std::string show;//显示的字串

	//一些状态标志
	BOOL bShow;
	BOOL bConnectable;
	BOOL bPassive;

	i_math::recti rc;//实际位置
	i_math::recti rcFocus;//鼠标拾取的区域

	CProtoGraph *graph;
};

struct GraphHit;
class CGraphNode
{
public:
	virtual CClass *GetClass()=0;
	virtual GraphNodeType GetType()=0;
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight)=0;

	virtual BOOL HitTest(int x,int y,GraphHit &hit)	{		return FALSE;	}


protected:
	GraphItem _title;
	i_math::pos2di _pt;

	i_math::recti _rc;

	friend class CProtoGraph;
};

//记录所有protonode的绘制信息
class CGraphStandard:public CGraphNode
{
public:
	DECLARE_CLASS_DERIVED(CGraphStandard,CGraphNode);

	CGraphStandard()
	{
		Zero();
	}
	~CGraphStandard()
	{
		Clear();
	}

	void Zero();
	void Clear();

	GraphNodeType GetType()	{		return Standard;	}
	virtual void RecalcLayout(GraphicsGraph *gg);
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight);
	virtual BOOL HitTest(int x,int y,GraphHit &hit);

	GraphItem *FindItem(const char *name);

protected:

	void _DrawTitle(GraphItem*item,BOOL bHilight,GraphicsGraph *gg);
	void _DrawProp(GraphItem*item,GraphicsGraph *gg);
	void _DrawSignal(GraphItem*item,GraphicsGraph *gg);
	void _DrawSlot(GraphItem*item,GraphicsGraph *gg);
	void _DrawCall(GraphItem*item,GraphicsGraph *gg);
	void _DrawMore(GraphItem*item,GraphicsGraph *gg);
	void _DrawSep(GraphicsGraph *gg,int ySep);
	void _DrawShrink(GraphItem*item,GraphicsGraph *gg);
	void _DrawCreate(GraphItem*item,BOOL bLeft,GraphicsGraph *gg);

	void _DrawDesc(GraphicsGraph *gg);

	BOOL _LocateMoreBtn(GraphItem*item,int x,int y);
	void _LocateProp(GraphItem *item,i_math::recti &rcItem);
	void _LocateSignal(GraphItem *item,i_math::recti &rcItem);
	void _LocateSlot(GraphItem *item,i_math::recti &rcItem);
	void _LocateCall(GraphItem *item,i_math::recti &rcItem);

	BOOL _ExistPropSeg();
	BOOL _ExistSignalSlotSeg();
	BOOL _ExistCallSeg();
	BOOL _NeedSep();//prop 后面的seperator
	BOOL _NeedSep2();//signal/slot后面的seperator

	BOOL _ItemsHitTest(int x,int y,GraphItem**items,DWORD c,int part,GraphHit &hit);
	BOOL _ItemHitTest(int x,int y,GraphItem*item,int part,GraphHit &hit);

	std::vector<GraphItem*>_props;//connectable props
	std::vector<GraphItem*>_signals;
	std::vector<GraphItem*>_slots;
	std::vector<GraphItem*>_calls;


	//lua obj 的创建item
	GraphItem _createProp;
	GraphItem _createSignal;
	GraphItem _createSlot;
	GraphItem _createCall;

	//显示隐藏的按钮
	GraphItem _moreProp;
	GraphItem _moreSignal;
	GraphItem _moreSlot;
	GraphItem _moreCall;

	//状态切换button
	GraphItem _shrink;

	int _ySep;//prop后的seperator
	int _ySep2;//signal slot后的seperator


	ProtoNodeType _typePN;
	BOOL _bDynamic;
	BOOL _bVirtual;
	PNDeferGrp _defergrp;
	BOOL _bLab;
	BOOL _bEditHelper;
	ProtoNodeID _id;
	DWORD _ver;

	std::string _desc;

	friend class CProtoGraph;
};

//exposed stub
class CGraphExposed:public CGraphNode
{
public:
	DECLARE_CLASS_DERIVED(CGraphExposed,CGraphNode);
	GraphNodeType GetType()	{		return ExposedStub;	}
	virtual void RecalcLayout(GraphicsGraph *gg);
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight);
	virtual BOOL HitTest(int x,int y,GraphHit &hit);

protected:
	void _DrawTitle(GraphItem*item,GraphicsGraph *gg);
	int _stubtype;//GStubType

	ProtoNodeID _idInner;
	std::string _nameInner;

	i_math::pos2di _ptDest;

	friend class CProtoGraph;
};

struct GraphHit
{
	enum Part
	{
		Blank,

		//items
		PropIn,
		PropOut,
		Signal,
		Slot,
		Call,

		//create items
		CreateProp,
		CreateSignal,
		CreateSlot,
		CreateCall,

		//more buttons
		MoreProp,
		MoreSignal,
		MoreSlot,
		MoreCall,

		Shrink,
	};

	GraphHit()
	{
		part=Blank;
		item=NULL;
		id=ProtoNodeID_Null;
	}
	Part part;
	ProtoNodeID id;

	std::string nameExpose;

	GraphItem *item;
};


//PG for ProtoGraph
struct ConnectDynPG
{
	BOOL IsEmpty()		{			return items.size()==0;		}

	void AddItem(GraphItem *item)
	{
		GraphItem *p=Class_New(GraphItem);
		p->id=item->id;
		p->name=item->name;
		p->rcFocus=item->rcFocus;
		items.push_back(p);
	}

	enum Type
	{
		Connecting,//连向外面某个item
		Connected,//向外寻找一个item连向我
		Void,//不连向任何一个item
	};

	Type type;
	std::vector<GraphItem *>items;//注意,每个item里只有id和name是有效的
	i_math::pos2di pt;

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(ConnectDynPG,1);
	GELEM_VAR_INIT(int,type,Connecting);
	GELEM_VARVECTOR(GraphItem *,items);
	GELEM_VAR(i_math::pos2di,pt);
	END_GOBJ();    


};


class IProto;
class IProtoNode;
//记录所有proto的绘制信息
class GuiLib_Api CProtoGraph
{
public:
	CProtoGraph()
	{
		Zero();
	}
	~CProtoGraph()
	{
		Clear();
	}

	void Zero();
	void Clear();

	void Load(IProto *proto,GraphicsGraph *gg,IEntitySystem *pES);

	void Draw(GraphicsGraph *gg,ProtoNodeID *sels,DWORD c);

	BOOL HitTest(int x,int y,GraphHit &hit);//return whether anything is hit
	ProtoNodeID *RectHitTest(i_math::recti &rc,DWORD &c);

	BOOL ConnectHitTest(int x,int y,PNConnect &conn);

	BOOL SetFocusItem(GraphItem *item);//返回focus有没有变化

	void SetFocusConnect(ConnectDynPG &c)	{		_connectDyn=c;	}
	void ClearFocusConnect();

	GraphItem **GetConnects(GraphItem *item,BOOL bConnecting,DWORD &c);

	GraphItem *FindItem(ProtoNodeID id,const char *name);


	GraphItem *GetFocus()	{		return _focus;	}

protected:
	struct _Connect
	{
		_Connect()
		{
			memset(this,0,sizeof(*this));
		}
		GraphItem *item[2];
		BOOL bToDyn;//是否是一个SpawnConn
		BOOL bToDefer;
		BOOL bErr;//数据不兼容
	};


	CGraphStandard* _LoadStandard(GraphNodeDescs &descs,IProtoNode *node,GraphicsGraph *gg);
	int _FindStandard(ProtoNodeID id);

	void _LoadExposes(IProto *proto,GraphicsGraph *gg);
	void _ClearExposes();

	void _LoadConnects(IProto *proto);

	void _MakeCreateItem(GraphItem *item,ProtoNodeID id,const char *name,GraphicsGraph *gg);

	BOOL _CalcConnSpot(_Connect *p,i_math::pos2di &pt1,i_math::pos2di &pt2);


	std::vector<CGraphStandard*> _standards;
	std::vector<CGraphExposed*>_exposes;

	std::vector<_Connect> _connects;

	//connection focus
	ConnectDynPG _connectDyn;

	DWORD _ver;
	BOOL _bEditMode;

	std::vector<GraphItem *>_temp;//for GetConnected()/GetConnecting()
	std::vector<ProtoNodeID> _temp2;//for RectHitTest

	GraphItem *_focus;


};
