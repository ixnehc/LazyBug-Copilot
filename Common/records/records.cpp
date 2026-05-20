
#include "stdh.h"


#include "records.h"
#include "stringparser/stringparser.h"
#include "Log/LogDump.h"

#include "commondefines/general_stl.h"

#include "datapacket/DataPacket.h"

#include "class/class.h"
#include "gds/GObj.h"

#include <assert.h>

//////////////////////////////////////////////////////////////////////////
//RecordEntry
void RecordEntry::Clear()
{
	Safe_Class_Delete(record);
}

//////////////////////////////////////////////////////////////////////////
//CRecord
CRecord *CRecord::Clone()
{
	CRecord *rec=(CRecord *)GetClass()->New();
	rec->GetGObj()->Copy(GetGObj());

	rec->_id=_id;
	rec->_idxSort=_idxSort;
	rec->_ver=_ver;

	rec->_bCloned=1;

	rec->AddRef();
	return rec;
}

//这个函数现在很慢,需要好好优化一下
BOOL CRecord::MakeDeltaOnCloned(CRecord *recOrg)
{
	VEC_EMPTY(BYTE,_bufDelta);

	if (!_bCloned)
		return FALSE;

	if (recOrg->GetID()!=_id)
		return FALSE;

	CDataPacket dp;
	if (!GetGObj()->SaveDelta(dp,recOrg->GetGObj()))
		return FALSE;
	_bufDelta.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(&(_bufDelta)[0]);
	GetGObj()->SaveDelta(dp,recOrg->GetGObj());

	return TRUE;
}

void CRecord::ApplyDelta(BYTE *data,DWORD c)
{
	if(!_bCloned)
		return;

	CDataPacket dp;
	dp.SetDataBufferPointer(data);

	GetGObj()->LoadDelta(dp,NULL);
}


BYTE *CRecord::GetDelta(DWORD &c)
{
	c=0;
	if (_bufDelta.empty())
		return NULL;
	c=_bufDelta.size();
	return _bufDelta.data();
}



//////////////////////////////////////////////////////////////////////////
//CRecords
void CRecords::Init(CClass *clss)
{
	_clssRecord=clss;

	//找到名为Name的那个Elem
	CRecord *t=(CRecord *)_clssRecord->New();
	GObjBase *gobj=t->GetGObj();
	GElemBase *elem=gobj->GetElems();

	std::string s="Name";
	while(elem)
	{
		if (s==elem->GetElemName())
		{
			if (elem->GetVarType()==GVT_String)
				break;
			elem=NULL;
			break;
		}
		elem=elem->next;
	}
	_elemName=elem;

	Safe_Class_Delete(t);

	RecordEntry entry;
	_entries.push_back(entry);

}

GElemBase *CRecords::FindElem(const char *str)
{
	CRecord *t=(CRecord *)_clssRecord->New();
	GObjBase *gobj=t->GetGObj();
	GElemBase *elem=gobj->GetElems();

	while(elem)
	{
		if (strcmp(str,elem->GetElemName())==0)
			break;
		elem=elem->next;
	}
	Safe_Class_Delete(t);

	return elem;
}



void CRecords::Clear()
{
	for (int i=0;i<_entries.size();i++)
		_entries[i].Clear();
	_entries.clear();

	Safe_Class_Delete(_empty);

	Zero();
}

const char *CRecords::GetName(RecordID id)
{
	if (!_elemName)
		return "";
	CRecord *record=GetSafeRecord(id);
	if (!record)
		return "";

	std::string *name;
	_elemName->GetVar(record,(void**)&name);
	return name->c_str();
}


RecordID CRecords::NewRecord()
{
	if (!_clssRecord)
		return RecordID_Invalid;

	assert(_entries.size()>0);

	//找一个空位置
	RecordEntry *entry=NULL;
	DWORD idx=0;
	for (int i=1;i<_entries.size();i++)
	{
		if (!_entries[i].record)
		{
			entry=&_entries[i];
			idx=i;
			break;
		}
	}
	if (!entry)
	{
		if (_entries.size()>=0xffff)
			return RecordID_Invalid;//放不下了
		_entries.resize(_entries.size()+1);
		entry=&_entries[_entries.size()-1];
		idx=_entries.size()-1;
	}

	WORD idxSort=0;
	if (TRUE)
	{
		for (int i=0;i<_entries.size();i++)
		{
			if (!_entries[i].record)
				continue;
			if (_entries[i].record->_idxSort>idxSort)
				idxSort=_entries[i].record->_idxSort;
		}
		idxSort++;
	}

	entry->record=(CRecord*)_clssRecord->New();
	entry->record->_idxSort=idxSort;
	DWORD serial=(DWORD)_NewSerial();
	entry->record->_id=(serial<<16)|idx;

	IncVer();

	return entry->record->_id;
}

RecordID CRecords::InsertRecord(RecordID id)
{
	if (!_clssRecord)
		return RecordID_Invalid;

	CRecord *record=GetSafeRecord(id);
	if (!record)
		return RecordID_Invalid;

	DWORD idxSort=record->GetSortIdx();

	RecordID idNew=NewRecord();
	record=GetSafeRecord(idNew);

	if (TRUE)
	{
		for (int i=0;i<_entries.size();i++)
		{
			if (!_entries[i].record)
				continue;
			if (_entries[i].record->_idxSort>=idxSort)
				_entries[i].record->_idxSort++;
		}
	}

	record->_idxSort=(WORD)idxSort;

	return idNew;

}


BOOL CRecords::RemoveRecord(RecordID id)
{
	DWORD idx=id&0xffff;
	if (idx>=_entries.size())
		return FALSE;

	if (!_entries[idx].record)
		return FALSE;
	if (id!=_entries[idx].record->_id)
		return FALSE;

	WORD idxSort=_entries[idx].record->GetSortIdx();
	_entries[idx].Clear();

	for (int i=0;i<_entries.size();i++)
	{
		if (_entries[i].record)
		{
			if (_entries[i].record->_idxSort>idxSort)
				_entries[i].record->_idxSort--;
		}
	}

	IncVer();

	return TRUE;
}

CRecord*CRecords::GetRecord(RecordID id)
{
	DWORD idx=id&0xffff;
	if (idx>=_entries.size())
		return NULL;
	return _entries[idx].record;
}


CRecord*CRecords::GetSafeRecord(RecordID id)
{
	DWORD idx=id&0xffff;
	if (idx>=_entries.size())
		return NULL;
	if (!_entries[idx].record)
		return NULL;
	if (id!=_entries[idx].record->_id)
		return NULL;
	return _entries[idx].record;
}

RecordID *CRecords::GetRecords(DWORD &c)
{
	_temp.clear();
	_temp.reserve(_entries.size());
	for (int i=0;i<_entries.size();i++)
	{
		if (!_entries[i].record)
			continue;
		if (_entries[i].record->_id==RecordID_Invalid)
			continue;
		_temp.push_back(_entries[i].record->_id);
	}
	c=_temp.size();
	return _temp.data();
}

BOOL CRecords::SetRecord(RecordID id,CRecord *record)
{
	DWORD idx=id&0xffff;
	if (idx>=_entries.size())
		return NULL;
	if (!_entries[idx].record)
		return NULL;
	if (id!=_entries[idx].record->_id)
		return NULL;

	CRecord *recordOld=_entries[idx].record;
	record->_idxSort=recordOld->_idxSort;
	record->_ver=recordOld->_ver;
	record->_id=recordOld->_id;

	Safe_Class_Delete(recordOld);

	_entries[idx].record=record;
	record->IncVer();

	IncVer();

	return TRUE;
}

void CRecords::MoveRecord(RecordID id1,RecordID id2)
{
	if (id1==id2)
		return;
	CRecord *record1,*record2;
	record1=GetSafeRecord(id1);
	record2=GetSafeRecord(id2);
	if ((!record1)||(!record2))
		return;

	int idx1=record1->GetSortIdx();
	int idx2=record2->GetSortIdx();

	if (idx1<idx2)
	{
		for (int i=0;i<_entries.size();i++)
		{
			if (!_entries[i].record)
				continue;
			CRecord *record=_entries[i].record;
			if (record->GetSortIdx()<idx1)
				continue;
			if (record->GetSortIdx()>=idx2)
				continue;
			record->_idxSort--;
		}
		record1->_idxSort=idx2-1;
	}
	else
	{
		for (int i=0;i<_entries.size();i++)
		{
			if (!_entries[i].record)
				continue;
			CRecord *record=_entries[i].record;
			if (record->GetSortIdx()<=idx2)
				continue;
			if (record->GetSortIdx()>=idx1)
				continue;
			record->_idxSort++;
		}
		record1->_idxSort=idx2;
		record2->_idxSort=idx2+1;
	}

	IncVer();
}


void CRecords::MoveRecordToTail(RecordID id)
{
	CRecord *record;
	record=GetSafeRecord(id);
	if (!record)
		return;

	int idxSort=record->GetSortIdx();
	int max=-1;

	for (int i=0;i<_entries.size();i++)
	{
		if (!_entries[i].record)
			continue;
		CRecord *record2=_entries[i].record;
		if (record2->_idxSort<=idxSort)
			continue;
		if (record2->_idxSort>max)
			max=record2->_idxSort;
		record2->_idxSort--;
	}

	if (max==-1)
		return;

	record->_idxSort=max;

	IncVer();
}


RecordID CRecords::FindRecord(const char *name)
{
	return RecordID_Invalid;
}


BOOL CRecords::Save(CDataPacket *dp)
{

	DWORD ver=1;
	dp->Data_NextDword()=ver;

	if (_clssRecord)
		dp->Data_WriteString(_clssRecord->GetName());
	else
		dp->Data_WriteString("");
	dp->Data_WriteSimple(_seedSerial);
	dp->Data_WriteSimple(_ver);

	if (_clssRecord)
	{
		dp->Data_NextDword()=_entries.size();
		for (int i=0;i<_entries.size();i++)
		{
			RecordEntry *entry=&_entries[i];
			if (!entry->record)
				dp->Data_NextByte()=0;
			else
			{
				dp->Data_NextByte()=1;
				dp->Data_WriteSimple(entry->record->_id);
				dp->Data_WriteSimple(entry->record->_idxSort);
				dp->Data_NextWord()=entry->record->_ver;
				SaveGObj(*dp,entry->record->GetGObj());
			}
		}
	}
	else
		dp->Data_NextDword()=0;

	return TRUE;
}

BOOL CRecords::Load(CDataPacket *dp)
{
	if (!_clssRecord)
		return FALSE;

	Clear();

	DWORD ver=dp->Data_NextDword();

	const char *nameClss;
	dp->Data_ReadString(nameClss);
	if (nameClss[0])
	{
		if (!_clssRecord->CheckName(nameClss))
			return FALSE;
	}
	dp->Data_ReadSimple(_seedSerial);
	dp->Data_ReadSimple(_ver);

	_entries.resize(dp->Data_NextDword());
	if (nameClss[0])
	{
		for (int i=0;i<_entries.size();i++)
		{
			RecordEntry *entry=&_entries[i];
			if (dp->Data_NextByte()==1)
			{
				entry->record=(CRecord*)_clssRecord->New();
				dp->Data_ReadSimple(entry->record->_id);
				dp->Data_ReadSimple(entry->record->_idxSort);
				entry->record->_ver=dp->Data_NextWord();
				LoadGObj(*dp,entry->record->GetGObj(),NULL);
			}
		}
	}

	if (_entries.size()<=0)
	{
		RecordEntry entry;
		_entries.push_back(entry);
	}

	return TRUE;
}


CRecord*CRecords::GetEmptyRecord()
{
	if (!_empty)
		_empty=(CRecord*)_clssRecord->New();
	return _empty;
}

