#pragma once


#include <vector>
#include <string>
#include <unordered_map>

#include "../mempool/mempool.h"
#include "../gds/GDefines.h"

struct ConfigEntry
{
	enum Type
	{
		None,
		Number,
		String,
	};

	ConfigEntry()
	{
		tp=None;
		num=0;
		ver=0;
		bDirty=FALSE;
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	Type tp;
	std::string str;
	int num;
	GSem sem;
	std::string desc;
	DWORD ver;
	BOOL bDirty;
};

class CDataPacket;
class CConfig
{
public:
	CConfig()
	{
		_ver=0;
	}
	~CConfig()
	{
		Clear();
	}
	void Clear()
	{
		_entries.clear();
		_pool.Reset(FALSE);
		_ver=0;
		_temp.clear();
	}

	void NewStr(const char *entry,const char *str);//如果entry已存在,什么事情也不做
	void NewNumber(const char *entry,int num);//如果entry已存在,什么事情也不做

	void SetStr(const char *entry,const char *str);//如果entry不存在,新建一个
	void SetNumber(const char *entry,int num);//如果entry不存在,新建一个

	void AssignSem(const char *entry,GSem &sem);//如果entry不存在,什么事情也不做
	void AssignDesc(const char *entry,const char *desc);//如果entry不存在,什么事情也不做

	ConfigEntry *GetEntry(const char *entry);
	const char *GetStr(const char *entry,const char *def="");//如果entry不存在,返回def
	int GetNumber(const char *entry,int def=0);//如果entry不存在,返回def

	BOOL IsDirty(const char *entry);//检查某个entry的数值有没有被修改过,如果entry不存在,返回FALSE
	void ClearDirty(const char *entry);//清除某个entry的数值的修改标志
	void ClearAllDirty();//清除所有entries的数值的修改标志

	DWORD GetVer(const char *entry);//返回某个entry的版本号。如果entry不存在,返回0
	DWORD GetVer();//返回整个config对象的版本号

	const char **EnumEntries(DWORD &c);//返回临时指针,不要保留

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);//注意载入后的config所有entry都不是dirty的

	void CopyFrom(CConfig *cfg);

protected:
	ConfigEntry *_Get(const char *entry,ConfigEntry::Type tp);
	ConfigEntry *_Obtain(const char *entry,ConfigEntry::Type tp,BOOL *bExist=NULL);
	std::unordered_map<std::string,ConfigEntry *> _entries;
	int _ver;

	CMemPool<ConfigEntry> _pool;
	std::vector<const char *>_temp;
};

