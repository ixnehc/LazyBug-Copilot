#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Chain 38

struct EoParamChain:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamChain);

	BEGIN_GOBJ_PURE(EoParamChain,1);

		GELEM_VAR_INIT(float,radiusAffect,8.0f);
			GELEM_EDITVAR("影响范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"影响范围");

		GELEM_VAR_INIT(float,radiusStep,2.0f);
			GELEM_EDITVAR("单次连锁作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"单次连锁作用范围");

		GELEM_VAR_INIT(BOOL,bIgnoreHostDeal,FALSE);
			GELEM_EDITVAR("忽略对Chain的起始对象的伤害",GVT_S,GSem_Boolean,"忽略对Chain的起始对象的伤害");
		GELEM_VAR_INIT(int,nSteps,1);GELEM_VERSION(2)
			GELEM_EDITVAR("有几次连锁",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"连锁几次");

		GELEM_VAR_INIT(int,nBranch,1);GELEM_VERSION(2)
			GELEM_EDITVAR("分支个数",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"每次连锁有几个分支");

		GELEM_VAR_INIT(AnimTick,dtStep,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("每次连锁间隔时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次连锁间隔时间");
	END_GOBJ();

	float radiusAffect;
	float radiusStep;
	BOOL bIgnoreHostDeal;
	int nSteps;
	int nBranch;
	AnimTick dtStep;
};



class EoChain:public CLoEffectObj
{
public:
	EoChain()
	{
		_nSteps=0;
		_tStart=0;
		_nDealed=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoChain,CLASSUID_Chain);

	virtual const char *GetShowName()	{		return "连锁效果";	}

	int GetDealedCount()	{		return _nDealed;	}

protected:
	virtual void _OnReadFirstSync(CBitPacket *bp){}

	virtual void _OnPostCreate() override;
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;

	virtual void _OnUpdate()  override;

	void _BuildChain();

	AnimTick _tStart;
	DWORD _nSteps;

	struct Entry
	{
		Entry()
		{
			memset(this,0,sizeof(*this));
		}
		CLevelObj *lo;
		LevelObjID id;
		short parent;
		short iStep;
	};

	std::deque<Entry> _entries;
	int _nDealedEntries;

	int _nDealed;
};
