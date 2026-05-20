#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#define DEFINE_TECHPARAM()				\
virtual CClass*GetTechClass() override;				\
virtual CClass*GetSyncClass() override;

#define DEFINE_TECHSYNC(clss)								\
DEFINE_CLASS(clss)											\
virtual void *GetData(DWORD &sz)					\
{																				\
	sz=sizeof(*this)-4;												\
	return ((BYTE*)this)+4;										\
}


#define BIND_TECH(clssTech,clssParam,clssSync)						\
CClass *clssParam::GetSyncClass()						\
{																			\
	return Class_Ptr2(clssSync);						\
}																		\
CClass *clssParam::GetTechClass()						\
{																			\
	return Class_Ptr2(clssTech);						\
}

#define GELEM_TECHPARAM_BASE()		GELEM_VAR_INIT(BOOL,bValid,0)


struct LevelTechParam
{
	virtual GObjBase *GetGObj()=0;
	virtual CClass*GetTechClass()=0;
	virtual CClass*GetSyncClass()=0;

	BOOL bValid;
};


struct LevelTechSync
{
	virtual CClass*GetClass()=0;
	virtual void *GetData(DWORD &szData)=0;

};

class CLevelAbility;
struct LevelItemState;
class CLevelTech
{
public:
	CLevelTech()
	{
		_owner=NULL;
		_param=NULL;
	}
	virtual CClass*GetClass()=0;
	virtual void OnCreate()=0;
	virtual void OnDestroy()=0;
	virtual void OnUpdate(AnimTick dt)=0;
	virtual void OnBuildRT()=0;
	virtual void OnClearRT()=0;
	virtual void OnEvent(LevelEvent &e){}
	virtual void OnEndDay(){}
	virtual void OnBuildArtifactState(LevelItemState &stateItem){}

	virtual void SaveSync(LevelTechSync &sync)=0;
	virtual void LoadSync(LevelTechSync &sync){}//保留,目前不需要重载
	 
protected:
	CLevelAbility *_owner;
	LevelTechParam *_param;

	friend class CLevelAbility;

};
