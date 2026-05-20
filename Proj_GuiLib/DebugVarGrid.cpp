/********************************************************************
	created:	3:4:2009   10:44
	filename: 	d:\IxEngine\Proj_GuiLib\DebugVarGrid.cpp
	author:		chenxi
	
	purpose:	用以显示debug variable值的property grid
*********************************************************************/
#include "stdh.h"

#include <vector>

#include "RichGridFloatItem.h"

#include "DebugVarGrid.h"


#include "WorldSystem/IDebugger.h"


#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include <assert.h>



//////////////////////////////////////////////////////////////////////////
//CDebugVarGrid
BEGIN_MESSAGE_MAP(CDebugVarGrid,CXTPReportControl)
END_MESSAGE_MAP()



BOOL CDebugVarGrid::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	if (!CXTPReportControl::Create(0x50010000,rect,pParentWnd,nID))
		return FALSE;

	if (!_bColumnAdded)
	{
		CXTPReportColumn *column=new CXTPReportColumn(0,_T("Key"),128);
		CXTPReportControl::AddColumn(column);
		column->SetTreeColumn(TRUE);

		CXTPReportControl::AddColumn(new CXTPReportColumn(1,_T("Value"),128));
		_bColumnAdded=TRUE;
	}

	CXTPReportControl::GetReportHeader()->AllowColumnRemove(FALSE);
	CXTPReportControl::GetReportHeader()->AllowColumnSort(FALSE);

	return TRUE;
}

CXTPReportRecord *CDebugVarGrid::_FindChildByName(CXTPReportRecord *record,const char *name)
{
	CXTPReportRecords *records;
	if (record)
		records=record->GetChilds();
	else
		records=GetRecords();
	
	for (int i=0;i<records->GetCount();i++)
	{
		CXTPReportRecord *record=records->GetAt(i);
		CXTPReportRecordItemText*item=(CXTPReportRecordItemText*)record->GetItem(0);
		if(item->GetValue()==name)
			return record;
	}

	return NULL;
}

CXTPReportRecord *CDebugVarGrid::_InsertRecord(CXTPReportRecord *recordParent,const char *key)
{
	CXTPReportRecord *record=new CXTPReportRecord;
	record->AddItem(new CXTPReportRecordItemText(fromMBCS(key)));
	record->AddItem(new CXTPReportRecordItemText(_T("")));

	AddRecordEx(record,recordParent);

	return record;
}



void CDebugVarGrid::_InsertRecord(const char *path,const char *str)
{
	std::vector<std::string>pieces;
	SplitStringBy(".",std::string(path),&pieces);

	if (pieces.size()<=0)
		return;

	CXTPReportRecord *record=NULL;

	for (int i=0;i<pieces.size();i++)
	{
		CXTPReportRecord *recordChild=_FindChildByName(record,pieces[i].c_str());

		if (!recordChild)
			recordChild=_InsertRecord(record,pieces[i].c_str());

		record=recordChild;
	}

	CXTPReportRecordItemText*item=(CXTPReportRecordItemText*)record->GetItem(1);
	item->SetValue(fromMBCS(str));
}

void DebugValueToString(DebugValue *v,std::string &s)
{
	s="";
	switch(v->vtype)
	{
		case DebugValue::String:
		{
			s="\"";
			s=s+v->str+"\"";
			break;
		}
		default:
			s=v->str;
	}

}


void CDebugVarGrid::Add(DebugVarDesc &var)
{
	std::string s;

	for (int i=0;i<var.values.size();i++)
	{
		DebugValue *v=&var.values[i];

		DebugValueToString(v,s);
		_InsertRecord(v->key,s.c_str());
	}

}


void CDebugVarGrid::AddInvalid(const char *varname)
{
	_InsertRecord(varname,"N/A");
}

const char *CDebugVarGrid::GetSelectVarPath()
{
	static std::string s;
	s="";
	CXTPReportSelectedRows* rows=GetSelectedRows();
	POSITION pos;
	pos=rows->GetFirstSelectedRowPosition();
	CXTPReportRow *row=rows->GetNextSelectedRow(pos);
	if (row)
	{
		CXTPReportRecord *record=row->GetRecord();
		std::list<std::string>pieces;
		while(record)
		{
			CXTPReportRecordItemText *item=(CXTPReportRecordItemText *)record->GetItem(0);
			pieces.push_front(std::string(toMBCS((LPCTSTR)item->GetValue())));
			record=record->GetRecords()->GetOwnerRecord();
		}
		LinkStringBy(".",s,&pieces);
	}
	return s.c_str();
}
