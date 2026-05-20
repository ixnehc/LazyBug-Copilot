#pragma once

#include "class/class.h"
#include "anim/animdefines.h"

#include <unordered_map>

#include "sheet.h"

class CClass;
class CSheetRow;
class CSheet;
struct SheetRecord
{
	SheetRecord()
	{
		Zero();
	}

	~SheetRecord()
	{
		Clear();		
	}

	virtual CClass *GetClass()=0;
	virtual void Load()=0;

	void Zero()
	{
		_row=NULL;
	}
	void Init(CSheetRow *row);
	void Clear();
	const char *GetSheetPath();

protected:
	void _DumpMissing(const char *nm);
	BOOL _Load(int &v,const char *nm);
	BOOL _Load(float &v,const char *nm);
	BOOL _Load(const char *&v,const char *nm);
	BOOL _Load(std::vector<int>&v,const char *nm);
	BOOL _LoadAnimTick(AnimTick &t,const char *nm);

	CSheetRow *_row;
};

#define SheetRecord_Load(__nm) {if (!_Load(__nm,#__nm)) _DumpMissing(#__nm);}
#define SheetRecord_LoadTo(__var,__nm) {if (!_Load(__var,__nm)) _DumpMissing(__nm);}
#define SheetRecord_LoadAnimTick(__nm) {if (!_LoadAnimTick(__nm,#__nm)) _DumpMissing(#__nm);}


class CClass;
class CSheetRecords
{
public:
	DEFINE_CLASS(CSheetRecords);
	CSheetRecords()
	{
		Zero();
	}
	void Zero()
	{
		_sht=NULL;
		_clssRecord=NULL;
	}
	void Init(CSheet *sht,CClass *clssRecord);
	void Clear();
	SheetRecord *ObtainRecord(DWORD kid);
protected:
	CSheet *_sht;
	CClass *_clssRecord;
	std::unordered_map<DWORD,SheetRecord *>_records;
};

