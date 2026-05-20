
#include "stdh.h"

#include "LevelObj.h"
#include "LevelEvents.h"

#include "LevelOSB.h"

#include "LevelAbility.h"

extern void LevelUtil_SendEvent(CLevelObj *lo,LevelEvent &e);

void LeKill::Send(LeKill &e)
{
	if (e.loTarget)
		LevelUtil_SendEvent(e.loTarget,e);
	if (e.osbSrc)
	{
		CLevelObj *owner=e.osbSrc->GetRootOwner();
		if (owner)
			LevelUtil_SendEvent(owner,e);
	}
}

void LeDamage::Send(LevelOSB &osbSrc,CLevelObj *loTarget,int nDmg,LevelStrike *strike,LevelOpLink &link)
{
	LeDamage e;
	e.osbSrc=&osbSrc;
	e.loTarget=loTarget;
	e.nDmg=nDmg;
	e.strike=strike;
	e.link=link;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osbSrc.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);
}


void LeHit::Send(LevelOSB &osbSrc,CLevelObj *loTarget,LevelOpLink &link)
{
	LeHit e;
	e.link=link;
	e.osbSrc=&osbSrc;
	e.loTarget=loTarget;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osbSrc.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);
}

void LeStun::Send(CLevelObj *loTarget,LevelOpLink &link)
{
	LeStun e;
	e.link=link;

	LevelUtil_SendEvent(loTarget,e);
}

BOOL LePostDamage::Send(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelOpLink &link)
{
	LePostDamage e;
	e.link=link;
	e.strike=strike;
	e.osbSrc=&osbSrc;
	e.loTarget=loTarget;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osbSrc.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);

	return e.bAbandon;
}

BOOL LePreDamage::Send(LevelOSB &osb,CLevelObj *loTarget,DamageResult *result,LevelStrike *strike,LevelOpLink &link,BOOL bStun)
{
	LePreDamage e;
	e.result=result;
	e.strike=strike;
	e.osb=&osb;
	e.loTarget=loTarget;
	e.link=link;
	e.bStun=bStun;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);
	if (e.overrideDmg>=0.0f)
		e.result->ApplyHPOverride(e.overrideDmg);
	else
	{
		if (e.scaleDmg!=1.0f)
			e.result->ApplyHPScale(e.scaleDmg);
	}

	return e.bAbandon;
}

BOOL LePreKill::Send(LevelOSB &osbSrc,CLevelObj *loTarget,LevelStrike *strike,LevelObliterateArg *argObliterate,LevelOpLink &link)
{
	LePreKill e;

	e.strike=strike;
	e.argObliterate=argObliterate;
	e.loTarget=loTarget;
	e.osbSrc=&osbSrc;
	e.link=link;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osbSrc.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);

	return e.bAbandon;
}


BOOL LeModBaseAttrs::Send(CLevelObj *loSrc,LevelAttr_BaseMod *mod)
{
	LeModBaseAttrs e;
	e.loSrc=loSrc;
	e.mod=mod;

	LevelUtil_SendEvent(loSrc,e);

	return e.bMod;

}

BOOL LeModDamageAttr::Send(LevelOSB &osb,CLevelObj *loTarget,const LevelAttr_Damages *dmgs,LevelAttr_AttackMods *modsAttack)
{
	LeModDamageAttr e;
	e.osbSrc=&osb;
	e.loTarget=loTarget;

	e.modsAttack=modsAttack;
	e.bAttackMods=FALSE;
	e.dmgs=dmgs;

	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);

	return e.bAttackMods;
}


LevelGrade LeModAbilityGrade::Send(CLevelAbility *ability,LevelGrade grdOrg)
{
	LeModAbilityGrade e;
	e.ability=ability;
	e.grdOrg=grdOrg;

	CLevelAbilities *abilities=ability->GetOwnerAbilities();
	if (abilities)
	{
		abilities->HandleEvent(e);
	}

	return e.grdOrg+e.grdAdd;
}

void LePostCreateEo::Send(LevelOSB &osb,CLoEffectObj *eo,LevelStrike *strike,LevelOpLink &link)
{
	LePostCreateEo e;

	e.strike=strike;
	e.eo=eo;
	e.osbSrc=&osb;
	e.link=link;

	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);

}

CLoEffectObj *LePreCreateEo::Send(LevelOSB &osb,LevelRecordEo *rec,LevelPos3D &pos,DealArg *arg)
{
	LePreCreateEo e;

	e.recEo=rec;
	e.pos=pos;
	e.osbSrc=&osb;
	e.argDeal=arg;

	e.eoCreated=NULL;

	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);

	return e.eoCreated;
}


void LeModWeaks::Send(LevelOSB &osb,CLevelObj *loTarget,LevelAttr_Weaks *attrWeaks)
{
	LeModWeaks e;
	e.osbSrc=&osb;
	e.loTarget=loTarget;

	e.attrWeaks=attrWeaks;

	LevelUtil_SendEvent(loTarget,e);
	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
		LevelUtil_SendEvent(owner,e);
}

void LeNotifyWeaksBroken::Send(CLevelObj *loTarget,LevelWeakCategory category,DamageAttrMask weaks)
{
	LeNotifyWeaksBroken e;
	e.loTarget=loTarget;
	e.category=category;
	e.weaks=weaks;
	LevelUtil_SendEvent(loTarget,e);
}


LevelAttr_Blocking *LePreBlocking::Send(LevelOSB &osb,CLevelObj *loTarget,RecordID &idBuff)
{
	LePreBlocking e;
	CLevelObj *owner=osb.GetRootOwner();
	if (owner)
	{
		e.loSrc=owner;
		e.loTarget=loTarget;
		LevelUtil_SendEvent(loTarget,e);
		idBuff=e.idBuff;
		return e.attrBlocking;
	}
	return NULL;
}

void LePostBlocking::Send(LePostBlocking &e)
{
	LevelUtil_SendEvent(e.loBlocker,e);
}

void LeOwnerDamage::Send(LevelOSB &osbSrc,CLevelObj *loTarget,LevelOpLink &link)
{
	LeOwnerDamage e;
	e.osbSrc=&osbSrc;
	e.loTarget=loTarget;
	e.link=link;

	LevelUtil_SendEvent(loTarget,e);
}


void LeModSPCost::Send(CLevelObj *loSrc, float &rate)
{
	LeModSPCost e;
	e.loSrc=loSrc;

	LevelUtil_SendEvent(loSrc,e);

	rate=i_math::clamp_f(1.0f-e.rateReduce,0.0f,1.0f)*e.rateMul;
}
