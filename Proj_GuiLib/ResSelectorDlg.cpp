// ResSelectorDlg.cpp : 实现文件
//

#include "stdh.h"
#include "Resource.h"
#include "ResSelectorDlg.h"
#include ".\resselectordlg.h"

#include "WorldSystem/IAssetShell.h"

#include "stringparser/stringparser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CResSelectorDlg 对话框

std::string CResSelectorDlg::ms_root;
std::string CResSelectorDlg::ms_filter;
int CResSelectorDlg::ms_selectMode = RSM_PARTIAL;
std::string CResSelectorDlg::ms_cursel;
std::string CResSelectorDlg::ms_lastsel;
CRect CResSelectorDlg::ms_lastrect;

CResSelectorDlg::CResSelectorDlg(IRenderSystem* rs, LPCTSTR pszRootDir, LPCTSTR pszFilter, CWnd* pParent /*=NULL*/)
	: CDialog(CResSelectorDlg::IDD, pParent), _pRS(rs), _lcResources(rs), _texViewer(rs)
{
	m_hIcon = NULL;

	if (pszRootDir)
		ms_root = toMBCS(pszRootDir);
	if (pszFilter)
		ms_filter = toMBCS(pszFilter);
}

void CResSelectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILELIST, _lcResources);
	DDX_Control(pDX, IDC_TEXTUREVIEWER, _texViewer);
	DDX_Control(pDX, IDC_FILENAME, _lbFileName);
	DDX_Control(pDX, IDC_COMBO_FILETYPE, _cmbFileType);
}

BEGIN_MESSAGE_MAP(CResSelectorDlg, CDialog)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RL_BROWSEDIR, OnFLBrowseDirChanged)
	ON_MESSAGE(WM_TV_UPDATEINFO, OnTVUpdateInfo)
//	ON_LBN_SELCHANGE(IDC_FILELIST, OnLbnSelchangeFilelist)
	ON_NOTIFY(TVN_SELCHANGED,IDC_FILELIST,OnSelChange)
	ON_COMMAND(IDC_COPYPATH,OnCopyPath)
END_MESSAGE_MAP()

// CResSelectorDlg 消息处理程序
void CResSelectorDlg::OnSelChange(NMHDR * pItem,LRESULT * pResult)
{
	OnLbnSelchangeFilelist();
}

void CResSelectorDlg::_GetUnitSizeRef(DWORD *&pw,DWORD *&ph)
{
	static DWORD w=0,h=0;
	pw=&w;
	ph=&h;
}

BOOL CResSelectorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	//SetIcon(m_hIcon, TRUE);			// 设置大图标
	//SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	_lcResources.SetSelector(this);	
	_lcResources.SetFilter(ms_filter.c_str());

	CComboBox *cbW,*cbH;
	if (TRUE)
	{
		cbW=(CComboBox *)GetDlgItem(IDC_UNITWIDTH);
		cbH=(CComboBox *)GetDlgItem(IDC_UNITHEIGHT);

		CString s;
		for (int i=0;i<=256;i++)
		{
			if (i==0)
				s = _T("n/a");
			else
				s.Format(_T("%d"),i);
			cbW->AddString((LPCTSTR)s);
			cbH->AddString((LPCTSTR)s);
		}
	}


	//设置last sel
	if (TRUE)
	{
		std::string path=ms_root+"\\"+ms_cursel;
		std::string pathFolder,filename;
		if (!_pRS->GetFS()->ExistFileAbs(path.c_str()))
		{
			if (ms_lastsel.empty())
				pathFolder=ms_root;
			else
			{
				path=ms_root+"\\"+ms_lastsel;
				pathFolder=GetFileFolderPath(path);
				filename=GetFileName(path);
			}
		}
		else
		{
			pathFolder=GetFileFolderPath(path);
			filename=GetFileName(path);
		}

		_lcResources.SetBrowseDir(pathFolder.c_str(),filename.c_str());
	}

	_texViewer.SetSelectMode(ms_selectMode);
	_viewerMgr.RegisterViewer(&_texViewer);

	if (ms_selectMode == RSM_ENTIRE)
	{
		RECT rcInfo;
		CWnd* pWndInfo = GetDlgItem(IDC_MOUSEINFO);
		pWndInfo->ShowWindow(SW_HIDE);
		pWndInfo->GetWindowRect(&rcInfo);
		ScreenToClient(&rcInfo);

		RECT rcViewer;
		CWnd* pTexViewer = GetDlgItem(IDC_TEXTUREVIEWER);
		pTexViewer->GetWindowRect(&rcViewer);
		ScreenToClient(&rcViewer);
		rcViewer.top = rcInfo.top;
		pTexViewer->MoveWindow(&rcViewer, FALSE);
	}

	if (!ms_filter.empty())
	{
		//char ext[MAX_PATH] = { 0 };

		//int len = 0;
		//char* p = const_cast<char*>(ms_filter.c_str());
		//char* q = p;
		//while (*q != '\0')
		//{
		//	q = strchr(p, '|');
		//	if (!q)
		//		break;

		//	len = q - p;
		//	strncpy(ext, p, len);
		//	_cmbFileType.AddString(ext);

		//	p = q + 1;
		//}
		_cmbFileType.AddString(fromMBCS(ms_filter.c_str()));
		_cmbFileType.SetCurSel(0);
	}

	_lbFileName.SetWindowText(fromMBCS(_lcResources.GetSelResource2()));

	OnLbnSelchangeFilelist();

	if (TRUE)
	{
		CComboBox *cbW,*cbH;
		cbW=(CComboBox *)GetDlgItem(IDC_UNITWIDTH);
		cbH=(CComboBox *)GetDlgItem(IDC_UNITHEIGHT);
		DWORD *pw,*ph;
		_GetUnitSizeRef(pw,ph);
		cbW->SetCurSel(*pw);
		cbH->SetCurSel(*ph);
		if (ms_selectMode!=RSM_PARTIAL)
		{
			cbW->EnableWindow(FALSE);
			cbH->EnableWindow(FALSE);
		}
		else
		{
			cbW->EnableWindow(TRUE);
			cbH->EnableWindow(TRUE);
		}
	}


	if (ms_selectMode == RSM_PARTIAL)
		_texViewer.SetSelectedRect(ms_lastrect);

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CResSelectorDlg::OnPaint() 
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
HCURSOR CResSelectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CResSelectorDlg::OnFLBrowseDirChanged(WPARAM, LPARAM)
{
	SetDlgItemText(IDC_ROOTDIR, fromMBCS(_lcResources.GetBrowseDir()));
	return 0;
}

LRESULT CResSelectorDlg::OnTVUpdateInfo(WPARAM wParam, LPARAM lParam)
{
	CPoint* point = reinterpret_cast<CPoint*>(wParam);
	RECT* rc = reinterpret_cast<RECT*>(lParam);

	// Selection rect & rect size
	int width = rc->right - rc->left;
	int height = rc->bottom - rc->top;

	const int MI_LENGTH = 31;
	TCHAR szMouse[MI_LENGTH + 1];
	memset(szMouse, ' ', MI_LENGTH * sizeof(char));
	szMouse[MI_LENGTH] = '\0';

	CString strMouse;
	strMouse.Format(_T("Mouse: x=%d, y=%d"), point->x, point->y);
	int nLength = strMouse.GetLength();
	if (nLength > MI_LENGTH)
		nLength = MI_LENGTH;
	_tcsncpy(szMouse, strMouse, nLength);
	
	CString strTemp;
	strTemp.Format(_T("%sSelection: %d,%d,%d,%d %dx%d"), 
		szMouse, 
		rc->left, rc->top, rc->right, rc->bottom, 
		width, height);

	SetDlgItemText(IDC_MOUSEINFO, strTemp);

	return 0;
}

void CResSelectorDlg::SetSelectedMode(int mode)
{
	ms_selectMode = mode;
}

void CResSelectorDlg::SetRootDir(const char* dir)
{
	if (dir)
		ms_root = dir;
}

const char* CResSelectorDlg::GetRootDir() const
{
	return ms_root.c_str();
}

void CResSelectorDlg::SetFilter(const char* filter)
{
	_lcResources.SetFilter(filter);
}

const char* CResSelectorDlg::GetSelResource() const
{
	return _lcResources.GetSelResource();
}

const RECT& CResSelectorDlg::GetSelectedRect() const
{
	return _texViewer.GetSelectedRect();
}

void CResSelectorDlg::OnLbnSelchangeFilelist()
{
	// TODO: Add your control notification handler code here
	const char *str=_lcResources.GetSelResource2();
	OnResourceChanged(str);
	_lbFileName.SetWindowText(fromMBCS(str));
//	_lcResources.RedrawList();
}

void CResSelectorDlg::_RecordUnitSizeRef()
{
	DWORD *pw,*ph;
	_GetUnitSizeRef(pw,ph);

	CComboBox *cbW,*cbH;
	cbW=(CComboBox *)GetDlgItem(IDC_UNITWIDTH);
	cbH=(CComboBox *)GetDlgItem(IDC_UNITHEIGHT);
	if (cbW&&cbH)
	{
		*pw=cbW->GetCurSel();
		*ph=cbH->GetCurSel();
	}

}


void CResSelectorDlg::OnCancel()
{
	_RecordUnitSizeRef();

	__super::OnCancel();
}


void CResSelectorDlg::OnOK()
{
	_RecordUnitSizeRef();

	// TODO: Add your specialized code here and/or call the base class
	const char *str= _lcResources.GetSelResource2();
	if (!str[0])
	{
		__super::OnCancel();
		return;
	}

	ms_lastsel=str;
	ms_lastsel=CutHeadPath(ms_lastsel.c_str(),ms_root.c_str());


	__super::OnOK();
}

void CResSelectorDlg::OnLbnDblClick()
{
	OnOK();
}


void CResSelectorDlg::OnCopyPath()
{
	std::string relPath;
	std::string res = GetSelResource();
	if (res.find(GetRootDir()) != std::string::npos)
	{
		size_t len = strlen(GetRootDir());
		relPath = res.substr(len + 1);
	}

	const RECT& rcSelection = GetSelectedRect();
	i_math::recti rc(rcSelection.left, rcSelection.top, rcSelection.right, rcSelection.bottom);

	std::string sResName;
	if (ms_selectMode==RSM_PARTIAL)
	{
		ComposeShellImageStr(sResName, relPath, rc);
	}
	else
		sResName=relPath;

	extern void CopyToClipboard(CWnd *wnd,const char *str);
	CopyToClipboard(this,sResName.c_str());
}


void CResSelectorDlg::GetUnitSize(DWORD &w,DWORD &h)
{
	w=h=0;
	CComboBox *cbW,*cbH;
	cbW=(CComboBox *)GetDlgItem(IDC_UNITWIDTH);
	cbH=(CComboBox *)GetDlgItem(IDC_UNITHEIGHT);
	if (cbW&&cbH)
	{
		CString s;
		cbW->GetWindowText(s);
		w = IntFromString(toMBCS((LPCTSTR)s));
		cbH->GetWindowText(s);
		h=IntFromString(toMBCS((LPCTSTR)s));
	}
}
