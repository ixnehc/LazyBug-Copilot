/********************************************************************
	created:	2006/9/10   9:48
	filename: 	d:\IxEngine\Proj_GuiLib\SpinEdit.cpp
	author:		cxi
	
	purpose:	an edit with a spin to its right side
*********************************************************************/
#include "stdh.h"
#include "SpinEdit.h"

#include "WMGuiLib.h"

#include "stringparser/stringparser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////////
//CSEInplaceEdit

CSEInplaceEdit::CSEInplaceEdit()
{
}

CSEInplaceEdit::~CSEInplaceEdit()
{
}

BEGIN_MESSAGE_MAP(CSEInplaceEdit, CEdit)
	//{{AFX_MSG_MAP(CSEInplaceEdit)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSEInplaceEdit message handlers

void CSEInplaceEdit::OnEnKillfocus()
{
	if (m_pSpinEdit)
		m_pSpinEdit->OnEditChange();

}

UINT CSEInplaceEdit::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}

void CSEInplaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_TAB) return;
	if (nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		m_pSpinEdit->SetFocus();
		return;
	}

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
void CSEInplaceEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN)
		return ;

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CSEInplaceEdit::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && IsDialogMessage(pMsg))
		return TRUE;
	return CEdit::PreTranslateMessage(pMsg);
}



////////////////////////////////////////////////////////////////////////////////////////////////

CFont CSpinEdit::_fontEdit;

BEGIN_MESSAGE_MAP(CSpinEdit,CWnd)
	ON_WM_SIZE()
	ON_MESSAGE(GLM_SlideSpin_Notify,OnSlideSpinNotify)
END_MESSAGE_MAP()

BOOL CSpinEdit::Create(CWnd *pParent,CRect &rc,UINT id)
{
	if (FALSE==CWnd::Create(AfxRegisterWndClass(0, LoadCursor(0, IDC_ARROW)),
		0, WS_CHILD|WS_VISIBLE, rc,pParent, 0))
		return FALSE;

	if (!_fontEdit.GetSafeHandle())//update the edit's font
	{
		LOGFONT lfIcon;
		VERIFY( ::SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lfIcon ), &lfIcon, 0 ) );
		lfIcon.lfWeight = FW_NORMAL;
		lfIcon.lfItalic = FALSE;
		_fontEdit.CreateFontIndirect(&lfIcon);

	}

	_spin.Create(UDS_ARROWKEYS|WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,2);
	_edit.Create(WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL|ES_WANTRETURN,CRect(0,0,0,0),this,1);
	_edit.Initialize(this);
	_edit.SetFont(&_fontEdit);
	_edit.m_pSpinEdit=this;
	_edit.DisableFlatLook(FALSE);

	OnSize(0,rc.Width(),rc.Height());

	return TRUE;
}

void CSpinEdit::OnSize(UINT nType, int cx, int cy)
{
	const int wSpin=17;
	CRect rcEdit,rcSpin;
	if (cx>wSpin)
	{
		rcEdit.SetRect(0,0,cx-wSpin,cy);
		rcSpin.SetRect(cx-wSpin,0,cx,cy);
	}
	else
	{
		rcEdit.SetRect(0,0,0,0);
		rcSpin.SetRect(0,0,cx,cy);
	}

	if (_edit.GetSafeHwnd())
		_edit.MoveWindow(&rcEdit);
	if (_spin.GetSafeHwnd())
		_spin.MoveWindow(&rcSpin);

	CWnd::OnSize(nType,cx,cy);
}

LRESULT CSpinEdit::OnSlideSpinNotify(WPARAM wParam,LPARAM lParam)
{
	if (wParam==SSN_BeginChange)
		OnBeginValueChange();
	if (wParam==SSN_EndChange)
		OnEndValueChange();
	if (wParam==SSN_Changing)
	{
		SlideSpinValue v;
		v=*((SlideSpinValue*)lParam);
		CString s;
		ValueToString(v,s);
		_edit.SetWindowText((LPCTSTR)s);

		OnValueChange(v);
	}

	return 0;
}

//called from the edit when it's content is changed
void CSpinEdit::OnEditChange()
{
	CString s;
	_edit.GetWindowText(s);

	SlideSpinValue v,vOrg;
	v = DoubleFromString(toMBCS((LPCTSTR)s));
	vOrg=_spin.GetValue();
	_spin.SetValue(v);
	v=_spin.GetValue();

	ValueToString(v,s);
	_edit.SetWindowText((LPCTSTR)s);

	if (v==vOrg)
		return;


	OnBeginValueChange();
	OnValueChange(v);
	OnEndValueChange();
}

void CSpinEdit::ValueToString(SlideSpinValue v,CString &str)
{
	if (!m_bFloatMode)
		str.Format(_T("%d"), (int)v);
	else
		str.Format(_T("%f"),(float)v);
}

void CSpinEdit::SetRange(SlideSpinValue low,SlideSpinValue high)
{
	_spin.SetRange(low,high);
	SetValue(GetValue());
}
void CSpinEdit::GetRange(SlideSpinValue &low,SlideSpinValue &high)
{
	_spin.GetRange(low,high);
}
void CSpinEdit::SetValue(SlideSpinValue v)
{
	_spin.SetValue(v);
	v=_spin.GetValue();

	CString str;
	ValueToString(v,str);

	_edit.SetWindowText((LPCTSTR)str);
}
SlideSpinValue CSpinEdit::GetValue()
{
	return _spin.GetValue();
}

void CSpinEdit::SetSpinSpeed(float speed)
{
	_spin.SetSlideSpeed(speed);
}
