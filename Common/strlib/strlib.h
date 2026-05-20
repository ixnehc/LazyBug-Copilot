
#pragma once

#include "strlibdefines.h"

#include <vector>
#include <string>



class CDataPacket;
struct DynString;
struct DynGroup;
class CStrLib
{
public:
	struct EntryOld
	{
		DWORD bDyn:1;
		DWORD iSerial:15;//这个值如果为0,表示这个entry为空
		DWORD idx:16;
	};

	struct Entry
	{
		DWORD bDyn:1;
		DWORD iSerial:16;//这个值如果为0,表示这个entry为空
		DWORD bGrp:1;
		DWORD bDynGrp:1;
		DWORD idxGrp:13;
		DWORD idx;
	};


	struct Group
	{
		//[start,end)
		DWORD start;
		DWORD end;
	};

	CStrLib();
	BOOL Init();
	void Clear();

	//用于访问的接口
	const char *GetStr(StringID id);
	StringID *EnumGroupSubs(StringID idGrp,DWORD &count);

	//下面这些接口函数主要用于编辑
	BOOL IsValid(StringID id);
	void SetPath(const char *path)	{		_path=path;	}
	const char *GetPath()	{		return _path.c_str();	}
	StringID AddStr(const char *str);
	StringID AddStr(DWORD iCategory,const char *str);
	BOOL RemoveStr(StringID id);
	BOOL ModifyStr(StringID id,const char *str);
	BOOL IsGroup(StringID idGrp);
	BOOL SetGroup(StringID id,BOOL bGroup);
	BOOL AddGroupSub(StringID idGrp,StringID id);
	BOOL RemoveGroupSub(StringID idGrp,StringID id);
	BOOL IsGroupSub(StringID idGrp,StringID id);
	StringID FindGroup(const char *str);//找第一个名为str的group的id
	StringID FindGroup(DWORD iCategory,const char *str);//找第一个名为str的group的id
	StringID FindStr(const char *str,const char *strGrp);//在一个group中找第一个名字为str的字符串id,这个函数采用遍历搜索的方式,比较慢
	StringID FindStr(DWORD iCategory,const char *str,const char *strGrp);//在一个group中找第一个名字为str的字符串id,这个函数采用遍历搜索的方式,比较慢
	StringID *Enum(DWORD &count);
	StringID *Enum(DWORD iCategory,DWORD &count);
	StringID *EnumGroup(DWORD iCategory,DWORD &count);
	StringID *EnumGroup(DWORD &count);
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	BOOL IsModified()	{		return _bModified;	}
	void ClearModified()	{		_bModified=FALSE;	}
	DWORD GetModifyVer()	{		return _verModify;	}
	void Repair();


protected:

	void _ClearContent();
	DWORD _NewSerial();
	Entry *_GetValidEntry(StringID id);

	void _ConvertToDynGrp(Entry *entry);
	StringID *_EnumGroupSubs(Entry *entry,DWORD &count);

	const char *_GetStr(Entry*entry);

	void _Save(CDataPacket &dp);
	void _Load(CDataPacket &dp);

	BOOL _bInit;

	std::string _path;

	std::vector<Entry>_entries[STRLIB_MAX_CATEGORY];
	std::vector<char>_buf;
	std::vector<DynString*>_bufDyn;
	std::vector<Group> _bufGrp;
	std::vector<DynGroup *> _bufDynGrp;
	std::vector<StringID >_bufSub;

	DWORD _seedSerial;

	BOOL _bModified;
	DWORD _verModify;

	std::vector<StringID>_temp;//for EnumXXX(..);
	BOOL _bGrpCache;
	std::vector<StringID>_cacheGrp[STRLIB_MAX_CATEGORY];//for EnumGroup(..)


};

extern CStrLib *StrLib_Get();
extern void StrLib_Set(CStrLib *strlib);
extern const char *StrLib_GetStr(StringID id);
extern StringID *StrLib_EnumGroupSubs(StringID idGrp,DWORD &count);
