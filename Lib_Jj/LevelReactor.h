#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "LevelDefines.h"
#include "LevelEvents.h"

#include "records/recordsdefine.h"

#include "LevelObj.h"
#include "LevelOSB.h"

#include "BuffCalc.h"

class CLevelReactor;
inline void reactor_verify(CLevelReactor*c) {}


#define DEFINE_REACTORPARAM_CLASS(clss)											\
	DEFINE_CLASS(clss);																	\
	virtual CClass*GetReactorClass()																				\
	{																																\
		extern CClass *GetReactorClass_##clss();																				\
		return GetReactorClass_##clss();																				\
	}

#define BIND_REACTORPARAM(clss,clssParam)													\
	CClass *GetReactorParamClass_##clssParam()																				\
	{																													\
		return Class_Ptr2(clssParam);																\
	}																													\
	CClass *GetReactorClass_##clssParam()														\
	{																													\
		return Class_Ptr2(clss);															\
	}


#define GELEM_DYNOBJPTR_REACTORPARAM(type,name0,initclss,editname,editdesc)											\
{																												\
	GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
	p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
	p->sz=sizeof(ptr->name0);																	\
	p->elemname=#name0;																		\
	extern CClass*GetReactorParamClass_##initclss();														\
	p->init=GetReactorParamClass_##initclss();																\
	p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
	p->name=editname;																			\
	p->desc=editdesc;																				\
	p->bEditable=TRUE;																			\
	_ELEM_LINK;																						\
}

#define GELEM_DYNOBJPTR_CLASS_REACTORPARAM(name,clss)																					\
	extern CClass*GetReactorParamClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetReactorParamClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);


#define GELEM_REACTORPARAM(param,editname,editdesc)																\
GELEM_DYNOBJPTR_REACTORPARAM(LevelReactorParam,param,ReactorParam_Damage,editname,editdesc)								\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("01.Damage",ReactorParam_Damage)																	\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("02.Hit",ReactorParam_Hit)																							\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("03.PostDamage",ReactorParam_PostDamage)													\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("04.DamageImmune",ReactorParam_DamageImmune)																							\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("05.Stun",ReactorParam_Stun)																			\
GELEM_DYNOBJPTR_CLASS_REACTORPARAM("06.PreKill",ReactorParam_PreKill)


struct LevelReactorParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual CClass*GetReactorClass()=0;
};

struct LevelReactorParamEntry
{
	LevelReactorParam *param;

	BEGIN_GOBJ_PURE(LevelReactorParamEntry,1);
		GELEM_REACTORPARAM(param,"参数","参数");
	END_GOBJ();

};


class CLevelOp;
class CLevelReactors;
class CLevelReactor
{
public:
	CLevelReactor()
	{
		Zero();
	}
	void Zero()
	{
		_reactors=NULL;
	}

	virtual CClass *GetClass()=0;

	virtual void OnEvent(LevelEvent &e)	{	}

	LevelReactorParam *GetParam()	{		return _param;	}

	void Create(LevelReactorParam *param,CLevelReactors *reactors);
	void Destroy();


	virtual void Update(AnimTick t)	{	}
	virtual void HandleEvent(LevelEvent &e)	{	}

protected:
	void _SetParam(LevelReactorParam *param)
	{
		_param=param;
	}

	virtual void _OnCreate(){}
	virtual void _OnDestroy(){}


	virtual void _OnUpdate(AnimTick dt){}

	CLevelObj*_GetOwner();
	CLevel*_GetLevel();
	LevelOSB _GetOSB();

	LevelReactorParam *_param;

	CLevelReactors *_reactors;

	friend class CLevelReactors;

};

class CLevelBuff;
class CLevelReactors
{
public:
	DEFINE_CLASS(CLevelReactors);
	CLevelReactors()
	{
		Zero();
	}
	void Zero()
	{
		_owner=NULL;
	}
	void Init(CLevelBuff *buff);
	void Clear();

	void SetOwner(CLevelBuff *buff)	{		_owner=buff;	}

	void Update(AnimTick t);

	CLevelReactor *FindReactor(CClass *clss);

	CLevelReactor **GetReactors(DWORD &c)
	{
		c=_reactors.size();
		return _reactors.data();
	}


	void HandleEvent(LevelEvent &e);

protected:

	void _LoadReactors(LevelReactorParamEntry *entries,DWORD c);

	std::vector<CLevelReactor*>_reactors;

	CLevelBuff *_owner;

	friend class CLevelDecider;
	friend class CLevelReactor;
};