#pragma once

#include "GuiLib.h"
#include ".\resource.h"
#include "ValueSetEdit.h"
#include "RichGrid.h"
#include "afxwin.h"

#include "ColorAlphaDialog.h"
#include "GuiData_frameproxy.h"


class CVSColorPage:public CColorAlphaPage
{
public:
	CVSColorPage(COLORREF clrNew, COLORREF clrCurrent, float alpha,DWORD dwFlags = 0L, CWnd* pWndParent = NULL);

	void SetOwnerName(const char *name)	{		_owner=name;	}
	const char *GetOwnerName()	{		return _owner.c_str();	}

	virtual void OnOK(){}
	virtual void OnCancel(){}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();


protected:
	std::string _owner;//这个名字用于Registry的访问

};


class GuiLib_Api CValueSetDialog : public CXTPDialog,RichGridHook
{
public:
	typedef fastdelegate::FastDelegate1<BOOL> ShowMeCallBack;

	CValueSetDialog( CWnd* pParent = NULL );
	// dialog template
	enum { IDD = IDD_VALUESET_DLG };

	void Create(CWnd *parent);
	void SetOwnerName(const char *name);

	RichGridHook *GetRGHook();
	void SetShowMeCallBack(ShowMeCallBack dlgt)	{		_dlgtShowMe=dlgt;	}

	void UpdateUI();


protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK()	{}
	virtual void OnCancel(){}
public:
	DECLARE_MESSAGE_MAP()

	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
public:

	//RichGridHook overriding
	void PostInsertItem(CRichGrid *grid,CXTPPropertyGridItem *item,const char *clss);
	void PostResetContent(CRichGrid *grid);


	void AddEntry(const char *grp,const char *entry,CRichGrid_ValueSetItem *item);//注意entry用"\\" 分隔
	void ClearGroup(const char *grp);
protected:

	void _RecalcLayout();
	void _SafeAddCombo(const char *str);

	void _UpdateSel();

	void _NotifyColorChange(int type);

	
	CValueSetEditor _editor;
	CVSColorPage _colpage;

	CGuiData_ValueSet		_data;
	CGuiView_ValueSet		_view;
	CGuiActor_ValueSet	_actor;	
	CGuiMgr		_mgr;

	UINT _idTimer;

	BOOL _bEditModeLast;
	BOOL _bVisible;//这个值记录当从edit mode变为not edit mode时,本窗口可见与否

	ShowMeCallBack _dlgtShowMe;

};



