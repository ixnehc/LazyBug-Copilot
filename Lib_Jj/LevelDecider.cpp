/********************************************************************
	created:	2012/01/04
	file base:	LevelDecider
	author:		cxi
	
	purpose:	结算(Decide)器
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelDropper.h"

#include "LevelBehavior.h"

#include "LevelUtil.h"

#include "LevelClasses.h"

#include "LevelDecider.h"

#include "LevelEvents.h"
#include "LevelEventMap.h"

#include "LevelSkill.h"

#include "LevelOp.h"

#include "LevelBlocking.h"

#include "LoUnit.h"
#include "LoItem.h"
#include "EoAbsorb.h"
#include "EoDemonBlood.h"

#include "Buff_KB.h"
#include "Buff_KD.h"
#include "Buff_Dead.h"
#include "Buff_Birth.h"
#include "Buff_FlyBirth.h"
#include "Buff_Bleeding.h"
#include "Buff_Blocking.h"
#include "Buff_Jink.h"

#include "Random/Random.h"

#include "LevelObjResidable.h"

#include "LevelOSB.h"

#include "LevelRtnus.h"

#include "LevelRecordUnit.h"
#include "LevelRecordGlobal.h"
#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordBuff.h"
#include "LevelRecordEO.h"

#include "Ability_MagicRing.h"

#include "Log/LogDump.h"


BOOL CLevelDecider::CheckInDir(CLevelObj *loSrc,LevelPos &posTarget,float torDir)
{
	if (torDir>=i_math::Pi)
		return TRUE;

	float face=loSrc->GetFrameFace();
	LevelPos pos=loSrc->GetFramePos();

	LevelPos dir=posTarget-pos;
	float faceTo=atan2f(dir.y,dir.x);

	if (i_math::get_radian_dist(face,faceTo)<torDir)
		return TRUE;

	return FALSE;
}


BOOL CLevelDecider::CheckInRange(CLevelObj *loSrc,CLevelObj*loTarget,float range)
{
	AssertAliveObj(loSrc);
	AssertAliveObj(loTarget);

	if (loTarget->GetShapeType()==LevelObjShape_SingleCircle)
	{
		float r=loSrc->GetRadius_()+loTarget->GetRadius_()+range;
		LevelPos dir=loSrc->GetFramePos()-loTarget->GetFramePos();
		if (dir.getLengthSQ()<r*r)
			return TRUE;
	}
	else
	{
		DWORD c;
		LevelObjCircle *circles=loTarget->GetShapeCircles(c);
		for (int i=0;i<c;i++)
		{
			float r=loSrc->GetRadius_()+circles[i].radius+range;
			float dist2=(loSrc->GetFramePos()-circles[i].center).getLengthSQ();
			if (dist2<r*r)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL CLevelDecider::CheckInRange(CLevelObj *loSrc,LevelPos &pos,float range)
{
	AssertAliveObj(loSrc);


	float dist2=(loSrc->GetFramePos()-pos).getLengthSQ();
	if (dist2<=range*range)
		return TRUE;
	return FALSE;
}

BOOL CLevelDecider::CheckInRange(CLevelObj *loSrc,LevelPos3D &pos3D,float range)
{
	AssertAliveObj(loSrc);

	float dist2=(float)(loSrc->GetFramePos3D()-pos3D).getLengthSQ();
	if (dist2<=range*range)
		return TRUE;
	return FALSE;
}


BOOL CLevelDecider::CheckSkillCDOver(CLevelObj *lo,LevelRecordSkill *rec)
{
	AssertAliveObj(lo);
	CLevelSkillCDs *cds=lo->GetSkillCDs();
	if (!cds)
		return TRUE;
	return cds->CheckCDOver(rec);
}


BOOL CLevelDecider::CheckOccupyResidable(CLevelObj *lo)
{
	AssertAliveObj(lo);
	if (LevelUtil_CheckDead(lo))
		return FALSE;
	if (LevelUtil_CheckInvisible(lo))
		return FALSE;

	CLevelObjResidable *residable=lo->GetResidable();
	if (!residable)
		return FALSE;
	if (!residable->CanOccupy())
		return FALSE;

	return TRUE;
}



void CLevelDecider::Init(CLevel *level)
{
	_level=level;


}

void CLevelDecider::Clear()
{

}

BOOL CLevelDecider::CommitMiss(LevelOSB &osb,CLevelObj*target,LevelOpLink &link)
{
	AssertAliveObj(target);
	assert(!osb.IsEmpty());

	if (LevelUtil_CheckDead(target))
		return FALSE;
	if (LevelUtil_CheckInvisible(target))
		return FALSE;
	LevelOp_Miss*op=osb.NewOp<LevelOp_Miss>(link);
	op->reason=LevelOp_Miss::Miss;
	target->AddOp(op);

	return TRUE;
}

BOOL CLevelDecider::CommitSuicide(CLevelObj*lo)
{
	AssertAliveObj(lo);

	LevelAttr_Base *attrs=lo->GetAttr_Base();
	if (!attrs)
		return FALSE;

	if (LevelUtil_CheckDead(lo))
		return FALSE;

	LevelOSB osb(lo);
	LevelStrike strike;
	LevelOp_HPMod *op=osb.NewOp<LevelOp_HPMod>(LevelOpLink());
	attrs->hp.MakeMod(-10000000.0f,TRUE,op->mod);
	lo->AddOp(op);

	MakeDead(osb,lo,strike,LevelObliterateArg(),LevelOpLink());

	return TRUE;
}

void CLevelDecider::MakeCure_MaxHP(LevelOSB &osb,CLevelObj *loTarget,int nCure,LevelOpLink &link)
{
	if (nCure<0)
		return;//目前不支持扣最大HP

	if (LevelUtil_CheckDead(loTarget))
		return;

	CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			if (loTarget->GetType()==LevelObjType_Unit)
			{
				lps->base.MaxHP+=nCure;
				lps->base.SetDirtyDB_Urgent();

				((CLoUnit*)loTarget)->UpdateAttrs(link);
			}
		}
	}
	else
	{
		LevelAttr_Base *attrs=loTarget->GetAttr_Base();
		if (!attrs)
			return;

		LevelOp_HPMod *op=osb.NewOp<LevelOp_HPMod>(link);
		LevelStrike strike;
		attrs->hp.MakeMaxMod((float)nCure,op->mod);
		op->strike=strike;
		loTarget->AddOp(op);
	}

}

void CLevelDecider::MakeCure_FullSP(LevelOSB &osb,CLevelObj *loTarget,int nCure,LevelOpLink &link)
{
	if (nCure<0)
		return;//目前不支持扣最大SP

	CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			if (loTarget->GetType()==LevelObjType_Unit)
			{
				lps->base.FullSP+=nCure;
				lps->base.SetDirtyDB_Urgent();

				((CLoUnit*)loTarget)->UpdateAttrs(link);
			}
		}
	}
}

void CLevelDecider::MakeMod_Str(CLevelObj *loTarget,int nMod,LevelOpLink &link)
{
	if (nMod<=0)
		return;
	CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			if (loTarget->GetType()==LevelObjType_Unit)
			{
				lps->base.str+=nMod;
				lps->base.SetDirtyDB_Urgent();

				((CLoUnit*)loTarget)->UpdateAttrs(link);
			}
		}
	}
}

void CLevelDecider::MakeMod_Magic(CLevelObj *loTarget,int nMod,LevelOpLink &link)
{
	if (nMod<=0)
		return;
	CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			if (loTarget->GetType()==LevelObjType_Unit)
			{
				lps->base.magic+=nMod;
				lps->base.SetDirtyDB_Urgent();

				((CLoUnit*)loTarget)->UpdateAttrs(link);
			}
		}
	}
}



BOOL CLevelDecider::CommitHPMod(int dHP,LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,BOOL &nMod)
{
	nMod=0;
	AssertAliveObj(loTarget);

	assert(!osb.IsEmpty());

	LevelAttr_Base *attrs=loTarget->GetAttr_Base();
	if (!attrs)
		return FALSE;

	if (LevelUtil_CheckDead(loTarget))
		return FALSE;

	LevelOp_HPMod *op=osb.NewOp<LevelOp_HPMod>(link);
	attrs->hp.MakeMod((float)dHP,strike.IsInstant(),op->mod);
	op->strike=strike;
	loTarget->AddOp(op);

	//发送事件
	LeDamage::Send(osb,loTarget,abs(dHP),&strike,link);
	if (loTarget->IsPlayer())
	{
		CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
		if (player)
		{
			CLevelRtnus *rtnus=player->GetRtnus();
			if (rtnus)
			{
				DWORD c;
				CLevelRtnu **buf=rtnus->GetValidRetinues(c);
				if (c>0)
				{
					LeOwnerDamage eOwnerDmg;
					eOwnerDmg.osbSrc=&osb;
					eOwnerDmg.link=link;
					for(int i=0;i<c;i++)
					{
						CLoUnit *lo=buf[i]->GetLo();
						if (lo)
						{
							eOwnerDmg.loTarget=lo;
							LevelUtil_SendEvent(lo,eOwnerDmg);
						}
					}
				}
			}
		}
	}

	//Add damage source
	LevelUtil_AddEventSrc(osb,loTarget,LET_Damage);

	nMod=FloatToNearestInt(op->mod.delta);

	if (attrs->hp.GetCur_Int()<=0)
		return TRUE;
	return FALSE;
}


BOOL CLevelDecider::CommitSPMod(float dSP,LevelOSB &osb,CLevelObj*target,LevelOpLink &link,BOOL bInstant)
{
	AssertAliveObj(target);

	assert(!osb.IsEmpty());

	LevelAttr_Base *attrs=target->GetAttr_Base();
	if (!attrs)
		return FALSE;

	if (attrs->sp.GetCur_Float()<attrs->sp.GetMax_Float())
	{
		LevelOp_SPMod*op=osb.NewOp<LevelOp_SPMod>(link);
		attrs->sp.MakeMod(dSP,bInstant,op->mod);
		target->AddOp(op);
	}

	return TRUE;
}

void CLevelDecider::CommitHPAutoRecovery(CLevelObj *lo,float dt)
{
	AssertAliveObj(lo);

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		if (LevelUtil_CheckDead(lo))
			return;
		if (LevelUtil_CheckInSlates(lo))
			return;

		if (attr->hp.GetCur_Float()<attr->hp.GetMax_Float())
		{
			float rate=2.0f;
			float dHP=5.0f*(float)dt*rate;

			LevelOp_HPMod *opHP=lo->NewOp<LevelOp_HPMod>(LevelOpLink());
			attr->hp.MakeMod(dHP,FALSE,opHP->mod);

			lo->AddOp(opHP);
		}
	}
}

BOOL CLevelDecider::CommitHPAutoRecovery_New(CLevelObj *lo,float dt)
{
	AssertAliveObj(lo);

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		if (LevelUtil_CheckDead(lo))
			return FALSE;
		if (LevelUtil_CheckInSlates(lo))
			return FALSE;

		if (attr->hp.GetCur_Float()<attr->hp.GetMax_Float())
		{
			float rate=0.01f*(float)attr->hpRecover;
			float dHP=1.0f*dt*rate*attr->hp.GetMax_Float();

			LevelOp_HPMod *opHP=lo->NewOp<LevelOp_HPMod>(LevelOpLink());
			attr->hp.MakeMod(dHP,FALSE,opHP->mod);

			lo->AddOp(opHP);
			return TRUE;
		}
	}
	return FALSE;
}


void CLevelDecider::CommitSPDrain(CLevelObj *lo,float sp)
{
	AssertAliveObj(lo);

	if (!lo->IsPlayer())
		return;

	CLoUnit *loUnit=(CLoUnit *)lo;
	LevelRecordUnit *rec=loUnit->GetRec();
	if (!rec)
		return;

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		if (LevelUtil_CheckDead(lo))
			return;

		LevelOp_SPMod *opSP=lo->NewOp<LevelOp_SPMod>(LevelOpLink());
		attr->sp.MakeMod(-sp,FALSE,opSP->mod);

		lo->AddOp(opSP);
	}
}

BOOL CLevelDecider::_CheckMiss(int accu,int evade)
{
	int sum=accu+evade;
	if (sum<=0)
		return FALSE;

	float rate=((float)accu)/(float)sum;
	return !CSysRandom::Roll(rate);
}

BOOL CLevelDecider::CheckMiss(LevelAttr_Hit *hitSrc,LevelAttr_AttackMods *modsSrc,
							  LevelAttr_Evade *evadeTarget,LevelAttr_DefendMods *modsTarget)
{
	if (!hitSrc)
		return TRUE;
	if (!evadeTarget)
		return FALSE;
	LevelAttr_Hit hit=hitSrc->Resolve(modsSrc);
	LevelAttr_Evade evade=evadeTarget->Resolve(modsTarget);
	if (hit.bIgnoreEvade)
		return FALSE;
	if (evade.bImmune)
		return TRUE;
	return _CheckMiss(hit.hit,evade.evade);
}

void CLevelDecider::CalcDamage(LevelAttr_Damages *dmgsSrc,LevelAttr_AttackMods *modsSrc,
							  LevelAttr_Resists *resistsTarget,LevelAttr_DefendMods *modsTarget,LevelAttr_Blocking *blocking,DamageResult &resultDmg)
{
	resultDmg.Zero();
	if ((!dmgsSrc)||(!resistsTarget))
		return;

	const DamageAttrType tps[]={
					DamageAttrType_Pierce,
					DamageAttrType_Crush,
					DamageAttrType_Fire,
					DamageAttrType_Lightning,
					DamageAttrType_Cold,
					DamageAttrType_Poison,
					DamageAttrType_CryticalBlocking,
					DamageAttrType_SpecialA,
					DamageAttrType_Explosion,
					DamageAttrType_Smash,
	};//XXXXX: More DamageAttrType

	float fDmgSP=0.0f;
	for (int i=0;i<ARRAY_SIZE(tps);i++)
	{ 
		Damage dmg=dmgsSrc->Resolve(modsSrc,tps[i]);
		if (!dmg.IsZero())
		{
			Resist resist=resistsTarget->Resolve(modsTarget,tps[i]);
			if (!resist.bImmune)
			{
				int vDmg2=CSysRandom::RandRangeInt<int>((int)dmg.lo,(int)dmg.hi+1);
				float reduce=0.0075f*((float)resist.def)/10.0f;//(((float)grade)+9.0f);
				if (reduce>0.75f)
					reduce=0.75f;
				float dmgCur=(1.0f-reduce)*(float)vDmg2;
				if (dmgCur>0.0f)
					resultDmg.maskHP|=(1<<tps[i]);
				resultDmg.bufHP[tps[i]]=dmgCur;

				if (blocking)
					fDmgSP+=blocking->converts[tps[i]]*dmgCur;
			}
		}
	}

	resultDmg.sp=fDmgSP;
}

BOOL CLevelDecider::CheckJink(CLevelObj *loTarget,LevelStrike &strike,DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,DamageAttrMask &weaksBroken,LevelWeakCategory &broken)
{
	LevelWeakCategory toBreak;
	toBreak=LevelWeakCategory_Jink;

	DamageAttrMask weaksToBreak=weaks->Cur().weaks[toBreak];

	weaksBroken=(~(maskDmg^weaksToBreak))&maskDmg;
	if (weaksBroken!=0)
	{
		broken=toBreak;
		return TRUE;
	}
	return FALSE;
}


BOOL CLevelDecider::CheckStun(CLevelObj *loTarget,LevelStrike &strike,DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,
							  DamageAttrMask &weaksBroken,LevelWeakCategory &broken)
{
	LevelFace faceTarget=loTarget->GetFrameFace();
	LevelFace faceStrike=strike.GetFace();

	LevelWeakCategory toBreak;
	if (TRUE)
	{
		float degree=i_math::get_radian_dist(faceTarget,faceStrike)*i_math::GRAD_PI;
		if (degree<60.0f)
			toBreak=LevelWeakCategory_StunBack;
		else
		{
			if (degree<120.0f)
				toBreak=LevelWeakCategory_StunSide;
			else
				toBreak=LevelWeakCategory_StunFwd;
		}
	}

	DamageAttrMask weaksToBreak=weaks->Cur().weaks[toBreak];

	weaksBroken=(~(maskDmg^weaksToBreak))&maskDmg;
	if (weaksBroken!=0)
	{
		broken=toBreak;
		return TRUE;
	}
	return FALSE;
}

BOOL CLevelDecider::CheckKB(DamageAttrMask maskDmg,LevelAttr_Weaks*weaks,DamageAttrMask &weaksBroken)
{
	DamageAttrMask weaksToBreak=weaks->Cur().weaks[LevelWeakCategory_KB];

	weaksBroken=(~(maskDmg^weaksToBreak))&maskDmg;
	if (weaksBroken!=0)
		return TRUE;
	return FALSE;
}

BOOL CLevelDecider::CanStartSkill(CLevelObj *lo)
{
	AssertAliveObj(lo);

	if (!lo->IsAlive())
		return FALSE;
	CLevelBuffs *buffs=lo->GetBuffs();
	if (buffs)
	{
		if (buffs->TestFlag(BuffFlag_Dead|BuffFlag_Birth|BuffFlag_LayDown|BuffFlag_Pausing|BuffFlag_PausingAnim))
			return FALSE;
	}

	return TRUE;

}

void CLevelDecider::MakeCure(LevelOSB &osb,CLevelObj *target,int nCure,LevelStrike &strike,LevelOpLink &link)
{
	int nMod;
	CommitHPMod(nCure,osb,target,strike,link,nMod);
}


BOOL CLevelDecider::MakeHit(LevelOSB &osb,CLevelObj*target,LevelAttr_Hit *hit,LevelOpLink &link)
{
	if (!hit->bValid)
		return TRUE;
	LeHit::Send(osb,target,link);
	LevelUtil_AddEventSrc(osb,target,LET_Hit);

	LevelAttr_Evade *evadeTarget=target->GetAttr_Evade();
	LevelAttr_DefendMods *modsTarget=target->GetAttr_DefendMods();
	LevelAttr_AttackMods *modsSrc=NULL;
	if (TRUE)
	{
		CLevelObj *owner=osb.GetOwner();
		if (owner)
			modsSrc=owner->GetAttr_AttackMods();
	}

	//计算命中与否
	if (CheckMiss(hit,modsSrc,evadeTarget,modsTarget))
	{
		CommitMiss(osb,target,link);
		return FALSE;
	}

	return TRUE;
}

void CLevelDecider::CommitVitaLost(LevelOSB &osb,CLevelObj *lo,float ratioToLost,LevelOpLink &link)
{
	AssertAliveObj(lo);

	if (lo->IsPlayer())
	{
		LevelAttr_Base *attr=lo->GetAttr_Base();
		if (attr)
		{
			CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
			if (player)
			{
				LevelPlayerStates *lps=player->GetLPS();
				if (lps)
				{
					BYTE vita=(BYTE)(((float)lps->base.vita_)*(1.0f-ratioToLost));
					if (vita<lps->base.vita_)
					{
						LevelOp_VitaMod *op=osb.NewOp<LevelOp_VitaMod>(link);
						op->delta=((char)vita)-(char)(lps->base.vita_);
						lo->AddOp(op);

						lps->base.vita_=vita;
						lps->base.SetDirtyDB_Urgent();
					}
				}
			}
		}
	}
}

BOOL CLevelDecider::MakeBlock(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,
									 float dmgSP,DmgBlockType tpBlock,RecordID idBlockingBuff,LevelOpLink &link)
{
	CLevelBlocking *blocking=target->GetBlocking();
	if (blocking)
	{
		LevelPos dirStrike=strike.GetDir();
		if (blocking->CanBlock(dirStrike,_level->GetT_()))
		{
			blocking->AddBlock(dirStrike,_level->GetT_());

			if (idBlockingBuff!=RecordID_Invalid)
			{
				BuffArg_Blocking arg;
				arg.strike=strike;

				MakeBuff(osb,target,idBlockingBuff,0,&arg,link);

				if (TRUE)
				{
					LevelAttr_Base *attr=target->GetAttr_Base();
					if (attr)
					{
// 						if (attr->sp_.GetCur_Int()>=dmgSP)
// 							bBlocked=TRUE;

						float spCost=dmgSP;
						float spMaxCost=0.0f;
						LevelUtil_ModSPCost(target,spCost,spMaxCost);

						//Mod SP
						if (spCost>0.0f)
						{
							LevelOp_SPMod *op=target->NewOp<LevelOp_SPMod>(link);
							attr->sp.MakeMod(-(float)spCost,TRUE,op->mod);
							target->AddOp(op);
						}
					}
				}

				if (TRUE)
				{
					LePostBlocking e;
					e.loBlocker=target;
					e.osbBlockee=&osb;
					e.tActivated=blocking->GetActivatedTime();
					e.strike=&strike;
					LevelStrike strikeBack=strike;
					strikeBack.SetDir(-strike.GetDir());
					strikeBack.idSrc=target->GetID();
					e.strikeBack=&strikeBack;
					e.link=link;
					e.dmgSP=dmgSP;
					e.tpBlock=tpBlock;
					LePostBlocking::Send(e);
				}

// 				CLevelObj *loOwner=osb.GetRootOwner();
// 				if (loOwner->GetType()==LevelObjType_Unit)
// 				{
// 					LevelStrike strikeBack=strike;
// 					strikeBack.SetDir(-strike.GetDir());
// 					strikeBack.idSrc=target->GetID();
// 					MakeKB(LevelOSB(target),loOwner,strikeBack,link);
// 				}
			}
			return TRUE;
		}
	}
	return FALSE;
}


//返回dmg
float CLevelDecider::CommitPain(LevelOSB &osb,CLevelObj *loTarget,LevelStrike &strike,LevelOpLink &link,DamageAttrMask weaksBroken,DamageResult &resultDmg)
{
	LevelAttr_Base *attrBase=loTarget->GetAttr_Base();
	if (!attrBase)
		return 0.0f;

	UnitPainInfo *infoPain=NULL;
	if (loTarget->GetType()==LevelObjType_Unit)
	{
		LevelRecordUnit *rec=((CLoUnit*)loTarget)->GetRec();
		if (rec)
			infoPain=&rec->pain;
	}
	if (!infoPain)
		return 0.0f;

	if (!infoPain->bEnable)
		return 0.0f;

	float deltaPain=0.0f;
	if (TRUE)
	{
		const DamageAttrType tps[]=
		{
			DamageAttrType_Pierce,
			DamageAttrType_Crush,
			DamageAttrType_Fire,
			DamageAttrType_Lightning,
			DamageAttrType_Cold,
			DamageAttrType_Poison,
			DamageAttrType_CryticalBlocking,
			DamageAttrType_SpecialA,
			DamageAttrType_Explosion,
			DamageAttrType_Smash,
		};//XXXXX: More DamageAttrType
		for (int i=0;i<ARRAY_SIZE(tps);i++)
		{ 
			if (weaksBroken&(1<<tps[i]))
				deltaPain+=(float)resultDmg.bufHP[tps[i]];
		}
	}

	float vPain=LevelUtil_CalcCurPain(attrBase->pain,*infoPain,loTarget->GetLevel()->GetT_());
	if (vPain>=(float)infoPain->full)
		return 0.0f;

	vPain+=deltaPain;

	BOOL bPainOverflow=FALSE;

	if (vPain>=(float)infoPain->full)
	{
		vPain=1000000.0f;
		bPainOverflow=TRUE;
	}

	LevelOp_PainMod *op=loTarget->NewOp<LevelOp_PainMod>(link);
	op->pain=vPain;
	op->tServer=loTarget->GetLevel()->GetT_();
	loTarget->AddOp(op);

	attrBase->pain.v=vPain;
	attrBase->pain.t=loTarget->GetLevel()->GetT_();

	float dmg=0.0f;
	if (bPainOverflow)
		dmg=infoPain->rateHPDmg*attrBase->hp.GetMax_Float();

	return dmg;
}


void CLevelDecider::MakeDamage(LevelOSB &osb,CLevelObj *target,
								LevelStrike &strike,LevelAttr_Damages *dmgs,LevelOpLink &link,DmgBlockType tpBlock,float multiply)
{
	AssertAliveObj(target);
	if (!dmgs)
		return;

	if (LevelUtil_CheckDamageImmune(target))
		return;

	LevelAttr_Resists *resistsTarget=target->GetAttr_Resists();
	LevelAttr_DefendMods *modsTarget=target->GetAttr_DefendMods();
	LevelAttr_AttackMods *modsSrc=NULL;
	if (dmgs->filtersMod&DamageModFilter_RootOwner)
	{
		CLevelObj *owner=osb.GetRootOwner();
		if (owner)
			modsSrc=owner->GetAttr_AttackMods();
	}

	LevelAttr_AttackMods modsSrc2;
	if (dmgs->filtersMod&DamageModFilter_RootOwner)
	{
		if (LeModDamageAttr::Send(osb,target,dmgs,&modsSrc2))
		{
			if (modsSrc)
				modsSrc2.MergeFrom(*modsSrc);
			modsSrc=&modsSrc2;
		}
	}

	RecordID idBlockingBuff=RecordID_Invalid;
	LevelAttr_Blocking *attrBlocking=NULL;
	if (tpBlock!=DmgBlockType_NotBlockable)
		attrBlocking=LePreBlocking::Send(osb,target,idBlockingBuff);

	DamageResult resultDmg;
	CalcDamage(dmgs,modsSrc,resistsTarget,modsTarget,attrBlocking,resultDmg);
	strike.maskDmg=(WORD)resultDmg.maskHP;//XXXXX: More DamageAttrType

	BOOL bStun=FALSE;
	float multiplyStun=1.0f;
	DamageAttrMask weaksStunBroken;
	LevelWeakCategory catStunBroken;
	LevelAttr_Weaks weaks;
	if (resultDmg.maskHP!=0)
	{
		LevelAttr_Weaks *weaksOrg=target->GetAttr_Weaks();
		if (weaksOrg)
			weaks.CopyFrom(*weaksOrg);

		LeModWeaks::Send(osb,target,&weaks);

		LevelAttr_WeaksMod *modWeaks=target->GetAttr_WeaksMod();
		if (modWeaks)
			modWeaks->Apply(weaks);

		weaks.Cur().MergeFrom(DamageAttrType_CryticalBlocking);

		if (TRUE)
		{
			DamageAttrMask weaksBroken=0;
			LevelWeakCategory broken=LevelWeakCategory_None;
			if (CheckJink(target,strike,resultDmg.maskHP,&weaks,weaksBroken,broken))
			{
				LeNotifyWeaksBroken::Send(target,broken,weaksBroken);
				if (LevelBuffID_Invalid!=MakeJink(osb,target,strike,link))
					return;
			}
		}

		if (TRUE)
		{
			DamageAttrMask weaksBroken=0;
			LevelWeakCategory broken=LevelWeakCategory_None;
			if (CheckStun(target,strike,resultDmg.maskHP,&weaks,weaksBroken,broken))
			{
				LeNotifyWeaksBroken::Send(target,broken,weaksBroken);

				bStun=TRUE;
				multiplyStun=1.5f;
				catStunBroken=broken;
				weaksStunBroken=weaksBroken;
			}
		}
	}

	resultDmg.ApplyHPScale(multiply*multiplyStun);

	if (tpBlock!=DmgBlockType_NotBlockable)
	{
		if (MakeBlock(osb,target,strike,resultDmg.GetSPTotal(),tpBlock,idBlockingBuff,link))
			return;
	}

	if (LePreDamage::Send(osb,target,&resultDmg,&strike,link,bStun))
		return;

	if (bStun)
	{
		float dmgFromPain=CommitPain(osb,target,strike,link,weaksStunBroken,resultDmg);
		if (dmgFromPain>0.0f)
		{
			MakePainDrop(osb,target,strike,link);
			resultDmg.ApplHPDelta(dmgFromPain);
		}
	}

	BOOL bDead=FALSE;

	int nMod;
	if (CommitHPMod(-resultDmg.GetHPTotal_Int(),osb,target,strike,link,nMod))
	{
		LevelObliterateArg argObliterate;
		if (!LePreKill::Send(osb,target,&strike,&argObliterate,link))
		{
			bDead=TRUE;
			MakeDead(osb,target,strike,argObliterate,link);
			MakeDeathDrop(osb,target,strike,link);
		}
	}
	
	if (!bDead)
	{
		BOOL bStunned=FALSE;
		if ((nMod<0)||bStun)
		{
			if (!LePostDamage::Send(osb,target,&strike,link))
			{
				if (bStun)
				{
					if (LevelBuffID_Invalid==MakeSkillStun(osb,target,strike,catStunBroken,weaksStunBroken,link))
					{
						DamageAttrMask weaksBroken=0;
						if (CheckKB(resultDmg.maskHP,&weaks,weaksBroken))
						{
							LeNotifyWeaksBroken::Send(target,LevelWeakCategory_KB,weaksBroken);
							MakeKB(osb,target,strike,link);
						}
						else
							MakeStun(osb,target,strike,link);
						bStunned=TRUE;
					}
				}
			}
			CommitVitaLost(osb,target,0.33f,link);
		}
	}
	else
		CommitVitaLost(osb,target,1.0f,link);
}

void CLevelDecider::MakeSuck_HP(LevelOSB &osb,CLevelObj *target,int nSuck,LevelOpLink &link)
{
	LevelStrike strike;
	int nMod;
	if (CommitHPMod(-nSuck,osb,target,strike,link,nMod))
	{
		LevelObliterateArg argObliterate;
		if (!LePreKill::Send(osb,target,&strike,&argObliterate,link))
		{
			MakeDead(osb,target,strike,argObliterate,link);
			MakeDeathDrop(osb,target,strike,link);
		}
	}

	if (nMod>0)
		MakeCure(osb,osb.GetOwner(),-nMod,strike,link);
}


// void CLevelDecider::MakeDamage(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelAttackAddOn &addon,LevelOpLink &link)
// {
// 	AssertAliveObj(target);
// 
// 	LevelAttack atk=osb.GetOwner()->GetAttack(strike.tp);
// 	addon.Apply(atk);
// 	LevelDefence def=target->GetDefence(strike.tp);
// 
// 	return MakeDamage(osb,target,atk,def,strike,link);
// }
// 
// 
// void CLevelDecider::MakeDamage(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link)
// {
// 	AssertAliveObj(target);
// 
// 	LevelAttack atk=osb.GetOwner()->GetAttack(strike.tp);
// 	LevelDefence def=target->GetDefence(strike.tp);
// 
// 	return MakeDamage(osb,target,atk,def,strike,link);
// }

LevelBuffID CLevelDecider::MakeStun(LevelOSB &osb,CLevelObj *target,LevelStrike &strike,LevelOpLink &link)
{
	LeStun::Send(target,link);
	BuffArg_Stun arg;
	arg.strike=strike;

	RecordID id=target->GetBuffID_Stun();

	if (id==RecordID_Invalid)
		id=_level->GetRecords()->GetGlobal()->idDefBuff_Stun;

	return MakeBuff(osb,target,id,0,&arg,link);
}

LevelBuffID CLevelDecider::MakeKB(LevelOSB &osb,CLevelObj *target,LevelStrike &strike,LevelOpLink &link)
{
	BuffArg_KB arg;
	arg.strike=strike;

	RecordID id=target->GetBuffID_KB();

	if (id==RecordID_Invalid)
		id=_level->GetRecords()->GetGlobal()->idDefBuff_KB;

	return MakeBuff(osb,target,id,0,&arg,link);
}

LevelBuffID CLevelDecider::MakeKD(LevelOSB &osb,CLevelObj *target,LevelBuffTypeID_ tid,i_math::vector2df &dir,LevelOpLink &link,AnimTick dur)
{
	BuffArg_KD param;
	param.dir=dir;

	return MakeBuff(osb,target,tid,dur,&param,link);
}

LevelBuffID CLevelDecider::MakeDead(LevelOSB &osb,CLevelObj *target,LevelStrike &strike,LevelObliterateArg &argObliterate,LevelOpLink &link)
{
	AssertAliveObj(target);

	CLevelObj *owner=osb.GetOwner();
	if (owner)
	{
		LevelPlayerID idPlayer=owner->GetPlayerID();
		if (idPlayer<LEVEL_MAX_PLAYER)
		{
			if (owner->IsPlayer())
				_level->GetEventMap()->AddPlayerKilling(idPlayer,owner->GetFramePos(),10.0f);
			else
				_level->GetEventMap()->AddUnitKilling(idPlayer,owner->GetFramePos(),10.0f);
		}
	}

	BuffArg_Dead arg;
	arg.strike=&strike;
	arg.argObliterate=&argObliterate;
	arg.link=link;
	arg.osbSrc=&osb;

	RecordID id=target->GetBuffID_Dead();

	if (id==RecordID_Invalid)
		id=_level->GetRecords()->GetGlobal()->idDefBuff_Dead;

	LevelBuffID idBuff=MakeBuff(osb,target,id,ANIMTICK_INFINITE,&arg,link);

	LeKill e;
	e.loTarget=target;
	e.osbSrc=&osb;
	e.link=link;
	e.strike=&strike;

	LeKill::Send(e);

	return idBuff;
}

LevelBuffID CLevelDecider::MakeBleed(LevelOSB &osb,CLevelObj *target,LevelStrike &strike,LevelOpLink &link)
{
	AssertAliveObj(target);

	BuffArg_Bleeding arg;

	RecordID id=target->GetBuffID_Bleed();

	if (id==RecordID_Invalid)
		id=_level->GetRecords()->GetGlobal()->idDefBuff_Bleed;

	return MakeBuff(osb,target,id,ANIMTICK_INFINITE,&arg,link);
}


void CLevelDecider::RemoveBuff(LevelOSB &osb,CLevelObj *loTarget,CLevelBuff *buff,LevelOpLink &link)
{
	LevelOp_ModBuff *op=osb.NewOp<LevelOp_ModBuff>(link);

	if (buff->NeedSync())
		op->removes.push_back(buff->GetID());
	buff->AddRef();
	buff->Destroy();

	if (!op->IsEmpty())
		loTarget->AddOp(op);
	else
	{
		Safe_Class_Delete(op);
	}

}  


LevelBuffID CLevelDecider::MakeBirth(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,LevelOpLink &link)
{
	BuffArg_Birth arg;
	LevelOp_AddBuff *op=osb.NewOp<LevelOp_AddBuff>(link);
	arg.descOp=op->GetDesc();
	Safe_Class_Delete(op);

	return MakeBuff(osb,target,tid,0,&arg,link);
}

LevelBuffID CLevelDecider::MakeFlyBirth(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,LevelPos3D &pos,LevelPos &dir,LevelOpLink &link)
{
	BuffArg_FlyBirth arg;
	arg.posInit=pos;
	arg.dir=dir;
	LevelOp_AddBuff *op=osb.NewOp<LevelOp_AddBuff>(link);
	arg.descOp=op->GetDesc();
	Safe_Class_Delete(op);

	return MakeBuff(osb,target,tid,0,&arg,link);
}


BOOL CLevelDecider::_ResolveBuffConflicts(CLevelBuffs *buffs,LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur,std::vector<CLevelBuff*> &removes)
{
	removes.clear();

	extern CLevelBuff*NewLevelBuff(LevelRecordBuff *rec);
	CLevelBuff *buffNew=NewLevelBuff(rec);
	if (!buffNew)
		return FALSE;

	for (int i=0;i<buffs->_buffs.size();i++)
	{
		CLevelBuff *buff=buffs->_buffs[i];
		if (!buff->IsAlive())
			continue;

		if (buff->Merge(rec,arg,dur))
		{
			Safe_Class_Delete(buffNew);
			return FALSE;
		}

		CLevelBuff::ConflictResult result=buffNew->CheckConflict(buff);
		if (result==CLevelBuff::Conflict_None)
			continue;
		if (result==CLevelBuff::Conflict_Forbid)
		{
			Safe_Class_Delete(buffNew);
			return FALSE;
		}
		if (result==CLevelBuff::Conflict_Replace)
			removes.push_back(buff);
	}

	Safe_Class_Delete(buffNew);

 	return TRUE;
}

LevelBuffID CLevelDecider::MakeBuff(LevelOSB &osb,CLevelObj*target,LevelRecordBuff *rec,AnimTick durDefault,LevelBuffArg *arg,LevelOpLink &link)
{
	CLevelBuffs *buffs=target->GetBuffs();

	if (!buffs)
		return LevelBuffID_Invalid;


	AnimTick dur=durDefault;
	if (dur<=0)
		dur=rec->Dur;

	if (dur==0)
	{
		LOG_DUMP_1P("CLevelDecider",Log_Error,"尝试添加一个持续时间为0的Buff:%s",rec->Name.c_str());
		return LevelBuffID_Invalid;
	}

	_temp.clear();//要Remove的Buff

	if (!_ResolveBuffConflicts(buffs,rec,arg,dur,_temp))
		return LevelBuffID_Invalid;


	//作真正的改变
	LevelBuffID idBuff=LevelBuffID_Invalid;
	if (_temp.size()==0)
	{
		CLevelBuff *buff=buffs->CreateBuff(rec,dur,arg);

		if (buff)
		{
			if (buff->NeedSync())
			{
				LevelOp_AddBuff *op=osb.NewOp<LevelOp_AddBuff>(link);
				buff->ToData(op->data);
				target->AddOp(op);
			}

			idBuff=buff->GetID();
			SAFE_RELEASE(buff);
		}
	}
	else
	{
		LevelOp_ModBuff *op=osb.NewOp<LevelOp_ModBuff>(link);
		for (int i=0;i<_temp.size();i++)
		{
			if (_temp[i]->NeedSync())
				op->removes.push_back(_temp[i]->GetID());
			_temp[i]->AddRef();
			_temp[i]->Destroy();
		}

		CLevelBuff *buff=buffs->CreateBuff(rec,dur,arg);
		if (buff)
		{

			if (buff->NeedSync())
				buff->ToData(op->data);

			if (!op->IsEmpty())
				target->AddOp(op);
			else
				Safe_Class_Delete(op);

			idBuff=buff->GetID();
			SAFE_RELEASE(buff);
		}
		else
			Safe_Class_Delete(op);
	}

	return idBuff;
}


LevelBuffID CLevelDecider::MakeBuff(LevelOSB &osb,CLevelObj*target,LevelBuffTypeID_ tid,AnimTick durDefault,LevelBuffArg *arg,LevelOpLink &link)
{
	AssertAliveObj(target);

	LevelRecordBuff *rec=_level->GetRecords()->GetBuff(tid);
	if (!rec)
		return LevelBuffID_Invalid;

	return MakeBuff(osb,target,rec,durDefault,arg,link);
}


LevelBuffID CLevelDecider::MakeBuff(CLevelObj*loTarget,LevelBuffTypeID_ tid,AnimTick durDefault,LevelBuffArg *arg,BOOL bNeedSync)
{
	AssertAliveObj(loTarget);

	CLevelBuffs *buffs=loTarget->GetBuffs();
	if (!buffs)
		return LevelBuffID_Invalid;

	LevelRecordBuff *rec=_level->GetRecords()->GetBuff(tid);
	if (!rec)
		return LevelBuffID_Invalid;

	AnimTick dur=durDefault;
	if (dur<=0)
		dur=rec->Dur;
	if (dur==0)
	{
		LOG_DUMP_1P("CLevelDecider",Log_Error,"尝试添加一个持续时间为0的Buff:%s",rec->Name.c_str());
		return LevelBuffID_Invalid;
	}

	_temp.clear();//要Remove的Buff

	if (!_ResolveBuffConflicts(buffs,rec,arg,dur,_temp))
		return LevelBuffID_Invalid;

	//作真正的改变
	LevelBuffID idBuff=LevelBuffID_Invalid;
	if (_temp.size()==0)
	{
		CLevelBuff *buff=buffs->CreateBuff(rec,dur,arg);

		if (buff)
		{
			if (bNeedSync)
			{
				if (buff->NeedSync())
				{
					LevelOp_AddBuff *op=loTarget->NewOp<LevelOp_AddBuff>(LevelOpLink());
					buff->ToData(op->data);
					loTarget->AddOp(op);
				}
			}

			idBuff=buff->GetID();
			SAFE_RELEASE(buff);
		}
	}
	else
	{

		LevelOp_ModBuff *op=NULL;
		if (bNeedSync)
			op=loTarget->NewOp<LevelOp_ModBuff>(LevelOpLink());

		for (int i=0;i<_temp.size();i++)
		{
			if (op)
			{
				if (_temp[i]->NeedSync())
					op->removes.push_back(_temp[i]->GetID());
			}
			_temp[i]->AddRef();
			_temp[i]->Destroy();
		}

		CLevelBuff *buff=buffs->CreateBuff(rec,dur,arg);

		if  (buff)
		{
			if (op)
			{
				if (buff->NeedSync())
					buff->ToData(op->data);

				if (!op->IsEmpty())
					loTarget->AddOp(op);
				else
					Safe_Class_Delete(op);
			}

			idBuff=buff->GetID();
			SAFE_RELEASE(buff);
		}
		else
			Safe_Class_Delete(op);
	}

	return idBuff;
}



void CLevelDecider::_MakeResAbsorb(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,RecordID eoAbsorb,int amount)
{
	if (amount>0)
	{
		CLevelObj *loOwner=osb.GetRootOwner();
		if (loOwner)
		{
			CLevelRecords *records=_level->GetRecords();
			if (eoAbsorb!=RecordID_Invalid)
			{
				LevelRecordEo *rec=records->GetEo(eoAbsorb);
				if (rec)
				{
					if (rec->param->GetEoClass()->IsSameWith(Class_Ptr2(EoAbsorb)))
					{
						for (int i=0;i<amount;i++)
						{
							EoAbsorb*eo=NULL;
							eo=(EoAbsorb*)_level->CreateObj(rec->param->GetEoClass());
							if (eo)
							{
								LevelGrade grd=0;
								eo->PostCreate(loOwner->GetPlayerID(),eoAbsorb,loTarget->GetFramePos(),LevelPos(0,0),grd,osb,link);

								EoAbsorbArg arg;
								arg.idSrc=loTarget->GetID();
								arg.idTarget=loOwner->GetID();
								arg.dirInitial=strike.GetDir();
								arg.strInitial=strike.GetStr();

								eo->Init(arg);

								_level->AddToActives(eo);

								SAFE_RELEASE(eo);
							}
						}
					}
				}
			}
		}
	}
}

void CLevelDecider::_MakeDemonBloodDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link,RecordID eoDemonBlood,int amount)
{
	if (amount>0)
	{
		CLevelObj *loOwner=osb.GetRootOwner();
		if (loOwner)
		{
			CLevelRecords *records=_level->GetRecords();
			if (eoDemonBlood!=RecordID_Invalid)
			{
				LevelRecordEo *rec=records->GetEo(eoDemonBlood);
				if (rec)
				{
					if (rec->param->GetEoClass()->IsSameWith(Class_Ptr2(EoDemonBlood)))
					{
						for (int i=0;i<amount;i++)
						{
							EoDemonBlood*eo=NULL;
							eo=(EoDemonBlood*)_level->CreateObj(rec->param->GetEoClass());
							if (eo)
							{
								LevelGrade grd=0;
								eo->PostCreate(loOwner->GetPlayerID(),eoDemonBlood,loTarget->GetFramePos(),LevelPos(0,0),grd,osb,link);

								EoDemonBloodArg arg;
								arg.idSrc=loTarget->GetID();
								arg.dirInitial=strike.GetDir();
								arg.strInitial=strike.GetStr();

								eo->Init(arg);

								_level->AddToActives(eo);

								SAFE_RELEASE(eo);
							}
						}
					}
				}
			}
		}
	}
}


void CLevelDecider::MakePainDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link)
{
	CLevelRecords *records=_level->GetRecords();
	LevelRecordGlobal *recGlobal=records->GetGlobal();

	_MakeDemonBloodDrop(osb,loTarget,strike,link,recGlobal->eoDropDemonBlood,1);
	_level->GetResPiles().Deposit(LevelObjID_Invalid,LevelResource_DemonBlood,1);

}



void CLevelDecider::MakeDeathDrop(LevelOSB &osb,CLevelObj*loTarget,LevelStrike &strike,LevelOpLink &link)
{
	AssertAliveObj(loTarget);
	assert(!osb.IsEmpty());

	CLevelDropper *dropper=_level->GetDropper();
	if (!dropper)
		return;

	dropper->MakeDrop(osb.GetOwner(),loTarget);

	DWORD nResult;
	LevelItemState *states=dropper->GetResults(nResult);

	for (int i=0;i<nResult;i++)
	{
		LevelPos pos=LevelUtil_FindPosAround(loTarget->GetFramePos(),2.0f,_level,4);
		if (!(pos==LevelPos_Invalid))
		{
			CLoItem* lo=(CLoItem*)_level->CreateObj(Class_Ptr2(CLoItem));

			lo->PostCreate(&states[i],pos,osb,link);

			_level->AddToActives(lo);

			SAFE_RELEASE(lo);
		}
	}

	if (TRUE)
	{
		int amountCrystal=dropper->GetResultCrystal();
		if (amountCrystal>0)
			_level->GetResPiles().Deposit(loTarget->GetID(),LevelResource_Crystal,amountCrystal);
	}

	//Resource Absorb
	if (TRUE)
	{
		CLevelRecords *records=_level->GetRecords();
		LevelRecordGlobal *recGlobal=records->GetGlobal();

		CLevelObj *loOwner=osb.GetRootOwner();
		if (loOwner)
		{
			if (dropper->GetResultSoul()>0)
			{
				if (LevelUtil_ExistArtifact(loOwner,LevelArtifact_SoulStone))
					_MakeResAbsorb(osb,loTarget,strike,link,recGlobal->eoDropSoul,dropper->GetResultSoul());
			}
			if (dropper->GetResultMP()>0)
			{
				if (LevelUtil_ExistArtifact(loOwner,LevelArtifact_MagicRing))
					_MakeResAbsorb(osb,loTarget,strike,link,recGlobal->eoDropMP,dropper->GetResultMP());
			}
		}
// 		_MakeResAbsorb(osb,loTarget,strike,link,recGlobal->eoDropFire,dropper->GetResultFire());
	}

}

void CLevelDecider::GatherResPile(CLevelObj *lo,LevelObjID idOwner,LevelResourceType tp,int amount)
{
	AssertAliveObj(lo);

	CLevelResPiles &respiles=_level->GetResPiles();

	int amountGathered=respiles.Fetch(idOwner,tp,amount);
	if (amountGathered>0)
	{
		LevelAttr_Resource *attrRes=lo->GetAttr_Resource();
		if (!attrRes)
			return;

		LevelPlayerStates *lps=LevelUtil_GetLPS(_level,lo);
		if (lps)
		{
			LevelOp_ResouceMod *op=lo->NewOp<LevelOp_ResouceMod>(LevelOpLink());
			attrRes->Get(tp)->MakeMod((float)amountGathered,FALSE,op->mod);
			op->tpRes=tp;

			lo->AddOp(op);

			(*lps->base.GetRes(tp))+=FloatToNearestInt(op->mod.delta);

			lps->base.SetDirtyDB_Low();
		}
	}
}


void CLevelDecider::GatherRes(CLevelObj *lo,LevelObjID idItem)
{
	AssertAliveObj(lo);

	CLoItem*loItem=_level->GetIDs()->LoFromID<CLoItem>(idItem);
	if (!loItem)
		return;

	LevelRecordItem *rec=loItem->GetRec();
	if (!rec)
		return;
	LevelRecordItemClass *recClass=_level->GetRecords()->GetItemClass(rec->clss);
	if (!recClass)
		return;
	if (recClass->tpRes==LevelResource_None)
		return;

	LevelAttr_Resource *attrRes=lo->GetAttr_Resource();
	if (!attrRes)
		return;

	float radiusGather=8.0f;
	switch(recClass->tpRes)
	{
		case LevelResource_Gold:
		case LevelResource_Gem:
		case LevelResource_Crystal:
		{
			if (lo->GetFramePos().getDistanceSQFrom(loItem->GetFramePos())<radiusGather*radiusGather)
			{
				int delta=loItem->GetState().nStack;
				LevelOp_ResouceMod *op=lo->NewOp<LevelOp_ResouceMod>(LevelOpLink());
				attrRes->Get(recClass->tpRes)->MakeMod((float)delta,FALSE,op->mod);
				op->tpRes=recClass->tpRes;

				lo->AddOp(op);

				LevelPlayerStates *lps=LevelUtil_GetLPS(_level,lo);
				if (lps)
				{
					(*lps->base.GetRes(recClass->tpRes))+=FloatToNearestInt(op->mod.delta);

					lps->base.SetDirtyDB_Low();
				}

			}
			break;
		}
	}

	loItem->DeferDestroy();

}

void CLevelDecider::MakeSoulRecover(CLevelObj *lo,int soul)
{
	AssertAliveObj(lo);
	MakeResModify(lo,LevelResource_Soul,soul);
}


void CLevelDecider::MakeResModify(LevelOSB &osb,CLevelObj *loTarget,LevelResourceType tpRes,int mod,LevelOpLink &link)
{
	AssertAliveObj(loTarget);
	assert(!osb.IsEmpty());

	if (mod==0)
		return;

	LevelAttr_Resource *attrRes=loTarget->GetAttr_Resource();
	if (!attrRes)
		return;

	if (TRUE)
	{
		int delta=mod;
		LevelOp_ResouceMod *op=osb.NewOp<LevelOp_ResouceMod>(link);
		Lav *lav=attrRes->Get(tpRes);
		if (!lav)
			return;
		lav->MakeMod((float)delta,FALSE,op->mod);
		op->tpRes=tpRes;

		loTarget->AddOp(op);

		LevelPlayerStates *lps=LevelUtil_GetLPS(_level,loTarget);
		if (lps)
		{
			DWORD *v=lps->base.GetRes(tpRes);
			if (v)
			{
				*v+=FloatToNearestInt(op->mod.delta);
				lps->base.SetDirtyDB_Low();
			}
		}
	}
}


void CLevelDecider::MakeResModify(CLevelObj *lo,LevelResourceType tpRes,int mod)
{
	AssertAliveObj(lo);

	LevelAttr_Resource *attrRes=lo->GetAttr_Resource();
	if (!attrRes)
		return;

	if (TRUE)
	{
		int delta=mod;
		LevelOp_ResouceMod *op=lo->NewOp<LevelOp_ResouceMod>(LevelOpLink());
		Lav *lav=attrRes->Get(tpRes);
		if (!lav)
			return;
		lav->MakeMod((float)delta,FALSE,op->mod);
		op->tpRes=tpRes;

		lo->AddOp(op);

		LevelPlayerStates *lps=LevelUtil_GetLPS(_level,lo);
		if (lps)
		{
			DWORD *v=lps->base.GetRes(tpRes);
			if (v)
			{
				*v+=FloatToNearestInt(op->mod.delta);
				lps->base.SetDirtyDB_Low();
			}
		}
	}
}

void CLevelDecider::RepairTemple(LevelOSB &osb,CLevelObj *target,LevelTempleType tp,DWORD iAltar)
{
	LevelOp_TempleMod *op=osb.NewOp<LevelOp_TempleMod>(LevelOpLink());

	LevelAttr_Temple *attrTemple=target->GetAttr_Temple();
	if (!attrTemple)
		return;

	attrTemple->Set(tp,iAltar);

	op->tp=tp;
	op->iAltar=(BYTE)iAltar;
	target->AddOp(op);

	LevelPlayerStates *lps=LevelUtil_GetLPS(_level,target);
	if (lps)
	{
		LevelTempleInfo *info=lps->base.GetTemple(tp);
		if (info)
		{
			info->SetAltar(iAltar);
			lps->base.SetDirtyDB_Urgent();
		}
	}


}


void CLevelDecider::MakeShapeModify(LevelOSB &osb,CLevelObj *target,int op,StringID nmShape)
{
	LevelOp_ShapeMod *opShapeMod=osb.NewOp<LevelOp_ShapeMod>(LevelOpLink());

	opShapeMod->op=op;
	opShapeMod->nm=nmShape;

	target->AddOp(opShapeMod);
}

void CLevelDecider::MakeBodyModify(LevelOSB &osb,CLevelObj *target,BOOL bEnable)
{
	LevelOp_EnableBody*op=osb.NewOp<LevelOp_EnableBody>(LevelOpLink());

	op->bEnable=bEnable;

	target->AddOp(op);
}



BOOL CLevelDecider::CheckCost(CLevelSkill *skill)
{
	LevelRecordSkill *rec=skill->GetRec();
	LevelSkillType tpSkill;
	if (rec)
		tpSkill.idSkill=rec->GetID();

	return LevelUtil_TestSkillCost(tpSkill,skill->GetOwner());
}

static BOOL _MakeResCost(LevelPlayerStates *lps,CLevelSkill *skill,LevelResourceType tpRes,int cost)
{
	CLevelObj *owner=skill->GetOwner();
	if (!owner)
		return FALSE;

	LevelAttr_Resource *attr=owner->GetAttr_Resource();
	if (attr)
	{
		LevelOp_ResouceMod *op=skill->NewOp<LevelOp_ResouceMod>(LevelOpLink());
		op->tpRes=tpRes;
		Lav *lav=attr->Get(tpRes);
		if (lav)
		{
			if (lps)
			{
				DWORD *v=lps->base.GetRes(tpRes);
				if (v)
				{
					lav->MakeMod(-(float)cost,TRUE,op->mod);
					owner->AddOp(op);
					(*v)+=FloatToNearestInt(op->mod.delta);
					return TRUE;
				}
			}
		}
	}

	return FALSE;

}

void CLevelDecider::MakeCost_MaxSP(CLevelObj *lo,float cost)
{
	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		float spExausted=0.0f;
		if (lo->GetType()==LevelObjType_Unit)
		{
			LevelRecordUnit *rec=((CLoUnit*)lo)->GetRec();
			if (rec)
				spExausted=rec->ExhaustedSP;
		}

		float spMax=attr->sp.GetMax_Float();
		float spCost=cost;
		if (spMax-spCost<spExausted)
			spCost=spMax-spExausted;
		if (spCost<0.0f)
			spCost=0.0f;

		LevelOp_SPMod *op=lo->NewOp<LevelOp_SPMod>(LevelOpLink());
		attr->sp.MakeMaxMod(-spCost,op->mod);
		lo->AddOp(op);
	}

}


void CLevelDecider::MakeCost(CLevelSkill *skill)
{
	LevelRecordSkill *recSkill=skill->GetRec();
	if (!recSkill)
		return;
	CLevelObj *owner=skill->GetOwner();  
	if (!owner)
		return;

	LevelPlayerStates *lps=LevelUtil_GetLPS(_level,owner);

	BOOL bLPSDirty=FALSE;

	LevelCost *cost=&recSkill->cost;
	float spCost,spMaxCost;
	spCost=(float)cost->sp;
	spMaxCost=cost->spMax;

	LevelUtil_ModSPCost(owner,spCost,spMaxCost);
	if (spCost>0.0f)
	{
		LevelAttr_Base *attr=owner->GetAttr_Base();
		if (attr)
		{
			LevelOp_SPMod *op=owner->NewOp<LevelOp_SPMod>(LevelOpLink());
			attr->sp.MakeMod(-spCost,TRUE,op->mod);
			owner->AddOp(op);
		}
	}

	if (spMaxCost>0.0f)
		MakeCost_MaxSP(owner,spMaxCost);

	//resource cost
	if (TRUE)
	{
		if (cost->gold>0)
		{
			if (_MakeResCost(lps,skill,LevelResource_Gold,cost->gold))
				bLPSDirty=TRUE;
		}
		if (cost->gem>0)
		{
			if (_MakeResCost(lps,skill,LevelResource_Gem,cost->gem))
				bLPSDirty=TRUE;
		}
		if (cost->soul>0)
		{
			if (_MakeResCost(lps,skill,LevelResource_Soul,cost->soul))
				bLPSDirty=TRUE;
		}
		if (cost->crystal_>0)
		{
			if (_MakeResCost(lps,skill,LevelResource_Crystal,cost->crystal_))
				bLPSDirty=TRUE;
		}
	}

	if (cost->mp>0)
	{
		CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)LevelUtil_GetActiveAbility(owner,LevelAbilityType_MagicRing);
		if (ability)
			ability->MakeCost(cost->mp);
	}

	if (lps)
	{
		LPS_DecSkillStack(lps,_level->GetRecords(),skill->GetRec()->GetID(),1);
	}

	if (lps)
	{
		if (bLPSDirty)
			lps->base.SetDirtyDB_Low();
	}
}


void CLevelDecider::Revive(LevelOSB &osb,CLevelObj *loTarget,LevelOpLink &link)
{
	AssertAliveObj(loTarget);

	CLevelBuffs *buffs=loTarget->GetBuffs();
	if (!buffs)
		return;

	CLevelBuff *buff=buffs->FindBuff(Class_Ptr2(Buff_Dead));
	if (buff)
		RemoveBuff(osb,loTarget,buff,link);

	MakeCure(osb,loTarget,1000000,LevelStrike(),link);
}

LevelBuffID CLevelDecider::MakeJink(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelOpLink &link)
{
	StringID nmHandler=target->GetBuffHandler_Jink();
	if (nmHandler!=StringID_Invalid)
	{
		CLevelBehavior *behavior=target->GetBehaviorAI();
		if (behavior)
		{
			_ctxMakeJink.bEnable=TRUE;
			_ctxMakeJink.osbSrc=&osb;
			_ctxMakeJink.strike=&strike;
			_ctxMakeJink.link=&link;

			behavior->StartRelay(nmHandler);

			_ctxMakeJink.Zero();

			CLevelBuff *buff=LevelUtil_FindBuff(target,Class_Ptr2(Buff_Jink));
			if (buff)
				return buff->GetID();
		}
	}

	return LevelBuffID_Invalid;
}

LevelBuffID CLevelDecider::MakeSkillStun(LevelOSB &osb,CLevelObj*target,LevelStrike &strike,LevelWeakCategory catBroken,DamageAttrMask &weaksBroken,LevelOpLink &link)
{
	StringID nmHandler=target->GetBuffHandler_SkillStun();
	if (nmHandler!=StringID_Invalid)
	{
		CLevelBehavior *behavior=target->GetBehaviorAI();
		if (behavior)
		{
			_ctxMakeSkillStun.bEnable=TRUE;
			_ctxMakeSkillStun.osbSrc=&osb;
			_ctxMakeSkillStun.strike=&strike;
			_ctxMakeSkillStun.catBroken=catBroken;
			_ctxMakeSkillStun.weaksBroken=weaksBroken;
			_ctxMakeSkillStun.link=&link;

			behavior->StartRelay(nmHandler);

			LevelBuffID idResultBuff=_ctxMakeSkillStun.idResultBuff;

			_ctxMakeSkillStun.Zero();

			return idResultBuff;
		}
	}

	return LevelBuffID_Invalid;
}
