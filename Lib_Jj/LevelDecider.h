#pragma once

#include "LevelDefines.h"

#include "LevelStrike.h"

#include "LevelAttrs_DamageAttr.h"

//Decide Information,用来帮助结算的信息

//来自于装备的DecideInfo,
struct DecideInfo_Equip
{
	void Zero()
	{

	}

};

struct LevelPhysAttack
{

};

struct LevelStrike;
struct LevelObliterateArg;
struct LevelOSB;

class CLevel;
class CLevelSkill;
class CLevelBuff;
class CLoUnit;
struct LevelBuffArg;
struct LevelRecordBuff;
struct LevelAttr_Damages;
struct LevelAttr_AttackMods;
struct LevelAttr_Resists;
struct LevelAttr_DefendMods;
struct LevelAttr_Oppresses;
class CLevelDecider
{
public:
	CLevelDecider()
	{
		_level=NULL;
	}
	void Init(CLevel *level);
	void Clear(); 

	struct MakeJinkContext
	{
		MakeJinkContext()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL bEnable;
		LevelOSB *osbSrc;
		LevelStrike *strike;
		LevelOpLink *link;
	};

	struct MakeSkillStunContext
	{
		MakeSkillStunContext()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}

		BOOL bEnable;
		LevelOSB *osbSrc;
		LevelStrike *strike;
		LevelOpLink *link;
		LevelWeakCategory catBroken;
		DamageAttrMask weaksBroken;
		LevelBuffID idResultBuff;
	};


	static BOOL CheckMiss(LevelAttr_Hit *hitSrc,LevelAttr_AttackMods *modsSrc,
											LevelAttr_Evade *evadeTarget,LevelAttr_DefendMods *modsTarget);
	static void CalcDamage(LevelAttr_Damages *attacksSrc,LevelAttr_AttackMods *modsSrc,LevelAttr_Resists *defendsTarget,LevelAttr_DefendMods *modsTarget,
		LevelAttr_Blocking *blocking,DamageResult &resultDmg);
	static BOOL CheckJink(CLevelObj *loTarget,LevelStrike &strike,DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,DamageAttrMask &weaksBroken,LevelWeakCategory &broken);
	static BOOL CheckStun(CLevelObj *loTarget,LevelStrike &strike,DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,DamageAttrMask &weaksBroken,LevelWeakCategory &broken);
	static BOOL CheckKB(DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,DamageAttrMask &weaksBroken);

	static BOOL Roll(float rate);

	static BOOL CheckInRange(CLevelObj *loSrc,CLevelObj*loTarget,float range);
	static BOOL CheckInRange(CLevelObj *loSrc,LevelPos &pos,float range);
	static BOOL CheckInRange(CLevelObj *loSrc,LevelPos3D &pos,float range);
	static BOOL CheckInDir(CLevelObj *loSrc,LevelPos &pos,float torDir);

	static BOOL CheckSkillCDOver(CLevelObj *lo,LevelRecordSkill *rec);

	static BOOL CheckOccupyResidable(CLevelObj *lo);

	static BOOL CanStartSkill(CLevelObj *lo);//返回lo是否能作为一个技能的使用者

	static void CommitHPAutoRecovery(CLevelObj *lo,float dt);
	static BOOL CommitHPAutoRecovery_New(CLevelObj *lo,float dt);

	static void CommitSPDrain(CLevelObj *lo,float sp);

	static void CommitVitaLost(LevelOSB &osb,CLevelObj *loTarget,float ratioToLost,LevelOpLink &link);

	BOOL CommitHPMod(int dHP,LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,int &nMod);//返回是否造成hp被减到0(非0-->0)
	static BOOL CommitSPMod(float dSP,LevelOSB &osb,CLevelObj*target,LevelOpLink &link,BOOL bInstant);

	BOOL CommitMiss(LevelOSB &osb,CLevelObj*target,LevelOpLink &link);

	BOOL CommitSuicide(CLevelObj*lo);//返回是否造成hp被减到0(非0-->0)

	float CommitPain(LevelOSB &osb,CLevelObj *loTarget,LevelStrike &strike,LevelOpLink &link,DamageAttrMask weaksBroken,DamageResult &resultDmg);


//	void MakeDamage(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link);
//	void MakeDamage(LevelOSB &osb,CLevelObj *target,LevelAttack &atk,LevelDefence &def,LevelStrike &strike,LevelOpLink &link);
	BOOL MakeHit(LevelOSB &osb,CLevelObj*target,LevelAttr_Hit *hit,LevelOpLink &link);
	void MakeDamage(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelAttr_Damages *dmgs,LevelOpLink &link,DmgBlockType tpBlock,float multiply=1.0f);
//	BOOL MakeLethal(LevelOSB &osb,CLevelObj *target,int lethal,LevelStrike &strike,LevelOpLink &link);//返回是否杀死

	void MakeCure(LevelOSB &osb,CLevelObj *target,int nCure,LevelStrike &strike,LevelOpLink &link);
	void MakeCure_MaxHP(LevelOSB &osb,CLevelObj *target,int nCure,LevelOpLink &link);
	void MakeCure_FullSP(LevelOSB &osb,CLevelObj *target,int nCure,LevelOpLink &link);

	void MakeMod_Str(CLevelObj *target,int nMod,LevelOpLink &link);
	void MakeMod_Magic(CLevelObj *target,int nMod,LevelOpLink &link);

	void MakeSuck_HP(LevelOSB &osb,CLevelObj *target,int nSuck,LevelOpLink &link);

	BOOL CheckCost(CLevelSkill *skill);
	void MakeCost(CLevelSkill *skill);
	void MakeCost_MaxSP(CLevelObj *lo,float cost);

	BOOL MakeBlock(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,float dmgSP,DmgBlockType tpBlock,RecordID idBlockingBuff,LevelOpLink &link);
	LevelBuffID MakeStun(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link);
	LevelBuffID MakeSkillStun(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelWeakCategory catBroken,DamageAttrMask &weaksBroken,LevelOpLink &link);
	LevelBuffID MakeJink(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link);
	LevelBuffID MakeKB(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link);
	LevelBuffID MakeKD(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,i_math::vector2df &dir,LevelOpLink &link,AnimTick dur);
	LevelBuffID MakeDead(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelObliterateArg &argObliterate,LevelOpLink &link);
	LevelBuffID MakeBirth(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,LevelOpLink &link);
	LevelBuffID MakeFlyBirth(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,LevelPos3D &pos,LevelPos &dir,LevelOpLink &link);
	LevelBuffID MakeBleed(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link);

	LevelBuffID MakeBuff(LevelOSB &osb,CLevelObj*loTarget,LevelRecordBuff *rec,AnimTick durDefault,LevelBuffArg *param,LevelOpLink &link);
	LevelBuffID MakeBuff(LevelOSB &osb,CLevelObj*loTarget,LevelBuffTypeID_ tid,AnimTick durDefault,LevelBuffArg *param,LevelOpLink &link);
	LevelBuffID MakeBuff(CLevelObj*loTarget,LevelBuffTypeID_ tid,AnimTick durDefault,LevelBuffArg *param,BOOL bNeedSync);//这个MakeBuff的版本,不发送同步信息
	void RemoveBuff(LevelOSB &osb,CLevelObj *loTarget,CLevelBuff *buff,LevelOpLink &link);
	void Revive(LevelOSB &osb,CLevelObj *loTarget,LevelOpLink &link);

	void MakeDeathDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link);
	void MakePainDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link);

	void GatherRes(CLevelObj *lo,LevelObjID idItem);
	void GatherResPile(CLevelObj *lo,LevelObjID idOwner,LevelResourceType tp,int amount);

	void MakeSoulRecover(CLevelObj *lo,int soul);
	void MakeResModify(CLevelObj *lo,LevelResourceType tpRes,int mod);
	void MakeResModify(LevelOSB &osb,CLevelObj *target,LevelResourceType tpRes,int mod,LevelOpLink &link);

	void RepairTemple(LevelOSB &osb,CLevelObj *target,LevelTempleType tpRes,DWORD iAltar);

	void MakeShapeModify(LevelOSB &osb,CLevelObj *target,int op,StringID nmShape=StringID_Invalid);
	void MakeBodyModify(LevelOSB &osb,CLevelObj *target,BOOL bEnable);

	MakeJinkContext *GetJinkContext()	{		return _ctxMakeJink.bEnable?&_ctxMakeJink:NULL;	}
	MakeSkillStunContext *GetSkillStunContext()	{		return _ctxMakeSkillStun.bEnable?&_ctxMakeSkillStun:NULL;	}

protected:

	BOOL _ResolveBuffConflicts(CLevelBuffs *buffs,LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur,std::vector<CLevelBuff*> &removes);

	static BOOL _CheckMiss(int accu,int evade);

	void _MakeResAbsorb(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,RecordID eoAbsorb,int amount);
	void _MakeDemonBloodDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,RecordID eoDemonBlood,int amount);
	CLevel *_level;

	MakeJinkContext _ctxMakeJink;
	MakeSkillStunContext _ctxMakeSkillStun;

	std::vector<CLevelBuff *>_temp;

};

