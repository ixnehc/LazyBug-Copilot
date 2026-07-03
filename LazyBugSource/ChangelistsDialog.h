#pragma once

#include "resource.h"
#include "afxwin.h"

#include "ChangelistsEdit.h"


class CChangelistsDialog : public CDialog
{
public:

	CChangelistsDialog( CWnd* pParent = NULL );
	// dialog template
	enum { IDD = IDD_CHANGELISTS_DLG };

	void Create(CWnd *parent);
	void SetOwnerName(const char *name);

	void UpdateUI();

	void SetChangelists(CChangelists* changelists);

	AbsTick FetchOpenSelRequest()	{		return _data.FetchOpenSelRequest();	}

	const FileChange* GetSelectedFileChange();

	void Redraw()	{		_view.Invalidate();	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK()	{}
	virtual void OnCancel(){}
public:
	DECLARE_MESSAGE_MAP()

	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
public:

protected:

	void _RecalcLayout();
	void _SafeAddCombo(const char *str);

	void _UpdateSel();

	CChangelistsEditor _editor;
	
	CGuiData_Changelists		_data;
	CGuiView_Changelists		_view;
	CGuiActor_Changelists	_actor;	
	CGuiMgr		_mgr;

};



