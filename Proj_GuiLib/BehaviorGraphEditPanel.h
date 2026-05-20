#pragma once
#include "Resource.h"
#include "ResEditPanel.h"
#include "ResAnchor.h"

#include "GraphBgPads.h"
#include "GuiAgent_GraphPads.h"

#include "WorldSystem/IAnimNodes.h"
#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/BehaviorGraphsUtil.h"

#include "BehaviorGraphGrid.h"



struct Reps_BehaviorGraph :public ResEditPanelState
{
	Reps_BehaviorGraph()
	{ 
	}
	virtual void Copy(ResEditPanelState &src);
	virtual void SetData(ResData *data);

	CGraphBgPads *GetGraph();

	std::vector<PadID> sels;
};

class CDataSrc_GraphBgPads:public CDataSrc_GraphPads
{
public:
	virtual CGraphPads *GetGraph(CGuiAgent *agent);
	virtual std::vector<PadID>*GetSelBuf(CGuiAgent *agent);
	virtual void NotifyChange(CGuiAgent *agent,BOOL bSave);
};

class CGuiAgent_NewBgPads:public CGuiAgent
{
public:
	CGuiAgent_NewBgPads();
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

protected:
	std::vector<std::string>_classes;
	i_math::pos2di _pt;
};

class CGuiAgent_BgPadsCommand:public CGuiAgent
{
public:
	CGuiAgent_BgPadsCommand(CDataSrc_GraphPads *src);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);
	virtual BOOL OnTimer(int dt,DWORD flag);
	virtual BOOL OnKeyDown(char c,DWORD flag);

protected:
	struct Include
	{
		Include()
		{
			memset(this,0,sizeof(*this));
		}
		PadID idPad;
		StringID nmBase;
		BOOL bIncluded;
	};
	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}

	void _TransformGGtoPads(CLinkPads *pads);
	void _TransformGGfromPads(CLinkPads *pads);

	void _EnsureVisible(PadID idPad);

	void _AddImportMenu();
	void _AddBasePadsMenu();

	CBehaviorGraphPads *_GetPads();

	CDataSrc_GraphPads *_src;
	PadID _sel;
	std::string _name;
	int _iStub;
	POINT _pt;

	PadID _bp;

	std::vector<StringID> _nmsToImport;
	std::vector<StringID> _nmsToRemove;
	std::vector<Include> _includes;

	std::vector<DWORD> _objsToDebug;

	CBehaviorGraphUtil _util;

};




class GuiLib_Api  CBehaviorGraphEditPanel : public CResEditPanel
{
public:
	CBehaviorGraphEditPanel(void);
	~CBehaviorGraphEditPanel(void);

	//Override function
	virtual UINT GetIDD(){return IDD_BEHAVIORGRAPHPANEL;};

	//3d 
	virtual void Init3d();
	virtual void Clear3d();	

	//anchor releted
	virtual void OnResDataChange(ResData *dataNew);
	
	//draw
	virtual void Draw(IRenderPort *rp);
	virtual void Draw(GraphicsGraph*gg)	;//should be overidden to draw something in subclass

	virtual void OnSelect();

	virtual BOOL RepairState(ResEditPanelState *state);
	virtual void RefreshStateMod(BOOL bSave=TRUE);

	//serialize
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual ResEditPanelState *_NewState();

	virtual void EnablePanel(BOOL bEnable=TRUE);

    virtual void UpdateUI();

	virtual BOOL _SupportUndo()	{		return TRUE;	}
	virtual ResEditPanelState *_GetStateToSave();

	CGraphBgPads *GetGraph()	{		return &_graph;	}
	BOOL GetMapInfo(RecordID &idMap,std::string &nmMap);
	BOOL GetAIName(StringID &nmAI);

	//UI relative
public:	
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);


protected:

	void _Copy(ResData *dest,ResData *src);

	void _UpdateView2Agent();

	CBehaviorGraphGrid _grid;

	CGraphBgPads _graph;

	BehaviorGraphData *_dataLast;

	CDataSrc_GraphBgPads _src;

	DWORD _verStrLib;

	CBehaviorGraphUtil _util;


};

