#include "stdh.h"
#include "ColorAlphaDialog.h"

#include "WndBase.h"

#include "graphicsgraph.h"

#include "stringparser/stringparser.h"

#define SNAP_KEY_TIME(t) (t)=(t)/(ANIMTICK_PER_SECOND/200)*(ANIMTICK_PER_SECOND/200);


//////////////////////////////////////////////////////////////////////////
//CAlphaSpin
void CAlphaSpin::OnBeginValueChange()
{
	_owner->BeginSetNewAlpha();
}

void CAlphaSpin::OnEndValueChange()
{
	_owner->EndSetNewAlpha();
}


void CAlphaSpin::OnValueChange(SlideSpinValue v)
{
	_owner->SetNewAlpha((float)v);
}


//////////////////////////////////////////////////////////////////////////
//CColorAlphaPage
BEGIN_MESSAGE_MAP(CColorAlphaPage,CXTColorDialog)
	//{{AFX_MSG_MAP(CXTColorDialog)
	ON_WM_CREATE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CColorAlphaPage::CColorAlphaPage(COLORREF clrNew, COLORREF clrCurrent, float alpha,DWORD dwFlags, CWnd* pWndParent):
	CXTColorDialog(clrNew,clrCurrent,dwFlags,pWndParent)
{
	_alpha=alpha;
	_spin._owner=this;

	_ggColor=NULL;

	_bAllowAlpha=TRUE;
	_bSetCurColor=FALSE;

}

CColorAlphaPage::~CColorAlphaPage()
{
	SAFE_DELETE(_ggColor);
}


int CColorAlphaPage::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret=CXTColorDialog::OnCreate(lpCreateStruct);

	_spin.Create(this,CRect(250,20,320,36),10002);
	_spin.ShowWindow(SW_SHOW);

	_spin.EnableFloatMode(TRUE);
	_spin.SetRange(0,1);
	_spin.SetValue(1);
	_spin.SetSpinSpeed(0.01f);

	if (!_bAllowAlpha)
		_spin.EnableWindow(FALSE);

	_rcColor.set(250,60,320,100);
	_ggColor=new GraphicsGraph;
	_ggColor->Create(_rcColor.getWidth(),_rcColor.getHeight());

	_UpdateCtrl_Color();
	_UpdateCtrl_Alpha();

	return ret;
}

#define ColorRGBA(col,alpha) (Color((BYTE)(alpha),(BYTE)GetRValue(col),(BYTE)GetGValue(col),(BYTE)GetBValue(col)))

void CColorAlphaPage::PaintColor()
{
	CClientDC dc(this);

	Gdiplus::Graphics grph(dc.m_hDC);


	if (_ggColor)
	{
		i_math::recti rc=_rcColor;
		rc.zeroBase();
		_ggColor->FillHatchRect(rc,0xffffff,0x0,22);

		if (_bAllowAlpha)
			_ggColor->FillSolidRect(rc,m_clrNew,(BYTE)(_alpha*255.0f));
		else
			_ggColor->FillSolidRect(rc,m_clrNew,(BYTE)0xff);

		grph.DrawImage(_ggColor->GetBg(), Gdiplus::Point(_rcColor.Left(),_rcColor.Top()));
	}

}


void CColorAlphaPage::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	PaintColor();
}

void CColorAlphaPage::_UpdateCtrl_Color()
{
	if (m_dwStyle & CPS_XT_SHOWHEXVALUE)
		m_wndHexEdit.SetWindowText(RGBtoHex(m_clrNew));
	if (GetTabControl() && GetTabControl()->GetSafeHwnd())
	{
		int iPage;
		for (iPage = 0; iPage < GetPageCount(); ++iPage)
		{
			CPropertyPage *page=GetPage(iPage);
			if (page->GetSafeHwnd())
				page->SendMessage(XTWM_UPDATECOLOR,
				(WPARAM)(COLORREF)m_clrNew);
		}
	}

}

void CColorAlphaPage::_UpdateCtrl_Alpha()
{
	_spin.SetValue(_alpha);
}

void CColorAlphaPage::BeginSetNewColor()
{
	if (_handler)
		_handler(1);
}

void CColorAlphaPage::EndSetNewColor()
{
	if (_handler)
		_handler(2);
}


void CColorAlphaPage::SetNewColor(COLORREF clr, BOOL bNotify)
{
	if (_bSetCurColor)
		return;

	m_clrNew = clr;

	PaintColor();
	if (bNotify)
		_UpdateCtrl_Color();

	if (_handler)
		_handler(0);
}

void CColorAlphaPage::SetNewAlpha(float alpha)
{
	_alpha=alpha;
	PaintColor();
	if (_handler)
		_handler(0);
}

void CColorAlphaPage::BeginSetNewAlpha()
{
	if (_handler)
		_handler(1);
}

void CColorAlphaPage::EndSetNewAlpha()
{
	if (_handler)
		_handler(2);
}


DWORD CColorAlphaPage::GetCurColor()
{
	DWORD col=COLOR_SWAP_RB(m_clrNew);
	if (!_bAllowAlpha)
		return col;
	return ColorAlpha(col,(BYTE)(_alpha*255.0f));
}

void CColorAlphaPage::SetCurColor(DWORD col)
{
	m_clrNew=COLOR_SWAP_RB(col);
	_alpha=((float)(ColorAlpha_Alpha(col)))/255.0f;

	_bSetCurColor=TRUE;
	_UpdateCtrl_Color();
	_UpdateCtrl_Alpha();
	_bSetCurColor=FALSE;
	PaintColor();
}


BOOL CColorAlphaPage::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message==WM_KEYDOWN)
		return FALSE;
	return CXTColorDialog::PreTranslateMessage(pMsg);
}



//////////////////////////////////////////////////////////////////////////
//CColorAlphaDialog

BEGIN_MESSAGE_MAP( CColorAlphaDialog, CXTPDialog )
	ON_WM_DESTROY()
END_MESSAGE_MAP()


CColorAlphaDialog::CColorAlphaDialog( CWnd* pParent /* = NULL  */)
	:CXTPDialog( CColorAlphaDialog::IDD, pParent )
{
	_page=NULL;
	_col=NULL;
	_bAllowAlpha=TRUE;
}

BOOL CColorAlphaDialog::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_CTRL,rc);
	HIDE_CONTROL(this,IDC_CTRL);

	_page= new CColorAlphaPage(0xffffffff, 0xffffffff,1.0f,(0),this);
	CColorAlphaPage::NotifyHandler dlgt;
	dlgt.bind(this,&CColorAlphaDialog::NotifyColorChange);
	_page->SetNotifyHandler(dlgt);
	_page->_bAllowAlpha=_bAllowAlpha;
	_page->Create(this,WS_CHILD|WS_VISIBLE);
	_page->SetWindowPos(NULL,0,0,0,0,SWP_NOZORDER|SWP_NOSIZE);

	_page->GetWindowRect(&rc);
	ScreenToClient(&rc);

	if (_col)
		SetCurColor(*_col);

	return TRUE;
}

void CColorAlphaDialog::OnOK()
{
	CXTPDialog::OnOK();
}

void CColorAlphaDialog::OnCancel()
{
	if (_col)
		(*_col)=_colBack;
	CXTPDialog::OnCancel();
}


DWORD CColorAlphaDialog::GetCurColor()
{
	if (_page)
		return _page->GetCurColor();
	return ColorAlpha(0,0xff);
}


void CColorAlphaDialog::Bind(DWORD *col)
{
	_col=col;
	if (col)
		_colBack=*col;
}


void CColorAlphaDialog::SetCurColor(DWORD col)
{
	if (_page)
		_page->SetCurColor(col);
}


void CColorAlphaDialog::OnDestroy()
{
	SAFE_DELETE(_page);
}


void CColorAlphaDialog::NotifyColorChange(int type)
{
	if(type==0)
	{
		if (_col)
			(*_col)=GetCurColor();
	}
}

