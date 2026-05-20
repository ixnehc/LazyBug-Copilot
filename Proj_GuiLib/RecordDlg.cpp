/********************************************************************
	created:	2008/5/7   16:06
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	dialog for selecting proto
*********************************************************************/


#include "stdh.h"
#include "resource.h"
#include "RecordDlg.h"

#include "WndBase.h"
#include ".\protoselectdlg.h"

#include "WorldSystem/IEntitySystem.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"

#include "RenderSystem/IRecords.h"

#include "strlib/strlib.h"

#include "log/LogDump.h"

#include "gds/GObjEx.h"



BOOL RecordDlg_Browse(RecordID &id,std::string &name,const char *nameRecords)
{
	IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes(nameRecords);
	if (!records)
	{
		LOG_DUMP_1P("RecordDlg",Log_Error,"不存在的records文件:\"%s\"!",nameRecords);
		return FALSE;
	}

	BOOL bOk=FALSE;
	CRecordDlg dlg;
	dlg.SetRecords(records->GetRecords());
	dlg.SetSel(id);

	if (IDOK==dlg.DoModal())
	{
		id=dlg.GetSel();
		name=records->GetRecords()->GetName(id);
		bOk=TRUE;
	}


	SAFE_RELEASE(records);

	return bOk;
}



//////////////////////////////////////////////////////////////////////////
//CXTPReportRecordItemInt


IMPLEMENT_SERIAL(CXTPReportRecordItemInt, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPReportRecordItemInt::CXTPReportRecordItemInt(int value)
: CXTPReportRecordItem(), m_value(value)
{
}


CString CXTPReportRecordItemInt::GetCaption(CXTPReportColumn* /*pColumn*/)
{
	if (!m_strCaption.IsEmpty())
		return m_strCaption;

	CString strCaption;
	strCaption.Format(_T("%d"), m_value);
	return strCaption;
}

int CXTPReportRecordItemInt::Compare(CXTPReportColumn*, CXTPReportRecordItem* pItem)
{
	CXTPReportRecordItemInt* pItemNumber = DYNAMIC_DOWNCAST(CXTPReportRecordItemInt, pItem);
	if (!pItemNumber)
		return 0;

	if (m_value == pItemNumber->m_value)
		return 0;
	else if (m_value > pItemNumber->m_value)
		return 1;
	else
		return -1;
}

void CXTPReportRecordItemInt::OnEditChanged(XTP_REPORTRECORDITEM_ARGS* /*pItemArgs*/, LPCTSTR szText)
{
	SetValue((int)atoi(toMBCS(szText)));
}

void CXTPReportRecordItemInt::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPReportRecordItem::DoPropExchange(pPX);

	PX_Long(pPX, _T("Value"), (long&)m_value);
}



//////////////////////////////////////////////////////////////////////////
//CXTPReportRecordItemFloat


IMPLEMENT_SERIAL(CXTPReportRecordItemFloat, CXTPReportRecordItem, VERSIONABLE_SCHEMA | _XTP_SCHEMA_CURRENT)

CXTPReportRecordItemFloat::CXTPReportRecordItemFloat(float value)
: CXTPReportRecordItem(), m_value(value)
{
}


CString CXTPReportRecordItemFloat::GetCaption(CXTPReportColumn* /*pColumn*/)
{
	if (!m_strCaption.IsEmpty())
		return m_strCaption;

	CString strCaption;
	strCaption.Format(_T("%.3f"), m_value);
	return strCaption;
}

int CXTPReportRecordItemFloat::Compare(CXTPReportColumn*, CXTPReportRecordItem* pItem)
{
	CXTPReportRecordItemFloat* pItemNumber = DYNAMIC_DOWNCAST(CXTPReportRecordItemFloat, pItem);
	if (!pItemNumber)
		return 0;

	if (m_value == pItemNumber->m_value)
		return 0;
	else if (m_value > pItemNumber->m_value)
		return 1;
	else
		return -1;
}

void CXTPReportRecordItemFloat::OnEditChanged(XTP_REPORTRECORDITEM_ARGS* /*pItemArgs*/, LPCTSTR szText)
{
	SetValue((float)atof(toMBCS(szText)));
}

void CXTPReportRecordItemFloat::DoPropExchange(CXTPPropExchange* pPX)
{
	CXTPReportRecordItem::DoPropExchange(pPX);

	PX_Float(pPX, _T("Value"), m_value);
}

// BOOL ParseComboString(GElemBase *elem,int v)
// {
// 	GSem &sem=elem->GetSem();
// 	if (sem.code==GSem_Interger)
// 	{
// 		if (!sem.constraint.empty())
// 		{
// 
// 		}
// 	}
// 
// }


GVarType ParseRecordValue(GElemBase *elem,CRecord *record,int &vInt,float &vFloat,std::string &s)
{
	GVarType gvt=elem->GetVarType();
	GSem &sem=elem->GetSem();
	if ((sem.code==GSem_Unknown)&&(sem.constraint=="DynObjPtr"))
	{
		CClass *clss;
		if (TRUE==elem->GetObjClass(record,&clss))
		{
			if (clss)
			{
				GElem_DynObjPtrBase *elem2=(GElem_DynObjPtrBase*)elem;
				std::unordered_map<std::string,CClass *>::iterator it;
				for (it=elem2->classes.begin();it!=elem2->classes.end();it++)
				{
					if ((*it).second==clss)
					{
						std::unordered_map<std::string,std::string>::iterator it2=elem2->names.find((*it).first);
						if (it2!=elem2->names.end())
						{
							s=(*it2).second;
							return GVT_String;
						}
					}
				}
			}
		}
	}
	if (!elem->GetVar(NULL,NULL))
		gvt=GVT_None;
	switch(gvt)
	{
		case GVT_B:
		{
			BYTE *p;
			elem->GetVar(record,(void**)&p);
			vInt=(int)(*p);
			FormatString(s,"%d",vInt);
			return GVT_S;
		}
		case GVT_F:
		{
			float*p;
			elem->GetVar(record,(void**)&p);
			vFloat=(*p);
			FormatString(s,"%.3f",vFloat);
			return GVT_F;
		}
		case GVT_S:
		case GVT_U:
		{
			int*p;
			elem->GetVar(record,(void**)&p);
			if (sem.code==GSem_RecordID)
			{
				const char *GetRecordName(const char *nameRecords,RecordID id);
				s=GetRecordName(sem.constraint.c_str(),(RecordID)(*p));
				return GVT_String;
			}
			else
			{
				if (sem.code==GSem_StringID)
				{
					s=StrLib_GetStr((StringID)(*p));
					return GVT_String;
				}
				else
				{
					if (sem.code==GSem_AnimTick)
					{
						vFloat=ANIMTICK_TO_SECOND((AnimTick)*p);
						FormatString(s,"%.3f",vFloat);
						return GVT_F;
					}
					else
					{
						vInt=(*p);
						FormatString(s,"%d",vInt);
						return GVT_S;
					}
				}
			}
			break;
		}
	case GVT_String:
		{
			std::string *p;
			elem->GetVar(record,(void**)&p);
			s=*p;
			return GVT_String;
		}
	case GVT_Bx8:
		{
			unsigned __int64*p;
			elem->GetVar(record,(void**)&p);

			s="n/a";
			if (*p)
			{
				if (g_ssGuiLib.pES)
				{
					IProto *proto=g_ssGuiLib.pES->GetProtoLib()->ObtainProto(*p);
					if (proto)
						s=proto->GetFilePath();
				}
			}

			return GVT_String;

		}
	default:
		s="...";
		return GVT_String;
	}

}

BOOL GetRecordName(CRecord* rec, CString& sName)
{
	sName = "";
	if (!rec)
		return TRUE;

	GObjBase* obj = rec->GetGObj();
	GElemBase* elem = obj->GetElems();

	static std::string nm = "Name";
	while (elem)
	{
		if (nm == elem->GetElemName())
		{
			int vInt;
			float vFloat;
			std::string s;

			GVarType gvt = ParseRecordValue(elem, rec, vInt, vFloat, s);
			if (gvt == GVT_String)
				sName = fromMBCS(s.c_str());
			break;
		}
		elem = elem->next;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CRecordSelListRecord
void CRecordSelListRecord::UpdateItem(DWORD idx,const char *s)
{
	CXTPReportRecord::AddItem(new CXTPReportRecordItemText(fromMBCS(s)));
}

void CRecordSelListRecord::UpdateItem(DWORD idx,int v)
{
	CXTPReportRecord::AddItem(new CXTPReportRecordItemInt(v));
}

void CRecordSelListRecord::UpdateItem(DWORD idx,float v)
{
	CXTPReportRecord::AddItem(new CXTPReportRecordItemFloat(v));
}

void CRecordSelListRecord::GetItemMetrics(XTP_REPORTRECORDITEM_DRAWARGS* pDrawArgs, XTP_REPORTRECORDITEM_METRICS* pItemMetrics)
{
	int nIndex = pDrawArgs->pRow->GetIndex();

	if (nIndex % 2)
	{
		pItemMetrics->clrBackground = RGB(245, 245, 245);
	}

}



//////////////////////////////////////////////////////////////////////////
//CRecordSelList


BEGIN_MESSAGE_MAP(CRecordSelList,CXTPReportControl)
	//{{AFX_MSG_MAP(CRecordSelList)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CRecordSelList::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CXTPReportControl::Create(WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL|WS_HSCROLL,rect,pParentWnd,nID))
		return FALSE;

	SetGridStyle(TRUE,xtpReportGridSolid);
	SetGridStyle(FALSE,xtpReportGridSolid);

	EnableToolTips(FALSE);
	AllowEdit(FALSE);

	SetMultipleSelection(FALSE);

	return TRUE;

}


void CRecordSelList::_AddColumns(CRecords *records)
{
	CXTPReportControl::GetReportHeader()->SetAutoColumnSizing(FALSE);

	CClass *clss=_records->GetRecordClass();
	if (clss)
	{
		DWORD idx=0;
		_columnSortIdx = CXTPReportControl::AddColumn(new CXTPReportColumn(idx, _T("序号"), 100, TRUE));
		_columnSortIdx->EnableResize(TRUE);
		idx++;

		CRecord *t=(CRecord *)clss->New();

		GObjBase *gobj=t->GetGObj();

		GElemBase *elem=gobj->GetElems();
		while(elem)
		{
			const char *name=elem->GetEditName();
			if (!name[0])
				name=elem->GetElemName();

			CXTPReportColumn* col = CXTPReportControl::AddColumn(new CXTPReportColumn(idx, fromMBCS(name), 100, TRUE));
			col->EnableResize(TRUE);
			idx++;

			elem=elem->next;
		}


		Safe_Class_Delete(t);
	}


	CXTPReportControl::GetReportHeader()->AllowColumnRemove(FALSE);
	CXTPReportControl::GetReportHeader()->AllowColumnResize(TRUE);
	CXTPReportControl::GetReportHeader()->AllowColumnSort(TRUE);
	if (_columnSortIdx)
		_columnSortIdx->AllowRemove(FALSE);
	EnableToolTips(TRUE);

}

void CRecordSelList::_UpdateRecord(CRecordSelListRecord* r,CRecord *record)
{
	DWORD idx=0;

	FormatString(_s,"%04d",(int)record->GetSortIdx());
	r->UpdateItem(idx++,_s.c_str());

	GObjBase *obj=record->GetGObj();
	GElemBase *elem=obj->GetElems();
	int vInt;
	float vFloat;
	std::string s;
	while(elem)
	{
		GVarType gvt=ParseRecordValue(elem,record,vInt,vFloat,s);
		switch(gvt)
		{
		case GVT_F:
			{
				r->UpdateItem(idx++,vFloat);
				break;
			}
		case GVT_S:
		case GVT_U:
			{
				r->UpdateItem(idx++,vInt);
				break;
			}
		case GVT_String:
			{
				r->UpdateItem(idx++,s.c_str());
				break;
			}
		}
		elem=elem->next;
	}

}

void CRecordSelList::_UpdateRecordSortIdx(CRecordSelListRecord* r,CRecord *record)
{
	FormatString(_s,"%04d",(int)record->GetSortIdx());
	r->UpdateItem(0,_s.c_str());
}

void CRecordSelList::Sel(RecordID idSel)
{
	CXTPReportRows *rows=GetRows();

	GetSelectedRows()->Clear();

	for (int i=0;i<rows->GetCount();i++)
	{
		CXTPReportRow *row=rows->GetAt(i);
		CRecordSelListRecord*r=(CRecordSelListRecord*)row->GetRecord();

		if (r->GetID()==idSel)
		{
			row->SetSelected(TRUE);
			EnsureVisible(row);
			return ;
		}
	}


	return;
}

BOOL CRecordSelList::_IsFilteredOut(CRecord* record)
{
	CRecordDlg* parent = (CRecordDlg*)GetParent();
	if (!parent) return FALSE;

	CString filter = parent->_filter;
	if (filter.IsEmpty()) return FALSE;

	CString sName;
	GetRecordName(record, sName);
	return (sName.Find(filter) < 0);
}

void CRecordSelList::Refresh()
{
	CRecords *records=_records;
	if (!records)
		return;

	ResetContent(FALSE);

	_AddColumns(records);

	//加入新增的
	DWORD c;
	RecordID*buf=records->GetRecords(c);

	std::string s;

	for (int i=0;i<c;i++)
	{
		CRecord *record=records->GetSafeRecord(buf[i]);
		assert(record);
		if (!record || _IsFilteredOut(record)) 
			continue;

		CRecordSelListRecord*r=new CRecordSelListRecord;
		_UpdateRecord(r,record);

		r->SetID(buf[i]);
		AddRecord(r);
	}

	GetColumns()->SetSortColumn(_columnSortIdx,TRUE);

	Populate();

}


RecordID CRecordSelList::GetSingleSel()
{
	CRecords *records=_records;
	if (!records)
		return RecordID_Invalid;

	CXTPReportSelectedRows*rows=GetSelectedRows();
	if (rows->GetCount()!=1)
		return RecordID_Invalid;
	CXTPReportRow *row=rows->GetAt(0);
	CRecordSelListRecord* r=(CRecordSelListRecord*)row->GetRecord();
	return r->GetID();
}



void CRecordSelList::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CXTPReportControl::OnLButtonDblClk(nFlags,point);

	((CRecordDlg *)GetParent())->OnOK();
}


//////////////////////////////////////////////////////////////////////////
//CRecordDlg

CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/)
	: CXTPDialog(IDD_RECORDDLG, pParent)
{
	_idSel=RecordID_Invalid;

	_bShowSelNone=FALSE;
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRecordDlg, CXTPDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_SELNONE, OnBnClickedSelnone)
	ON_CBN_SELCHANGE(ID_FILTERCOMBO, &CRecordDlg::OnCbnSelchangeFilterCombo)
	ON_CBN_EDITCHANGE(ID_FILTERCOMBO, &CRecordDlg::OnCbnEditchangeFilterCombo)
END_MESSAGE_MAP()


BOOL CRecordDlg::Create(CWnd *pParent)
{
	return CDialog::Create(IDD_PROTOSELECTDLG,pParent); 
}

// CRecordDlg 消息处理程序

BOOL CRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 创建过滤组合框
	CRect rcCombo, rcList;
	GET_CONTROL_RECT(this, IDC_RECORD, rcList);
	HIDE_CONTROL(this, IDC_RECORD);

	// 调整位置：组合框在上方，列表在下
	rcCombo = rcList;
	rcCombo.bottom = rcCombo.top + 24; // 高度24
	_comboFilter.Create(WS_CHILD | CBS_DROPDOWN | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL | CBS_NOINTEGRALHEIGHT | ES_LEFT | WS_VISIBLE, rcCombo, this, ID_FILTERCOMBO);

	// 调整列表位置
	rcList.top += 24;
	_list.Create(rcList, this, 1);

	_list.Refresh();
	_list.Sel(_idSel);

	// 初始化过滤器内容
	_InitFilterCombo();

	if (!_bShowSelNone)
		HIDE_CONTROL(this,ID_SELNONE);

	_idSel=RecordID_Invalid;

	return TRUE;  // 除非设置了控件的焦点，否则返回 TRUE
}

void CRecordDlg::OnOK()
{
	_idSel=_list.GetSingleSel();
	CXTPDialog::OnOK();
}

void CRecordDlg::OnCancel()
{
	CXTPDialog::OnCancel();
}

void CRecordDlg::OnBnClickedSelnone()
{
	// TODO: Add your control notification handler code here
	CXTPDialog::OnOK();
}

BOOL CullRecordNameKey(const char* s, std::string& key)
{
	key = "";
	if (s[0] == '-')
	{
		if (s[1] == '-')
		{
			if (s[2] == '(')
			{
				const char* p = s;
				while (*p)
				{
					if ((*p) == '-')
					{
						p++;
						continue;
					}
					key += *p;
					p++;
				}

				RemoveBlank(key);
				return !key.empty();
			}
		}
	}
	return FALSE;
}

void InitFilterCombo(CComboBox &comboFilter, CRecords* records)
{
	comboFilter.ResetContent();


	DWORD c;
	RecordID* buf = records->GetRecords(c);

	std::string s;

	CString sName;
	std::string sKey;

	comboFilter.AddString(_T("[ALL]"));

	for (int i = 0;i < c;i++)
	{
		CRecord* record = records->GetSafeRecord(buf[i]);

		extern BOOL GetRecordName(CRecord * rec, CString & sName);

		GetRecordName(record, sName);

		const char* s = toMBCS( (LPCTSTR)sName);
		if (CullRecordNameKey(s,sKey))
			comboFilter.AddString(fromMBCS(sKey.c_str()));
	}

	if (TRUE)
	{
		CRect rect;
		comboFilter.GetWindowRect(&rect);

		// 调整控件的高度（上部分是固定的，下部分是下拉框的高度）
		int dropHeight = 1000;  // 下拉框的高度（单位：像素）
		comboFilter.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height() + dropHeight, SWP_NOMOVE | SWP_NOZORDER);
	}
}

// 初始化过滤器内容
void CRecordDlg::_InitFilterCombo()
{
	InitFilterCombo(_comboFilter, _list._records);
}

// 处理过滤条件变化
void CRecordDlg::_UpdateFilterCombo()
{
	CString newFilter;
	_comboFilter.GetWindowText(newFilter);
	if (newFilter == _T("[ALL]"))
		newFilter = _T("");
	if (newFilter != _filter)
	{
		_filter = newFilter;
		_list.Refresh(); // 需在CRecordSelList中实现过滤逻辑
		_list.RedrawControl();
	}
}


void CRecordDlg::OnCbnSelchangeFilterCombo()
{
	_UpdateFilterCombo();
}

void CRecordDlg::OnCbnEditchangeFilterCombo()
{
	_UpdateFilterCombo();
}