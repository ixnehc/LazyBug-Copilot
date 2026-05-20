#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "SptViewPanel.h"

#include "EditListBoxEx.h"

#include "SscBtn.h"

#include "ModBlockBack.h"

#include "PinControls.h"

#include "RichGrid.h"

#include "BrushLibLVCtrl.h"

#include "WorldSystem/IWorldSystem.h"

#include "WorldSystem/ISpt.h"

#include "BrushLibTVCtrl.h"

class CGuiAgent_treeadd;
class CGuiAgent_treeselect;
class CGuiAgent_TreeMove;

class GuiLib_Api CGuiPanel_Forest:public CGuiPanel
{
public:

	CGuiPanel_Forest(CWnd *pParent = NULL);

	// override actor
	virtual const char *GetName(){ return "forest";}
	virtual void UpdateUI();
	
	virtual void OnEnterActivity();	
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	
	// user interface
	BOOL Create(CWnd *pParent);
	virtual BOOL OnInitDialog();
	
	//
	void CommitStatusData(CModBlockBack * mod);
	void RestorStatusData(CModBlockBack * mod);
	void Reset();

protected:
	virtual const char *_GetModMgrName()	{		return "world";	}
	IForestEditor * GetEditor();
	void DoDataExchange(CDataExchange* pDX);
	afx_msg void UpdateInfo();
	//message
	DECLARE_MESSAGE_MAP()
protected:
	IBrushLib * GetSptLib();
	
	void NotifySelect(HMapObj & htree);
	BOOL OnWindChange(DWORD eventID,DWORD dwParam,void * pParam);

	void OnBakeSptlib();
	void OnSetAddTree();
	void OnTreeDoubleClicked(NMHDR * pNotifyStruct,LRESULT * pResult);
	void OnSptLibChange(NMHDR * pNotifyStruct,LRESULT * pResult);

private:
	i_math::matrix43f _matEdit;
	CGuiAgent_TreeMove * _matAgent;
	CSptViewPanel _viewPanel;
	
	CBrushLibLVCtrl _windLibWnd;
	CBrushLibTVCtrl  _sptLibWnd;

	int _selIdx;
	CSscBtn _sscBt;	
	
	BOOL _bShowDummies;
	CPinboardEdit _editScaleMin,_editScaleMax;
	CPinSpinner  _spinScaleMin,_spinScaleMax;
};






