/********************************************************************
	created:	07/15/2009
	filename: 	StrLibDlg
	author:		chenxi
	
	purpose:	dialog for select&edit string in string lib
*********************************************************************/

#include "stdh.h"
#include "resource.h"
#include "commondefines/general_stl.h"
#include "StrLibDlg.h"

#include "WndBase.h"
#include "CommonCtrlBase.h"

#include "WorldSystem/IEntitySystem.h"
#include "FileSystem/IFileSystem.h"
#include "FileSystem/ISscSystem.h"

#include "strlib/strlib.h"
#include ".\strlibdlg.h"
#include "SscBase.h"


#include "stringparser/stringparser.h"

#define SEARCH_TIMER_ID 10

BOOL StrLibDlg_Browse(DWORD iCategory,StringID &id,const char *grp)
{
	assert(g_ssGuiLib.reg);
	if (!g_ssGuiLib.reg)
		return FALSE;

	CStrLibDlg dlg;

	ISscSystem *pSS=NULL;
	if (g_ssGuiLib.ssc2->HasConfig(*g_ssGuiLib.reg))
		pSS=g_ssGuiLib.ssc2->GetSS();
	else
		pSS=g_ssGuiLib.ssc->GetSS();

	dlg.SetSscSystem(pSS);
	dlg.BindSel(id);
	dlg.SetCategory(iCategory);
	dlg.SetCurGrp(StrLib_Get()->FindGroup(iCategory,grp));

	if (IDCANCEL!=dlg.DoModal())
	{
		id=dlg.GetSel();
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//CReportItemCheck
IMPLEMENT_SERIAL(CReportItemCheck, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)
IMPLEMENT_SERIAL(CReportItemGroups, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CReportItemCheck::CReportItemCheck(BOOL bCheck)
{
	HasCheckbox(TRUE);
	SetChecked(bCheck);
}


int CReportItemCheck::Compare(CXTPReportColumn* /*pColumn*/, CXTPReportRecordItem* pItem)
{
	return int(IsChecked()) - int(pItem->IsChecked());

}


//////////////////////////////////////////////////////////////////////////
//CReportItemGroups
#define GROUPID_START 1000
#define GROUPID_END 5000

CReportItemGroups::CReportItemGroups(const char* str) : CXTPReportRecordItemText(fromMBCS(str))
{
	_owner=NULL;
	
}

void CReportItemGroups::OnInplaceButtonDown(CXTPReportInplaceButton* pButton)
{

	StringID id=(StringID)GetItemData();

	DWORD nGrps;
	StringID *grps=StrLib_Get()->EnumGroup(_iCategory,nGrps);

	if (nGrps<=0)
		return;

	CMenu menu;
	menu.CreatePopupMenu();

	CPoint pt;
	GetCursorPos(&pt);

	StringID idCurGrp=_owner->GetCurGrp();

	for (int i=0;i<nGrps;i++)
	{
		if (idCurGrp!=StringID_Invalid)
		{
			if (grps[i]==idCurGrp)
				continue;
			if (!StrLib_Get()->IsGroupSub(idCurGrp,grps[i]))
				continue;
		}
		if (id==grps[i])
			continue;

		int idx;
		VEC_FIND(_grps,grps[i],idx);
		if (idx==-1)
			menu.InsertMenu(i, MF_ENABLED | MF_STRING, GROUPID_START + i, fromMBCS(StrLib_GetStr(grps[i])));
		else
			menu.InsertMenu(i,MF_ENABLED|MF_STRING|MF_CHECKED,GROUPID_START+i, fromMBCS(StrLib_GetStr(grps[i])));
	}

	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x,pt.y,_owner,NULL );
}

BOOL CanSaveStrLib()
{
	CStrLib *strlib=StrLib_Get();
	if (!strlib)
		return FALSE;

	FileAttr attr=g_ssGuiLib.pFS->GetFileAttrAbs(strlib->GetPath());
	if ((attr==File_ReadOnly)||(attr==File_Miss))
		return FALSE;
	return TRUE;
}

BOOL SaveStrLib()
{
	CStrLib *strlib=StrLib_Get();
	if (!strlib)
		return FALSE;

	IFile *fl=g_ssGuiLib.pFS->OpenFileAbs(strlib->GetPath(),FileAccessMode_Write);
	if (!fl)
		return FALSE;

	CDataPacket dp;
	std::vector<BYTE>buf;
	DP_BeginSave(dp,buf);
		strlib->Save(dp);
	DP_EndSave();

	IFile_WriteVector(fl,buf);
	fl->Close();
	return TRUE;
}

BOOL LoadStrLib()
{
	CStrLib *strlib=StrLib_Get();
	if (!strlib)
		return FALSE;

	IFile *fl=g_ssGuiLib.pFS->OpenFileAbs(strlib->GetPath(),FileAccessMode_Read);
	if (!fl)
		return FALSE;

	CDataPacket dp;
	std::vector<BYTE>buf;

	IFile_ReadVector(fl,buf);
	dp.SetDataBufferPointer(buf.data());
	strlib->Load(dp);

	fl->Close();
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CStrLibList

BEGIN_MESSAGE_MAP(CStrLibList,CXTPReportControl)
	//{{AFX_MSG_MAP(CStrLibList)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(GROUPID_START,GROUPID_END,OnGroupRange)
END_MESSAGE_MAP()

BOOL CStrLibList::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CXTPReportControl::Create(0x50010000,rect,pParentWnd,nID))
		return FALSE;

	SetGridStyle(TRUE,xtpReportGridSolid);
	SetGridStyle(FALSE,xtpReportGridSolid);

	CXTPReportControl::AddColumn(new CXTPReportColumn(0,_T("组"),80));

	CXTPReportColumn *column=new CXTPReportColumn(1,_T("内容"),400);
	CXTPReportControl::AddColumn(column);
	column->SetEditable(FALSE);

	column=new CXTPReportColumn(2,_T("属于的组"),300);
	CXTPReportControl::AddColumn(column);
	column->SetEditable();
	column->GetEditOptions()->AddExpandButton();
	column->GetEditOptions()->m_bAllowEdit=FALSE;
	CXTPReportControl::AddColumn(new CXTPReportColumn(3,_T("注释"),200));

	CXTPReportControl::GetReportHeader()->AllowColumnRemove(FALSE);
	CXTPReportControl::GetReportHeader()->AllowColumnSort(TRUE);


	EnableToolTips(FALSE);
	AllowEdit(TRUE);
	SetMultipleSelection(TRUE);

	return TRUE;

}

void CStrLibList::_AddRecord(StringID id)
{
	CXTPReportRecord *record=new CReportRecord;
	const char *s=StrLib_GetStr(id);
	BOOL bGroup=StrLib_Get()->IsGroup(id);
	record->AddItem(new CReportItemCheck(bGroup));
	CXTPReportRecordItem* item = record->AddItem(new CXTPReportRecordItemText(fromMBCS(s)));
	item->SetItemData(id);

	CReportItemGroups *grp=new CReportItemGroups("");
	grp->_owner=this;
	grp->_iCategory=_iCategory;
	grp->SetItemData(id);
	record->AddItem(grp);

	record->AddItem(new CXTPReportRecordItemText(_T("-")));

	AddRecord(record);
}



void CStrLibList::Refresh(LPCTSTR lpszFilter)
{
	ResetContent();

	DWORD count;
	StringID *ids;
	if (_curgrp==StringID_Invalid)
		ids=StrLib_Get()->Enum(_iCategory,count);
	else
		ids=StrLib_Get()->EnumGroupSubs(_curgrp,count);

	const char* strFilter = toMBCS(lpszFilter);

	for (int i = 0; i < count; i++)
	{
		const char* str = StrLib_GetStr(ids[i]);
		// 添加过滤条件
		if (lpszFilter && *lpszFilter)
		{
			if (std::string(str).find(strFilter) == -1)
				continue;
		}
		_AddRecord(ids[i]);
	}

	Populate();

	_RefreshGroups(StringID_Invalid);
}

DWORD CStrLibList::GetCount()
{
	return GetRows()->GetCount();
}

StringID CStrLibList::GetStringID(CXTPReportRecord *record)
{
	if (!record)
		return StringID_Invalid;
	return (StringID)record->GetItem(1)->GetItemData();
}


StringID CStrLibList::GetStringID(int iSel)
{
	CXTPReportRows*rows=GetRows();

	if (((DWORD)iSel)>=rows->GetCount())
		return StringID_Invalid;

	return GetStringID(rows->GetAt(iSel)->GetRecord());
}

void CStrLibList::OnGroupCheck(CXTPReportRecord *record)
{
	CReportItemCheck *item=(CReportItemCheck *)record->GetItem(0);
	StringID id=GetStringID(record);

	if (!item->IsChecked())
	{
		std::string s;
		FormatString(s,"确认要将组\"%s\"去除吗?",StrLib_GetStr(id));
		if (IDOK != AfxMessageBox(fromMBCS(s.c_str()), MB_OKCANCEL))
		{
			item->SetChecked(TRUE);//恢复check的状态
			return;
		}
	}
	StrLib_Get()->SetGroup(id,item->IsChecked());
	_RefreshGroups(StringID_Invalid);
}

void CStrLibList::SetCurSel(StringID idSel)
{
	int iSel=-1;
	for (int i=0;i<GetCount();i++)
	{
		if (GetStringID(i)==idSel)
		{
			iSel=i;
			break;
		}
	}

	if (iSel!=-1)
	{
		GetSelectedRows()->Select(GetRows()->GetAt(iSel));
		SetFocusedRow(GetRows()->GetAt(iSel));
		EnsureVisible(GetRows()->GetAt(iSel));
	}
	else
		GetSelectedRows()->Clear();
}

CXTPReportRecord*CStrLibList::_GetCurSel()
{
	CXTPReportSelectedRows*sels=GetSelectedRows();
	POSITION pos=sels->GetFirstSelectedRowPosition();
	if (pos==0)
		return NULL;
	CXTPReportRow*row=sels->GetNextSelectedRow(pos);
	if (pos==0)
		return row->GetRecord();//唯一的一个选中
	return NULL;//选中多个
}


StringID CStrLibList::GetCurSel()
{
	return GetStringID(_GetCurSel());
}

DWORD CStrLibList::IsSel()
{
	CXTPReportSelectedRows*sels=GetSelectedRows();
	POSITION pos=sels->GetFirstSelectedRowPosition();
	if (pos==0)
		return FALSE;
	return TRUE;
}


void CStrLibList::_RefreshGroups(StringID id)
{
	std::multimap<StringID,StringID>lookup;
	_BuildGrpLookup(lookup);

	std::string s;

	CXTPReportRows*rows=GetRows();

	for (int i=0;i<GetCount();i++)
	{
		StringID idCur=GetStringID(i);
		if ((idCur==id)||(id==StringID_Invalid))
		{//需要更新

			CReportItemGroups *item=(CReportItemGroups *)rows->GetAt(i)->GetRecord()->GetItem(2);
			item->_grps.clear();

			std::multimap<StringID,StringID>::iterator it=lookup.find(idCur);

			s="";
			if (it!=lookup.end())
			{
				s=StrLib_GetStr((*it).second);
				item->_grps.push_back((*it).second);
				it++;
				while((it!=lookup.end())&&((*it).first==idCur))
				{
					s+=",";
					s+=StrLib_GetStr((*it).second);
					item->_grps.push_back((*it).second);
					it++;
				}
			}

			item->SetCaption(fromMBCS(s.c_str()));

			if (id!=StringID_Invalid)
				break;
		}
	}

}

void CStrLibList::OnGroupRange(UINT idCmd)
{
	DWORD nGrps;
	StringID *grps=StrLib_Get()->EnumGroup(_iCategory,nGrps);

	if (idCmd-GROUPID_START>=nGrps)
		return;
	StringID idGrp=grps[idCmd-GROUPID_START];

	StringID id=GetCurSel();
	if (id!=StringID_Invalid)
	{
		if (StrLib_Get()->IsGroupSub(idGrp,id))
			StrLib_Get()->RemoveGroupSub(idGrp,id);
		else
		{
			if (!_VerifyGrpUnique(StrLib_GetStr(id),idGrp))
				return;
			StrLib_Get()->AddGroupSub(idGrp,id);
		}

		_RefreshGroups(id);
	}
}

void CStrLibList::_BuildGrpLookup(std::multimap<StringID,StringID>&lookup)
{
	lookup.clear();

	DWORD nGrps;
	StringID *grps=StrLib_Get()->EnumGroup(_iCategory,nGrps);

	for (int i=0;i<nGrps;i++)
	{
		StringID idGrp=grps[i];

		DWORD nSubs;
		StringID *subs=StrLib_Get()->EnumGroupSubs(idGrp,nSubs);

		for (int j=0;j<nSubs;j++)
		{
			StringID id=subs[j];
			lookup.insert(std::pair<StringID,StringID>(id,idGrp));
		}
	}
}

BOOL CStrLibList::_VerifyGrpUnique(const char *str,StringID idGrp)
{
	if (idGrp!=StringID_Invalid)
	{
		StringID id=StrLib_Get()->FindStr(_iCategory,str,StrLib_GetStr(idGrp));
		if (id!=StringID_Invalid)
		{
			std::string s;
			FormatString(s,"在组[\"%s\"]中发现重复的字符串:\"%s\"!",StrLib_GetStr(idGrp),str);
			AfxMessageBox(fromMBCS(s.c_str()),MB_OK);
			return FALSE;
		}
	}
	return TRUE;
}


void CStrLibList::OnChange(const char *str)
{
	CXTPReportRecord*record=_GetCurSel();
	if (!record)
		return;

	StringID id=GetStringID(record);
	if (strcmp(StrLib_GetStr(id),str)==0)
		return;//没有变化
	if (!_VerifyGrpUnique(str,_curgrp))
		return;
	StrLib_Get()->ModifyStr(id,str);
	record->GetItem(1)->SetCaption(fromMBCS(str));
	if (StrLib_Get()->IsGroup(id))
		_RefreshGroups(StringID_Invalid);

	RedrawControl();
}

void CStrLibList::OnNew(const char *str)
{
	//首先我们检查有没有在当前组中重名
	if (!_VerifyGrpUnique(str,_curgrp))
		return;

	StringID id=StrLib_Get()->AddStr(_iCategory,str);
	if (_curgrp!=StringID_Invalid)
		StrLib_Get()->AddGroupSub(_curgrp,id);
	_AddRecord(id);
	Populate();

	_RefreshGroups(id);

	SetCurSel(id);
}

void CStrLibList::OnDelete()
{
	if (IDOK != AfxMessageBox(_T("确认要删除这些(个)字符串吗?"), MB_OKCANCEL))
		return;

	CXTPReportSelectedRows*sels=GetSelectedRows();
	POSITION pos=sels->GetFirstSelectedRowPosition();
	if (pos==0)
		return;
	std::vector<CXTPReportRecord*>records;
	while(pos)
	{
		CXTPReportRow*row=sels->GetNextSelectedRow(pos);
		CXTPReportRecord*record=row->GetRecord();
		records.push_back(record);
	}

	for (int i=0;i<records.size();i++)
	{
		CXTPReportRecord*record=records[i];
		StringID id=GetStringID(record);

		StrLib_Get()->RemoveStr(id);

		RemoveRecordEx(record,TRUE);
	}

	_RefreshGroups(StringID_Invalid);

}

void CStrLibList::OnSortChange()
{
	SetCurSel(GetCurSel());
}


//////////////////////////////////////////////////////////////////////////
//CStrLibDlg

CStrLibDlg::CStrLibDlg(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_DIALOG_STRLIB, pParent)
{
	_pSS=NULL;

	_idSel=StringID_Invalid;
	_idEdit=StringID_Invalid;
	_iCategory=STRLIB_CATEGORY_DEFAULT;
	_bReadOnly=FALSE;

	_bEditModified=FALSE;

	_idTimer=0;
}

void CStrLibDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CStrLibDlg, CXTPDialog)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_SEARCH, OnEnChangeSearch)
	ON_EN_CHANGE(IDC_EDIT_SEARCH, OnEnChangeSearch)
	ON_NOTIFY(XTP_NM_REPORT_CHECKED, IDC_LIST,OnReportCheckItem)
	ON_NOTIFY(XTP_NM_REPORT_SORTORDERCHANGED, IDC_LIST,OnReportSortChange)
	ON_NOTIFY(NM_DBLCLK,IDC_LIST, OnDblClick)
	ON_WM_TIMER()
	ON_COMMAND(IDC_EMPTY,OnEmpty)
	ON_COMMAND(IDC_CHANGE,OnChange)
	ON_COMMAND(IDC_NEW,OnNew)
	ON_COMMAND(IDC_DELETE,OnDelete)

END_MESSAGE_MAP()
   

BOOL CStrLibDlg::Create(CWnd *pParent)
{
	return CXTPDialog::Create(IDD_DIALOG_STRLIB,pParent); 
}


void CStrLibDlg::UpdateGrpCombo()
{
	CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_GROUPCOMBO);
	pCB->ResetContent();
	int idx = pCB->AddString(_T("<全部>"));
	pCB->SetItemData(idx,StringID_Invalid);

	int iCurSel=0;
	DWORD count;
	StringID *ids=StrLib_Get()->EnumGroup(_iCategory,count);
	for (int i=0;i<count;i++)
	{
		idx = pCB->AddString(fromMBCS(StrLib_GetStr(ids[i])));
		if (ids[i]==_list.GetCurGrp())
			iCurSel=idx;
		pCB->SetItemData(idx,ids[i]);
	}

	pCB->SetCurSel(iCurSel);
}


BOOL CStrLibDlg::OnInitDialog()
{
	CXTPDialog::OnInitDialog();

	CRect rc;
	GET_CONTROL_RECT(this,IDC_LIST,rc);
	HIDE_CONTROL(this,IDC_LIST);


	BOOL bSscOk=FALSE;
	BOOL bNeedReload=FALSE;
	if (_pSS)
	{
		if (_pSS->IsConnected())
		{
			bSscOk=FALSE;
			SscState state;
			const char *pathFull=StrLib_Get()->GetPath();
			if (_pSS->GetState(pathFull,state))
			{
				if ((state==SSC_NOTCHECKEDOUT)||(state==SSC_CHECKEDOUT_ME))
				{
					if (TRUE==_pSS->CheckOut(pathFull,128))
					{
						bNeedReload=TRUE;
						bSscOk=TRUE;
					}
				}
				if (state==SSC_NOTCONTROLLED)
				{
					if (_pSS->CheckIn(pathFull,0))
					{
						if (_pSS->CheckOut(pathFull,128))
							bSscOk=TRUE;
					}
				}
			}
		}
	}

	if (!bSscOk)
	{
		if (AfxMessageBox(_T("无法独占的check out字符串库,是否要强制编辑?"), MB_OKCANCEL) == IDOK)
			bSscOk = TRUE;
	}


	if (!bSscOk)
	{
		AfxMessageBox(_T("无法独占的check out字符串库,将无法进行编辑!"), MB_OK);
		_bReadOnly=TRUE;
	}
	else
	{
		if (bNeedReload)
			LoadStrLib();
		if (!CanSaveStrLib())
		{
			AfxMessageBox(_T("字符串库为只读文件,将无法进行编辑!"),MB_OK);
			_bReadOnly=TRUE;
		}
	}

	_list.Create(rc,this,IDC_LIST);

	//	_list.EnableEdit(FALSE);

	_list.ShowWindow(SW_SHOW);

	_list._owner=this;
	_list.SetCategory(_iCategory);

	_list.Refresh();

	_list.SetCurSel(_idSel);
	_idSel=_list.GetCurSel();

	//更新Group Combo
	UpdateGrpCombo();

	_idTimer=(UINT)SetTimer(1,10,NULL);

	return TRUE;

}

void CStrLibDlg::_Finish()
{
	if (!_bReadOnly)
	{
		if (StrLib_Get()->IsModified())
		{
			SaveStrLib();
			StrLib_Get()->ClearModified();
		}
	}

	if (_pSS)
	{
		if (_pSS->IsConnected())
		{
			const char *pathFull=StrLib_Get()->GetPath();
			_pSS->CheckIn(pathFull,0);
		}
	}

	KillTimer(_idTimer);
}


void CStrLibDlg::OnOK()
{
	if (_bEditModified)
		OnChange();
	_Finish();
	CXTPDialog::OnOK();
}

void CStrLibDlg::OnCancel()
{
	_Finish();
	CXTPDialog::OnCancel();
}

void CStrLibDlg::OnEnChangeEdit()
{
	_bEditModified=TRUE;
}


void CStrLibDlg::UpdateEdit()
{
	CEdit *edit=(CEdit*)GetDlgItem(IDC_EDIT);
	if (_bReadOnly)
		edit->EnableWindow(FALSE);
	else
	{
		if (_idSel!=_idEdit)
		{
			if (_idSel!=StringID_Invalid)
			{
				edit->SetWindowText(fromMBCS(StrLib_GetStr(_idSel)));
				edit->EnableWindow(!_bReadOnly);
			}
			else
			{
				edit->SetWindowText(_T(""));
				edit->EnableWindow(!_bReadOnly);
			}
			_idEdit=_idSel;

			_bEditModified=FALSE;
		}
	}

}


void CStrLibDlg::SelectEdit()
{
	CEdit *edit=(CEdit*)GetDlgItem(IDC_EDIT);
	edit->SetFocus();
	edit->SetSel(0,-1,TRUE);
}

void CStrLibDlg::OnReportCheckItem(NMHDR*  pNotifyStruct, LRESULT* )
{
     XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
     ASSERT(pItemNotify != NULL);

	 CXTPReportRecord *record=pItemNotify->pRow->GetRecord();

	 _list.OnGroupCheck(record);
	 UpdateGrpCombo();
}

void CStrLibDlg::OnReportSortChange(NMHDR*  pNotifyStruct, LRESULT* )
{
	_list.OnSortChange();
}


void CStrLibDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SEARCH_TIMER_ID)
	{
		KillTimer(SEARCH_TIMER_ID);
		CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SEARCH);
		pEdit->GetWindowText(m_strSearchFilter);
		_list.Refresh(m_strSearchFilter);
	} 
	else
	{
		CComboBox *pCB=(CComboBox *)GetDlgItem(IDC_GROUPCOMBO);
		int iSel=pCB->GetCurSel();
		if (iSel!=-1)
		{
			StringID id=(StringID)pCB->GetItemData(iSel);
			if (id!=_list.GetCurGrp())
			{
				_list.SetCurGrp(id);
				_list.Refresh(m_strSearchFilter);
			}
		}
	}

	_idSel=_list.GetCurSel();
	UpdateEdit();
	UpdateBtns();
}

void CStrLibDlg::UpdateBtns()
{
	if (_bEditModified)
	{
		ENABLE_CONTROL(this,IDC_NEW);
	}
	else
	{
		DISABLE_CONTROL(this,IDC_NEW);
	}

	if ((_bEditModified)&&(_idSel!=StringID_Invalid))
	{
		ENABLE_CONTROL(this,IDC_CHANGE);
	}
	else
	{
		DISABLE_CONTROL(this,IDC_CHANGE);
	}

	if (!_list.IsSel())
	{
		DISABLE_CONTROL(this,IDC_DELETE);
	}
	else
	{
		ENABLE_CONTROL(this,IDC_DELETE);
	}

	if (_idSel!=StringID_Invalid)
	{
		ENABLE_CONTROL(this,IDOK);
	}
	else
	{
		DISABLE_CONTROL(this,IDOK);
	}


	if (TRUE)
	{
		std::string s="选择";
		if (_bEditModified)
			s="修改并选择";
		CWnd *wnd=GetDlgItem(IDOK);
		CString sOld;
		wnd->GetWindowText(sOld);
		if (sOld!=s.c_str())
			wnd->SetWindowText(fromMBCS(s.c_str()));
	}
}

void CStrLibDlg::OnChange()
{
	CWnd *wnd=GetDlgItem(IDC_EDIT);
	CString s;
	wnd->GetWindowText(s);

	_list.OnChange(toMBCS((LPCTSTR)s));
	UpdateGrpCombo();//有可能修改一个group的名字,所以要更新一下group combo

	_bEditModified=FALSE;
}

void CStrLibDlg::OnNew()
{
	CWnd *wnd=GetDlgItem(IDC_EDIT);
	CString s;
	wnd->GetWindowText(s);

	_list.OnNew(toMBCS((LPCTSTR)s));
}

void CStrLibDlg::OnDelete()
{
	_list.OnDelete();
}

void CStrLibDlg::OnEmpty()
{
	_idSel=StringID_Invalid;
	_Finish();
	CXTPDialog::OnOK();
}


void CStrLibDlg::OnDblClick(NMHDR * pNotifyStruct, LRESULT * result)
{
	_idSel=_list.GetCurSel();
	_Finish();
	CXTPDialog::OnOK();
}

void CStrLibDlg::OnEnChangeSearch()
{
	SetTimer(SEARCH_TIMER_ID, 500, NULL); // 500ms延时
// 
// 	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SEARCH);
// 	pEdit->GetWindowText(m_strSearchFilter);
// 
// 	// 立即刷新列表（或可添加延时逻辑）
// 	_list.Refresh(m_strSearchFilter);
}