/********************************************************************
	created:	2010/7/22   10:37
	file path:	d:\IxEngine\Common\config
	author:		chenxi
	
	purpose:	配置
*********************************************************************/
#include "stdh.h"

#include "config.h"
#include "../stringparser/stringparser.h"

//////////////////////////////////////////////////////////////////////////
//ConfigEntry
void ConfigEntry::Save(CDataPacket &dp)
{
	dp.Data_NextInt()=tp;
	dp.Data_WriteString(str);
	dp.Data_NextInt()=num;
	dp.Data_NextInt()=sem.code;
	dp.Data_WriteString(sem.constraint);
	dp.Data_WriteString(desc);
}

void ConfigEntry::Load(CDataPacket &dp)
{
	tp=(Type)dp.Data_NextInt();
	dp.Data_ReadString(str);
	num=dp.Data_NextInt();
	sem.code=(GSemCode)dp.Data_NextInt();
	dp.Data_ReadString(sem.constraint);
	dp.Data_ReadString(desc);
	ver=1;
	bDirty=FALSE;
}


//////////////////////////////////////////////////////////////////////////
//CConfig

ConfigEntry *CConfig::_Get(const char *entry,ConfigEntry::Type tp)
{
	std::unordered_map<std::string,ConfigEntry*>::iterator it=_entries.find(std::string(entry));
	if (it==_entries.end())
		return NULL;
	if ((*it).second->tp!=tp)
		return NULL;
	return (*it).second;
}
ConfigEntry *CConfig::_Obtain(const char *entry,ConfigEntry::Type tp,BOOL *bExist)
{
	if (bExist)
		(*bExist)=FALSE;
	std::unordered_map<std::string,ConfigEntry*>::iterator it=_entries.find(std::string(entry));
	if (it==_entries.end())
	{
		ConfigEntry *t=_pool.Alloc();
		t->tp=tp;
		_entries[std::string(entry)]=t;
		return t;
	}
	if ((*it).second->tp!=tp)
	{
		//原来的类型和要求的类型不符,我们认为不存在(bExist设为FALSE)
		(*it).second->tp=tp;
		return (*it).second;
	}

	if (bExist)
		(*bExist)=TRUE;
	return (*it).second;
}


void CConfig::SetStr(const char *entry,const char *str)
{
	BOOL bExist;
	ConfigEntry *e=_Obtain(entry,ConfigEntry::String,&bExist);
	if (bExist)
	{
		if (e->str==str)
			return;
	}

	e->str=str;
	e->tp=ConfigEntry::String;
	e->ver++;
	e->bDirty=TRUE;
	_ver++;
}

void CConfig::SetNumber(const char *entry,int num)
{
	BOOL bExist;
	ConfigEntry *e=_Obtain(entry,ConfigEntry::Number,&bExist);
	if (bExist)
	{
		if (e->num==num)
			return;
	}

	e->num=num;
	e->tp=ConfigEntry::Number;
	e->ver++;
	e->bDirty=TRUE;
	_ver++;
}

void CConfig::AssignSem(const char *entry,GSem &sem)
{
	ConfigEntry *e=GetEntry(entry);
	if (e)
	{
		e->sem=sem;
		_ver++;
	}
}

void CConfig::AssignDesc(const char *entry,const char *desc)
{
	ConfigEntry *e=GetEntry(entry);
	if (e)
	{
		e->desc=desc;
		_ver++;
	}
}


void CConfig::NewStr(const char *entry,const char *str)
{
	if (_Get(entry,ConfigEntry::String))
		return;
	SetStr(entry,str);
}

void CConfig::NewNumber(const char *entry,int num)
{
	if (_Get(entry,ConfigEntry::Number))
		return;
	SetNumber(entry,num);
}

ConfigEntry *CConfig::GetEntry(const char *entry)
{
	std::unordered_map<std::string,ConfigEntry*>::iterator it=_entries.find(std::string(entry));
	if (it==_entries.end())
		return NULL;
	return (*it).second;
}

const char *CConfig::GetStr(const char *entry,const char *def)
{
	ConfigEntry *e=_Get(entry,ConfigEntry::String);
	if (e)
		return e->str.c_str();
	return def;
}

int CConfig::GetNumber(const char *entry,int def)
{
	ConfigEntry *e=_Get(entry,ConfigEntry::Number);
	if (e)
		return e->num;
	return def;
}


BOOL CConfig::IsDirty(const char *entry)
{
	ConfigEntry *e=GetEntry(entry);
	if (e)
		return e->bDirty;
	return FALSE;
}

void CConfig::ClearDirty(const char *entry)
{
	ConfigEntry *e=GetEntry(entry);
	if (e)
		e->bDirty=FALSE;
}

void CConfig::ClearAllDirty()
{
	std::unordered_map<std::string,ConfigEntry *>::iterator it;
	for (it=_entries.begin();it!=_entries.end();it++)
		(*it).second->bDirty=FALSE;
}


DWORD CConfig::GetVer(const char *entry)
{
	ConfigEntry *e=GetEntry(entry);
	if (e)
		return e->ver;
	return 0;
}

DWORD CConfig::GetVer()
{
	return _ver;
}

const char **CConfig::EnumEntries(DWORD &c)
{
	_temp.clear();
	_temp.reserve(_entries.size());
	std::unordered_map<std::string,ConfigEntry*>::iterator it;
	for (it=_entries.begin();it!=_entries.end();it++)
		_temp.push_back((*it).first.c_str());

	c=_temp.size();
	return _temp.data();
}

#define CUR_VER 1
void CConfig::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=CUR_VER;
	dp.Data_NextDword()=_entries.size();

	std::unordered_map<std::string,ConfigEntry *>::iterator it;
	for (it=_entries.begin();it!=_entries.end();it++)
	{
		dp.Data_WriteString((std::string &)(*it).first);
		ConfigEntry *e=(*it).second;
		e->Save(dp);
	}
}

void CConfig::Load(CDataPacket &dp)
{
	Clear();

	DWORD ver=dp.Data_NextDword();
	DWORD sz=dp.Data_NextDword();

	std::string s;
	for (int i=0;i<sz;i++)
	{
		dp.Data_ReadString(s);
		ConfigEntry *e=_pool.Alloc();
		e->Load(dp);
		_entries[s]=e;
	}

	_ver=1;
}

void CConfig::CopyFrom(CConfig *cfg)
{
	if (cfg)
	{
		std::vector<BYTE>buf;
		DP_BeginSave(dp,buf);
		cfg->Save(dp);
		DP_EndSave();

		CDataPacket dp;
		dp.SetDataBufferPointer(buf.data());
		Load(dp);
	}
}
