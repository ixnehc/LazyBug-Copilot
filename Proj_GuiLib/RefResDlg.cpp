/********************************************************************
	created:	2011/5/30   12:46
	file path:	e:\IxEngine\Proj_GuiLib
	author:		chenxi
	
	purpose:	refering resource list dialog
*********************************************************************/
#include "stdh.h"
#include "GuiLib.h"
#include "WndBase.h"
#include "SscBase.h"
#include "RefResDlg.h"
#include "StrLibDlg.h"//for CReportItemCheck

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IUtilRS.h"

#include "resdata/ResData.h"
#include "stringparser/stringparser.h"



class CRefResRecord:public CXTPReportRecord
{
public:
	CRefResRecord()
	{
	}

	BOOL IsSel()
	{
		CReportItemCheck*item=(CReportItemCheck*)GetItem(0);
		return item->IsChecked();
	}
	void Select(BOOL bSel)
	{
		CReportItemCheck*item=(CReportItemCheck*)GetItem(0);
		item->SetChecked(bSel);
	}

	void GetItemMetrics (XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics)
	{
		int nIndex = pDrawArgs->pRow->GetIndex();

		if (nIndex % 2)
		{
			pItemMetrics->clrBackground = RGB(245, 245, 245);
		}
	}

protected:
	std::string _pathAbs;
	friend class CRefResDlg;
};



// CRefResDlg dialog

IMPLEMENT_DYNAMIC(CRefResDlg, CXTPDialog)
CRefResDlg::CRefResDlg(CWnd* pParent /*=NULL*/)
	: CXTPDialog(CRefResDlg::IDD, pParent)
{
}

CRefResDlg::~CRefResDlg()
{
}

void CRefResDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRefResDlg, CXTPDialog)
	ON_COMMAND(ID_CHECKIN,OnCheckIn)
	ON_COMMAND(ID_CHECKOUT,OnCheckOut)
	ON_COMMAND(ID_GET,OnGet)
	ON_COMMAND(ID_SELECTALL,OnSelectAll)
	ON_COMMAND(ID_DESELECTALL,OnDeselectAll)
END_MESSAGE_MAP()


// CRefResDlg message handlers

BOOL CRefResDlg::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);


	if (_list.Create(0x50010000,rc,this,IDC_LIST))
	{
		_list.SetGridStyle(TRUE,xtpReportGridSolid);
		_list.SetGridStyle(FALSE,xtpReportGridSolid);

		_list.AddColumn(new CXTPReportColumn(0,_T("  "),40));
		_list.AddColumn(new CXTPReportColumn(1,_T(" 存在"),88));
		_list.AddColumn(new CXTPReportColumn(2,_T(" 资源路径"),500));

		_list.GetReportHeader()->AllowColumnRemove(FALSE);
		_list.GetReportHeader()->AllowColumnSort(TRUE);


		_list.EnableToolTips(FALSE);
		_list.AllowEdit(FALSE);
		_list.SetMultipleSelection(TRUE);
		_list.SelectionEnable(FALSE);
		_list.ShowRowFocus(FALSE);
		_imglist.Create(16,16,ILC_COLOR32|ILC_MASK,0,1);
		CBitmap bmp;
		bmp.LoadBitmap(IDB_REFRESICON);
		_imglist.Add(&bmp,RGB(255,255,255));

		_list.SetImageList(&_imglist);

		if (_buf.size()>0)
		{
			std::string pathResRoot=g_ssGuiLib.pRS->GetPath(Path_Res);
			pathResRoot+="\\";
			int len=pathResRoot.length();

			for (int i=0;i<_buf.size();i++)
				_AddRecord(_buf[i].c_str()+len,_buf[i].c_str());
			while(_UpdateRef());
			_UpdateState();
			_list.Populate();
		}

	}

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CRefResDlg::_UpdateState()
{
	CXTPReportRecords *records=_list.GetRecords();
	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		CXTPReportRecordItemText *item=(CXTPReportRecordItemText *)record->GetItem(2);
		CXTPReportRecordItemText *item2=(CXTPReportRecordItemText *)record->GetItem(1);
		std::string s=record->_pathAbs;

		int idxIcon=0;

		SscState state;
		if (g_ssGuiLib.ssc->GetState(s.c_str(),state))
		{
			switch(state)
			{
				case SSC_UNKNOWN:
					idxIcon=5;break;
				case SSC_NOTCONTROLLED:
					idxIcon=1;break;
				case SSC_NOTCHECKEDOUT:
					idxIcon=3;break;
				case SSC_CHECKEDOUT:
					idxIcon=2;break;
				case SSC_CHECKEDOUT_ME:
					idxIcon=4;break;
			}
		}
		else
			idxIcon=5;
		if (!g_ssGuiLib.pFS->ExistFileAbs(s.c_str()))
		{
			item->SetTextColor(RGB(255,0,0));
			item2->SetValue(_T("  [Miss]"));
		}
		else
		{
			if (g_ssGuiLib.pFS->GetFileAttrAbs(s.c_str())==File_ReadOnly)
				item->SetTextColor(RGB(128,128,128));
			else
				item->SetTextColor(RGB(0,0,0));

			item2->SetValue(_T(""));
		}

		item->SetIconIndex(idxIcon);
	}

}

void CRefResDlg::_AddRecord(const char *path,const char *pathAbs)
{
	//过滤掉一些资源
	if (path[0]=='_')
		return;

	CRefResRecord *record=new CRefResRecord;

	CXTPReportRecordItemText *item;
	record->AddItem(new CReportItemCheck(TRUE));
	record->AddItem(new CXTPReportRecordItemText(_T("")));

	record->AddItem(item = new CXTPReportRecordItemText(fromMBCS(path)));
	item->SetIconIndex(0);

	record->_pathAbs=pathAbs;

	_list.AddRecord(record);
}

BOOL UniqueAddPath(std::vector<std::string>&buf,std::string &path)
{
	for (int i=0;i<buf.size();i++)
	{
		if (StringEqualNoCase(buf[i].c_str(),path.c_str()))
			return FALSE;
	}
	buf.push_back(path);

	return TRUE;
}

BOOL CRefResDlg::_UpdateRef()
{
	IUtilRS *pUtilRS=g_ssGuiLib.pUtilRS;
	std::vector<std::string>refs;
	std::string pathResRoot=g_ssGuiLib.pRS->GetPath(Path_Res);

	int len=pathResRoot.length()+1;
	std::string pathRes;
	std::vector<std::string>buf2;
	for (int i=0;i<_buf.size();i++)
	{
		//过滤掉那些不会引用其它资源的资源 
		if (TRUE)
		{
			BOOL bRef=FALSE;

			if (CheckFileSuffix(_buf[i].c_str(),"mtl"))
				bRef=TRUE;

			if (!bRef)
				continue;
		}

		if (!ResolveRelativePath(pathRes,pathResRoot.c_str(),_buf[i].c_str()))
			continue;


		refs.clear();
		ResData *data=pUtilRS->LoadRes(_buf[i].c_str(),FALSE);
		if (data)
		{
			data->CollectRefs(refs);
			ResData_Delete(data);

			for (int k=0;k<refs.size();k++)
			{
				const char *path=ResolveRefPath(refs[k].c_str(),pathRes.c_str());
				refs[k]=pathResRoot+"\\"+path;
				UniqueAddPath(buf2,refs[k]);
			}
		}
	}

	DWORD nOrg=_buf.size();

	for (int i=0;i<buf2.size();i++)
	{
		if (UniqueAddPath(_buf,buf2[i]))
			_AddRecord(buf2[i].c_str()+len,buf2[i].c_str());
	}

	if (_buf.size()>nOrg)
		return TRUE;

	return FALSE;
}



BOOL CRefResDlg::_CanOp()
{
	if (!g_ssGuiLib.ssc->IsConnected())
		return FALSE;
	return TRUE;
}

void CRefResDlg::SelectAll()
{
	CXTPReportRecords *records=_list.GetRecords();
	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		record->Select(TRUE);
	}
}

void CRefResDlg::DeselectAll()
{
	CXTPReportRecords *records=_list.GetRecords();
	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		record->Select(FALSE);
	}
}


void CRefResDlg::OnCheckIn()
{
	if (!_CanOp())
		return;

	CXTPReportRecords *records=_list.GetRecords();
	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		if (!record->IsSel())
			continue;
		const char *path=record->_pathAbs.c_str();
		g_ssGuiLib.ssc->CheckIn(path);
	}
	DeselectAll();

	_UpdateState();
	_list.Populate();
}

void CRefResDlg::OnCheckOut()
{
	if (!_CanOp())
		return;

	CXTPReportRecords *records=_list.GetRecords();

	std::vector<const char *>buf;
	buf.reserve(records->GetCount());

	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		if (!record->IsSel())
			continue;
		buf.push_back(record->_pathAbs.c_str());
	}

	g_ssGuiLib.ssc->CheckOut(buf.data(),buf.size());

	DeselectAll();

	while(_UpdateRef());
	_UpdateState();
	_list.Populate();
}

void CRefResDlg::OnGet()
{
	if (!_CanOp())
		return;

	CXTPReportRecords *records=_list.GetRecords();

	std::vector<const char *>buf;
	buf.reserve(records->GetCount());

	for (int i=0;i<records->GetCount();i++)
	{
		CRefResRecord*record=(CRefResRecord*)records->GetAt(i);
		if (!record->IsSel())
			continue;
		buf.push_back(record->_pathAbs.c_str());
	}

	g_ssGuiLib.ssc->GetLatestVersion(buf.data(),buf.size());

	DeselectAll();

	while(_UpdateRef());
	_UpdateState();
	_list.Populate();
}


void CRefResDlg::OnSelectAll()
{
	SelectAll();
	_list.Populate();
}

void CRefResDlg::OnDeselectAll()
{
	DeselectAll();
	_list.Populate();
}
