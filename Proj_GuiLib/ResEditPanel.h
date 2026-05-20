
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "ResAnchor.h"


#include "mod/ModBase.h"

#include "SlidePanel.h"

#include "GuiEditor.h"

class IUtilRS;
class IRenderSystem;
class IRenderPort;
class IMeshMgr;
class IMtrlMgr;
class IAnimMgr;
class IMesh;
class IMtrl;
class CModManager;	
class IDummiesMgr;
class ISptMgr;

class CResEditPanel;
struct ResData;
struct ResEditPanelState;

class CModRes:public CModBase
{
public:
	CModRes(CResEditPanel *panel)
	{
		_panel=panel;
		_state=NULL;
	}
	virtual ~CModRes();

	virtual BOOL TestUndo();
	virtual BOOL TestRedo();

	virtual BOOL Undo();
	virtual BOOL Redo();

	virtual BOOL IsEmpty()	{		return !(_panel&&_state);	}

protected:

	CResEditPanel *_panel;
	ResEditPanelState *_state;

	friend class CResEditPanel;
};


struct ResEditPanelState
{
	ResEditPanelState()
	{
		Zero();
	}
	virtual ~ResEditPanelState()
	{

	}
	void Zero()
	{
		panel=NULL;
		resdata=NULL;
	}
	virtual void CleanAndDelete();
	virtual void Copy(ResEditPanelState &src);
	virtual void SetData(ResData *data);

	ResData *resdata;
	CResEditPanel *panel;
};



class CXTPTabControl;
class CXTPTabManagerItem;
class CSlideTab;
class CResEditCtrl;
class CGuiView;
class GuiLib_Api CResEditPanel : public CSlidePanel
{
public:
	CResEditPanel();

	BOOL Create(CSlideTab *tabctrl,const char *name,int idxIcon=-1);

	void SetGuiView(CGuiView *view)	{		_view=view;	}
	void SetGuiView2(CGuiView *view)	{		_view2=view;	}

//3d 
	virtual void Init3d()	{	}
	virtual void Clear3d()	{	}

//anchor releted
	virtual void OnResDataChange(ResData *dataNew)=0;

	CResAnchor *GetAnchor()	{		return &_anchor;	}
	BOOL SetAnchorData(CResAnchor *anchor,ResData *data);

	//this is gennerally called from the ResTree after it modify its content(such as 
	//remove/rename a res file),which possibly make some anchor path no lonager available
	void ValidateAnchors();
	
	virtual void UpdateUI();

	void SwitchActive();

//Draw
	virtual void Draw(IRenderPort *rp)	{	}//should be overidden to draw something in subclass
	virtual void Draw(GraphicsGraph*gg)	{	}//should be overidden to draw something in subclass

//State related
	virtual ResEditPanelState *NewState();
	virtual BOOL RepairState(ResEditPanelState *state);
	virtual void RefreshStateMod(BOOL bSave=TRUE);
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual BOOL StateToFile(ResEditPanelState *state);//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
	void BeginBackup();

//Controls existing in this panel
	void AddCtrl(CResEditCtrl *ctrl);

//Control Lock
	//After panel state changed,we should update them to the controls,but sometimes it's
	//the control that make this change,so the control need not be updated.To solve this 
	//problem,a control could lock itself before it make the change notify,(thus the StateToControl()
	//could ignore those locked),and unlock itself after the notify 
	void LockControl(CResEditCtrl *ctrl);
	void UnlockControl(CResEditCtrl *ctrl);
	BOOL IsControlLocked(CResEditCtrl *ctrl);

//selection
	virtual void OnSelect(){}
	virtual void OnDeselect(){}
	
	virtual void EnablePanel(BOOL bActive=TRUE);

//for GuiAgent
	void ClearAgent_View();
	void ClearAgent_View2();
	void ClearAgent();
	void AddAgent(CGuiAgent *agent,DWORD priority=AGENTPRIORITY_STANDARD);


protected:
	virtual ResEditPanelState *_NewState()=0;
	virtual BOOL _SupportUndo()	{		return FALSE;	}
	virtual BOOL _NeedUndo(ResEditPanelState *cur,ResEditPanelState *last);
	virtual ResEditPanelState *_GetStateToSave()	{		return _stateToMod;	}
	BOOL _SaveAnchorData(CResAnchor &anchor,ResData *data);//if anchor's path is "",will do nothing and return TRUE

	void _AddCameraController();

protected:
	CGuiView *_view;
	CGuiView *_view2;

	ResEditPanelState *_stateToMod;//the state for controls to mod
	ResEditPanelState *_stateBackup;
	
	std::vector<CResEditCtrl*>_ctrls;
	std::vector<CResEditCtrl *>_lockedctrl;

	CResAnchor _anchor;

	CSlideTab * _tabctrl;
	CXTPTabManagerItem *_pThisItem;

	BOOL _bEnable;

private:

protected:
	virtual UINT GetIDD()=0;
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual afx_msg void OnDestroy();
	afx_msg void OnClose();

	friend class CModRes;
};

#undef AFX_DATA
#define AFX_DATA
