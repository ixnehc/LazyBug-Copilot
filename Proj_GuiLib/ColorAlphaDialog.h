#pragma once

#include "GuiLib.h"
#include ".\resource.h"
#include "afxwin.h"

#include "SpinEdit.h"

#include "ruler/ruler.h"

#include "fastdelegate/FastDelegate.h"

class CColorAlphaPage;
class CAlphaSpin:public CSpinEdit
{
public:
	virtual void OnBeginValueChange();
	virtual void OnEndValueChange();

	virtual void OnValueChange(SlideSpinValue v);
protected:

	CColorAlphaPage *_owner;
	friend class CColorAlphaPage;

};

class GraphicsGraph;
class CColorBar;
class CColorAlphaDialog;
class CColorAlphaPage:public CXTColorDialog
{
public:
	typedef fastdelegate::FastDelegate1<int> NotifyHandler;

	CColorAlphaPage(COLORREF clrNew, COLORREF clrCurrent, float alpha,DWORD dwFlags = 0L, CWnd* pWndParent = NULL);
 
	virtual ~CColorAlphaPage();


	void SetNewAlpha(float alpha);
	void BeginSetNewAlpha();
	void EndSetNewAlpha();

	virtual void SetNewColor(COLORREF clr, BOOL bNotify = TRUE);
	virtual void BeginSetNewColor();
	virtual void EndSetNewColor();
	void PaintColor();

	void SetCurColor(DWORD col);

	DWORD GetCurColor();

	void SetNotifyHandler(NotifyHandler handler)	{		_handler=handler;	}

	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:

	void _UpdateCtrl_Color();//
	void _UpdateCtrl_Alpha();



	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();

	CAlphaSpin _spin;

	float _alpha;

	i_math::recti _rcColor;
	GraphicsGraph *_ggColor;

	NotifyHandler _handler;
	BOOL _bAllowAlpha;

	BOOL _bSetCurColor;

	friend class CColorAlphaDialog;

};

class CColorAlphaDialog;

class GuiLib_Api CColorAlphaDialog : public CXTPDialog
{
public:
	CColorAlphaDialog( CWnd* pParent = NULL );
	// dialog template
	enum
	{ 
		IDD = IDD_COLORALPHA_DLG,
	};

	DWORD GetCurColor();

	void SetCurColor(DWORD col);

	void SetAllowAlpha(BOOL bAllowAlpha)	{		_bAllowAlpha=bAllowAlpha;	}

	void Bind(DWORD *col)	;

	void NotifyColorChange(int type);

protected:
	virtual BOOL		OnInitDialog();
	virtual void		OnOK();
	virtual void		OnCancel();

	CColorAlphaPage*_page;

	DWORD *_col;//binding

	//binding 的备份
	DWORD _colBack;

	BOOL _bAllowAlpha;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();

};

