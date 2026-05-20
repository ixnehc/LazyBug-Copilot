
#include "stdh.h"

#include "LevelObjSrc.h"
#include "LevelObj.h"

#include "Level.h"


//////////////////////////////////////////////////////////////////////////
//CLevelIDs

void CLevelIDs::Init(CLevel *level)
{
	_level=level;

}

void CLevelIDs::Clear()
{
	_objs.clear();
	_items.clear();
	Zero();
}

void CLevelIDs::Register(CLevelObj *obj)
{
	obj->_id=_level->GetWorld()->NewLevelObjID();
	_objs[obj->_id]=obj;
	CLevelObjSrc *los=obj->GetLos();
	if (los)
	{
		LevelGUID guid=los->GetGUID();
		if (guid!=LevelGUID_Invalid)
			_objs2[guid]=obj;
	}
}

void CLevelIDs::UnRegister(CLevelObj *obj)
{
	if (obj->_id==LevelObjID_Invalid)
		return;

	CLevelObjSrc *los=obj->GetLos();
	if (los)
	{
		LevelGUID guid=los->GetGUID();
		if (guid!=LevelGUID_Invalid)
		{
			std::unordered_map<LevelGUID,CLevelObj *>::iterator it=_objs2.find(guid);
			if (it!=_objs2.end())
				_objs2.erase(it);
		}
	}


	std::unordered_map<LevelObjID,CLevelObj *>::iterator it=_objs.find(obj->_id);
	if (it!=_objs.end())
		_objs.erase(it);

	obj->_id=LevelObjID_Invalid;
}

CLevelObj *CLevelIDs::LoFromID(LevelObjID id)
{
	std::unordered_map<LevelObjID,CLevelObj *>::iterator it=_objs.find(id);
	if (it!=_objs.end())
		return (*it).second;
	return NULL;
}

CLevelObj *CLevelIDs::LoFromGUID(LevelGUID guid)
{
	std::unordered_map<LevelGUID,CLevelObj *>::iterator it=_objs2.find(guid);
	if (it!=_objs2.end())
		return (*it).second;
	return NULL;
}


void CLevelIDs::Register(CLevelItem *item)
{
	item->_id=_level->GetWorld()->NewLevelObjID();
	_items[item->_id]=item;
}

void CLevelIDs::UnRegister(CLevelItem *item)
{
	if (item->_id==LevelObjID_Invalid)
		return;

	std::unordered_map<LevelItemID,CLevelItem *>::iterator it=_items.find(item->_id);
	if (it!=_items.end())
		_items.erase(it);

	item->_id=LevelItemID_Invalid;
}

CLevelItem *CLevelIDs::LevelItemFromID(LevelItemID id)
{
	std::unordered_map<LevelItemID,CLevelItem *>::iterator it=_items.find(id);
	if (it!=_items.end())
		return (*it).second;
	return NULL;
}

void CLevelIDs::EnumObjs(LevelIDsEnumCallBack&dlgt)
{
	_enums.clear();

	std::unordered_map<LevelObjID,CLevelObj *>::iterator it;
	for (it=_objs.begin();it!=_objs.end();it++)
	{
		CLevelObj *lo=(*it).second;
		if (!lo)
			continue;

		if (!dlgt(lo))
			continue;

		_enums.push_back(lo);
	}
}

CLevelObj **CLevelIDs::GetEnumObjs(DWORD &c)
{
	c=_enums.size();
	if (c<=0)
		return NULL;
	return _enums.data();
}
