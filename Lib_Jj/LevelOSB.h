#pragma once

#include "LevelObj.h"
#include "LevelSkill.h"
#include "LevelBuff.h"


//OSB´ú±í: Object/Skill/Buff
struct LevelOSB
{
	LevelOSB()
	{
		memset(this,0,sizeof(*this));
	}
	LevelOSB(CLevelObj *lo)
	{
		memset(this,0,sizeof(*this));
		o=lo;
	}
	LevelOSB(CLevelSkill *skill)
	{
		memset(this,0,sizeof(*this));
		s=skill;
	}
	LevelOSB(CLevelBuff *buff)
	{
		memset(this,0,sizeof(*this));
		b=buff;
	}

	BOOL IsEmpty()
	{
		return (o==NULL)&&(s==NULL)&&(b==NULL);
	}
	CLevelObj *o;
	CLevelSkill *s;
	CLevelBuff *b;

	CLevelObj *GetOwner()
	{
		if (o)
			return o;
		if (s)
			return s->GetOwner();
		if (b)
			return b->GetOwner();

		return NULL;
	}

	LevelObjID GetOwnerID()
	{
		CLevelObj *owner=GetOwner();
		if (owner)
			return owner->GetID();
		return LevelObjID_Invalid;
	}
	LevelObjID GetRootOwnerID()
	{
		CLevelObj *owner=GetOwner();
		if (owner)
			return owner->GetRootOwnerID();
		return LevelObjID_Invalid;
	}
	CLevelObj *GetRootOwner()
	{
		CLevelObj *owner=GetOwner();
		if (owner)
		{
			LevelObjID idRootOwner=GetRootOwnerID();
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			return LevelUtil_GetAliveLo(owner->GetLevel(),idRootOwner);
		}
		return NULL;
	}
	CLevel *GetLevel()
	{
		CLevelObj *owner=GetOwner();
		if (owner)
			return owner->GetLevel();
		return NULL;
	}

	CLevelSkill *GetSkill()
	{
		return s;
	}

	CLevelSkill *GetRootSkill()
	{
		if (GetSkill())
			return GetSkill();

		CLevelObj *owner=GetOwner();
		if (owner)
			return owner->GetRootSkill();
		return NULL;
	}

	void AddRef()
	{
		SAFE_ADDREF(o);
		SAFE_ADDREF(s);
		SAFE_ADDREF(b);
	}

	void Release()
	{
		SAFE_RELEASE(o);
		SAFE_RELEASE(s);
		SAFE_RELEASE(b);
	}


	template <typename T>
	T *NewOp(LevelOpLink &link)
	{
		if (o)
			return o->NewOp<T>(link);
		if (s)
			return s->NewOp<T>(link);
		if (b)
			return b->NewOp<T>(link);
		return NULL;
	}
};
