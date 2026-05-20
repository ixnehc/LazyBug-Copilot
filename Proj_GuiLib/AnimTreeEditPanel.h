#pragma once
#include "Resource.h"
#include "ResEditPanel.h"
#include "ResAnchor.h"

#include "GraphATPads.h"
#include "GuiAgent_GraphPads.h"

#include "WorldSystem/IAnimNodes.h"

#include "AnimTreeGrid.h"

#include "timer/timer.h"



struct Reps_AnimTree :public ResEditPanelState
{
	Reps_AnimTree()
	{ 
	}
	virtual void Copy(ResEditPanelState &src)
	{
		ResEditPanelState::Copy(src);
	}

	CGraphATPads *GetGraph();

	std::vector<PadID> sels;
};

class CDataSrc_GraphATPads:public CDataSrc_GraphPads
{
public:
	virtual CGraphPads *GetGraph(CGuiAgent *agent);
	virtual std::vector<PadID>*GetSelBuf(CGuiAgent *agent);
	virtual void NotifyChange(CGuiAgent *agent,BOOL bSave);
};

class CGuiAgent_NewATPads:public CGuiAgent
{
public:
	CGuiAgent_NewATPads();
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	std::vector<std::string>_classes;
	i_math::pos2di _pt;
};

class CGuiAgent_ATPadsCommand:public CGuiAgent
{
public:
	CGuiAgent_ATPadsCommand(CDataSrc_GraphPads *src)
	{
		_src=src;
		_sel=PadID_Null;
		_iStub=-1;
	}
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}
	CDataSrc_GraphPads *_src;
	PadID _sel;
	std::string _name;
	int _iStub;
	POINT _pt;

};

class CGuiAgent_AtnDbg:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_AtnDbg(CDataSrc_GraphPads *src)
	{
		_src=src;
		_sel=PadID_Null;
	}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);

protected:

	void _UpdateCV(int x,int y,BOOL bEnd);//更新Control Value
	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}
	CDataSrc_GraphPads *_src;
	PadID _sel;
	i_math::recti _rc;

};

struct AnimNodeATDbg:public IAnimNode
{
public:
	DEFINE_CLASS(AnimNodeATDbg);
	IMPLEMENT_REFCOUNT_C
	AnimNodeATDbg()
	{
		idStr=StringID_Invalid;
	}

	virtual float *GetValue(AnimTick t)	{		return &v.x;	}
	virtual StringID *GetStringID(AnimTick t) 	{		return &idStr;	}
	virtual BOOL IsFixed()	{		return TRUE;	}

	i_math::vector2df v;
	StringID idStr;

};

class CAnimNodeMatFix_AnimTreeView:public IAnimNode
{
public:
	DEFINE_CLASS(CAnimNodeMatFix_AnimTreeView);

	virtual int AddRef()	{		return 0;	}
	virtual int Release()	{		return 0;	}
	virtual void ReleaseAll()	{	}

	virtual i_math::matrix43f*GetMat(AnimTick t)	
	{		
		return &_mat;	
	}
	virtual BOOL IsFixed()	{		return TRUE;	}

public:
	i_math::matrix43f _mat;

};


class IAnimTree;

class IBoneAnim;
class IAnimTreeCtrl;
class IMesh;
class IMtrl;
class CAnimTreePreview
{
public:
	CAnimTreePreview()
	{
		Zero();
	}
	~CAnimTreePreview()
	{
		Clear();
	}
	struct Event
	{
		AnimEvent *e;
		DWORD t;
	};
	void Zero()
	{
		_animtree=NULL;
		_mats=NULL;
		_ctrl=NULL;
		_t=0;

		_meshCross=NULL;
		_mtrlCross=NULL;

		_dataLast=NULL;
	};
	void Clear();
	BOOL Reset(AnimTreeData*data,IWorldSystem *pWS);

	void Draw(IRenderPort *rp,CWnd *wnd);

	void SyncPadDyns(CGraphATPads *graph);

	AnimTick GetT()	{		return _t;	}

	IAnimTreeCtrl *GetCtrl()	{		return _ctrl;	}


protected:
	std::vector<IMesh *>_mesh;
	std::vector<IMtrl *>_mtrl;
	std::vector<IBoneAnim *>_anims;
	IAnimTree *_animtree;
	IAnimTreeCtrl *_ctrl;

	IMesh *_meshCross;
	IMtrl *_mtrlCross;

	std::unordered_map<PadID,AnimNodeATDbg*> _dbgs;
	CAnimNodeMatFix_AnimTreeView _anIkEffector;

	AnimTreeData *_dataLast;

	std::deque<Event>_events;

	std::vector<i_math::xformf> _xfms;
	IMatrice43 *_mats;
	AnimTick _t;

	CTimer _timer;
};


class GuiLib_Api  CAnimTreeEditPanel : public CResEditPanel
{
public:
	CAnimTreeEditPanel(void);
	~CAnimTreeEditPanel(void);

	//Override function
	virtual UINT GetIDD(){return IDD_ANIMTREEPANEL;};

	//3d 
	virtual void Init3d();
	virtual void Clear3d();	

	//anchor releted
	virtual void OnResDataChange(ResData *dataNew);
	
	//draw
	virtual void Draw(IRenderPort *rp);
	virtual void Draw(GraphicsGraph*gg)	;//should be overidden to draw something in subclass

	virtual void OnSelect();


	//serialize
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual ResEditPanelState *_NewState();

	virtual void EnablePanel(BOOL bEnable=TRUE);


	virtual BOOL _SupportUndo()	{		return TRUE;	}

	CGraphATPads *GetGraph()	{		return &_graph;	}
	//UI relative
public:	
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);

protected:

	void _UpdateView2Agent();

	CAnimTreeGrid _grid;
	CAnimTreePreview _preview;

	CGraphATPads _graph;

	AnimTreeData *_dataLast;

	CDataSrc_GraphATPads _src;

};

