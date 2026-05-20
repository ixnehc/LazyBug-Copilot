#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

#include "class/class.h"
#include "gds/GObj.h"

#include "linkpad/LinkPad.h"

#include <map>

class GraphicsGraph;

struct GraphPadItem;
struct GraphPadHit
{
	enum Part
	{
		Blank,

		//items
		In,
		Out,
		CIn,
		COut,
		Ctrl,
		//这两个专门被state graph使用
		StateCore,
		TransCore,

	};

	GraphPadHit()
	{
		part=Blank;
		item=NULL;
		id=PadID_Null;
	}
	Part part;
	PadID id;

	GraphPadItem *item;
};


class CGraphPads;

struct GraphPadItem
{
	DEFINE_CLASS(GraphPadItem);
	GraphPadItem()
	{
		bConnectable=FALSE;
		id=PadID_Null;
		graph=NULL;
		iStub=-1;
	}
	void SetShow(const char *s)	{		show=s;	}
	void SetName(const char *s)	{		name=s;	}
	void UpdateSize(GraphicsGraph *gg);
	BOOL IsFocus();
	virtual i_math::pos2di GetConnectSpot(BOOL bConnecting);//or connected

	PadStubType GetStubType();


	PadID id;//owner
	int iStub;//第几个stub,如果为-1,表示不是一个stub
	std::string name;
	i_math::size2di sz;
	std::string show;//显示的字串

	//一些状态标志
	BOOL bConnectable;

	i_math::recti rc;//实际位置
	i_math::recti rcFocus;//鼠标拾取的区域

	CGraphPads *graph;
};

struct GraphPadHit;
class CGraphPad
{
public:
	CGraphPad()
	{
		_id=PadID_Null;
		_bLayoutDirty=TRUE;
		_bFolder=FALSE;
		_bCurFolder=FALSE;
	}
	virtual CClass *GetClass()=0;
	virtual void RecalcLayout(GraphicsGraph *gg)	{	}
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight)=0;

	virtual BOOL HitTest(int x,int y,GraphPadHit &hit)	{		return FALSE;	}
	virtual GraphPadItem *FindItem(const char *name)	{		return NULL;	}

	void SetPos(i_math::pos2di &pt);
	i_math::pos2di GetPos()	{		return _pt;	}
public://take it as protected
	PadID _id;
	i_math::pos2di _pt;
	BOOL _bFolder;
	BOOL _bCurFolder;


	GraphPadItem _title;

	i_math::recti _rc;
	BOOL _bLayoutDirty;

	friend class CGraphPads;
};


struct ConnectDyn
{
	BOOL IsEmpty()		{			return items.size()==0;		}

	void AddItem(GraphPadItem *item)
	{
		GraphPadItem *p=(GraphPadItem *)item->GetClass()->New();
		p->id=item->id;
		p->iStub=item->iStub;
		p->graph=item->graph;
		p->name=item->name;
		p->rcFocus=item->rcFocus;
		items.push_back(p);
	}

	enum Type
	{
		Connecting,//连向外面某个item
		Connected,//向外寻找一个item连向我
		ConnectingC,//连向外面某个item
		ConnectedC,//向外寻找一个item连向我
		Void,//不连向任何一个item
	};

	Type type;
	std::vector<GraphPadItem *>items;//注意,每个item里只有id和name是有效的
	i_math::pos2di pt;

	// GObj Defination --------------------------------------------------
    BEGIN_GOBJ_PURE(ConnectDyn,1);
		GELEM_VAR_INIT(int,type,Connecting);
		GELEM_VARVECTOR(GraphPadItem *,items);
		GELEM_VAR(i_math::pos2di,pt);
    END_GOBJ();    


};

class CLinkPads;
//记录所有proto的绘制信息
class GuiLib_Api CGraphPads
{
public:
	CGraphPads()
	{
		Zero();
	}
	~CGraphPads()
	{
		Clear();
	}

	void Zero();
	virtual void Clear();

	virtual void Load(CLinkPads *pads);
	CLinkPads *GetPads()	{		return _pads;	};

	void RecalcLayout(GraphicsGraph *gg);
	void Draw(GraphicsGraph *gg,PadID *sels,DWORD c);

	BOOL HitTest(int x,int y,GraphPadHit &hit);//return whether anything is hit
	PadID *RectHitTest(i_math::recti &rc,DWORD &c);

	BOOL SetFocusItem(GraphPadItem *item);//返回focus有没有变化

	void SetFocusConnect(ConnectDyn &c)	{		_connectDyn=c;	}
	void ClearFocusConnect();

	GraphPadItem **GetConnects(GraphPadItem *item,BOOL bConnecting,DWORD &c);

	GraphPadItem *FindItem(PadID id,const char *name);
	CGraphPad *FindPad(PadID id);

	GraphPadItem *GetFocus()	{		return _focus;	}

	void SetVer(DWORD ver)	{		_ver=ver;	}
	DWORD GetVer()	{		return _ver;	}

	i_math::pos2di GetPadPos(PadID id);
	void SetPadPos(PadID id,i_math::pos2di &pos);

protected:
	struct _Connect
	{
		GraphPadItem *item[2];
	};

	virtual CGraphPad *_LoadPad(CLinkPad *pad)=0;
	virtual void _SortPads()	{	}
	virtual void _DrawDynConnect(GraphicsGraph *gg,ConnectDyn &conn);
	virtual void _DrawPermConnect(GraphicsGraph *gg);
	int _FindPad(PadID id);
	void _FillGraphPad(CGraphPad *gpad,CLinkPad *pad);//填充一些CGraphPad的基本信息



	std::vector<CGraphPad*> _buf;

	std::vector<_Connect> _connects;

	//connection focus
	ConnectDyn _connectDyn;

	DWORD _ver;

	std::vector<GraphPadItem *>_temp;//for GetConnected()/GetConnecting()
	std::vector<PadID> _temp2;//for RectHitTest

	GraphPadItem *_focus;

	CLinkPads *_pads;


};
