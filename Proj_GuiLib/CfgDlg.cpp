/********************************************************************
	created:	2010/7/22   17:28
	file path:	d:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	config dialog
*********************************************************************/

#include "stdh.h"
#include "resource.h"

#include "cfgdlg.h"

#include "WndBase.h"

#include "stringparser/stringparser.h"


//////////////////////////////////////////////////////////////////////////
//CCfgGrid
void CCfgGrid::_Insert(CConfig*cfg,std::vector<std::string>&pieces,const char *entry)
{
	if (pieces.size()<=0)
		return;

	PushInsert();

		if (pieces.size()>1)
		{
			if (!SeekItem(pieces[0].c_str()))
				InsertCategory(pieces[0].c_str(), "", new CXTPPropertyGridItem(fromMBCS(pieces[0].c_str())));
			pieces.erase(pieces.begin());
			_Insert(cfg,pieces,entry);
		}
		else
		{
			ConfigEntry *e=cfg->GetEntry(entry);
			if (e->tp==ConfigEntry::Number)
				InsertVar(&e->num,pieces[0].c_str(),e->desc.c_str(),GVT_S,e->sem);
			if (e->tp==ConfigEntry::String)
				InsertVar(&e->str,pieces[0].c_str(),e->desc.c_str(),GVT_String,e->sem);
		}

	PopInsert();
}

void CCfgGrid::Bind(CConfig *cfg)
{
	DWORD c;
	const char **entries=cfg->EnumEntries(c);


	std::vector<std::string>pieces;
	for (int i=0;i<c;i++)
	{
		BeginInsert();
		SplitStringBy(".",std::string(entries[i]),&pieces);
		_Insert(cfg,pieces,entries[i]);
		EndInsert();
	}


}

//////////////////////////////////////////////////////////////////////////
//CCfgDlg
CCfgDlg::CCfgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CONFIGDLG, pParent)
{
	m_hIcon = NULL;

	_cfg=NULL;

}

void CCfgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CCfgDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CCfgDlg::Create(CWnd *pParent)
{

	return CDialog::Create(IDD_CONFIGDLG,pParent); 
}

// CCfgDlg 消息处理程序

BOOL CCfgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);

	_cfg2.CopyFrom(_cfg);

	_grid.Create(rc,this,1);
	_grid.Bind(&_cfg2);

	_grid.ExpandAll();


	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCfgDlg::OnPaint() 
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
HCURSOR CCfgDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
 


void CCfgDlg::OnOK()
{
	DWORD c;
	const char **entries=_cfg2.EnumEntries(c);
	for (int i=0;i<c;i++)
	{
		ConfigEntry *e=_cfg2.GetEntry(entries[i]);
		if (e->tp==ConfigEntry::String)
			_cfg->SetStr(entries[i],e->str.c_str());
		if (e->tp==ConfigEntry::Number)
			_cfg->SetNumber(entries[i],e->num);
	}

	return CDialog::OnOK();
}

