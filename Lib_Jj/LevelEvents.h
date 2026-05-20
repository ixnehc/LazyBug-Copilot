#pragma once

#include "LevelDefines.h"
#include "LevelAttrs_DamageAttr.h"

class CLevelSkill;
class CLevelObj;
struct LevelStrike;
struct LevelOSB;



//Level Event
enum LevelEventType
{
	LET_None=0,
	LET_Kill,
	LET_Damage,
	LET_Hit,//被击打
	LET_Stun,//被Stun
	LET_PostDamage,//伤害完,但还未KB或者Stun
	LET_PreDamage,//伤害前,还未Damage
	LET_PreKill,//即将杀死
	LET_ModDamageAttr,
	LET_ModAbilityGrade,
	LET_PostCreateEo,//创建完一个Eo后调用
	LET_PreCreateEo,//创建一个Eo之前调用
	LET_ModWeaks,
	LET_NotifyWeaksBroken,
	LET_PreBlocking,
	LET_PostBlocking,
	LET_OwnerDamage,
	LET_ModBaseAttrs,
	LET_ModSPCost,

	ForceDword=0xffffffff,
};


typedef DWORD LevelEventTypeMask;

#define LevelEventMapFlag_PlayerKilling 1  //主角及其随从杀了一个单位


struct LeKill:public LevelEvent
{
	LeKill()
	{
		loTarget=NULL;
		osbSrc=NULL;
		strike=NULL;
	}

	DWORD GetType()	{		return LET_Kill;	}

	static void Send(LeKill &e);

	LevelOSB *osbSrc;
	CLevelObj *loTarget;
	LevelStrike *strike;
	LevelOpLink link;
};

struct LeDamage:public LevelEvent
{
	LeDamage()
	{
		nDmg=0;
		strike=NULL;
		osbSrc=NULL;
		loTarget=NULL;
	}

	static void Send(LevelOSB &osb,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link);

	DWORD GetType()	{		return LET_Damage;	}

	LevelOSB *osbSrc;
	CLevelObj *loTarget;

	int nDmg;
	LevelStrike *strike;
	LevelOpLink link;

};

struct LeHit:public LevelEvent
{
	LeHit()
	{
		osbSrc=NULL;
		loTarget=NULL;
	}

	DWORD GetType()	{		return LET_Hit;	}

	static void Send(LevelOSB &osbSrc,CLevelObj *loTarget,LevelOpLink &link);

	LevelOSB *osbSrc;
	CLevelObj *loTarget;
	LevelOpLink link;
};

struct LeStun:public LevelEvent
{
	LeStun()
	{
	}

	DWORD GetType()	{		return LET_Stun;	}

	static void Send(CLevelObj *loTarget,LevelOpLink &link);

	LevelOpLink link;
};


struct LePostDamage:public LevelEvent
{
	LePostDamage()
	{
		strike=NULL;
		bAbandon=FALSE;
	}

	DWORD GetType()	{		return LET_PostDamage;	}

	static BOOL Send(LevelOSB &osb,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link);

	LevelStrike *strike;
	LevelOpLink link;
	CLevelObj *loTarget;
	LevelOSB *osbSrc;

	//Feedbacks
	BOOL bAbandon;//是否要终止后续步骤
};


struct LevelOSB;
struct LePreDamage:public LevelEvent
{
	LePreDamage()
	{
		result=NULL;
		strike=NULL;
		bAbandon=FALSE;
		scaleDmg=1.0f;
		overrideDmg=-1.0f;
		osb=NULL;
		bStun=FALSE;
	}

	DWORD GetType()	{		return LET_PreDamage;	}

	//返回是否要继续
	static BOOL Send(LevelOSB &osb,CLevelObj *loTarget,DamageResult *result,LevelStrike *strike,LevelOpLink &link,BOOL bStun);

	DamageResult *result;
	LevelStrike *strike;
	LevelOSB *osb;
	CLevelObj *loTarget;
	LevelOpLink link;
	BOOL bStun;

	//Feedbacks
	BOOL bAbandon;//是否要终止后续步骤
	float scaleDmg;
	float overrideDmg;//大于等于0有效

};

struct LevelObliterateArg;
struct LePreKill:public LevelEvent
{
	LePreKill()
	{
		osbSrc=NULL;
		strike=NULL;
		argObliterate=NULL;
		bAbandon=FALSE;
		loTarget=NULL;
	}

	DWORD GetType()	{		return LET_PreKill;	}

	//返回是否要继续
	static BOOL Send(LevelOSB &osb,CLevelObj *loTarget,LevelStrike *strike,LevelObliterateArg *argObliterate,LevelOpLink &link);

	LevelOSB *osbSrc;
	LevelStrike *strike;
	CLevelObj *loTarget;
	LevelObliterateArg *argObliterate;
	LevelOpLink link;

	//Feedbacks
	BOOL bAbandon;//是否要终止后续步骤

};

struct LevelAttr_AttackMods;
struct LeModDamageAttr:public LevelEvent
{
	LeModDamageAttr()
	{
		osbSrc=NULL;
		loTarget=NULL;
		modsAttack=NULL;
		bAttackMods=FALSE;
		dmgs=NULL;
	}
	DWORD GetType()	{		return LET_ModDamageAttr;	}

	static BOOL Send(LevelOSB &osb,CLevelObj *loTarget,const LevelAttr_Damages *dmgs,LevelAttr_AttackMods *modsAttack);//返回有无mod

	LevelOSB *osbSrc;
	CLevelObj *loTarget;
	const LevelAttr_Damages *dmgs;
	LevelAttr_AttackMods *modsAttack;
	BOOL bAttackMods;
};

struct LevelAttr_BaseMod;
struct LeModBaseAttrs:public LevelEvent
{
	LeModBaseAttrs()
	{
		loSrc=NULL;
		mod=NULL;
		bMod=FALSE;
	}
	DWORD GetType()	{		return LET_ModBaseAttrs;	}

	static BOOL Send(CLevelObj *loSrc,LevelAttr_BaseMod *mod);//返回有无mod
	CLevelObj *loSrc;
	LevelAttr_BaseMod *mod;
	BOOL bMod;

};


class CLevelAbility;
struct LeModAbilityGrade:public LevelEvent
{
	LeModAbilityGrade()
	{
		grdOrg=0;
		grdAdd=0;
		ability=NULL;
	}
	DWORD GetType()	{		return LET_ModAbilityGrade;	}

	static LevelGrade Send(CLevelAbility *ability,LevelGrade grdOrg);

	LevelGrade grdOrg;
	LevelGrade grdAdd;
	CLevelAbility *ability;
};

class CLoEffectObj;
struct LePostCreateEo:public LevelEvent
{
	LePostCreateEo()
	{
		osbSrc=NULL;
		strike=NULL;
		eo=NULL;
	}

	DWORD GetType()	{		return LET_PostCreateEo;	}

	static void Send(LevelOSB &osb,CLoEffectObj *eo,LevelStrike *strike,LevelOpLink &link);

	LevelOSB *osbSrc;
	LevelStrike *strike;
	CLoEffectObj *eo;
	LevelOpLink link;
};

struct LevelRecordEo;
struct DealArg;
struct LePreCreateEo:public LevelEvent
{
	LePreCreateEo()
	{
		osbSrc=NULL;
		eoCreated=NULL;
		recEo=NULL;
	}

	DWORD GetType()	{		return LET_PreCreateEo;	}

	//返回非空指针,表示已经创建了一个Eo,不要再创建原来那个了
	static CLoEffectObj *Send(LevelOSB &osb,LevelRecordEo *rec,LevelPos3D &pos,DealArg *arg);

	LevelOSB *osbSrc;
	LevelRecordEo *recEo;
	LevelPos3D pos;
	DealArg *argDeal;

	//Feedbacks
	CLoEffectObj *eoCreated;

};

struct LevelAttr_Weaks;
struct LeModWeaks:public LevelEvent
{
	LeModWeaks()
	{
		osbSrc=NULL;
		loTarget=NULL;
		attrWeaks=NULL;
	}
	DWORD GetType()	{		return LET_ModWeaks;	}

	static void Send(LevelOSB &osb,CLevelObj *loTarget,LevelAttr_Weaks *attrWeaks);

	LevelOSB *osbSrc;
	CLevelObj *loTarget;

	LevelAttr_Weaks *attrWeaks;
};

struct LeNotifyWeaksBroken:public LevelEvent
{
	LeNotifyWeaksBroken()
	{
		loTarget=NULL;
		category=LevelWeakCategory_None;
		weaks=0;
	}
	DWORD GetType()	{		return LET_NotifyWeaksBroken;	}

	static void Send(CLevelObj *loTarget,LevelWeakCategory category,DamageAttrMask weaks);

	CLevelObj *loTarget;

	LevelWeakCategory category;
	DamageAttrMask weaks;
};

struct LevelAttr_Blocking;
struct LePreBlocking:public LevelEvent
{
	LePreBlocking()
	{
		idBuff=RecordID_Invalid;
		loSrc=NULL;
		loTarget=NULL;
		attrBlocking=NULL;
	}
	DWORD GetType()	{		return LET_PreBlocking;	}

	static LevelAttr_Blocking *Send(LevelOSB &osb,CLevelObj *loTarget,RecordID &idBuff);

	CLevelObj *loSrc;
	CLevelObj *loTarget;
	RecordID idBuff;
	LevelAttr_Blocking *attrBlocking;

};

struct LePostBlocking:public LevelEvent
{
	LePostBlocking()
	{
		loBlocker=NULL;
	}
	DWORD GetType()	{		return LET_PostBlocking;	}

	static void Send(LePostBlocking &e);

	CLevelObj *loBlocker;
	LevelOSB *osbBlockee;

	AnimTick tActivated;//举盾时间
	LevelStrike *strike;
	LevelStrike *strikeBack;
	LevelOpLink link;
	float dmgSP;
	DmgBlockType tpBlock;

};

struct LeOwnerDamage:public LevelEvent
{
	LeOwnerDamage()
	{
		osbSrc=NULL;
		loTarget=NULL;
	}

	static void Send(LevelOSB &osb,CLevelObj *loTarget,LevelOpLink &link);

	DWORD GetType()	{		return LET_OwnerDamage;	}

	LevelOSB *osbSrc;
	CLevelObj *loTarget;

	LevelOpLink link;

};

struct LeModSPCost:public LevelEvent
{
	LeModSPCost()
	{
		loSrc=NULL;
		rateReduce=0.0f;
		rateMul=1.0f;
	}
	DWORD GetType()	{		return LET_ModSPCost;	}

	static void Send(CLevelObj *loSrc, float &rate);
	CLevelObj *loSrc;
	float rateReduce;
	float rateMul;

};
