#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "LevelObj.h"

#include "LevelAttrs.h"
#include "LevelAttrs_Weak.h"

#include "LevelSkillCDs.h"
#include "LevelSkillDriver.h"

#include "LevelBuff.h"

#include "LevelObjPauser.h"

#include "LevelObjMove.h"
#include "LevelEventSrc.h"

#include "LevelOps.h"

#include "LevelAIContext.h"

class CLoUnit;



class CLoUnit;


class CUnit;
struct LevelRecordUnit;
class CLevelItem;
class CLevelSkill;
class CLevelOp;
struct LevelOpDesc;
struct LevelPlayerStates;
struct ExprEquips;
class CLevelCoSkill;
class CBehaviorMem;
class CLevelRtnu;
struct LevelUnitArg;
class CLevelNPC;
class CLoUnit:public CLevelObj
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoUnit,4);

	CLoUnit()
	{
		Zero();
	}
	~CLoUnit()
	{

	}

	virtual int AddRef()
	{
		return __super::AddRef();
	}
	void Zero()
	{
		_idPlayer=LevelPlayerID_Invalid;
		_rec=NULL;
		_tidUnit=RecordID_Invalid;
		_lps=NULL;
		_verLPS=0;
		_param=NULL;

		_tUpdate=0;

		_seedTeleportID=0;

		_equipsExpr=NULL;

		_cycleUpdate=0;

		_bDisableAI=0;
		_bLastAI=0;
		_bhvAI=NULL;

		_talks=NULL;
		_coskill=NULL;
		_counter=NULL;
		_sensor=NULL;

		_attrResists=NULL;
		_attrEvade=NULL;
		_attrDefendMods=NULL;
		_attrAttackMods=NULL;
		_attrSpeedMod=NULL;
		_attrResource=NULL;
		_attrTemple=NULL;
		_attrMagicBoard=NULL;

		_ctxAI=NULL;
		_troop=NULL;

		_scenarioAI=StringID_Invalid;
		_iPickedEquipSet=EquipSetPick_None;

		_rtnu=NULL;

		_blocking=NULL;
	}

	virtual LevelObjType GetType() override	{		return LevelObjType_Unit;	}
	virtual LevelObjID GetRootOwnerID() override	{		return GetID();	}

	void PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos);
	void PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos,float xEuler);
	void PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos3D&pos3D);
	void PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos3D&pos3D,LevelFace face);
	void PostCreate_Floating(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos,float htFloating);
	void PostCreate_Teleport(CLoUnit *loUnitOrg,LevelPos &pos);//根据原始的LevelObj创建一个Teleport后的LevelObj
	virtual void OnDestroy() override;
	void OnPlayerIDChanged() override;

	virtual LevelAttr_Base*GetAttr_Base()	override{		return &_attrBase;	}
	virtual LevelAttr_Resists*GetAttr_Resists()	override{		return _attrResists;	}
	virtual LevelAttr_Evade*GetAttr_Evade()	override{		return _attrEvade;	}
	virtual LevelAttr_Weaks*GetAttr_Weaks()	override{		return &_attrWeaks;	}
	virtual LevelAttr_WeaksMod*GetAttr_WeaksMod()	override{		return &_attrWeaksMod;	}
	virtual LevelAttr_DefendMods*GetAttr_DefendMods()	override{		return _attrDefendMods;	}
	virtual LevelAttr_AttackMods*GetAttr_AttackMods()	override{		return _attrAttackMods;	}
	virtual LevelAttr_Drop*GetAttr_Drop()	override{		return &_attrDrop;	}
	virtual LevelAttr_Resource*GetAttr_Resource()	override{		return _attrResource;	}
	virtual LevelAttr_Temple*GetAttr_Temple()	override{		return _attrTemple;	}
	virtual LevelAttr_MagicBoard*GetAttr_MagicBoard()override	{		return _attrMagicBoard;	}
	virtual LoMiscFlags*GetMiscFlags() override;
	virtual LevelPos GetFramePos()override;
	virtual float GetFrameFace()override;
	virtual LevelPos3D GetFramePos3D()override;
	virtual float GetRadius_()override;
	virtual float GetHeight()override;
	virtual float GetAimHeight()override;
	virtual float GetCastHeight()override;
	virtual float GetModelScale()override;
	virtual CUnit *GetUnit()override	{		return _move.GetGroundUnit();	}
	virtual CUnit3D *GetUnit3D()override	{		return _move.GetFlyingUnit();	}
	virtual CRtnuUnit *GetRtnuUnit()	override{		return _move.GetGroundRtnuUnit();	}
	virtual CLevelObjMove*GetMove()override	{		return &_move;	}
	virtual LevelMoveMethod GetMoveMethod()override	{		return _move.GetMethod();}
	virtual CLevelSkillCDs *GetSkillCDs()override	{		return &_cds;	}
	virtual CLevelSkillDriver *GetSkillDriver()	override{		return &_skilldriver;	}
	virtual CLevelBuffs *GetBuffs()override	{		return &_buffs;	}
	virtual CLevelOps *GetOps()override	{		return &_ops;	}
	virtual CLevelObjPauser*GetPauser()override	{		return &_pauser;	}
	virtual ExprEquips *GetExprEquips()override	{		return _equipsExpr;	}
	virtual LevelTeleportID GenTeleportID()override	
	{		
		_seedTeleportID++;
		if (_seedTeleportID==LevelTeleportID_Invalid)
			_seedTeleportID++;
		return _seedTeleportID;
	}
	virtual DWORD GetBuffID_Dead()override;
	virtual DWORD GetBuffID_Stun()override;
	virtual DWORD GetBuffID_KB()override;
	virtual DWORD GetBuffID_Bleed()override;
	virtual DWORD GetBuffID_Ash()override;
	virtual DWORD GetBuffHandler_Jink()override;
	virtual DWORD GetBuffHandler_SkillStun()override;
	virtual CLevelBehavior*GetBehaviorAI()override	{		return _bDisableAI?NULL:_bhvAI;	}
	virtual CLevelTalks*GetTalks()override	{		return _talks;}
	virtual CLevelSensor*GetSensor()	override{		return _sensor;}
	virtual CLevelCoSkill *ObtainCoSkill()override;
	virtual CLevelCounter *GetCounter()override	{		return _counter;	}
	virtual CLevelCounter *ObtainCounter()override;
	virtual CLevelEventSrc *GetEventSrc()override	{		return &_dmgsrc;	}
	virtual CLevelBlocking *GetBlocking()override	{		return _blocking;	}
	virtual LevelAIContext *ObtainAIContext()override;
	virtual LevelAIContext *GetAIContext()override	{		return _ctxAI;	}
	virtual void SetAICmd(StringID idCmd)override;
	virtual StringID GetAICmd()override	;
	virtual void SetTroop(CLevelTroop *troop)override	{		_troop=troop;	}
	virtual CLevelTroop *GetTroop()override	{		return _troop;	}
	virtual LevelRtnuRank GetRtnuRank()override;

	DWORD GetSimulateSpheres(i_math::spheref *sphs,DWORD nMaxSphs);

	LevelRecordUnit *GetRec()	{		return _rec;	}
	RecordID GetRecID()	{		return _tidUnit;	}

	template<typename T>
	T *GetArg()
	{
		if (_arg)
		{
			if (_arg->GetClass()->IsSameWith(Class_Ptr2(T)))
				return (T*)_arg;
		}
		return NULL;
	}

	float GetCastRate();

	void EnableAI(BOOL bEnable)	{		_bDisableAI=!bEnable;	}
	void ResetAI();//重置AI
	void UpdateAI(BOOL bRun);
	void SetAIScenario(StringID scenario)	{		_scenarioAI=scenario;	}
	StringID GetAIScenario()	{		return _scenarioAI;	}

	EquipSetPick GetPickedEquipSet()	{		return _iPickedEquipSet;	}
	void UpdateExprEquips(LevelPlayerStates *lps);//更新ExprEquips,并且生成相应的LevelOp

	BOOL AddExprEquips(RecordID idItem);//更新ExprEquips,并且生成相应的LevelOp
	BOOL RemoveExprEquips(RecordID idItem);//更新ExprEquips,并且生成相应的LevelOp

	void UpdateAttrs(LevelOpLink &link)	{		_UpdateAttrs(FALSE,link);	}

	void SetRtnu(CLevelRtnu *rtnu)	{		_rtnu=rtnu;	}
	CLevelRtnu *GetRtnu()	{		return _rtnu;	}

	virtual void Update()override;


	virtual void WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	virtual void WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	virtual void WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	virtual void PostWriteSync()override;

	void EquipNpc(CLevelNPC *npc);

	//一些帮助函数
	BOOL NeedPauseMove();

protected:

	void _PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet);
	void _CreateMove();

	//更新时间
	AnimTick _tUpdate;

	//静态属性
	LevelRecordUnit * _rec;
	RecordID _tidUnit;
	LevelUnitArg *_arg;

	//LPS
	LevelPlayerStates *_lps;
	DWORD _verLPS;

	//外观装备
	ExprEquips *_equipsExpr;
	EquipSetPick _iPickedEquipSet;

	//位置/移动
	CLevelObjMove _move;

	//暂停器
	CLevelObjPauser _pauser;

	//技能
	CLevelSkillDriver _skilldriver;
	CLevelSkillCDs _cds;

	//属性
	void _InitAttrs(LevelPlayerStates *lps,LevelGrade grd);
	void _UpdateAttrs(BOOL bInit,LevelOpLink &link);//bInit表示是不是初始化attr,还是更新它们
	LevelAttr_Base _attrBase;
	LevelAttr_Resists *_attrResists;
	LevelAttr_Evade *_attrEvade;
	LevelAttr_Weaks _attrWeaks;
	LevelAttr_WeaksMod _attrWeaksMod;
	LevelAttr_Drop _attrDrop;
	LevelAttr_DefendMods *_attrDefendMods;
	LevelAttr_AttackMods *_attrAttackMods;
	LevelAttr_SpeedMod *_attrSpeedMod;
	LevelAttr_Resource *_attrResource;
	LevelAttr_Temple *_attrTemple;
	LevelAttr_MagicBoard *_attrMagicBoard;

	//用于同步的SkillOps
	CLevelOps _ops;

	//Buffs
	CLevelBuffs _buffs;

	//TeleportID
	LevelTeleportID _seedTeleportID;

	//Update的周期数,用来实现隔几帧更新一次的效果
	BYTE _cycleUpdate;

	CLevelTalks *_talks;

	CLevelCoSkill *_coskill;

	CLevelCounter *_counter;

	CLevelEventSrc _dmgsrc;

	CLevelSensor *_sensor;

	CLevelRtnu *_rtnu;

	CLevelBlocking *_blocking;

	//AI
	CBehaviorMem *_GetBhvMem();
	void _EnsureAIContext();
	BYTE _bDisableAI:1;
	BYTE _bLastAI:1;//用于AI数据的同步,记录上一帧有没有AI数据
	CLevelBehavior *_bhvAI;
	StringID _scenarioAI;
	LevelAIContext *_ctxAI;
	CLevelTroop *_troop;


};
