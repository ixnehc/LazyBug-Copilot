#pragma once

#include "../strlib/strlibdefines.h"
#include "../class/class.h"
#include "../gds/GObj.h"
#include "../gds/GObjEx.h"
#include "../records/recordsdefine.h"
#include "behaviordefines.h"

#include "BehaviorCustomConst.h"


struct BehaviorCustomConst;



struct BhvValType
{
	BYTE tp;
	BYTE gvt;
	BYTE codeSem;
	BYTE tpMem;
	std::string subname;
	std::string constraintSem;

	void From(GElemBase *elem);

	BOOL IsCompatible(GElemBase *elem)
	{
		if (tp!=(BYTE)elem->GetTypeID())
			return FALSE;
		if (gvt!=(BYTE)elem->GetVarType())
			return FALSE;
		if (subname!=elem->subtype)
			return FALSE;
		if (codeSem!=(BYTE)elem->sem.code)
			return FALSE;
		if (constraintSem!=elem->sem.constraint)
		{
			if (elem->contraintsLegacy.find(constraintSem)==elem->contraintsLegacy.end())
				return FALSE;
		}
		return TRUE;
	}

	BOOL IsCompatible(BhvValType &other)
	{
		if (tp!=other.tp)
			return FALSE;
		if (subname!=other.subname)
			return FALSE;
		if (codeSem!=other.codeSem)
			return FALSE;
		switch(codeSem)
		{
			case GSem_Float:
				break;//某些Sem不需要constraint一致
			default:
			{
				if (constraintSem!=other.constraintSem)
					return FALSE;
			}
		}

		return TRUE;
	}

	BEGIN_GOBJ_PURE(BhvValType,1);
		GELEM_VAR_INIT( BYTE,tp,0);	
		GELEM_VAR_INIT( BYTE,gvt,(BYTE)GVT_None);	
		GELEM_VAR_INIT( BYTE,codeSem,(BYTE)GSem_Unknown);	
		GELEM_VAR_INIT( BYTE,tpMem,(BYTE)BehaviorMemType_None);	
		GELEM_STRING(constraintSem);
		GELEM_STRING(subname);
	END_GOBJ();    

};

struct BhvVal;
struct BhvValDeclare
{
	void AssignDefault(BhvVal &e);
	BOOL CheckDefault(BhvVal &e);
	BOOL IsCompatible(BhvValDeclare &other);

	StringID nm;
	BhvValType tp;
	std::vector<BYTE>dataDef;

};

struct BhvConstDeclare:public BhvValDeclare
{
	BEGIN_GOBJ_PURE(BhvConstDeclare,1);
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
		GELEM_OBJ(BhvValType,tp);
		GELEM_VARVECTOR_INIT(BYTE,dataDef,0);
		GELEM_VAR_INIT(DWORD,tpShow,0);
			GELEM_EDITVAR("显示类型",GVT_U,GSem(GSem_Interger,"Source和Param中都会显示:0,仅在Source中显示:1,仅在Param中显示:2"),"显示类型");
	END_GOBJ();    
	DWORD tpShow;
	
};

// struct BhvVarDeclare:public BhvValDeclare
// {
// 	DWORD flags;
// 	BEGIN_GOBJ_PURE(BhvVarDeclare,1);
// 		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
// 		GELEM_OBJ(BhvValType,tp);
// 		GELEM_VARVECTOR_INIT(BYTE,dataDef,0);
// 		GELEM_VAR_INIT(BehaviorMemFlag,flags,BehaviorMemFlag_None);
// 			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"需要保存:1,需要网络同步:2"),"标志");
// 	END_GOBJ();    
// };

struct BhvParamDeclare:public BhvValDeclare
{
	BEGIN_GOBJ_PURE(BhvParamDeclare,1);
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
		GELEM_OBJ(BhvValType,tp);
		GELEM_VARVECTOR_INIT(BYTE,dataDef,0);
	END_GOBJ();    
};


#define StringID_BhvValInvalidRef (0xffffffff)


struct BhvVal
{
	BhvVal()
	{
		Zero();
	}
	~BhvVal()
	{
		Clear();
	}
	void Zero()
	{
		nm=StringID_Invalid;
		nmRef=StringID_BhvValInvalidRef;
	}
	void Clear();
	void CopyFrom(BhvVal &src);
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	BOOL Equals(BhvVal &entryOther);
	BOOL IsCompatible(BhvVal &entryOther);

	void SetInt(int v);
	void SetFloat(float v);

	StringID nm;
	BhvValType tp;

	StringID nmRef;

	std::vector<BYTE>data;

};

struct BhvValues
{
	std::deque<BhvVal> entries;
	std::unordered_map<StringID,BhvVal*>lookup;

	BEGIN_GOBJ_PURE(BhvValues,1);

	END_GOBJ();    

	void Zero(BOOL bIntuitive)
	{
	}

	void Clear()
	{
		for (int i=0;i<entries.size();i++)
			entries[i].Clear();
		entries.clear();
		lookup.clear();
	}

	void Copy(BhvValues*src)
	{
		lookup.clear();

		entries.resize(src->entries.size());
		for (int i=0;i<entries.size();i++)
			entries[i].CopyFrom(src->entries[i]);
	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextWord()=0;//版本号
		dp.Data_NextWord()=(WORD)entries.size();
		for (int i=0;i<entries.size();i++)
			entries[i].Save(dp);
	}


	BOOL Load(CDataPacket &dp)
	{
		WORD ver=dp.Data_NextWord();
		entries.resize(dp.Data_NextWord());
		for (int i=0;i<entries.size();i++)
			entries[i].Load(dp);
		lookup.clear();
		return TRUE;
	}

	void SaveDelta(CDataPacket &dp,BhvValues *pRef)
	{
		dp.Data_NextWord()=0;//版本号
		std::vector<BhvVal*>deltas;
		for (int i=0;i<entries.size();i++)
		{
			BhvVal *value=&entries[i];
			BhvVal *valueRef=pRef->Find(value->nm);
			if (valueRef)
			{
				if (value->Equals(*valueRef))
					continue;
			}
			deltas.push_back(value);
		}

		dp.Data_NextWord()=deltas.size();
		for (int i=0;i<deltas.size();i++)
		{
			BhvVal *value=deltas[i];
			value->Save(dp);
		}
	}

	void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		WORD ver=dp.Data_NextWord();

		int sz=(WORD)dp.Data_NextWord();

		for (int i=0;i<sz;i++)
		{
			BhvVal value;
			value.Load(dp);
			BhvVal *valueOrg=Find(value.nm);
			if (valueOrg)
			{
				if (valueOrg->Equals(value))
					continue;
				valueOrg->CopyFrom(value);
				if (ptrsDelta)
					ptrsDelta->push_back(valueOrg);
				continue;
			}

			entries.resize(entries.size()+1);
			entries[entries.size()-1].CopyFrom(value);
			if (ptrsDelta)
				ptrsDelta->push_back(&entries[entries.size()-1]);
		}

		lookup.clear();

	}


	BhvVal*Find(StringID nm)
	{
		if (lookup.size()<entries.size())
		{
			for (int i=0;i<entries.size();i++)
				lookup[entries[i].nm]=&entries[i];
		}

		std::unordered_map<StringID,BhvVal*>::iterator it=lookup.find(nm);
		if (it!=lookup.end())
			return (*it).second;

		return NULL;
	}

};



