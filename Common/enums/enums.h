/********************************************************************
	created:	22:5:2010   9:47
	file path:	d:\IxEngine\Common\enums
	author:		cxi
	
	purpose:	help class 用来获取enum的名字的字符串
*********************************************************************/
#pragma once

#include "../stringparser/stringparser.h"


class CEnums
{
public:
	CEnums()
	{
	}

	struct EnumEntry
	{
		EnumEntry(int v_,const char *name_,const char *desc_="")
		{
			v=v_;
			name=name_;
			desc=desc_;
		}
		int v;
		const char *name;
		const char *desc;
	};

	DWORD GetCount()
	{
		_EnsureLoad();
		return _entries().size();
	}
	int GetValue(DWORD idx)
	{
		_EnsureLoad();
		return _entries()[idx].v;
	}
	const char *GetName(DWORD idx)
	{
		_EnsureLoad();
		if (idx>=_entries().size())
			return "";
		return _entries()[idx].name;
	}
	const char *GetDesc(DWORD idx)
	{
		_EnsureLoad();
		if (idx>=_entries().size())
			return "";
		return _entries()[idx].desc;
	}
	const char *FindName(int value)
	{
		_EnsureLoad();
		return GetName(FindIndex(value));
	}
	int FindValue(const char *name)
	{
		_EnsureLoad();
		for (int i=0;i<_entries().size();i++)
		{
			if (strcmp(name,_entries()[i].name)==0)
				return _entries()[i].v;
		}
		return -1;
	}
	int FindIndex(int value)
	{
		_EnsureLoad();
		if (value>=_indices().size())
			return -1;
		return _indices()[value];
	}
	const char *GetGSemStr()
	{
		_EnsureLoad();
		return _strGSem().c_str();
	}

	virtual const char *GetName()=0;//返回这个枚举的名称

protected:

	void _EnsureLoad()
	{
		if (!_bLoaded())
		{
			_Load();
			_bLoaded()=1;
			std::string s;
			for (int i=0;i<_entries().size();i++)
			{
				int v=GetValue(i);
				const char *desc=GetDesc(i);
				if (!desc[0])
					desc=GetName(i);
				if (i==0)
					FormatString(s,"%s:%d",desc,v);
				else
					FormatString(s,",%s:%d",desc,v);
				if (v>=_indices().size())
				{
					int sz=_indices().size();
					_indices().resize(v+1);
					for (int j=sz;j<v+1;j++)
						_indices()[j]=-1;//初始值
				}
				_indices()[v]=i;
				_strGSem()+=s;
			}
		}
	}
	virtual void _Load()=0;
	virtual BOOL &_bLoaded()=0;
	virtual std::vector<EnumEntry>&_entries()=0;
	virtual std::vector<int>&_indices()=0;
	virtual std::string &_strGSem()=0;

};

#define ENUM_ENTRY(name) _entries().push_back(EnumEntry(name,#name+__lenPrefix));

//D代表desc
#define ENUM_ENTRY_D(name,desc) _entries().push_back(EnumEntry(name,#name+__lenPrefix,desc));

#define BEGIN_ENUMS(ename,prefix)																	\
class enumclss_##ename:public CEnums															\
{																																\
public:																														\
	virtual const char *GetName()	{		return #ename;	}									\
protected:																												\
	virtual BOOL &_bLoaded()																					\
		{		static BOOL bLoaded=FALSE;		return bLoaded;	}							\
	virtual std::vector<EnumEntry>&_entries()														\
		{		static std::vector<EnumEntry> t;		return t;	}									\
	virtual std::vector<int>&_indices()																	\
		{		static std::vector<int> t;		return t;	}												\
	virtual std::string &_strGSem()																			\
		{		static std::string s;		return s;	}															\
	virtual void _Load()																								\
	{																															\
		int __lenPrefix=strlen(#prefix);

#define END_ENUMS()																							\
	}																															\
};

#define Enum_Class(ename) enumclss_##ename

#define Enums_GetCount(ename)	enumclss_##ename().GetCount()
#define Enums_GetValue(ename,idx) (ename)enumclss_##ename().GetValue(idx)
#define Enums_GetName(ename,idx) enumclss_##ename().GetName(idx)
#define Enums_FindValue(ename,name) (ename)enumclss_##ename().FindValue(name)
#define Enums_FindName(ename,value) enumclss_##ename().FindName(value)
#define Enums_GetGSemStr(ename)	enumclss_##ename().GetGSemStr()
#define Enums_FindIndex(ename,value) enumclss_##ename().FindIndex(value)

