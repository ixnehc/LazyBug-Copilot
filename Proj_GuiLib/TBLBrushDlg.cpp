/********************************************************************
	created:	2007/2/6   14:22
	filename: 	e:\IxEngine\Proj_GuiLib\TrrnBrushLibDlg.cpp
	author:		cxi
	
	purpose:	Terrain Brush Lib Dialog--Brush Panel
*********************************************************************/


#include "stdh.h"
#include "TBLBrushDlg.h"
#include ".\tblbrushdlg.h"

#include "WorldSystem/ITrrn.h"


#include "CommonCtrlBase.h"
#include "WndBase.h"
#include "ImageBase.h"
#include "FileDialogBase.h"

#include "TrrnBrushLibDlg.h"

#include <assert.h>



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_TEXCTRL3 1232


//////////////////////////////////////////////////////////////////////////
//CChangeBrIDDlg

IMPLEMENT_DYNAMIC(CChangeBrIDDlg, CDialog)
CChangeBrIDDlg::CChangeBrIDDlg(CWnd* pParent /*=NULL*/)
: CDialog(CChangeBrIDDlg::IDD, pParent)
{
	_idBr=BRUSHID_INVALID;
	_brlib=NULL;
}

CChangeBrIDDlg::~CChangeBrIDDlg()
{
}

void CChangeBrIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChangeBrIDDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO, OnCbnSelchangeCombo)
END_MESSAGE_MAP()


// CChangeBrIDDlg message handlers

BOOL CChangeBrIDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString s;
	CWnd *pWnd=GetDlgItem(IDC_STATIC2);
	s.Format(_T("Lv%02d:  [%d]   \"%s\""),BRUSHID_GetLevel(_idBr)+1,_idBr,_brlib->GetBrushName(_idBr));
	pWnd->SetWindowText(s);


	// TODO:  Add extra initialization here
	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);

	for (int i=0;i<BRUSHID_INVALID;i++)
	{
		if (i==_idBr)
			continue;
		if (BRUSHID_GetLevel(i)>=MAX_TRRN_BRUSHLEVEL)
			continue;
		s.Format(_T("Lv%02d:  [%d]"),BRUSHID_GetLevel(i)+1,i);

		CString name = fromMBCS(_brlib->GetBrushName(i));
		if (name!="")
			s += _T("   \"") + name + _T("\"");
		else
			s+= _T("   --Available");

		int iItem=pCB->AddString(s);
		pCB->SetItemData(iItem,i);
	}

	pCB->SetCurSel(-1);


	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeBrIDDlg::OnCbnSelchangeCombo()
{
	// TODO: Add your control notification handler code here
	CComboBox *pCB=(CComboBox*)GetDlgItem(IDC_COMBO);

	if (pCB->GetCurSel()>=0)
		_idBr=(BrushID)pCB->GetItemData(pCB->GetCurSel());
}




//////////////////////////////////////////////////////////////////////////
//CBrushList

int CBrushList::_GetBrushLevel()
{
	return ((CTBLBrushDlg*)GetParent())->GetBrushLevel();
}

BYTE CBrushList::GetBrushID()
{
	ITrrnBrushLib *brlib=_GetBrLib();
	return brlib->FindBrushID(_nameSel.c_str());
}



BOOL CBrushList::_CheckItemName(int iItem,const char *name)
{
	std::string s=name;
	if (s=="")
	{
		AfxMessageBox(_T("Name should not be empty"));
		return FALSE;
	}

	if (s.length()>=31)
	{
		AfxMessageBox(_T("Name should not be longer than 31"));
		return FALSE;
	}

	if ((iItem==-1)||(std::string(name)!=ListBox_GetString(this,iItem)))
	{
		ITrrnBrushLib *brlib=_GetBrLib();
		BrushID idBr;
		if ((idBr=brlib->FindBrushID(name))!=BRUSHID_INVALID)
		{
			CString s;
			s.Format(_T("Name not unique! A brush (Level%02d) with the same name already exists!"),BRUSHID_GetLevel(idBr)+1);
			AfxMessageBox(s);
			return FALSE;
		}
	}

	if (iItem==-1)
	{
		ITrrnBrushLib *brlib=_GetBrLib();
		int iLevel=_GetBrushLevel();
		if (brlib->IsBrushLevelFull(iLevel))
		{
			CString s;
			s.Format(_T("Level%02d 已满!"),iLevel+1);
			AfxMessageBox(s);
			return FALSE;
		}
	}

	return TRUE;
}


void CBrushList::OnDeleteItem()
{
	BrushID idBr=GetBrushID();
// 	if (idBr==0)
// 	{
// 		AfxMessageBox("The default brush (BrushID [0]) could not be removed!",MB_OK);
// 		return;
// 	}

	if (IDOK!=AfxMessageBox(
		_T("WARNING: if this brush id is used in some terrain map,removing it will cause ")
		_T("un-predicatable result when loading that map! Take care of this!")
		_T("\r\nAre you sure?"),MB_OKCANCEL))
		return;
	ITrrnBrushLib *brlib=_GetBrLib();

	brlib->RemoveBrush(idBr);

	CXTEditListBoxEx::OnDeleteItem();

	CString s;
	s.Format(
		_T("The brush is succesfully removed. If you meet some map-loading problem due to ")
		_T("the incorrect brush removal,you may use the following BrushID to recover the brush,")
		_T("which may help to solve that problem:\r\n\r\n\r\nThe Removed BrushID:[%d]"),(int)idBr);
	AfxMessageBox(s);
}

void CBrushList::OnMoveItemUp()
{
	CXTEditListBoxEx::OnMoveItemUp();
	ITrrnBrushLib *brlib=_GetBrLib();
	BrushID idBr=GetBrushID();

	brlib->ModifyBrushPriority(idBr,FALSE);
}
void CBrushList::OnMoveItemDown()
{
	CXTEditListBoxEx::OnMoveItemDown();
	ITrrnBrushLib *brlib=_GetBrLib();
	BrushID idBr=GetBrushID();
	brlib->ModifyBrushPriority(idBr,TRUE);
}

void CBrushList::OnSelChange()
{
	_nameSel=ListBox_GetSelString(this);

	((CTBLBrushDlg*)GetParent())->OnBrushSelChange();
}


void CBrushList::OnNewItem(int iItem,const char *name)
{
	ITrrnBrushLib *brlib=_GetBrLib();

	if (iItem==-1)
		brlib->NewBrush(_GetBrushLevel(),name);
	else
	{
		BrushID idBr=GetBrushID();
		brlib->RenameBrush(idBr,name);
	}
}

void CBrushList::Refresh(BOOL bReset)
{
	ITrrnBrushLib *brlib=_GetBrLib();
	assert(brlib);

	ResetContent();

	if (bReset)
		_nameSel="";

	std::vector<std::string>temp;
	std::vector<DWORD>tempPr;

	int iBrLevel=_GetBrushLevel();
	for (int i=0;i<brlib->GetBrushCount();i++)
	{
		BrushID idBr=brlib->GetBrushID(i);
		if (BRUSHID_GetLevel(idBr)!=iBrLevel)
			continue;
		temp.push_back(std::string(brlib->GetBrushName(idBr)));
		tempPr.push_back(brlib->GetBrushPriority(idBr));
	}

	if (tempPr.size()>0)//Sort by priority
	{
		for (int i=0;i<tempPr.size()-1;i++)
		for (int j=i+1;j<tempPr.size();j++)
		{
			if (tempPr[i]>tempPr[j])
			{
				DWORD t=tempPr[i];
				tempPr[i]=tempPr[j];
				tempPr[j]=t;

				std::string s=temp[i];
				temp[i]=temp[j];
				temp[j]=s;
			}			
		}
	}

	ListBox_UpdateItems(this,temp,TRUE);

	SetCurSel(ListBox_Find(this,_nameSel.c_str(),TRUE));
}



//////////////////////////////////////////////////////////////////////////
//CTBLBrushDlg

CTBLBrushDlg::CTBLBrushDlg(CWnd* pParent /*=NULL*/)
	: CTBLBaseDlg(CTBLBrushDlg::IDD, pParent)
{
	m_hIcon = NULL;

}

void CTBLBrushDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BRUSHLIST, _list);
}

BEGIN_MESSAGE_MAP(CTBLBrushDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_LEVELCOMBO, OnCbnSelchangeLevelcombo)
	ON_CBN_SELCHANGE(IDC_TEXSETCOMBO, OnCbnSelchangeTexsetcombo)
	ON_CBN_SELCHANGE(IDC_REPEATCOMBO, OnCbnSelchangeRepeatcombo)
	ON_CBN_SELCHANGE(IDC_FALLBACKCOMBO, OnCbnSelchangeFallBackcombo)
	ON_BN_CLICKED(IDC_CHANGEBRID, OnBnClickedChangebrid)
END_MESSAGE_MAP()


BOOL CTBLBrushDlg::Create(CWnd *pParent)
{

	return CDialog::Create(IDD,pParent); 
}

void CTBLBrushDlg::Refresh(BOOL bReset)
{
	OnCbnSelchangeLevelcombo();

}

void CTBLBrushDlg::_RefreshFallBack(BrushID idSel)
{
	CComboBox* pCB=(CComboBox*)GetDlgItem(IDC_FALLBACKCOMBO);
	pCB->ResetContent();
	pCB->AddString(_T(""));

	ITrrnBrushLib *brlib=GetBrLib();
	for (int i=0;i<brlib->GetBrushCount();i++)
	{
		BrushID id=brlib->GetBrushID(i);
		if (!BRUSHID_IsNSLevel(id))
			pCB->AddString(fromMBCS(brlib->GetBrushName(id)));
	}
	if (idSel!=BRUSHID_INVALID)
	{
		BrushID idFallBack=BRUSHID_INVALID;
		brlib->GetFallBack(idSel,idFallBack);
		if (idFallBack!=BRUSHID_INVALID)
		{
			pCB->SelectString(-1, fromMBCS(brlib->GetBrushName(idFallBack)));
			_texctrl2.SetTexSet(brlib->GetBrushTexSet(idFallBack));
		}
		else
			_texctrl2.SetTexSet(TTSID_INVALID);
	}
	else
		_texctrl2.SetTexSet(TTSID_INVALID);

}

BOOL CTBLBrushDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	InitLevelCombo();


	if (TRUE)
	{
		RECT rc;
		GET_CONTROL_RECT(this,IDC_TEXCTRL,rc);
		HIDE_CONTROL(this,IDC_TEXCTRL);
		_texctrl.Create(rc,IDC_TEXCTRL,this);
		_texctrl.EnableEdit(FALSE);
	}
	if (TRUE)
	{
		RECT rc;
		GET_CONTROL_RECT(this,IDC_TEXCTRL2,rc);
		HIDE_CONTROL(this,IDC_TEXCTRL2);
		_texctrl2.Create(rc,IDC_TEXCTRL3,this);
		_texctrl2.EnableEdit(FALSE);
	}


	_list.Initialize();
	_list.SetListEditStyle(_T("Brushes:"),	LBS_XT_DEFAULT);

	if (TRUE)
	{
		CComboBox* pCB=(CComboBox*)GetDlgItem(IDC_REPEATCOMBO);
		if (pCB)
		{
			pCB->AddString(_T("1x1"));
			pCB->AddString(_T("2x2"));
			pCB->AddString(_T("4x4"));
			pCB->AddString(_T("8x8"));
			pCB->AddString(_T("16x16"));
		}
		pCB->SetCurSel(0);
	}

	Refresh(TRUE);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTBLBrushDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CTBLBrushDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 

void CTBLBrushDlg::OnCbnSelchangeLevelcombo()
{
	_iBrushLevel=((CComboBox*)GetDlgItem(IDC_LEVELCOMBO))->GetCurSel();


	_list.Refresh(TRUE);
	_list.SetCurSel(0);
	_list.OnSelChange();

// 	if (BRUSHLEVEL_IsNSLevel(_iBrushLevel))
	if(FALSE)
	{
		SHOW_CONTROL(this,IDC_TEXCTRL3);
		SHOW_CONTROL(this,IDC_FALLBACKCOMBO);
		SHOW_CONTROL(this,IDC_FALLBACKSTATIC);
	}
	else
	{
		HIDE_CONTROL(this,IDC_TEXCTRL3);
		HIDE_CONTROL(this,IDC_FALLBACKCOMBO);
		HIDE_CONTROL(this,IDC_FALLBACKSTATIC);
	}

	_owner->SetLevelCombo(1,_iBrushLevel);

}

void CTBLBrushDlg::OnBrushSelChange()
{
	if (TRUE)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_TEXSETCOMBO);
		pCB->ResetContent();
		ITrrnBrushLib *brlib=GetBrLib();

		CString s;
		for (int i=0;i<brlib->GetTexSetCount(_iBrushLevel);i++)
			pCB->AddString(fromMBCS(brlib->GetTexSetName(brlib->GetTexSetID(_iBrushLevel, i))));
	}

	ITrrnBrushLib *brlib=GetBrLib();
	BrushID idBr=_list.GetBrushID();
	TTSID idTS=brlib->GetBrushTexSet(idBr);

	CComboBox *pCB;
	pCB=(CComboBox *)GetDlgItem(IDC_TEXSETCOMBO);
	if (idTS!=TTSID_INVALID)
	{
		int iSel=ComboBox_Find(pCB,brlib->GetTexSetName(idTS),TRUE);
		pCB->SetCurSel(iSel);
	}
	else
		pCB->SetCurSel(-1);
	OnCbnSelchangeTexsetcombo();

	DWORD repeat;
	pCB=(CComboBox *)GetDlgItem(IDC_REPEATCOMBO);
	if (brlib->GetBrushRepeat(idBr,repeat))
		pCB->SetCurSel(i_math::fastlog2(repeat));
	else
		pCB->SetCurSel(-1);

	_RefreshFallBack(idBr);

	if (TRUE)
	{
		CString s;
		CWnd *pWnd=GetDlgItem(IDC_BRUSHIDSTATIC);
		if (idBr==BRUSHID_INVALID)
			s = _T("n/a  ");
		else
			s.Format(_T("[ %d ]  "),(int)idBr);
		pWnd->SetWindowText(s);
	}

}


void CTBLBrushDlg::OnCbnSelchangeTexsetcombo()
{
	ITrrnBrushLib *brlib=GetBrLib();

	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_TEXSETCOMBO);
	std::string name=ComboBox_GetSelString(pCB);
	TTSID idTS=brlib->FindTexSetID(_iBrushLevel,name.c_str());

	if (_list.GetBrushID()==BRUSHID_INVALID)
		idTS=TTSID_INVALID;

	if (idTS!=TTSID_INVALID)
		brlib->SetBrushTexSet(_list.GetBrushID(),idTS);

	_texctrl.SetTexSet(idTS);
}


void CTBLBrushDlg::OnBnClickedChangebrid()
{
	// TODO: Add your control notification handler code here
	BrushID idBr=_list.GetBrushID();

	if (idBr==0)
	{
		AfxMessageBox(_T("The default brush (BrushID [0]) could not be changed to other!"),MB_OK);
		return;
	}

	if (idBr==BRUSHID_INVALID)
		return;
	CChangeBrIDDlg dlg;
	dlg.SetInfo(idBr,GetBrLib());
	if (IDOK!=dlg.DoModal())
		return;

	BrushID idBrNew=dlg.GetBrushID();
	if (FALSE==GetBrLib()->AssignBrushID(idBr,idBrNew))
	{
		AfxMessageBox(_T("Change Brush ID failure!"));
		return;
	}

	if (BRUSHID_GetLevel(idBrNew)!=BRUSHID_GetLevel(idBr))
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_LEVELCOMBO);
		pCB->SetCurSel(BRUSHID_GetLevel(idBrNew));

		OnCbnSelchangeLevelcombo();

		_list.SetCurSel(ListBox_Find(&_list,GetBrLib()->GetBrushName(idBrNew),TRUE));
	}
	_list.OnSelChange();

}


void CTBLBrushDlg::OnCbnSelchangeRepeatcombo()
{
	BrushID idBr=_list.GetBrushID();
	ITrrnBrushLib *brlib=GetBrLib();

	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_REPEATCOMBO);

	int iSel=pCB->GetCurSel();
	if (iSel==-1)
		return;

	brlib->SetBrushRepeat(idBr,(1<<iSel));
}

void CTBLBrushDlg::OnCbnSelchangeFallBackcombo()
{
	BrushID idSel=_list.GetBrushID();
	if (idSel==BRUSHID_INVALID)
		return;
	ITrrnBrushLib *brlib=GetBrLib();

	CComboBox* pCB=(CComboBox*)GetDlgItem(IDC_FALLBACKCOMBO);
	CString s;
	pCB->GetWindowText(s);
	BrushID id = brlib->FindBrushID(toMBCS((LPCTSTR)s));

	brlib->SetFallBack(idSel,id);

	_texctrl2.SetTexSet(brlib->GetBrushTexSet(id));
}

void CTBLBrushDlg::SetLevelCombo(int iLevel)
{
	CTBLBaseDlg::SetLevelCombo(iLevel);
	OnCbnSelchangeLevelcombo();
}
