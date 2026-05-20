/********************************************************************
	created:	2010/5/27   15:45
	file path:	d:\IxEngine\Common\strlib
	author:		chenxi
	
	purpose:	string lib
*********************************************************************/

#include "stdh.h"
#include "../commondefines/general_stl.h"

#include "../class/class.h"

#include "strlib.h"
#include "../datapacket/DataPacket.h"

CStrLib *g_strlib=NULL;

CStrLib *StrLib_Get()
{
	return g_strlib;
}

void StrLib_Set(CStrLib *strlib)
{
	g_strlib=strlib;
}

const char *StrLib_GetStr(StringID id)
{
	if (id==StringID_Invalid)
		return "[null]";
	return g_strlib?g_strlib->GetStr(id):"StrLib未被初始化";
}

StringID *StrLib_EnumGroupSubs(StringID idGrp,DWORD &count)
{
	if (g_strlib)
		return g_strlib->EnumGroupSubs(idGrp,count);
	count=0;
	return NULL;
}




#define StringID_Make(iCategory,iEntry,iSerial)  (((DWORD)(iSerial))<<16)|(((DWORD)(iEntry))&0x3fff)|((((DWORD)(iCategory))<<14)&0xc000)
#define StringID_Serial(id) (((id)>>16)&0xffff)
#define StringID_EntryIdx(id) ((id)&0x3fff)
#define StringID_Category(id) (((id)>>14)&0x3)

struct DynString:public std::string
{
	DEFINE_CLASS(DynString);
};

struct DynGroup
{
	DEFINE_CLASS(DynGroup)
	std::vector<StringID> subs;
};




  
CStrLib::CStrLib()
{
	_bInit=FALSE;
	_seedSerial=1;
	_bModified=FALSE;
	_verModify=0;
	_bGrpCache=FALSE;
}

BOOL CStrLib::Init()
{
	return TRUE;
}

void CStrLib::_ClearContent()
{
	for (int i=0;i<STRLIB_MAX_CATEGORY;i++)
		_entries[i].clear();
	_buf.clear();
	_bufGrp.clear();
	_bufSub.clear();
	for (int i=0;i<_bufDyn.size();i++)
		Safe_Class_Delete(_bufDyn[i]);
	for (int i=0;i<_bufDynGrp.size();i++)
		Safe_Class_Delete(_bufDynGrp[i]);

	_bufDyn.clear();
	_bufDynGrp.clear();
}

void CStrLib::Clear()
{
	_ClearContent();
	_path="";
	_bInit=FALSE;
	_bModified=FALSE;
	_bGrpCache=FALSE;


}

DWORD CStrLib::_NewSerial()
{
	_seedSerial++;
	if (_seedSerial>0x7fff)
		_seedSerial=1;
	return _seedSerial;
}

CStrLib::Entry *CStrLib::_GetValidEntry(StringID id)
{
	DWORD iEntry=StringID_EntryIdx(id);
	DWORD iSerial=StringID_Serial(id);
	DWORD iCategory=StringID_Category(id);
	if (iEntry>=_entries[iCategory].size())
		return NULL;
	Entry*entry=&_entries[iCategory][iEntry];
	if ((entry->iSerial!=iSerial)||(entry->iSerial==0))
		return NULL;
	return entry;
}

//假定这个entry不是空的
const char *CStrLib::_GetStr(Entry *entry)
{
	if (!entry->bDyn)
		return (const char *)&_buf[entry->idx];
	else
		return _bufDyn[entry->idx]->c_str();
}


BOOL CStrLib::IsValid(StringID id)
{
	if (id==StringID_Invalid)
		return FALSE;
	Entry *entry=_GetValidEntry(id);
	if (!entry)
		return FALSE;
	return TRUE;
}


//interfaces
const char *CStrLib::GetStr(StringID id)
{
	if (id==StringID_Invalid)
		return "";
	Entry *entry=_GetValidEntry(id);
	if (!entry)
		return "[StringLib--无效字符串ID]";
	return _GetStr(entry);
}

StringID CStrLib::AddStr(DWORD iCategory,const char *str)
{
	if (iCategory>=STRLIB_MAX_CATEGORY)
		return StringID_Invalid;
	//先找一个空的entry
	int iEntry=-1;
	for (int i=0;i<_entries[iCategory].size();i++)
	{
		if (_entries[iCategory][i].iSerial==0)
		{
			iEntry=i;
			break;
		}
	}

	if (iEntry==-1)
	{
		if (_entries[iCategory].size()>0x3fff)
			return StringID_Invalid;//已满
		iEntry=_entries[iCategory].size();
		_entries[iCategory].resize(iEntry+1);
	}

	_entries[iCategory][iEntry].iSerial=_NewSerial();

	DynString *t=Class_New2(DynString);
	(*(std::string*)t)=str;
	_bufDyn.push_back(t);

	_entries[iCategory][iEntry].idx=_bufDyn.size()-1;
	_entries[iCategory][iEntry].bDyn=1;
	_entries[iCategory][iEntry].bGrp=0;
	_entries[iCategory][iEntry].bDynGrp=0;
	_entries[iCategory][iEntry].idxGrp=0;

	DWORD iSerial=_entries[iCategory][iEntry].iSerial;
	_bModified=TRUE;
	_verModify++;
	return StringID_Make(iCategory,iEntry,iSerial);
}

StringID CStrLib::AddStr(const char *str)
{
	return AddStr(STRLIB_CATEGORY_DEFAULT,str);
}

BOOL CStrLib::RemoveStr(StringID id)
{
	//先从group中删除自己
	if (TRUE)
	{
		DWORD c;
		StringID *grps=EnumGroup(c);
		for (int i=0;i<c;i++)
			RemoveGroupSub(grps[i],id);
	}

	Entry *entry=_GetValidEntry(id);
	if (!entry)
		return FALSE;

	entry->iSerial=0;//置为空
	entry->bDyn=0;
	entry->idx=0;
	_bGrpCache=FALSE;//清除group的cache


	_bModified=TRUE;
	_verModify++;
	return TRUE;
}

BOOL CStrLib::ModifyStr(StringID id,const char *str)
{
	Entry *entry=_GetValidEntry(id);
	if (!entry)
		return FALSE;

	if (!entry->bDyn)
	{
		DynString *t=Class_New2(DynString);
		(*(std::string*)t)=str;
		_bufDyn.push_back(t);
		entry->idx=_bufDyn.size()-1;
		entry->bDyn=1;
	}
	else
	{
		(*(std::string*)_bufDyn[entry->idx])=str;
	}
	_bGrpCache=FALSE;//清除group的cache

	_bModified=TRUE;
	_verModify++;
	return TRUE;
}

BOOL CStrLib::IsGroup(StringID idGrp)
{
	Entry *e=_GetValidEntry(idGrp);
	if (!e)
		return FALSE;
	return e->bGrp?TRUE:FALSE;
}

BOOL CStrLib::SetGroup(StringID id,BOOL bGroup)
{
	Entry *e=_GetValidEntry(id);
	if (!e)
		return FALSE;
	if (IsGroup(id)==bGroup)
		return TRUE;//没有变化

	if (!bGroup)
		e->bGrp=0;
	else
	{
		e->bGrp=1;
		e->bDynGrp=1;
		DynGroup *grp=Class_New2(DynGroup);
		_bufDynGrp.push_back(grp);
		e->idxGrp=_bufDynGrp.size()-1;
	}
	_bGrpCache=FALSE;//清除group的cache
	_bModified=TRUE;
	_verModify++;
	return TRUE;
}

void CStrLib::_ConvertToDynGrp(Entry *e)
{
	assert(e->bGrp);
	if (e->bDynGrp)
		return;

	DynGroup *grp=Class_New2(DynGroup);
	for (int i=_bufGrp[e->idxGrp].start;i<_bufGrp[e->idxGrp].end;i++)
		grp->subs.push_back(_bufSub[i]);

	e->bDynGrp=1;
	_bufDynGrp.push_back(grp);
	e->idxGrp=_bufDynGrp.size()-1;
}


BOOL CStrLib::AddGroupSub(StringID idGrp,StringID id)
{
	if (!IsGroup(idGrp))
		return FALSE;
	if (!_GetValidEntry(id))
		return FALSE;

	if (StringID_Category(idGrp)!=StringID_Category(id))
		return FALSE;

	Entry *e=_GetValidEntry(idGrp);

	_ConvertToDynGrp(e);
	UNIQUE_VEC_ADD(_bufDynGrp[e->idxGrp]->subs,id);
	_bModified=TRUE;
	_verModify++;
	return TRUE;
}

BOOL CStrLib::RemoveGroupSub(StringID idGrp,StringID id)
{
	if (!IsGroup(idGrp))
		return FALSE;
	if (!IsGroupSub(idGrp,id))
		return TRUE;

	if (StringID_Category(idGrp)!=StringID_Category(id))
		return FALSE;

	Entry *e=_GetValidEntry(idGrp);

	_ConvertToDynGrp(e);
	VEC_REMOVE(_bufDynGrp[e->idxGrp]->subs,id);
	_bModified=TRUE;
	_verModify++;
	return TRUE;

}

StringID *CStrLib::EnumGroup(DWORD iCategory,DWORD &count)
{
	if (iCategory>=STRLIB_MAX_CATEGORY)
	{
		count=0;
		return NULL;
	}
	if (!_bGrpCache)
	{
		for (int k=0;k<STRLIB_MAX_CATEGORY;k++)
		{
			_cacheGrp[k].clear();
			for (int i=0;i<_entries[k].size();i++)
			{
				if (_entries[k][i].iSerial!=0)
				{
					if (_entries[k][i].bGrp)
						_cacheGrp[k].push_back(StringID_Make(k,i,_entries[k][i].iSerial));
				}
			}
		}
		_bGrpCache=TRUE;
	}
	count=_cacheGrp[iCategory].size();
	return &_cacheGrp[iCategory][0];

}


StringID *CStrLib::EnumGroup(DWORD &count)
{
	return EnumGroup(STRLIB_CATEGORY_DEFAULT,count);
}

StringID *CStrLib::_EnumGroupSubs(Entry *e,DWORD &count)
{
	if (!e->bDynGrp)
	{
		Group *grp=&_bufGrp[e->idxGrp];
		count=grp->end-grp->start;
		if (count==0)
			return NULL;
		return &_bufSub[grp->start];
	}

	DynGroup *grp=_bufDynGrp[e->idxGrp];
	count=grp->subs.size();
	if (count==0)
		return NULL;
	return &grp->subs[0];
}


StringID *CStrLib::EnumGroupSubs(StringID idGrp,DWORD &count)
{
	count=0;
	if (!IsGroup(idGrp))
		return NULL;

	Entry *e=_GetValidEntry(idGrp);
	if (!e->bGrp)
		return NULL;

	return _EnumGroupSubs(e,count);
}


BOOL CStrLib::IsGroupSub(StringID idGrp,StringID id)
{
	DWORD count;
	StringID *subs=EnumGroupSubs(idGrp,count);

	for (int i=0;i<count;i++)
	{
		if (subs[i]==id)
			return TRUE;
	}
	return FALSE;
}



#define StringLib_Ver 3
void CStrLib::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=StringLib_Ver;//版本号

	dp.Data_NextDword()=_seedSerial;

	std::vector<Entry>entries[STRLIB_MAX_CATEGORY];
	std::vector<char>buf;
	std::vector<Group> bufGrp;
	std::vector<StringID>bufSub;

	for (int k=0;k<STRLIB_MAX_CATEGORY;k++)
	{
		for (int i=0;i<_entries[k].size();i++)
		{
			Entry *entry=&_entries[k][i];
			if (entry->iSerial==0)
			{
				entries[k].push_back(*entry);
				continue;
			}

			Entry t;
			t.bDyn=0;
			t.iSerial=entry->iSerial;
			t.idx=buf.size();
			t.bGrp=entry->bGrp;
			t.bDynGrp=0;
			if (entry->bGrp)
			{
				t.idxGrp=bufGrp.size();

				StringID *subs=NULL;
				DWORD count=0;
				subs=_EnumGroupSubs(entry,count);
				Group t2;
				t2.start=bufSub.size();
				t2.end=t2.start+count;
				bufGrp.push_back(t2);

				VEC_APPEND_BUFFER(bufSub,subs,count);
			}

			entries[k].push_back(t);

			const char *str=_GetStr(entry);
			int len=strlen(str);
			VEC_APPEND_BUFFER(buf,str,len+1);
		}
	}

	for (int k=0;k<STRLIB_MAX_CATEGORY;k++)
	{
		DP_WriteVector(dp,entries[k]);
	}
	DP_WriteVector(dp,buf);
	DP_WriteVector(dp,bufGrp);
	DP_WriteVector(dp,bufSub);
}

void CStrLib::Load(CDataPacket &dp)
{
	DWORD ver=dp.Data_NextDword();

	_seedSerial=dp.Data_NextDword();

	if (ver==2)
	{
		DP_ReadVector(dp,_entries[STRLIB_CATEGORY_DEFAULT]);
		DP_ReadVector(dp,_buf);
		DP_ReadVector(dp,_bufGrp);
		DP_ReadVector(dp,_bufSub);
	}
	else
	{
		for (int k=0;k<STRLIB_MAX_CATEGORY;k++)
		{
			DP_ReadVector(dp,_entries[k]);
		}
		DP_ReadVector(dp,_buf);
		DP_ReadVector(dp,_bufGrp);
		DP_ReadVector(dp,_bufSub);
	}
	_bGrpCache=FALSE;
}


StringID *CStrLib::Enum(DWORD iCategory,DWORD &count)
{
	count=0;
	if (iCategory>=STRLIB_MAX_CATEGORY)
		return NULL;
	_temp.reserve(_entries[iCategory].size());
	_temp.clear();
	for (int i=0;i<_entries[iCategory].size();i++)
	{
		if (_entries[iCategory][i].iSerial!=0)
			_temp.push_back(StringID_Make(iCategory,i,_entries[iCategory][i].iSerial));
	}
	count=_temp.size();
	return _temp.data();

}

StringID *CStrLib::Enum(DWORD &count)
{
	return Enum(STRLIB_CATEGORY_DEFAULT,count);
}

void CStrLib::Repair()
{

}

//找第一个名为str的group的id
StringID CStrLib::FindGroup(DWORD iCategory,const char *str)
{
	if (iCategory>=STRLIB_MAX_CATEGORY)
		return StringID_Invalid;
	DWORD c;
	EnumGroup(iCategory,c);
	for (int i=0;i<_cacheGrp[iCategory].size();i++)
	{
		if (strcmp(GetStr(_cacheGrp[iCategory][i]),str)==0)
			return _cacheGrp[iCategory][i];
	}
	return StringID_Invalid;
}

StringID CStrLib::FindGroup(const char *str)
{
	return FindGroup(STRLIB_CATEGORY_DEFAULT,str);
}


//在一个group中找第一个名字为str的字符串id
StringID CStrLib::FindStr(DWORD iCategory,const char *str,const char *strGrp)
{
	if (iCategory>=STRLIB_MAX_CATEGORY)
		return StringID_Invalid;

	StringID idGrp=FindGroup(iCategory,strGrp);
	if (idGrp==StringID_Invalid)
		return StringID_Invalid;
	DWORD c;
	StringID *subs=EnumGroupSubs(idGrp,c);
	for (int i=0;i<c;i++)
	{
		if (strcmp(GetStr(subs[i]),str)==0)
			return subs[i];
	}
	return StringID_Invalid;
}


StringID CStrLib::FindStr(const char *str,const char *strGrp)
{
	return FindStr(STRLIB_CATEGORY_DEFAULT,str,strGrp);
}
