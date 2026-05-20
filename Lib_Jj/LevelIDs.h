#pragma once

#include "class/class.h"
#include <unordered_map>

#include "fastdelegate/FastDelegate.h"

#include "LevelDefines.h"

class CLevelObj;
typedef fastdelegate::FastDelegate1<CLevelObj *,BOOL> LevelIDsEnumCallBack;

class CLevelObj;
class CLevelItem;
class CLevel;
class CLevelIDs
{
public:
	CLevelIDs()
	{
		Zero();
	}
	~CLevelIDs()
	{
		Clear();
	}
	void Zero()
	{
		_level=NULL;
	}
	void Init(CLevel *level);
	void Clear();
	void Register(CLevelObj *obj);
	void UnRegister(CLevelObj *obj);
	void Register(CLevelItem*obj);
	void UnRegister(CLevelItem*obj);

	//返回CLevelObj可以确保是Alive的
	CLevelObj *LoFromID(LevelObjID id);
	template<typename T>
	T*LoFromID(LevelObjID id)
	{
		CLevelObj *lo=LoFromID(id);
		if (!lo)
			return NULL;
		if (lo->GetClass()!=Class_Ptr2(T))
			return NULL;
		return (T*)lo;
	}
	CLevelObj *LoFromGUID(LevelGUID guid);
	template<typename T>
	T*LoFromGUID(LevelGUID guid)
	{
		CLevelObj *lo=LoFromGUID(guid);
		if (!lo)
			return NULL;
		if (lo->GetClass()!=Class_Ptr2(T))
			return NULL;
		return (T*)lo;
	}


	CLevelItem *LevelItemFromID(LevelItemID id);

	void EnumObjs(LevelIDsEnumCallBack&dlgt);
	CLevelObj **GetEnumObjs(DWORD &c);

protected:
	std::unordered_map<LevelObjID,CLevelObj *>_objs;
	std::unordered_map<LevelGUID,CLevelObj *> _objs2;
	std::unordered_map<LevelItemID,CLevelItem *>_items;

	std::vector<CLevelObj *> _enums;

	CLevel *_level;
};

