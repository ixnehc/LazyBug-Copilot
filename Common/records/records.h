#pragma once

#include "class/class.h"

class CClass;
struct GObjBase;

#include "recordsdefine.h"

class CRecord
{
public:
	CRecord()
	{
		_id=RecordID_Invalid;
		_idxSort=0;
		_ver=1;
		_bCloned=0;
	}
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;

	IMPLEMENT_REFCOUNT_OVERRIDE;
	virtual void OnRelease()
	{
		if (_bCloned)
		{
			Class_Delete(this);
		}
	}

	BOOL IsCloned()	{		return _bCloned;	}
	CRecord *Clone();//注意Clone的指针带一个引用计数
	BOOL MakeDeltaOnCloned(CRecord *org);
	void ApplyDelta(BYTE *data,DWORD c);
	BYTE *GetDelta(DWORD &c);


	RecordID GetID()	{		return _id;	}
	RecordSimpleID GetSimpleID()	{		return (RecordSimpleID)_id;	}

	WORD GetSortIdx()	{		return _idxSort;	}
	WORD GetVer()	{		return _ver;	}
	void IncVer()	
	{		
		_ver++; if (_ver==0) _ver++;	
	}

protected:
	RecordID _id;
	WORD _idxSort;//序号,注意序号的修改,不会影响_ver
	WORD _ver:15;
	WORD _bCloned:1;

	std::vector<BYTE> _bufDelta;

	friend class CRecords;

};

struct RecordEntry
{
	RecordEntry()
	{
		record=NULL;
	}
	void Clear();
	CRecord*record;
};

class CDataPacket;
struct GElemBase;
class CRecords
{
public:
	DEFINE_CLASS(CRecords);
	CRecords()
	{
		Zero();
		_clssRecord=NULL;
		_elemName=NULL;
	}
	~CRecords()
	{
		Clear();
	}

	void Zero()
	{
		_seedSerial=1;
		_ver=1;
		_empty=NULL;
	}
	void Init(CClass *clss);
	void Clear();

	DWORD GetVer()	{		return _ver;	}
	void IncVer()	{		_ver++; if (_ver==0) _ver++;	}

	CClass *GetRecordClass()	{		return _clssRecord;	}

	const char *GetName(RecordID id);
	GElemBase *GetNameElem()	{		return _elemName;	}
	GElemBase *FindElem(const char *str)	;


	RecordID NewRecord();
	RecordID InsertRecord(RecordID id);
	void MoveRecord(RecordID id1,RecordID id2);//将id1移到id2的前面,和id2相邻,但在它前面
	void MoveRecordToTail(RecordID id1);//将id1移到末尾
	BOOL RemoveRecord(RecordID id);
	CRecord*GetRecord(RecordID id);//不进行校验
	CRecord*GetSafeRecord(RecordID id);//会进行校验
	CRecord*GetEmptyRecord();//返回一个空的Record
	BOOL SetRecord(RecordID id,CRecord *record);//注意,设进来的指针将由CRecords管理,外部不能再使用这个指针
	RecordID FindRecord(const char *name);

	RecordID*GetRecords(DWORD &c);

	BOOL MakeDelta(CRecord *rec,CRecord *recOrg);

	BOOL Save(CDataPacket *dp);
	BOOL Load(CDataPacket *dp);


protected:

	WORD _NewSerial()	
	{		
		_seedSerial++;
		if (_seedSerial==0) 
			_seedSerial++;
		return _seedSerial;	
	}

	WORD _seedSerial;

	DWORD _ver;

	std::vector<RecordEntry> _entries;

	CClass *_clssRecord;
	GElemBase *_elemName;

	std::vector<RecordID> _temp;

	CRecord *_empty;




};