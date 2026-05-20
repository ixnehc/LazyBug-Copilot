#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"

#include "math/circle.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "EoEnv.h"

#include "LevelDeal.h"

#include "behaviorgraph/BehaviorParam.h"

#include "spline/CubicSpline.h"

#include "unitmgr/UnitMgrNavMesh.h"

#include <unordered_set>

#define CLASSUID_Belly 73


struct LopBelly:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopBelly,CLASSUID_Belly);

	BEGIN_GOBJ_PURE(LopBelly,1);

		GELEM_ALLOWDISABLE();


		GELEM_VARVECTOR(i_math::matrix43f,locsBelly); 
			GELEM_EDITVAR("Belly位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"Belly位点");
		GELEM_VARVECTOR(i_math::matrix43f,locsBellyQueen); 
			GELEM_EDITVAR("BellyQueen位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"BellyQueen位点");
		GELEM_VARVECTOR(i_math::matrix43f,locsBellyKing); 
			GELEM_EDITVAR("BellyKing位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"BellyKing位点");
		GELEM_VARVECTOR(i_math::matrix43f,locsBellyMinion); 
			GELEM_EDITVAR("BellyMinion位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"BellyMinion位点");

		GELEM_VARVECTOR(i_math::spheref,zonesBellyEel)
			GELEM_EDITVAR("Eel活动区域定义",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"Eel活动区域定义");
		GELEM_VARVECTOR(i_math::spheref,zonesBellyEelObstacle)
			GELEM_EDITVAR("Eel的障碍区域定义",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"Eel的障碍区域定义");
		GELEM_VARVECTOR(i_math::spheref,zoneCombat)
			GELEM_EDITVAR("战斗区域定义",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"战斗区域定义");

	END_GOBJ();


	std::vector<i_math::matrix43f> locsBelly;
	std::vector<i_math::matrix43f> locsBellyQueen;
	std::vector<i_math::matrix43f> locsBellyKing;
	std::vector<i_math::matrix43f> locsBellyMinion;
	std::vector<i_math::spheref> zonesBellyEel;
	std::vector<i_math::spheref> zonesBellyEelObstacle;

	std::vector<i_math::spheref> zoneCombat;

};

struct LosBelly:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosBelly,CLASSUID_Belly);

	BEGIN_GOBJ_PURE(LosBelly,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

		GELEM_VAR_INIT(RecordID,idEnvEo,RecordID_Invalid);
			GELEM_EDITVAR("战斗环境",GVT_U,GSem(GSem_RecordID,"eos"),"战斗环境");
		GELEM_VAR_INIT(RecordID,idUnit_Belly,RecordID_Invalid);
			GELEM_EDITVAR("Belly单位",GVT_U,GSem(GSem_RecordID,"units"),"Belly单位");
		GELEM_VAR_INIT(RecordID,idUnit_BellyKing,RecordID_Invalid);
			GELEM_EDITVAR("BellyKing单位",GVT_U,GSem(GSem_RecordID,"units"),"BellyKing单位");
		GELEM_VAR_INIT(RecordID,idUnit_BellyMinion,RecordID_Invalid);
			GELEM_EDITVAR("BellyMinion单位",GVT_U,GSem(GSem_RecordID,"units"),"BellyMinion单位");
		GELEM_VAR_INIT(RecordID,idBuff_SacredOrb,RecordID_Invalid);
			GELEM_EDITVAR("SacredOrb",GVT_U,GSem(GSem_RecordID,"buffs"),"SacredOrb");

	END_GOBJ();


	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	RecordID idEnvEo;
	RecordID idUnit_Belly;
	RecordID idUnit_BellyKing;
	RecordID idUnit_BellyMinion;

	RecordID idBuff_SacredOrb;

};

class CBellyMinionCombatState
{
public:
	enum Stage
	{
		Stage_None,
		Stage_Attack,
	};
	enum ActionType
	{
		ActionType_None,
		ActionType_Hop,
		ActionType_StompEgg,
	};
	struct Entry
	{
		Entry()
		{
			action=ActionType_None;
			idGuideEgg=LevelObjID_Invalid;
		}
		ActionType action;
		LevelPos posGuide;
		LevelObjID idGuideEgg;
	};

	CBellyMinionCombatState()
	{
		_stage=Stage_None;
		_level=NULL;
		_idKing=LevelObjID_Invalid; 
		_idEnemy=LevelObjID_Invalid;
	}

	void Init(CLevel *level,std::vector<LevelObjID>&idsMinion,LevelObjID idKing);
	void ResetAttack(CLevelObj *loEnemy);

	void RefreshGuidePos();
	BOOL GetGuidePos(LevelObjID id,LevelPos &posGuide);

	CLevelObj *GetEnemyLo();
	BOOL CheckEnemySacredOrb();

	CLevel *_level;
	LevelObjID _idKing;
	LevelObjID _idEnemy;

	Stage _stage;

	std::unordered_map<LevelObjID,Entry> _entries;

};

struct BellySetting;
class CLoBelly:public CLoAgent
{
public:
	CLoBelly()
	{
		Zero();
	}
	DEFINE_LEVELOBJ_CLASS(CLoBelly,CLASSUID_Belly);

	void Zero()
	{
		_idBelly=LevelObjID_Invalid;
		_idBellyKing=LevelObjID_Invalid;
		_idBellyQueen=LevelObjID_Invalid;
		_unitEnemy=NULL;
		_unitKing=NULL;
		_unitPlayerSurround=NULL;

		_hEnemyLichenDispel=EoEnvLichenHandle_Invalid;
	}

	virtual const char *GetShowName()	{		return "Belly";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL OnActivate();
	virtual void OnDeactivate();

	virtual void Update();

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_SingleCircle;	}

	virtual BOOL IsServerOnly()	{		return TRUE;	}

	void Activate(CLevelObj *loFrom);

	LopBelly *GetLop()	{		return (LopBelly*)_param;	}

	CBellyMinionCombatState::ActionType RequestMinionAction(LevelObjID idLo,LevelPos &posTarget,LevelObjID &idTarget);

	BOOL RequestKingSpawnEgg(float rangeMin,float rangeMax,LevelPos &posEgg);
	BOOL RequestKingEvadeJump(float rangeMin,float rangeMax,LevelPos &posEvadeJump);
	BOOL RequestKingApproachPos(LevelPos &posApproach);

	LevelObjID GetObjID_King()	{		return _idBellyKing;	}

	void RegisterEgg(LevelObjID idEgg);
	void UnregisterEgg(LevelObjID idEgg);

	void StompEgg(CLevelObj *loStomper,LevelOpLink &link);

	BOOL ValidateEelTargetPos(LevelPos &posCur,LevelPos &posTarget);
	BOOL FindValidEelTargetPos(LevelPos &posCur,float distMin,LevelPos &posTarget);

protected:

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	LevelPos _CalcMinionTargetPos(LevelObjID idMinion,CLevelObj *loEnemy,BOOL bEnemySacredOrb,LevelPos &posTarget);
	LevelPos _CalcMinionHopTargetPos(LevelObjID idMinion);
	LevelObjID _FindEggToStomp(LevelObjID idMinion);

	CUnit *_FindMinionUnit(LevelObjID idMinion);
	BellySetting &_GetBellySetting();

	BOOL _CheckSacredOrb();

	void _RefreshGuidePos();

	LevelPos _FindStepPosAroundCircle(LevelPos &posSrc,LevelPos &posTarget,float distStep,i_math::circlef &circle);

	EoEnv *_GetEoEnv();

	LevelObjID _idBelly;
	LevelObjID _idBellyKing;
	LevelObjID _idBellyQueen;
	std::vector<LevelObjID> _idsBellyMinion;

	EoEnvLichenHandle _hEnemyLichenDispel;

	CBellyMinionCombatState _stateCombat;

	std::unordered_set<LevelObjID> _eggs;

	std::unordered_map<LevelObjID,CUnit*> _unitsMinion;
	std::unordered_map<LevelObjID,CUnit*> _unitsEgg;
	CUnit *_unitKing;
	CUnit *_unitEnemy;
	CUnit *_unitPlayerSurround;
	CUnitMgrNavMesh _unitmgr;

};
