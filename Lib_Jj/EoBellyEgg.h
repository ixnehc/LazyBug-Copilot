#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "LevelOps.h"

#define CLASSUID_BellyEgg 74

struct EoParamBellyEgg:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamBellyEgg);


	BEGIN_GOBJ_PURE(EoParamBellyEgg,1);

		GELEM_OBJVECTOR(DealEntry,dealsFromPlayer); 
			GELEM_EDITOBJ("玩家侧的结算列表","多个结算");

	END_GOBJ();

	std::vector<DealEntry> dealsFromPlayer;

};



class EoBellyEgg:public CLoEffectObj
{
public:
	EoBellyEgg()
	{
		_bTriggered=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoBellyEgg,CLASSUID_BellyEgg);

	virtual const char *GetShowName()	{		return "BellyEgg";	}

	BOOL IsTriggered()	{		return _bTriggered;	}
	float GetOffendRange()	{		return 4.0f;	}
	void Trigger(LevelOpLink &link,CLevelObj *loPlayer=NULL);

protected:

	void _OnPostCreate() override;
	void _OnDetroy() override;
	BOOL _NeedOps() override	{		return TRUE;	}

	void _OnUpdate();

	BOOL _bTriggered;//有没有触发

};
