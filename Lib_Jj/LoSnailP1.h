#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "LevelDeal.h"

#include "behaviorgraph/BehaviorParam.h"

#include "spline/CubicSpline.h"

#define CLASSUID_SnailP1 54


struct LopSnailP1:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSnailP1,CLASSUID_SnailP1);

	BEGIN_GOBJ_PURE(LopSnailP1,1);

		GELEM_ALLOWDISABLE();


		GELEM_VARVECTOR(i_math::spheref,locs眉须攻击_A); 
			GELEM_EDITVAR("眉须A攻击位点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"攻击目标位点");
		GELEM_VARVECTOR(i_math::spheref,locs眉须攻击_B); 
			GELEM_EDITVAR("眉须B攻击位点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"攻击目标位点");
		//XXXXX: More 眉须类型

		GELEM_VARVECTOR(i_math::spheref,locs舌虫毒爆点); 
			GELEM_EDITVAR("舌虫毒爆点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"舌虫毒爆喷射点");

	END_GOBJ();

	std::vector<i_math::spheref>*GetAttackLocs(int type)
	{
		switch(type)
		{
			case 0:
				return &locs眉须攻击_A;
			case 1:
				return &locs眉须攻击_B;
			//XXXXX: More 眉须类型
		}
		return NULL;
	}


	std::vector<i_math::spheref> locs眉须攻击_A;
	std::vector<i_math::spheref> locs眉须攻击_B;
	//XXXXX: More 眉须类型

	std::vector<i_math::spheref> locs舌虫毒爆点;


};

struct LosSnailP1:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSnailP1,CLASSUID_SnailP1);

	BEGIN_GOBJ_PURE(LosSnailP1,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();


	END_GOBJ();


	virtual BOOL NeedSyncGUID()	{		return TRUE;	}


};

class CLoSnailP1:public CLoAgent
{
public:
	CLoSnailP1()
	{
		_loSnailUnit=NULL;
		_hpTongueFlyKnot=-1;
		_tTongueBroken=ANIMTICK_INFINITE;
	}
	DEFINE_LEVELOBJ_CLASS(CLoSnailP1,CLASSUID_SnailP1);

	virtual const char *GetShowName()	{		return "SnailP1";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();
	virtual void OnDeactivate();

	virtual void Update();

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_SingleCircle;	}

	virtual BOOL IsServerOnly()	{		return TRUE;	}

	void Activate(CLevelObj *loFrom);

	LopSnailP1 *GetLop()	{		return (LopSnailP1*)_param;	}

	void SetSnailUnit(CLevelObj *lo)
	{
		SAFE_REPLACE(_loSnailUnit,lo);
	}
	CLevelObj *GetSnailUnit()
	{
		return _loSnailUnit;
	}

	void SetTongueUnit(LevelObjID idTongueUnit)	{		_idTongueUnit=idTongueUnit;	}
	LevelObjID GetTongueUnit()	{		return _idTongueUnit;	}
	void SetTongueFlyKnotHP(int hp)	{		_hpTongueFlyKnot=hp;	}
	int GetTongueFlyKnotHP()	{		return _hpTongueFlyKnot;	}

	void BreakTongue();
	BOOL IsTongueBrokenForAWhile(AnimTick dur);

protected:

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	CLevelObj *_loSnailUnit;

	LevelObjID _idTongueUnit;
	AnimTick _tTongueBroken;
	int _hpTongueFlyKnot;


};
