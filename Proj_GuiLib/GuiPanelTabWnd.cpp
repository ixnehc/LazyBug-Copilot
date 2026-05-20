// InputNameDlg.cpp : implementation file
//

#include "stdh.h"
#include "GuiEditor.h"
#include "GuiPanelTabWnd.h"

#include "SlideContainer.h"

#include "WndBase.h"
#include ".\guipaneltabwnd.h"


// CGuiPanelCombo dialog

IMPLEMENT_DYNAMIC(CGuiPanelCombo, CXTPDialog)
CGuiPanelCombo::CGuiPanelCombo(CWnd* pParent /*=NULL*/)
	: CXTPDialog(CGuiPanelCombo::IDD, pParent)
{
	_sel=NULL;
}

CGuiPanelCombo::~CGuiPanelCombo()
{
}


BEGIN_MESSAGE_MAP(CGuiPanelCombo, CXTPDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO, OnCbnSelchangeCombo)
END_MESSAGE_MAP()


BOOL CGuiPanelCombo::Create(CWnd *pParent)
{
	return CXTPDialog::Create(CGuiPanelCombo::IDD,pParent);	

}


BOOL CGuiPanelCombo::OnInitDialog()
{
	CXTPDialog::OnInitDialog();


	_RecalcLayout();


	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGuiPanelCombo::AddGuiPanel(const char *name,CGuiPanel *panel)
{

	CSlideContainer *container=new CSlideContainer;
	CRect rc(0,0,1,1);
	container->Create(rc,this,1,CSlideContainer::Scroll_Vertical);

	panel->Create(container);

	panel->GetClientRect(&rc);

	container->SetControl(panel,rc);

	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_COMBO);
	if (pCB)
	{
		int iSel = pCB->AddString(fromMBCS(name));
		pCB->SetCurSel(iSel);
	}

	_sel=container;
	_panels[std::string(name)]=container;;

	_UpdateSel();

}

void CGuiPanelCombo::OnDestroy()
{
	std::unordered_map<std::string,CSlideContainer *>::iterator it;
	for (it=_panels.begin();it!=_panels.end();it++)
	{
		((*it).second)->DestroyWindow();
		delete ((*it).second);
	}

	_panels.clear();

	CXTPDialog::OnDestroy();
}


void CGuiPanelCombo::_UpdateSel()
{
	std::unordered_map<std::string,CSlideContainer *>::iterator it;
	for (it=_panels.begin();it!=_panels.end();it++)
	{
		if ((*it).second==_sel)
			(*it).second->ShowWindow(SW_SHOW);
		else
			(*it).second->ShowWindow(SW_HIDE);
	}

}

void CGuiPanelCombo::OnSize(UINT nType, int cx, int cy)
{
	CXTPDialog::OnSize(nType, cx, cy);

	_RecalcLayout();
}


void CGuiPanelCombo::_RecalcLayout()
{
	i_math::recti rc,rc2;
	GetClientRect((LPRECT)&rc);
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_COMBO);

	if (_panels.size()>1)
	{
		rc.cutout(1,32,rc2);

		rc2.inflate(-4,-2,-4,-2);
		if (pCB->GetSafeHwnd())
		{
			pCB->ShowWindow(SW_SHOW);
			::SetWindowPos(pCB,rc2);
		}
	}
	else
	{
		if (pCB->GetSafeHwnd())
			pCB->ShowWindow(SW_HIDE);
	}

	if (_sel)
		::SetWindowPos(_sel,rc);

}

void CGuiPanelCombo::OnCbnSelchangeCombo()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_COMBO);
	int iSel=pCB->GetCurSel();
	CString s;
	pCB->GetLBText(iSel,s);

	std::unordered_map<std::string, CSlideContainer*>::iterator it = _panels.find(std::string(toMBCS((LPCTSTR)s)));

	if (it!=_panels.end())
	{
		_sel=(*it).second;
		_UpdateSel();
		_RecalcLayout();
	}
}

CGuiActor *CGuiPanelCombo::GetActiveActor()
{
	if (!_sel)
		return NULL;
	CGuiPanel *panel=(CGuiPanel *)_sel->GetControl();
	return static_cast<CGuiActor*>(panel);
}

BOOL CGuiPanelCombo::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		BOOL bEditFocus=FALSE;
		CWnd *pWnd=GetFocus();
		if (pWnd)
		{
			TCHAR szTemp[32];
			::GetClassName(pWnd->GetSafeHwnd(), szTemp, ARRAY_SIZE(szTemp));
			if (strcmp(toMBCS(szTemp), "Edit") == 0)
				bEditFocus=TRUE;
		}
		if (!bEditFocus)
			return FALSE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}




//////////////////////////////////////////////////////////////////////////
//	CGuiPanelTabWnd
BEGIN_MESSAGE_MAP(CGuiPanelTabWnd,CXTPTabControl)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CGuiPanelTabWnd::Create(CWnd *pParent)
{
	if (FALSE==CXTPTabControl::Create(WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, CRect(0, 0, 0, 0), pParent,203))
		return FALSE;
	CXTPTabControl::SetLayoutStyle(xtpTabLayoutMultiRow);
	GetPaintManager()->SetAppearance(xtpTabAppearanceVisualStudio2005);
	GetPaintManager()->m_bHotTracking = TRUE;
	GetPaintManager()->m_bShowIcons = TRUE;
	GetPaintManager()->DisableLunaColors(FALSE);

	return TRUE;
}

void CGuiPanelTabWnd::OnDestroy()
{
	CXTPTabControl::OnDestroy();

	for (int i=0;i<_combos.size();i++)
	{
		_combos[i]->DestroyWindow();
		delete _combos[i];
	}
	_combos.clear();
}



void CGuiPanelTabWnd::AddTab(const char *nameTab,int idIcon)
{
	CGuiPanelCombo *combo=new CGuiPanelCombo;
	combo->Create(this);

	int nTab=GetItemCount();

	CXTPTabManagerItem* item = InsertItem(nTab, fromMBCS(nameTab), combo->GetSafeHwnd(), idIcon);

	item->SetData((DWORD_PTR)combo);

	_combos.push_back(combo);
}

void CGuiPanelTabWnd::AddGuiPanel(const char *nameTab,const char *nameCombo,CGuiPanel *panel)
{
	DWORD nItems=GetItemCount();
	for (int i=0;i<nItems;i++)
	{
		CXTPTabManagerItem *item=GetItem(i);
		if (item->GetCaption()==nameTab)
		{
			CGuiPanelCombo *combo=(CGuiPanelCombo *)item->GetData();
			combo->AddGuiPanel(nameCombo,panel);
		}
	}
}

CGuiActor *CGuiPanelTabWnd::GetActiveActor()
{
	CXTPTabManagerItem *item=GetSelectedItem();
	if (item)
	{
		CGuiPanelCombo *combo=(CGuiPanelCombo *)item->GetData();
		return combo->GetActiveActor();
	}
	return NULL;
}
