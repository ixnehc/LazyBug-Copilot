#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_PushAdv 65

struct EoParamPushAdv:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamPushAdv);

	EoParamPushAdv()
	{
		GConstructor();

	}

	~EoParamPushAdv()
	{
		GDestructor();
	}


	BEGIN_GOBJ(EoParamPushAdv,1);

	END_GOBJ();

};



class EoPushAdv:public CLoEffectObj
{
public:
	EoPushAdv()
	{
		_tStart=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoPushAdv,CLASSUID_PushAdv);

	virtual const char *GetShowName()	{		return "PushAdv";	}
	virtual LevelAttr_AttackMods*GetAttr_AttackMods();

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void UpdateSubframe() override;

	AnimTick _tStart;

	std::unordered_set<LevelObjID> _handled;
};
