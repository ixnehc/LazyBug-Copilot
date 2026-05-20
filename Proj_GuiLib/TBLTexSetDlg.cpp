/********************************************************************
	created:	2007/2/6   14:22
	filename: 	e:\IxEngine\Proj_GuiLib\TrrnBrushLibDlg.cpp
	author:		cxi
	
	purpose:	Terrain Brush Lib Dialog-TexSet panel
*********************************************************************/


#include "stdh.h"

#include "WorldSystem/IWorldSystemDefines.h"

#include "TBLTexSetDlg.h"

#include "RenderSystem/IUtilRS.h"
#include "WorldSystem/IWorldSystem.h"

#include "CommonCtrlBase.h"
#include "WndBase.h"
#include "ImageBase.h"
#include "FileDialogBase.h"


#include "Log/LogFile.h"

#include "TBLImageLib.h"
#include "TrrnBrushLibDlg.h"

#include "ximage.h"

#include <assert.h>
#include ".\tbltexsetdlg.h"

#include "resdata/TexData.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MINTEXLENSLOT_LOG2 7//(128)
#define MAXTEXLENSLOT_LOG2 10//(1024)


//////////////////////////////////////////////////////////////////////////
//CTexSetList

#pragma message("------------------------------CTexSetList should derive from CEditListBoxEx")


int CTexSetList::_GetBrushLevel()
{
	return ((CTBLTexSetDlg*)GetParent())->GetBrushLevel();
}

DWORD CTexSetList::GetTTSID()
{
	ITrrnBrushLib *brlib=_GetBrLib();
	return brlib->FindTexSetID(_GetBrushLevel(),_nameSel.c_str());
}



BOOL CTexSetList::_CheckItemName(int iItem,const char *name)
{
	std::string s=name;
	if (s=="")
	{
		AfxMessageBox(_T("Name should not be empty"));
		return FALSE;
	}

	if (iItem!=ListBox_Find(this,name,TRUE)&&(ListBox_Find(this,name,TRUE)!=-1))
	{
		AfxMessageBox(_T("Name should be unique"));
		return FALSE;
	}

	if (s.length()>=31)
	{
		AfxMessageBox(_T("Name should not be longer than 31"));
		return FALSE;
	}

	return TRUE;
}


void CTexSetList::OnDeleteItem()
{
	if (!LogFile::PromptOkCancel("Are you sure?"))
		return;
	ITrrnBrushLib *brlib=_GetBrLib();

	TTSID idTS=GetTTSID();
	brlib->RemoveTexSet(idTS);
	_GetImageLib()->SyncForTexSet(idTS);

	CXTEditListBoxEx::OnDeleteItem();
	((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

}

void CTexSetList::OnMoveItemUp()
{
//	CXTEditListBoxEx::OnMoveItemUp();
}
void CTexSetList::OnMoveItemDown()
{
//	CXTEditListBoxEx::OnMoveItemDown();
}

void CTexSetList::OnSelChange()
{
	_nameSel=ListBox_GetSelString(this);

	((CTBLTexSetDlg*)GetParent())->OnTexSetSelChange();
}

void CTexSetList::OnNewItem(int iItem,const char *name)
{
	ITrrnBrushLib *brlib=_GetBrLib();

	if (iItem==-1)
		brlib->NewTexSet(_GetBrushLevel(),name,512);
	else
	{
		TTSID idTS=GetTTSID();
		brlib->RenameTexSet(idTS,name);
	}
	((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

}


void CTexSetList::Refresh(BOOL bReset)
{
	ITrrnBrushLib *brlib=_GetBrLib();
	assert(brlib);

	ResetContent();

	if (bReset)
		_nameSel="";

	std::vector<std::string>temp;

	int iBrLevel=_GetBrushLevel();
	for (int i=0;i<brlib->GetTexSetCount(iBrLevel);i++)
		temp.push_back(std::string(brlib->GetTexSetName(brlib->GetTexSetID(iBrLevel,i))));

	ListBox_UpdateItems(this,temp,TRUE);

	SetCurSel(ListBox_Find(this,_nameSel.c_str(),TRUE));
}



//////////////////////////////////////////////////////////////////////////
//CTexCtrl

BEGIN_MESSAGE_MAP(CTexCtrl, CWnd)
	ON_WM_PAINT()
	//	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_COMMON_NEW, OnAdd)
	ON_COMMAND(ID_COMMON_REPLACE, OnReplace)
	ON_COMMAND(ID_COMMON_DELETE, OnDelete)
	ON_COMMAND(ID_COMMON_REPLACE_NS, OnReplaceNS)
	ON_COMMAND(ID_COMMON_REMOVE_NS, OnRemoveNS)
END_MESSAGE_MAP()

CTexCtrl::CTexCtrl()
{
	_idTS=TTSID_INVALID;
	_iTex=0;
	_imageBlank=NULL;

	_bEnableEdit=TRUE;
	_bSupportNS=FALSE;
}

CTexCtrl::~CTexCtrl()
{
	SAFE_DELETE(_imageBlank);
}


void CTexCtrl::Create(RECT &rc,UINT id,CWnd *pParent)
{
	CreateEx(WS_EX_STATICEDGE, _T("BUTTON"), _T(""),WS_VISIBLE|WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
		rc,pParent,id);

	CDC *pDC=GetDC();
	_bmpCompDC.CreateCompatibleBitmap(pDC,((CRect&)rc).Width(),((CRect&)rc).Height());
	ReleaseDC(pDC);
}


void CTexCtrl::SetTexSet(TTSID idTS)
{

	_idTS=idTS;
	_iTex=0;
	InvalidateRect(NULL,FALSE);
}

void CTexCtrl::_LoadImages()
{
	if (!_imageBlank)
		_imageBlank=ImageFromBMP(IDB_IMAGEBLANKMARK,NULL);
}

void CTexCtrl::_Draw(CDC *pDC)
{
	CTBLImageLib *imagelib=_GetImageLib();

	_LoadImages();
	i_math::recti rc;
	GetClientRect((RECT*)&rc);

	pDC->FillSolidRect((RECT*)&rc,RGB(192,192,192));

	CString s= _T("n/a");
	if (_idTS!=TTSID_INVALID)
	{
		CxImage *image=imagelib->GetTex(_idTS,_iTex,FALSE);
		if (image)
		{
			image->Draw(pDC->m_hDC,(RECT&)rc);
			s.Format(_T("%d/%d"),_iTex+1,imagelib->GetTexCount(_idTS));
		}
		else
		{
			i_math::recti rc2=rc.arrangeCenter(_imageBlank->GetWidth(),_imageBlank->GetHeight());
			_imageBlank->Draw(pDC->m_hDC,(RECT&)rc2);
			s="0/0";
		}
		image=imagelib->GetTex(_idTS,_iTex,TRUE);
		if (image)
		{
			image->AlphaSet(0xff);
			rc.Left()=rc.Right()-rc.getWidth()/4;
			rc.Top()=rc.Bottom()-rc.getHeight()/4;
			image->Draw(pDC->m_hDC,(RECT&)rc);
		}
	}

	if (TRUE)
	{
		CWnd *pWnd=GetParent()->GetDlgItem(IDC_TEXDESC);
		if (pWnd)
			pWnd->SetWindowText(s);
	}


}


void CTexCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages

	CRect rc;
	GetClientRect(&rc);

	CDC dcCompatible;
	CBitmap *pBmpOld;
	dcCompatible.CreateCompatibleDC(&dc);
	pBmpOld=dcCompatible.SelectObject(&_bmpCompDC);

	_Draw(&dcCompatible);

	dc.BitBlt(0,0,rc.Width(),rc.Height(),&dcCompatible,0,0,SRCCOPY);
	dcCompatible.SelectObject(pBmpOld);

}

void CTexCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags,point);
}

void CTexCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	_iTex++;

	DWORD c=_GetImageLib()->GetTexCount(_idTS);
	if (c==0)
		_iTex=0;
	else
		_iTex%=c;

	InvalidateRect(NULL,FALSE);
}

void CTexCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (_idTS==TTSID_INVALID)
		return;
	if (!_bEnableEdit)
		return;
	DWORD c=_GetImageLib()->GetTexCount(_idTS);

	CMenu menu;	
	menu.CreatePopupMenu();
	int idx;
	idx=0;
	menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_NEW, _T("新增..."));
	if (c>0)
	{
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_DELETE, _T("删除"));
		menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_REPLACE, _T("替换..."));
		if (_bSupportNS)
		{
			menu.InsertMenu(idx++,MF_ENABLED|MF_SEPARATOR,0, _T(""));
			menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_REPLACE_NS, _T("选择法线/高光贴图..."));
			menu.InsertMenu(idx++,MF_ENABLED|MF_STRING,ID_COMMON_REMOVE_NS, _T("清除法线/高光贴图..."));
		}
	}


	ClientToScreen(&point);
	if (idx>0)
		XTFuncContextMenu(&menu,TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,IDR_TOOLBAREXT);

	CWnd::OnRButtonUp(nFlags, point);
}

void CTexCtrl::OnAdd()
{
	if (_idTS==TTSID_INVALID)
		return;
	std::string path=FD_BrowseTex(TRUE);
	if (path!="")
	{
		CTBLImageLib *imagelib=_GetImageLib();
		IUtilRS *pUtilRS=imagelib->GetUtilRS();

		TexData td;

		if (!pUtilRS->LoadTexData(path.c_str(),&td))
		{
			LogFile::Prompt("Failed to load texture file \"%s\"!",path.c_str());
			return;
		}

		if (FALSE==imagelib->GetBrLib()->AddTex(_idTS,&td))
		{
			LogFile::Prompt("Failed to add texture!");
			return;
		}

		imagelib->SyncForTexSet(_idTS);
		((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

		_iTex=imagelib->GetTexCount(_idTS)-1;

		InvalidateRect(NULL,FALSE);
	}

	InvalidateRect(NULL,FALSE);
}

void CTexCtrl::_OnReplace(BOOL bNS)
{
	if (_idTS==TTSID_INVALID)
		return;
	std::string path=FD_BrowseTex(TRUE);
	if (path!="")
	{
		CTBLImageLib *imagelib=_GetImageLib();

		IUtilRS *pUtilRS=imagelib->GetUtilRS();

		TexData td;
		if (!pUtilRS->LoadTexData(path.c_str(),&td))
		{
			LogFile::Prompt("Failed to load texture file \"%s\"!",path.c_str());
			return;
		}
	
		if (TRUE)
		{
			if (!imagelib->GetBrLib()->ReplaceTex(_idTS,_iTex,&td,bNS))
			{
				LogFile::Prompt("Failed to replace texture!");
				return;
			}
		}
		imagelib->SyncForTexSet(_idTS);
		((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

		InvalidateRect(NULL,FALSE);
	}
}


void CTexCtrl::OnReplace()
{
	_OnReplace(FALSE);
}

void CTexCtrl::OnReplaceNS()
{
	_OnReplace(TRUE);
}


void CTexCtrl::OnDelete()
{
	if (IDOK != AfxMessageBox(_T("Are you sure?"), MB_OKCANCEL))
		return;

	CTBLImageLib *imagelib=_GetImageLib();

	imagelib->GetBrLib()->RemoveTex(_idTS,_iTex);

	imagelib->SyncForTexSet(_idTS);
	((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

	_iTex=i_math::clampdown_i(_iTex,(int)imagelib->GetTexCount(_idTS)-1);
	InvalidateRect(NULL,FALSE);
}


void CTexCtrl::OnRemoveNS()
{
	if (_idTS==TTSID_INVALID)
		return;
	CTBLImageLib *imagelib=_GetImageLib();
	imagelib->GetBrLib()->ReplaceTex(_idTS,_iTex,NULL,TRUE);
	imagelib->SyncForTexSet(_idTS);
	((CTBLBaseDlg*)GetParent())->UpdateDueToPSTSChange();

	InvalidateRect(NULL,FALSE);
}



//////////////////////////////////////////////////////////////////////////
//CTBLTexSetDlg

CTBLTexSetDlg::CTBLTexSetDlg(CWnd* pParent /*=NULL*/)
	: CTBLBaseDlg(CTBLTexSetDlg::IDD, pParent)
{
	m_hIcon = NULL;
	_idTimer=0;
	_clock=0;

}

void CTBLTexSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXSETLIST, _list);

}

BEGIN_MESSAGE_MAP(CTBLTexSetDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_LEVELCOMBO, OnCbnSelchangeLevelcombo)
	ON_CBN_SELCHANGE(IDC_LENSLOT, OnCbnSelchangeLenslot)
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CTBLTexSetDlg::Create(CWnd *pParent)
{

	return CDialog::Create(IDD,pParent); 
}

void CTBLTexSetDlg::Refresh(BOOL bReset)
{
	_iBrushLevel=0;
	OnCbnSelchangeLevelcombo();
}

BOOL CTBLTexSetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	InitLevelCombo();

	if (TRUE)
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_LENSLOT);
		CString s;
		for (int i=MINTEXLENSLOT_LOG2;i<=MAXTEXLENSLOT_LOG2;i++)
		{
			s.Format(_T("%dx%d"),1<<i,1<<i);
			pCB->AddString(s);
		}
	}


	if (TRUE)
	{
		RECT rc;
		GET_CONTROL_RECT(this,IDC_TEXCTRL,rc);
		HIDE_CONTROL(this,IDC_TEXCTRL);
		_texctrl.Create(rc,IDC_TEXCTRL,this);
	}



	_list.Initialize();
	_list.SetListEditStyle(_T("TexSets:"),	LBS_XT_DEFAULT);

	_idTimer=(UINT)SetTimer(2,100,NULL);


	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CTBLTexSetDlg::OnPaint() 
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
HCURSOR CTBLTexSetDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTBLTexSetDlg::OnCbnSelchangeLevelcombo()
{
	_iBrushLevel=((CComboBox*)GetDlgItem(IDC_LEVELCOMBO))->GetCurSel();

	if (BRUSHLEVEL_IsNSLevel(_iBrushLevel))
		_texctrl.EnableNS(TRUE);
	else
		_texctrl.EnableNS(FALSE);

	_list.Refresh(TRUE);
	_list.SetCurSel(0);
	_list.OnSelChange();
	OnTexSetSelChange();

	_owner->SetLevelCombo(0,_iBrushLevel);
}


void CTBLTexSetDlg::OnTexSetSelChange()
{
	TTSID idTS=_list.GetTTSID();
	_texctrl.SetTexSet(idTS);
	ITrrnBrushLib *brlib=GetImageLib()->GetBrLib();

	CComboBox *pCB;
	if (TRUE)
	{
		pCB=(CComboBox *)GetDlgItem(IDC_LENSLOT);
		DWORD lenSlot;

		if (brlib->GetTexSetLenSlot(idTS,lenSlot))
		{
			int i;
			for (i=MINTEXLENSLOT_LOG2;i<=MAXTEXLENSLOT_LOG2;i++)
			{
				if (lenSlot==(1<<i))
					break;
			}
			pCB->SetCurSel(i-MINTEXLENSLOT_LOG2);
		}
		else
			pCB->SetCurSel(-1);
	}

	
}


void CTBLTexSetDlg::OnCbnSelchangeLenslot()
{
	TTSID idTS=_list.GetTTSID();
	if (idTS==TTSID_INVALID)
		return;
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_LENSLOT);
	int iSel=pCB->GetCurSel();
	if (iSel==-1)
		return;
	DWORD lenSlot=1<<(iSel+MINTEXLENSLOT_LOG2);
	GetImageLib()->GetBrLib()->SetTexSetLenSlot(idTS,lenSlot);

}


void CTBLTexSetDlg::SetLevelCombo(int iLevel)
{
	CTBLBaseDlg::SetLevelCombo(iLevel);
	OnCbnSelchangeLevelcombo();
}

void CTBLTexSetDlg::OnOK()
{
	KillTimer(_idTimer);
}


void CTBLTexSetDlg::OnTimer(UINT_PTR nID)
{
	CTBLBaseDlg::OnTimer(nID);

	_clock++;

	ITrrnBrushLib *brlib=GetImageLib()->GetBrLib();

	int n=brlib->GetTexSetCount(_iBrushLevel);
	int total=2048*2048;
	int cur=0;
	for (int i=0;i<n;i++)
	{
		TTSID id=brlib->GetTexSetID(_iBrushLevel,i);
		DWORD v,n;
		brlib->GetTexSetLenSlot(id,v);
		n=brlib->GetTexCount(id);
		cur+=v*v*n;
	}
	total/=128*128;
	cur/=128*128;

	CString s;
	s.Format(_T("[Level%02d] 已使用: %d/%d"),_iBrushLevel+1,cur,total);

	if (cur>total)
	{
		if ((_clock/3)%2==0)
			s= _T("");
	}

	CWnd *wnd=GetDlgItem(IDC_STATISTIC);
	CString sOld;
	wnd->GetWindowText(sOld);
	if (sOld!=s)
		wnd->SetWindowText((LPCTSTR)s);




}
