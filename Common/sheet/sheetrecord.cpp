/********************************************************************
	created:	2011/8/24   10:41
	file path:	e:\IxEngine\Proj_Client
	author:		chenxi
	
	purpose:	游戏表格record
*********************************************************************/
#include "stdh.h"
#include "SheetRecord.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//SheetRecord

void SheetRecord::Init(CSheetRow *row)
{
	SAFE_REPLACE(_row,row);
}

void SheetRecord::Clear()
{
	SAFE_RELEASE(_row);
}

const char *SheetRecord::GetSheetPath()
{
	return _row->GetOwner()->GetPath();
}


void SheetRecord::_DumpMissing(const char *nm)
{
	LOG_DUMP_2P("Client",Log_Error,"无法在表格文件(\"%s\")中找到名为\"%s\"的栏位!",GetSheetPath(),nm);
}

BOOL SheetRecord::_Load(int &v,const char *nm)
{
	return _row->Find(v,nm);
}

BOOL SheetRecord::_Load(float &v,const char *nm)
{
	return _row->Find(v,nm);
}

BOOL SheetRecord::_LoadAnimTick(AnimTick &t,const char *nm)
{
	float v;
	if (!_Load(v,nm))
		return FALSE;
	t=ANIMTICK_FROM_SECOND(v);
	return TRUE;
}


BOOL SheetRecord::_Load(const char *&v,const char *nm)
{
	return _row->Find(v,nm);
}

BOOL SheetRecord::_Load(std::vector<int>&v,const char *nm)
{
	ShtColID col=_row->FindColumn(nm);
	if (col==ColumnID_Invalid)
		return FALSE;
	v.clear();
	int idx=0;
	while(1)
	{
		SheetCell *cell=_row->GetCell(col,idx);
		idx++;
		if (!cell)
			break;
		if (!cell->s[0])
			continue;
		v.push_back((int)cell->v);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CSheetRecords


//////////////////////////////////////////////////////////////////////////
//CSheetRecords
void CSheetRecords::Init(CSheet *sht,CClass *clssRecord)
{
	SAFE_REPLACE(_sht,sht);
	_clssRecord=clssRecord;
}

void CSheetRecords::Clear()
{
	std::unordered_map<DWORD,SheetRecord *>::iterator it;
	for (it=_records.begin();it!=_records.end();it++)
	{
		SheetRecord *p=(*it).second;
		Safe_Class_Delete(p);
	}
	_records.clear();
	SAFE_RELEASE(_sht);
	Zero();
}

SheetRecord *CSheetRecords::ObtainRecord(DWORD kid)
{
	SheetRecord *record=NULL;

	std::unordered_map<DWORD,SheetRecord *>::iterator it=_records.find(kid);
	if (it!=_records.end())
	{
		record=(*it).second;
		return record;
	}

	CSheetRow *row=_sht->ObtainRow((int)kid);
	if (row)
	{
		record=(SheetRecord *)_clssRecord->New();
		record->Init(row);
		record->Load();
		SAFE_RELEASE(row);
	}
	else
	{
		LOG_DUMP_2P("GameSheets",Log_Error,"无法在表格文件(\"%s\")中找到ID(%d)对应的行!",_sht->GetPath(),(int)kid);
	}

	_records[kid]=record;

	return record;
}

