/********************************************************************
	created:	2012/12/11 
	author:		cxi
	
	purpose:	Level 的工具函数
*********************************************************************/
#include "stdh.h"

#include "LevelUtil.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LevelOSB.h"

#include "LevelObjResidable.h"
#include "LevelObjMove.h"

#include "LevelAttrs.h"

#include "LevelRecords.h"

#include "LevelRecordRegion.h"
#include "LevelRecordEo.h"
#include "LevelRecordUnit.h"
#include "LevelRecordAgent.h"
#include "LevelRecordUpgrade.h"
#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordPosture.h"
#include "LevelRecordGlobal.h"

#include "Ability_MagicRing.h"
#include "Ability_ShieldAmulet.h"

#include "LevelResources.h"

#include "LevelTalks.h"

#include "unitmgr/UnitMgrNavMesh.h"
#include "Random/Random.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "rvo2/RvoUnit.h"

#include "LoUnit.h"
#include "LoAgent.h"

#include "LoSlatesA.h"

#include "EoEnv.h"

#include "Buff_InSlates.h"
#include "Buff_SkillStun.h"
#include "Skill_GeneralAdvS.h"

#include "LevelCoSkill.h"

#include "spline/CubicSpline.h"

#include "Buff_Slime.h"

BOOL LevelUtil_GetFramePos(CLevel *level,LevelObjID id,LevelPos &pos)
{
	if (!level)
		return FALSE;

	CLevelObj *lo=level->GetIDs()->LoFromID(id);
	if (!lo)
		return FALSE;
	if (!lo->IsAlive())
		return FALSE;

	pos=lo->GetFramePos();
	return TRUE;
}



LevelPos LevelUtil_FindPosAround(LevelPos &center,float radius0,CLevel *level,DWORD nTry)
{
	for (int i=0;i<nTry;i++)
	{
		float radian=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		float radius=CSysRandom::RandRange(radius0/4.0f,radius0);

		LevelPos pos;
		pos.x=center.x+cosf(radian)*radius;
		pos.y=center.y+sinf(radian)*radius;

		if (!level->GetUnitMgr()->IsWalkable(UnitFindPath_Walkable,pos))
			continue;
		return pos;
	}

	return center;
}

BOOL LevelUtil_FindPosAround(CLevelObj *lo,float radius0,DWORD nTry,LevelPos &pos0,LevelFace &face0)
{
	LevelPos pos;
	LevelFace face;
	if (lo->GetLevel())
	{
		CUnitMgrNavMesh *unitmgr=lo->GetLevel()->GetUnitMgr();
		if (unitmgr)
		{
			LevelPos posCenter=lo->GetFramePos();
			for (int i=0;i<nTry;i++)
			{
				face=LevelUtil_GenRandomFace();
				LevelPos dir=LevelFaceToDir(face);

				pos=posCenter-dir*(lo->GetRadius_()+radius0);
				if (unitmgr->IsWalkable(UnitFindPath_Walkable,pos))
				{
					pos0=pos;
					face0=face;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}


//nSideStep指两侧各用几个step进行检测
//euler返回更新过的角度
//如果返回FALSE,pos里保存能找到的最远的点
BOOL LevelUtil_DetectPos(CLevelObj *lo,float &euler,float &dist,float fov,int nSideSteps,LevelPos &pos)
{
	pos=lo->GetFramePos();
	CLevel *level=lo->GetLevel();
	if (level)
	{
		CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
		if (unitmgr)
		{
			if (unitmgr->ToClosestWalkable(UnitFindPath_Walkable,pos))
			{
				LevelPos dir;
				const float step=fov/2.0f/(float)nSideSteps;
				LevelPos posTarget;
				LevelPos posHit;
				int sign=1;

				float eulerBest=euler;
				LevelPos posBest;
				float dist2Max=-1.0f;

				for (int i=0;i<nSideSteps*2+1;i++)
				{
					euler=euler+sign*i;
					sign=-sign;

					dir.setEuler(euler);

					posTarget=pos+dir*dist;

					if (!unitmgr->StaticRayCast(UnitFindPath_Walkable,pos,posTarget,posHit))
					{
						pos=posTarget;
						return TRUE;
					}

					float dist2=posHit.getDistanceSQFrom(pos);
					if (dist2>dist2Max)
					{
						posBest=posHit;
						eulerBest=euler;
						dist2Max=dist2;
					}
				}

				euler=eulerBest;
				dist=sqrtf(dist2Max);
				pos=posBest;
				return FALSE;
			}
		}
	}

	dist=0.0f;
	return FALSE;
}


BOOL LevelUtil_MoveTo(CLevelObj *lo,LevelPos &pos,float rangeFollow)
{
	AssertAliveObj(lo);

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		target.SetPos(pos);
		driver->StartFollow(target,rangeFollow);
	}

	return TRUE;
}

BOOL LevelUtil_IsMoving_(CLevelObj *lo)
{
	AssertAliveObj(lo);

	CLevelObjMove *move=lo->GetMove();
	if (!move)
		return FALSE;
	return move->IsMoving_();
}

BOOL LevelUtil_IsMovingOrRotating(CLevelObj *lo)
{
	AssertAliveObj(lo);

	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
	{
		CLevelPlayerMove &move=player->GetMove();
		LevelMoveStep step;
		move.GetRecentMoveStep(lo->GetLevel()->GetT_(),ANIMTICK_FROM_SECOND(0.1f),step);
		if (step.dist<0.001f)
			return FALSE;
		return TRUE;
	}

	CLevelObjMove *move=lo->GetMove();
	if (!move)
		return FALSE;
	return move->IsMovingOrRotating();
}


BOOL LevelUtil_CheckSimilarTarget(LevelPos &posCur,LevelPos &targetOld,LevelPos&targetNew)
{
	if (targetOld==targetNew)
		return TRUE;//一模一样

	LevelPos vOld=targetOld-posCur;
	LevelPos vNew=targetNew-posCur;
	float dist2Old=vOld.getLengthSQ();
	float dist2New=vNew.getLengthSQ();

	float dist2Min=dist2Old;
	if (dist2Min>dist2New)
		dist2Min=dist2New;

	if (dist2Min<5.0f*5.0f)
		return FALSE;//太近了

	vOld.normalize();
	vNew.normalize();

	if (vOld.dotProduct(vNew)>0.98f)
		return TRUE;//方向几乎一致,又足够远

	return FALSE;
}

CLevelObj *LevelUtil_DetectClosestPlayer(CLevelObj *lo,float range)
{
	AssertAliveObj(lo);

	LevelPos posOrg=lo->GetFramePos();
	CLevelObj *loFound=NULL;
	float dist2Min=100000000.0f;
	CLevel *level=lo->GetLevel();
	DWORD c;
	LevelPlayerID *ids=level->GetPlayerIDs(c);
	for (int i=0;i<c;i++)
	{
		CLevelPlayer *player=level->GetPlayer(ids[i]);
		if (player)
		{
			CLevelObj*loUnit=(CLevelObj*)player->GetLoUnit();
			if (loUnit)
			{
				float dist2=loUnit->GetFramePos().getDistanceSQFrom(posOrg);
				if (dist2>=range*range)
					continue;
				if (dist2<dist2Min)
				{
					loFound=loUnit;
					dist2Min=dist2;
				}
			}
		}
	}
	return loFound;
}

CLevelObj *LevelUtil_DetectPlayer(CLevelObj *lo,float range)
{
	AssertAliveObj(lo);

	LevelPos posOrg=lo->GetFramePos();
	CLevel *level=lo->GetLevel();
	DWORD c;
	LevelPlayerID *ids=level->GetPlayerIDs(c);
	for (int i=0;i<c;i++)
	{
		CLevelPlayer *player=level->GetPlayer(ids[i]);
		if (player)
		{
			CLevelObj*loUnit=(CLevelObj*)player->GetLoUnit();
			if (loUnit)
			{
				if (loUnit->GetFramePos().getDistanceSQFrom(posOrg)<range*range)
					return loUnit;
			}
		}
	}
	return NULL;
}

CLevelObj *LevelUtil_DetectClosestAgent(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent)
{
	AssertAliveObj(lo);

	CLevelObjMap *om=lo->GetLevel()->GetObjMap();
	om->SetEnumRange(lo,0.0f,range);
	om->SetEnumFilter_AllAgents();
	om->SetEnumFilter_Ignore(toIgnore);
	if (idAgent!=RecordID_Invalid)
		om->SetEnumFilter_AgentIDs(&idAgent,1);

	LevelDetectWeightsBase weight;
	om->SetEnumFilter_DetectWeights(&weight);

	return om->EnumBest(NULL);
}

CLevelObj *LevelUtil_DetectClosestAgent(CLevel *level,LevelPos pos,float range,CLevelObj *toIgnore,RecordID idAgent)
{
	CLevelObjMap *om=level->GetObjMap();
	om->SetEnumRange(pos,0.0f,range);
	om->SetEnumFilter_AllAgents();
	om->SetEnumFilter_Ignore(toIgnore);
	if (idAgent!=RecordID_Invalid)
		om->SetEnumFilter_AgentIDs(&idAgent,1);

	LevelDetectWeightsBase weight;
	om->SetEnumFilter_DetectWeights(&weight);

	return om->EnumBest(NULL);
}

extern LevelRelation LevelUtil_CalcPlayerRelation(LevelRelationMatrix *matRelation,LevelPlayerID idPlayer1,LevelPlayerID idPlayer2);
CLevelObj **LevelUtil_DetectEnemies(CLevelObj *lo,LevelPos pos,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods,DWORD &c)
{
	AssertAliveObj(lo);
	if (toIgnore)
	{
		AssertAliveObj(toIgnore);
	}

	CLevel *level=lo->GetLevel();
	LevelPlayerID idPlayer=lo->GetPlayerID();

	DWORD nPlayers;
	LevelPlayerID *idPlayers=level->GetPlayerIDs(nPlayers);

	CLevelObjMap *om=level->GetObjMap();

	for (int i=0;i<nPlayers;i++)
	{
		LevelPlayerID idPlayer2=idPlayers[i];
		LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayer,idPlayer2);
		if(relation!=LevelRelation_Enemy)
			continue;
		om->SetEnumFilter_Player(idPlayer2,methods);
		om->SetEnumFilter_UnitsOfPlayer(idPlayer2,methods);
	}
	LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayer,LevelPlayerID_Wild);
	if(relation==LevelRelation_Enemy)
		om->SetEnumFilter_UnitsOfPlayer(LevelPlayerID_Wild,methods);

	relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayer,LevelPlayerID_PlayerWild);
	if(relation==LevelRelation_Enemy)
		om->SetEnumFilter_UnitsOfPlayer(LevelPlayerID_PlayerWild,methods);

	extern LevelPlayerMask LevelUtil_GetPlayersByRelation(CLevel *level,LevelPlayerID idPlayer,LevelRelation relation);
	om->SetEnumFilter_Agents_(LevelUtil_GetPlayersByRelation(level,idPlayer,LevelRelation_Enemy));

	om->SetEnumFilter_Ignore(lo);
	if (toIgnore)
		om->SetEnumFilter_Ignore(toIgnore);

	om->SetEnumRange(pos,0.0f,range);

	LevelObjRequire requires[]={LevelObjRequire_Attackable};
	om->SetEnumFilter_Require(requires,ARRAY_SIZE(requires));

	return om->Enum(NULL,c);
}


CLevelObj **LevelUtil_DetectEnemies(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods,DWORD &c)
{
	AssertAliveObj(lo);

	return LevelUtil_DetectEnemies(lo,lo->GetFramePos(),range,toIgnore,methods,c);
}

LevelPlayerMask LevelUtil_GetPlayersByRelation(CLevel *level,LevelPlayerID idPlayer,LevelRelation relation)
{
	LevelPlayerMask maskPlayers=level->GetPlayerMask();
	if (idPlayer==LevelPlayerID_NeutralWild)
	{
		if (relation==LevelRelation_Neutral)
			return maskPlayers|(1<<LevelPlayerID_Wild)|(1<<LevelPlayerID_PlayerWild);
		if (relation==LevelRelation_Native)
			return (1<<idPlayer);
		return 0;
	}
	else
	{
		LevelRelationMatrix *mat=level->GetWorld()->GetRelationMatrix();
		if (relation==LevelRelation_Enemy)
			return mat->GetEnemies(idPlayer)&maskPlayers;
		if (relation==LevelRelation_Ally)
			return mat->GetAllies(idPlayer)&maskPlayers;
		if (relation==LevelRelation_Native)
			return (1<<idPlayer);
		if (relation==LevelRelation_Neutral)
		{
			LevelPlayerMask enemies,allies;
			enemies=mat->GetEnemies(idPlayer);
			allies=mat->GetAllies(idPlayer);
			return ((~(enemies|allies))&maskPlayers)|(1<<LevelPlayerID_NeutralWild);
		}
	}

	return 0;
}


LevelPlayerMask LevelUtil_GetPlayersByDetectFlags(CLevel *level,LevelPlayerID idPlayer,LevelDetectTargetFlag flags)
{
	LevelPlayerMask mask=0;
	LevelPlayerMask maskPlayers=level->GetPlayerMask()|
										(1<<LevelPlayerID_PlayerWild)|
										(1<<LevelPlayerID_Wild)|
										(1<<LevelPlayerID_NeutralWild);
	if (idPlayer==LevelPlayerID_NeutralWild)
	{
		if (flags&LevelDetectTarget_Neutral)
			mask|=maskPlayers|(1<<LevelPlayerID_Wild)|(1<<LevelPlayerID_PlayerWild);
		if (flags&LevelDetectTarget_Native)
			mask|=(1<<idPlayer);
	}
	else
	{
		LevelRelationMatrix *mat=level->GetWorld()->GetRelationMatrix();
		if (flags&LevelDetectTarget_Enemy)
			mask|=mat->GetEnemies(idPlayer)&maskPlayers;
		if (flags&LevelDetectTarget_Ally)
			mask|=mat->GetAllies(idPlayer)&maskPlayers;
		if (flags&LevelDetectTarget_Native)
			mask|=(1<<idPlayer);
		if (flags&LevelDetectTarget_Neutral)
		{
			LevelPlayerMask enemies,allies;
			enemies=mat->GetEnemies(idPlayer);
			allies=mat->GetAllies(idPlayer);
			mask|=((~(enemies|allies))&maskPlayers)|(1<<LevelPlayerID_NeutralWild);
		}
	}

	return mask;
}


extern LevelRelation LevelUtil_CalcPlayerRelation(LevelRelationMatrix *matRelation,LevelPlayerID idPlayer1,LevelPlayerID idPlayer2);
void SetEnumFilter(CLevelObjMap *om,CLevelObj *lo,LevelDetectTargetFlag flags)
{
	CLevel *level=lo->GetLevel();
	LevelPlayerID idPlayer=lo->GetPlayerID();

	static LevelDetectTargetFlag cnvtRelation[]={LevelDetectTarget_Native,LevelDetectTarget_Ally,LevelDetectTarget_Enemy,LevelDetectTarget_Neutral};

	LevelDetectTargetFlag flagRelation=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Relation);
	LevelDetectTargetFlag flagType=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Type);
	LevelDetectTargetFlag flagMethod=(LevelDetectTargetFlag)(flags&LevelDetectTargetFlag_Method);
	LevelMoveMethodMask methods=flagMethod/LevelDetectTarget_Ground;

	DWORD nPlayers;
	LevelPlayerID *idPlayers=level->GetPlayerIDs(nPlayers);

	if (flagType&(LevelDetectTarget_Player|LevelDetectTarget_Unit))
	{
		for (int i=0;i<nPlayers;i++)
		{
			LevelPlayerID idPlayer2=idPlayers[i];
			LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayer,idPlayer2);
			if (cnvtRelation[relation]&flagRelation)
			{
				if (flagType&LevelDetectTarget_Player)
					om->SetEnumFilter_Player(idPlayer2,methods);
				if (flagType&LevelDetectTarget_Unit)
					om->SetEnumFilter_UnitsOfPlayer(idPlayer2,methods);
			}
		}
		if (flagType&(LevelDetectTarget_Unit))
		{
			if (flagRelation&LevelDetectTarget_Neutral)
			{
				if (flagType&LevelDetectTarget_Unit)
					om->SetEnumFilter_UnitsOfPlayer(LevelPlayerID_NeutralWild,methods);
			}
			if (TRUE)
			{
				LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayer,LevelPlayerID_Wild);
				if (cnvtRelation[relation]&flagRelation)
				{
					if (flagType&LevelDetectTarget_Unit)
						om->SetEnumFilter_UnitsOfPlayer(LevelPlayerID_Wild,methods);
				}
			}
		}
	}

	if (flagType&LevelDetectTarget_Agent)
	{
		LevelPlayerMask mask=LevelUtil_GetPlayersByDetectFlags(level,idPlayer,flagRelation);
		om->SetEnumFilter_Agents_(mask);
	}


	if (flagType&LevelDetectTarget_Item)
		om->SetEnumFilter_Items();

// 	om->SetEnumFilter_Ignore(lo);

}


static CLevelObjMap *LevelUtil_SetDetectParam(LevelUtilDetectParam &param)
{
	AssertAliveObj(param.loSrc);
	if (param.nIgnores>0)
	{
		for (int i=0;i<param.nIgnores;i++)
		{
			AssertAliveObj(param.toIgnores[i]);
		}
	}

	CLevel *level=param.loSrc->GetLevel();
	LevelPlayerID idPlayer=param.loSrc->GetPlayerID();

	DWORD nPlayers;
	LevelPlayerID *idPlayers=level->GetPlayerIDs(nPlayers);

	CLevelObjMap *om=level->GetObjMap();

	if (param.flags)
	{
		for (int i=0;i<param.nFlags;i++)
			SetEnumFilter(om,param.loSrc,param.flags[i]);
	}

	om->SetEnumFilter_Ignore(param.loSrc);
	for (int i=0;i<param.nIgnores;i++)
		om->SetEnumFilter_Ignore(param.toIgnores[i]);

	if (param.rangeMax>0.0f)
		om->SetEnumRange(param.pos,param.rangeMin,param.rangeMax);
	else
		om->SetEnumRange(param.rc);

	if (param.requires)
		om->SetEnumFilter_Require(param.requires,param.nRequires);

	om->SetEnumFilter_Touching(param.bTouching);

	om->SetEnumFilter_RecentTarget(param.recent);
	om->SetEnumFilter_FailTarget(param.fail);

	om->SetEnumFilter_AgentIDs(param.idAgents,param.nAgents);

	om->SetEnumFilter_DetectWeights(&param.weights);

	om->SetEnumFilter_Src(param.loSrc);

	return om;
}

CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c)
{
	CLevelObjMap *om=LevelUtil_SetDetectParam(param);
	return om->Enum(dlgt,c);
}

CLevelObj **LevelUtil_DetectInAll(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c)
{
	CLevelObjMap *om=LevelUtil_SetDetectParam(param);
	return om->EnumInAll(dlgt,c);
}

CLevelObj *LevelUtil_DetectFirst(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt)
{
	CLevelObjMap *om=LevelUtil_SetDetectParam(param);
	return om->EnumFirst(dlgt);
}

CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt)
{
	CLevelObjMap *om=LevelUtil_SetDetectParam(param);
	return om->EnumBest(dlgt);
}

//在candidates里选择best,忽略range条件
CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,std::vector<CLevelObj *>&candidates)
{
	CLevel *level=param.loSrc->GetLevel();
	LevelPlayerID idPlayerSrc=param.loSrc->GetPlayerID();
	static LevelDetectTargetFlag cnvtRelation[]={LevelDetectTarget_Native,LevelDetectTarget_Ally,LevelDetectTarget_Enemy,LevelDetectTarget_Neutral};

	CalcDetectRateContext ctx;
	if (TRUE)
	{
		ctx.radiusMin=param.rangeMin;
		ctx.radiusMax=param.rangeMax;

		ctx.center=param.loSrc->GetFramePos();

		ctx.bTouching=param.bTouching;

		ctx.weights=&param.weights;
		ctx.recent=param.recent;
		ctx.fail=param.fail;
		ctx.src=param.loSrc;
	}

	float rateMax=-1.0f;
	CLevelObj *found=NULL;

	for (int i=0;i<candidates.size();i++)
	{
		CLevelObj *lo=candidates[i];
		if (!lo)
			continue;

		if (TRUE)
		{
			float dist2=lo->GetFramePos().getDistanceSQFrom(ctx.center);
			if (!CLevelObjMap::CheckInRange(lo,dist2,param.rangeMin,param.rangeMax,param.bTouching))
				continue;
		}

		//计算flag
		LevelDetectTargetFlag flag;
		if (TRUE)
		{
			LevelPlayerID idPlayer=lo->GetPlayerID();
			LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),idPlayerSrc,idPlayer);
			flag=cnvtRelation[relation];
			switch(lo->GetType())
			{
				case LevelObjType_Agent:
					flag=(LevelDetectTargetFlag)(flag|LevelDetectTarget_Agent);
					break;
				case LevelObjType_Item:
					flag=(LevelDetectTargetFlag)(flag|LevelDetectTarget_Item);
					break;
				case LevelObjType_Unit:
				{
					flag=(LevelDetectTargetFlag)(flag|(lo->IsPlayer()?LevelDetectTarget_Player:LevelDetectTarget_Unit));
					break;
				}
				default:
					continue;
			}

			LevelMoveMethodMask maskMethod=lo->GetMoveMethodMask();
			flag=(LevelDetectTargetFlag)(flag|(maskMethod*LevelDetectTarget_Ground));
		}

		//检测flag是否满足
		if (TRUE)
		{
			BOOL bFlagMet=FALSE;
			for (int i=0;i<param.nFlags;i++)
			{
				if ((flag&param.flags[i])==flag)
				{
					bFlagMet=TRUE;
					break;
				}
			}
			if (!bFlagMet)
				continue;
		}

		//是否被ignore
		if (TRUE)
		{
			BOOL bIgnored=FALSE;
			for (int i=0;i<param.nIgnores;i++)
			{
				if (lo==param.toIgnores[i])
				{
					bIgnored=TRUE;
					break;
				}
			}
			if (bIgnored)
				continue;
		}

		//检测require
		if (TRUE)
		{
			extern BOOL LevelUtil_CheckLoRequire(CLevelObj *lo,LevelObjRequire *requires,DWORD c);
			if (!LevelUtil_CheckLoRequire(lo,param.requires,param.nRequires))
				continue;
		}

		if (param.nAgents>0)
		{
			extern RecordID LevelUtil_GetAgentRecID(CLevelObj *lo);
			RecordID idAgent=LevelUtil_GetAgentRecID(lo);
			int i;
			for (i=0;i<param.nAgents;i++)
			{
				if (idAgent==param.idAgents[i])
					break;
			}
			if (i>=param.nAgents)
				continue;
		}

		extern LevelDetectRate CalcDetectRate(CLevelObj *obj,CalcDetectRateContext *ctx);
		LevelDetectRate rate=CalcDetectRate(lo,&ctx);
		if (rate.rate<0.0f)
			continue;

		if (dlgt!=NULL)
		{
			if (!dlgt(lo,rate.dist*rate.dist))
				continue;
		}

		if (rate.rate>rateMax)
		{
			found=lo;
			rateMax=rate.rate;
		}
	}

	return found;
}

CLevelObj **LevelUtil_DetectInZone(LevelUtilDetectParam &param,AnimEventZone &ezone,i_math::xformf&xfm,LevelTick t,DWORD &c)
{
	c=0;

	AnimEventZone::KeyFan k;
	if (!ezone.CalcKeyFan(t,k))
		return NULL;

	k.xfmCenter.applyBase(xfm);

	i_math::circlef circle=k.CalcBoundingCircle();

	LevelPos posOld;
	float radiusMinOld,radiusMaxOld;
	posOld=param.pos;
	radiusMinOld=param.rangeMin;
	radiusMaxOld=param.rangeMax;

	param.pos=circle.center;
	param.rangeMin=0.0f;
	param.rangeMax=circle.radius;

	DWORD cTotal;
	CLevelObj **result=LevelUtil_Detect(param,NULL,cTotal);

	for (int i=0;i<cTotal;i++)
	{
		CLevelObj *lo=result[i];
		if (lo)
		{
			LevelPos posTarget=lo->GetFramePos();
			float radiusTarget=lo->GetRadius_();

			if (k.CheckIn(posTarget,radiusTarget))
			{
				result[c]=lo;
				c++;
			}
		}
	}

	return result;
}


BOOL LevelUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff)
{
	if (!lo)
		return FALSE;
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (!buffs)
		return FALSE;
	return buffs->TestFlag(flagBuff);
}

//注意这个函数只返回第一个具有flagBuff的Buff的age(不对其它Buff作判断)
AnimTick LevelUtil_GetBuffFlagAge(CLevelObj *lo,DWORD flagBuff)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (!buffs)
		return 0;
	DWORD c;
	CLevelBuff **buf=buffs->GetBuffs(c);
	for (int i=0;i<c;i++)
	{
		if (buf[i]->GetFlags()&flagBuff)
			return buf[i]->GetAge();
	}

	return 0;
}

CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (!buffs)
		return NULL;
	return buffs->FindBuff(clssBuff);
}

CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (!buffs)
		return NULL;
	return buffs->FindBuffByID(idBuff);
}


CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (!buffs)
		return NULL;
	return buffs->FindBuffByRecordID(idBuff);
}

CLevelBuff* LevelUtil_FindBuffByRecordID(CLevel* level, LevelObjID idLo, RecordID idBuff)
{
	CLevelObj* lo = LevelUtil_GetAliveLo(level, idLo);
	if (!lo)
		return NULL;

	return LevelUtil_FindBuffByRecordID(lo, idBuff);
}

void LevelUtil_RemoveBuffByRecordID(CLevelObj *lo,RecordID idBuff)
{
	if (lo)
	{
		CLevelBuff *buff=LevelUtil_FindBuffByRecordID(lo,idBuff);
		if (buff)
		{
			LevelOSB osb(lo);
			lo->GetLevel()->GetDecider()->RemoveBuff(osb,lo,buff,LevelOpLink());
		}
	}
}

// BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius)
// {
// 	AssertAliveObj(lo);
// 	AssertAliveObj(loEscapeFrom);
// 	CLevelSkillDriver *driver=lo->GetSkillDriver();
// 	if (!driver)
// 		return FALSE;
// 	CLevel *level=lo->GetLevel();
// 	if (!level)
// 		return FALSE;
// 	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
// 	if (!unitmgr)
// 		return FALSE;
// 
// 	UnitFindPathType tpFindPath=UnitFindPath_Walkable;
// 
// 	if (lo&&loEscapeFrom)
// 	{
// 		if (lo->IsAlive()&&loEscapeFrom->IsAlive())
// 		{
// 			LevelPos posSrc=lo->GetFramePos();
// 
// 			if (FALSE==unitmgr->ToClosestWalkable(tpFindPath,posSrc))
// 				return FALSE;
// 
// 			LevelPos dir=posSrc-loEscapeFrom->GetFramePos();
// 			float dist=dir.getLength();
// 			float distToGo=distKeep-dist;
// 			float distToGoMin=1.0f;
// 
// 			if (dist<0.01f)
// 				dir.set(0.0f,1.0f);
// 			else
// 				dir.normalize();
// 
// 			float euler=dir.getEuler();
// 			float step=10.0f*i_math::GRAD_PI2;
// 			int nStep=(int)(i_math::Pi*2.0f/step);
// 			float sign=CSysRandom::Roll(0.5f)?1.0f:-1.0f;
// 
// 			LevelPos posTarget,posHit;
// 			BOOL bFound=FALSE;
// 
// 			LevelPos posCenter;
// 			if (loCenter)
// 				posCenter=loCenter->GetFramePos();
// 
// 			for (int i=0;i<nStep;i++)
// 			{
// 				if (signAvoid==0)
// 				{
// 					euler=euler+step*sign*(float)i;
// 					signAvoid=sign>0.0f?1:-1;
// 					sign*=-1.0f;
// 				}
// 				else
// 					euler=euler+step*(float)signAvoid;
// 
// 				dir.setEuler(euler);
// 
// 				dir*=distToGo;
// 				posTarget=lo->GetFramePos()+dir;
// 
// 				//限制到center范围内
// 				if (loCenter)
// 				{
// 					if (posTarget.getDistanceSQFrom(posCenter)>radius*radius)
// 					{
// 						LevelPos dir=posTarget-posCenter;
// 						dir.normalize();
// 						dir*=radius;
// 						posTarget=posCenter+dir;
// 					}
// 				}
// 
// 				if (unitmgr->StaticRayCast(tpFindPath,posSrc,posTarget,posHit))
// 				{
// 					if (posSrc.getDistanceSQFrom(posHit)<distToGoMin*distToGoMin)
// 						continue;//这个方向上卡住了,继续尝试
// 					posTarget=posHit;
// 				}
// 
// 				LevelSkillTarget target;
// 				target.SetPos(posTarget);
// 				driver->StartFollow(target,0.0f);
// 				
// 				bFound=TRUE;
// 				break;
// 			}
// 
// 			if (!bFound)
// 				signAvoid=0;
// 		}
// 	}
// 
// 	return TRUE;
// }

BOOL LevelUtil_FindFleePos(CLevel *level,LevelPos &posCur,LevelPos &posEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius,
						   LevelPos &posResult,FindFleePosCallBack dlgt)
{
	if (!level)
		return FALSE;
	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
	if (!unitmgr)
		return FALSE;

	UnitFindPathType tpFindPath=UnitFindPath_Walkable;

	LevelPos posSrc=posCur;

	if (FALSE==unitmgr->ToClosestWalkable(tpFindPath,posSrc))
		return FALSE;

	LevelPos dir=posSrc-posEscapeFrom;
	float dist=dir.getLength();
	float distToGo=distKeep-dist;
	float distExt=1.0f;

	if (dist<0.01f)
		dir.set(0.0f,1.0f);
	else
		dir.normalize();

	LevelPos dir0=dir;

	float euler=dir.getEuler();
	float step=10.0f*i_math::GRAD_PI2;
	int nStep=(int)(i_math::Pi*2.0f/step);
	float sign;
	BOOL bFixedSign=FALSE;
	if (signAvoid==0)
		sign=CSysRandom::Roll(0.5f)?1.0f:-1.0f;
	else
	{
		sign=(signAvoid>0)?1.0f:-1.0f;
		bFixedSign=TRUE;
	}

	LevelPos posTarget,posHit;
	BOOL bFound=FALSE;

	LevelPos posCenter;
	if (loCenter)
		posCenter=loCenter->GetFramePos();


	for (int i=0;i<nStep;i++)
	{
		if (!bFixedSign)
		{
			euler=euler+step*sign*(float)i;
			if (i==0)
				signAvoid=0;//i==0时,角度没有偏转,相当于没有avoid
			else
				signAvoid=sign>0.0f?1:-1;
			sign*=-1.0f;
		}
		else
			euler=euler+step*sign;

		dir.setEuler(euler);

		dir*=(distToGo+distExt);
		posTarget=posSrc+dir;

		//限制到center范围内
		if (loCenter)
		{
			if (posTarget.getDistanceSQFrom(posCenter)>radius*radius)
			{
				LevelPos dir=posTarget-posCenter;
				dir.normalize();
				dir*=radius;
				posTarget=posCenter+dir;
			}
		}

		if (dlgt)
		{
			if (!dlgt(posTarget))
				continue;
		}

		if (unitmgr->StaticRayCast(tpFindPath,posSrc,posTarget,posHit))
			continue;

		bFound=TRUE;
		posResult=posTarget-dir0*distExt;
		return TRUE;
	}

	if (!bFound)
		signAvoid=0;

	return bFound;
}


//signAvoid表示是否要优先朝一个方向上偏转,如果为0表示左右方向的优先级同等
BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius)
{
	AssertAliveObj(lo);
	AssertAliveObj(loEscapeFrom);
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return FALSE;
	CLevel *level=lo->GetLevel();
	if (!level)
		return FALSE;
	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
	if (!unitmgr)
		return FALSE;

	UnitFindPathType tpFindPath=UnitFindPath_Walkable;

	if (lo&&loEscapeFrom)
	{
		if (lo->IsAlive()&&loEscapeFrom->IsAlive())
		{
			LevelPos posTarget;
			BOOL bFound=LevelUtil_FindFleePos(level,lo->GetFramePos(),loEscapeFrom->GetFramePos(),distKeep,signAvoid,loCenter,radius,posTarget);
			if (bFound)
			{
				LevelSkillTarget target;
				target.SetPos(posTarget);
				driver->StartFollow(target,0.0f);

				return TRUE;
			}
		}
	}

	return TRUE;
}


//range设成-1表示使用缺省的follow range
BOOL LevelUtil_Follow(CLevelObj *lo,CLevelObj *loTarget,float range,BOOL bClosestFollow)
{
	AssertAliveObj(lo);

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		target.SetObjID(loTarget->GetID());
		driver->StartFollow(target,range,bClosestFollow);
	}

	return TRUE;
}

BOOL LevelUtil_CheckFollow(CLevelObj *lo,CLevelObj *loTarget)
{
	//目前只考虑都是地行Unit的情况
	CUnit *unit=lo->GetUnit();
	CUnit *unitTarget=loTarget->GetUnit();

	assert(unit);
	if (!unit)
		return FALSE;

	UnitStage stage=unit->GetStage();
	if ((stage==UnitStage_NotMove)||(stage==UnitStage_Reached)||(stage==UnitStage_Abort))
		return FALSE;

	if (TRUE)
	{
		CUnitTarget *target=unit->GetTarget();
		if (target)
		{
			if (target->_unit==unitTarget)
				return TRUE;
		}
	}
	if (TRUE)
	{
		CUnitTarget *target=unit->GetPendingTarget();
		if (target)
		{
			if (target->_unit==unitTarget)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL LevelUtil_StopMove(CLevelObj *lo)
{
	AssertAliveObj(lo);

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
		driver->StopMove();

	return TRUE;
}

LevelRecordAgent* LevelUtil_GetAgentRec(CLevelObj *lo)
{
	AssertAliveObj(lo);

	if (lo)
	{
		if (lo->GetType()==LevelObjType_Agent)
		{
			CLevelObjSrc *los=lo->GetLos();
			if (los)
				return lo->GetLevel()->GetRecords()->GetAgent(los->GetRecID());
		}
	}
	return NULL;
}


RecordID LevelUtil_GetAgentRecID(CLevelObj *lo)
{
	LevelRecordAgent*rec=LevelUtil_GetAgentRec(lo);
	if (rec)
		return rec->GetID();
	return RecordID_Invalid;
}

CLevelObj *LevelUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent)
{
	AssertAliveObj(lo);

	CLevelObjMap *om=lo->GetLevel()->GetObjMap();
	om->SetEnumFilter_AllAgents();
	om->SetEnumRange(lo,0.0f,range);
	om->SetEnumFilter_Ignore(toIgnore);
	om->SetEnumFilter_AgentIDs(&idAgent,1);

	CLevelObj **objs;
	DWORD c;
	float dist2Min=10000000000.0f;
	CLevelObj *objFound=NULL;
	objs=om->Enum(NULL,c);
	for (int i=0;i<c;i++)
	{
		CLevelObj *obj=objs[i];
		if (obj)
		{
			if (obj->GetBuffs())
			{
				if (obj->GetBuffs()->TestFlag(BuffFlag_Dead|BuffFlag_Invisible))
					continue;
			}
			CLevelObjResidable *residable=obj->GetResidable();
			if (residable)
			{
				if (residable->CanPreserve())
				{
					float dist2=obj->GetFramePos().getDistanceSQFrom(lo->GetFramePos());
					if (dist2<dist2Min)
					{
						dist2Min=dist2;
						objFound=obj;
					}
				}
			}
		}
	}

	return objFound;
}


BOOL LevelUtil_FindLandingSpot(CLevelObj *lo,float fwd,float rangeDescend,float rangeLand,float height,LevelPos3D &posStart,LevelPos3D &posEnd)
{
	AssertAliveObj(lo);

	GameTileMap *gtm=lo->GetLevel()->GetGtm();
	if (!gtm)
		return FALSE;

	CUnit3D *unit=lo->GetUnit3D();
	if (!unit)
		return FALSE;
	i_math::vector3df &pos=unit->GetPos();
	i_math::vector3df &vel=unit->GetVel();

	if (vel.getLengthSQ()<0.0001f)
		return FALSE;

	i_math::vector2df org,dir;
	dir.set(vel.x,vel.z);
	dir.normalize();
	org.set(pos.x,pos.z);

	i_math::vector2df start;
	start=org+dir*fwd;

	float step=gtm->hdr.lenTile;

	float dist=rangeDescend+rangeLand;
	if (dist<=0.0f)
		return FALSE;

	BOOL bBase=FALSE;
	float htBase;
	float distBase;
	while(dist>=0.0f)
	{
		i_math::vector2df posTest;
		posTest=start+dir*dist;
		GameTile *tile=gtm->GetTile(posTest.x,posTest.y);
		if (!tile)
			return FALSE;

		float ht=VUS13_MtrFromVu(tile->ht);
		BOOL bWalkable=tile->bWalkable;
		if (dist>rangeDescend)
		{
			if (!bWalkable)
				return FALSE;
			if (!bBase)
			{
				htBase=ht;
				bBase=TRUE;
			}
			else
			{
				if (fabsf(htBase-ht)>0.2f)
					return FALSE;
			}
			posEnd.set(posTest.x,ht,posTest.y);
			distBase=dist;
		}
		else
		{
			if (height+htBase>pos.y)
				height=pos.y-htBase;
			float htLimit=(distBase-dist)/distBase*height+htBase;//不能高于这个高度
			if (ht>htLimit)
				return FALSE;
		}

		dist-=step;
	}

	posStart.set(start.x,htBase+height,start.y);

	return TRUE;
}

LevelPlayerStates *LevelUtil_GetLPS(CLevel *lvl,CLevelObj *lo)
{
	AssertAliveObj(lo);
	LevelPlayerID idPlayer=lo->GetPlayerID();
	return lvl->GetLPS(idPlayer);
}

LevelPlayerStates *LevelUtil_GetLPS(CLevelObj *lo)
{
	if (lo)
	{
		if (lo->IsAlive())
		{
			CLevel *level=lo->GetLevel();
			if (level)
				return level->GetLPS(lo->GetPlayerID());
		}
	}
	return NULL;
}


BOOL LevelUtil_CheckOwningItem(CLevelObj *lo,RecordID idItem)
{
	CLevel *lvl=lo->GetLevel();
	if (!lvl)
		return FALSE;
	LevelPlayerStates *lps=LevelUtil_GetLPS(lvl,lo);
	if (!lps)
		return FALSE;

	extern BOOL LPS_CheckOwnArtifact(LevelPlayerStates *lps,RecordID idItem);
	if (LPS_CheckOwnArtifact(lps,idItem))
		return TRUE;

	if (LPS_EquipPartFromItem(lps,idItem)!=EquipPart_Invalid)
		return TRUE;

	return FALSE;
}

BOOL LevelUtil_CheckDead(CLevelObj *lo)
{
	if (LevelUtil_TestAnyBuff(lo,BuffFlag_Dead))
		return TRUE;

// 	extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
// 	Buff_Slime *buffSlime=(Buff_Slime *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_Slime));
// 	if (buffSlime)
// 	{
// 		if (buffSlime->IsDead())
// 			return TRUE;
// 	}

	return FALSE;
}

BOOL LevelUtil_CheckDead(CLevel *level,LevelObjID id)
{
	CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
	if (lo)
		return LevelUtil_CheckDead(lo);
	return TRUE;
}

BOOL LevelUtil_CheckInSlates(CLevelObj *lo)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (buffs)
		return (NULL!=buffs->FindBuff(Class_Ptr2(Buff_InSlates)));
	return FALSE;
}

float LevelUtil_GetCurHP(CLevelObj *lo)
{
	AssertAliveObj(lo);

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if(attr)
		return attr->hp.GetCur_Float();
	return 0.0f;
}


float LevelUtil_GetHealthRatio(CLevelObj *lo)
{
	AssertAliveObj(lo);

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if(attr)
		return attr->hp.GetRatio();
	return 0.0f;
}


float LevelUtil_GetHealthRatio(CLevel *level,LevelObjID id)
{
	CLevelObj *lo=level->GetIDs()->LoFromID(id);
	if (lo)
	{
		if (!lo->IsAlive())
			return FALSE;
		return LevelUtil_GetHealthRatio(lo);
	}
	return 0.0f;
}


BOOL LevelUtil_CheckInvisible(CLevelObj *lo)
{
	return LevelUtil_TestAnyBuff(lo,BuffFlag_Invisible);
}



BOOL LevelUtil_CheckDamageImmune(CLevelObj *lo)
{
	AssertAliveObj(lo);

	CLevelBuffs *buffs=lo->GetBuffs();
	if (buffs)
	{
		if (buffs->TestFlag(BuffFlag_DamageImmune))
			return TRUE;
	}

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return FALSE;
	if (driver->IsSkillCasting())
	{
		CLevelSkill *skill=driver->GetSkill();
		return skill->IsImmune();
	}

	return FALSE;
}


LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso)
{
	float ht=0.0f;
	if(lvl)
	{
		if (!bHiReso)
		{
			GameTileMap *gtm=lvl->GetGtm();
			if (gtm)
				ht=gtm->GetHeight(x,y);
		}
		else
		{
			CGameTrisMap *gtr=lvl->GetGtr();
			if (gtr)
				ht=lvl->GetGtr()->GetHeight(x,y);
		}
	}

	return LevelPos3D(x,ht,y);
}

LevelPos3D LevelUtil_GetWalkableGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso,UnitFindPathType tpFindPath)
{
	i_math::vector2df pos(x,y);
	if (lvl)
	{
		CUnitMgrNavMesh *unitmgr=lvl->GetUnitMgr();
		if (unitmgr)
		{
			unitmgr->ToClosestWalkable(tpFindPath,pos);
		}
	}
	return LevelUtil_GetGroundHeight(lvl,pos.x,pos.y,bHiReso);
}


CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id)
{
	if (!lvl)
		return NULL;
	if (!lvl->GetIDs())
		return NULL;
	CLevelObj *lo=lvl->GetIDs()->LoFromID(id);
	if (!lo)
		return NULL;
	if (!lo->IsAlive())
		return NULL;
	return lo;
}


CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo)
{
	if (!lo)
		return NULL;
	if (!lo->IsRetinue())
	{
		if (lo->IsPlayer())
			return lo;
		else
			return NULL;
	}

	CLevelPlayer *player=lo->GetLevel()->GetPlayer(lo->GetPlayerID());
	if (!player)
		return NULL;

	return (CLevelObj*)player->GetLoUnit();
}

CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo)
{
	if (!lo)
		return NULL;
	if (!lo->IsPlayer())
		return NULL;

	if (!lo->GetLevel())
		return NULL;

	return lo->GetLevel()->GetPlayer(lo->GetPlayerID());
}

CLevelPlayer *LevelUtil_GetOwnerPlayer(CLevelObj *lo)
{
	if (!lo)
		return NULL;
	if (lo->IsPlayer())
		return NULL;

	if (!lo->GetLevel())
		return NULL;

	return lo->GetLevel()->GetPlayer(lo->GetPlayerID());
}


CLevelObj *LevelUtil_GetOwnerLo(CLevel*lvl,LevelObjID id)
{
	if (!lvl)
		return NULL;
	if (!lvl->GetIDs())
		return NULL;
	CLevelObj *lo=lvl->GetIDs()->LoFromID(id);
	return LevelUtil_GetOwnerLo(lo);
}


BOOL LevelUtil_CheckLoRequire(CLevelObj *lo,LevelObjRequire *requires,DWORD c)
{
	if (c<=0)
		return TRUE;

	BOOL bRequireNotDead=TRUE;
	BOOL bRequireNotInvisible=TRUE;
	CLevelBuffs *buffs=lo->GetBuffs();
	for (int i=0;i<c;i++)
	{
		LevelObjRequire rqr=requires[i];

		switch(rqr)
		{
			case LevelObjRequire_Attackable:
			{
				if(buffs)
				{
					if (buffs->TestFlag(BuffFlag_NotAttackable|BuffFlag_Birth|BuffFlag_LayDown))
						return FALSE;
				}
				if (TRUE)
				{
					if (lo->GetType()==LevelObjType_Agent)
					{
						LevelRecordAgent *rec=((CLoAgent *)lo)->GetRec();
						if (rec)
						{
							if(!rec->bAttackable)
								return FALSE;
						}
					}
				}
				break;
			}
			case LevelObjRequire_Dormant:
			{
				if (buffs)
				{
					if (!buffs->TestFlag(BuffFlag_Dormant))
						return FALSE;
				}
				break;
			}
		}
	}

	if (bRequireNotDead)
	{
		if (LevelUtil_CheckDead(lo))
			return FALSE;
	}
	if (bRequireNotInvisible)
	{
		if (LevelUtil_CheckInvisible(lo))
			return FALSE;
	}

	return TRUE;
}


BOOL LevelUtil_CheckRelations(LevelRelationMask relations,LevelRelation relation)
{
	return (relations&(1<<relation))?TRUE:FALSE;
}

BOOL LevelUtil_CheckRelations(std::vector<LevelDetectTargetFlag>&flagsDetect,LevelRelation relation)
{
	static LevelDetectTargetFlag cnvtRelation[]={LevelDetectTarget_Native,LevelDetectTarget_Ally,LevelDetectTarget_Enemy,LevelDetectTarget_Neutral};
	LevelDetectTargetFlag flag=cnvtRelation[relation];

	for (int i=0;i<flagsDetect.size();i++)
	{
		if (flagsDetect[i]&flag)
			return TRUE;
	}

	return FALSE;
}


BOOL LevelUtil_CheckMoveMethod(std::vector<LevelDetectTargetFlag>&flagsDetect,LevelMoveMethod method)
{
	static LevelDetectTargetFlag cnvtMethod[]={LevelDetectTarget_None,LevelDetectTarget_Ground,LevelDetectTarget_Resided,LevelDetectTarget_Flying,LevelDetectTarget_None};
	LevelDetectTargetFlag flag=cnvtMethod[method];

	for (int i=0;i<flagsDetect.size();i++)
	{
		if (flagsDetect[i]&flag)
			return TRUE;
	}

	return FALSE;
}



//检查lo是否可以成为技能对象
BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget)
{
	if (!loTarget)
		return FALSE;
	if (!loTarget->IsAlive())
		return FALSE;

	if (!loSrc)
		return FALSE;
	if (!loSrc->IsAlive())
		return FALSE;

	if (recSkill)
	{
		CLevel *level=loSrc->GetLevel();

		LevelRelation relation=LevelUtil_CalcPlayerRelation(level->GetRelationMatrix(),loSrc->GetPlayerID(),loTarget->GetPlayerID());
		if (!LevelUtil_CheckRelations(recSkill->flagsDetect,relation))
		{
			return FALSE;//不满足技能的Relation要求
		}

		if (!LevelUtil_CheckLoRequire(loTarget,&recSkill->requires[0],recSkill->requires.size()))
			return FALSE;//不满足技能的特定需求

		LevelMoveMethod method=loTarget->GetMoveMethod();
		if (!LevelUtil_CheckMoveMethod(recSkill->flagsDetect,method))
			return FALSE;//不满足技能的空间状态需求
	}

	return TRUE;
}


LevelGrade LevelUtil_GetGrade(CLevelObj *lo)
{
	if (lo)
	{
		if (lo->IsAlive())
		{
			LevelAttr_Base *attrBase=lo->GetAttr_Base();
			if (attrBase)
				return attrBase->grade;
		}
	}
	return 0;
}

struct AgentDistributeInfo;
AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z)
{
	if (idAgent==RecordID_Invalid)
		return NULL;
	CGameRgnGrids *grids=lvl->GetRgnGrids();
	GameRgnGridRT *grid=grids->GetGridRT(x,z);
	if (!grid)
		return NULL;

	CLevelRecords *records=lvl->GetRecords();

	LevelRecordRegion *rec=records->GetRegion((RecordID)grid->id);
	if (!rec)
		return NULL;

	for (int i=0;i<rec->adis.size();i++)
	{
		if (rec->adis[i].idAgent==idAgent)
			return &rec->adis[i];
	}

	return NULL;
}

LevelObjID LevelUtil_GetTargetObjID(CLevel* level, LevelSkillTarget& target)
{
	if ((target.tp == LevelSkillTarget::Target_DefObj) || (target.tp == LevelSkillTarget::Target_FixPosAndObj))
		return target.ObjID();
	return LevelObjID_Invalid;
}


CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target)
{
	return LevelUtil_GetAliveLo(level, LevelUtil_GetTargetObjID(level, target));
}



BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D)
{
	switch(target.tp)
	{
	case LevelSkillTarget::Target_DefObj:
	case LevelSkillTarget::Target_FixPosAndObj:
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(level,target.ObjID());
			if (!lo)
				return FALSE;
			pos3D=lo->GetFramePos3D();
			return TRUE;
		}
	case LevelSkillTarget::Target_Pos:
	case LevelSkillTarget::Target_Aim:
		{
			LevelPos pos=(target.tp==LevelSkillTarget::Target_Pos)?target.Pos():target.Aim();
			GameTileMap *gtm=level->GetGtm();
			LevelPos3DFrom2D(pos3D,pos,gtm);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos)
{
	switch(target.tp)
	{
	case LevelSkillTarget::Target_DefObj:
	case LevelSkillTarget::Target_FixPosAndObj:
		{
			CLevelObj *lo=level->GetIDs()->LoFromID(target.ObjID());
			if (!lo)
				return FALSE;
			pos=lo->GetFramePos();
			return TRUE;
		}
	case LevelSkillTarget::Target_ObjPos:
		{
			CLevelObj *lo=level->GetIDs()->LoFromID(target.ObjID());
			if (!lo)
				pos=target.Pos();
			else
				pos=lo->GetFramePos();
			return TRUE;
		}
	case LevelSkillTarget::Target_Pos:
	case LevelSkillTarget::Target_Aim:
		{
			pos=(target.tp==LevelSkillTarget::Target_Pos)?target.Pos():target.Aim();
			return TRUE;
		}
	}
	return FALSE;
}



BOOL LevelUtil_CalcTargetAimPos3D(CLevel *level,LevelSkillTarget &target,float htCast,LevelPos3D &pos3D)
{
	switch(target.tp)
	{
	case LevelSkillTarget::Target_DefObj:
	case LevelSkillTarget::Target_FixPosAndObj:
		{
			CLevelObj *lo=level->GetIDs()->LoFromID(target.ObjID());
			if (!lo)
				return FALSE;
			pos3D=lo->GetFramePos3D();
			pos3D.y+=lo->GetAimHeight();
			return TRUE;
		}
	case LevelSkillTarget::Target_Pos:
	case LevelSkillTarget::Target_Aim:
		{
			LevelPos pos=(target.tp==LevelSkillTarget::Target_Pos)?target.Pos():target.Aim();
			GameTileMap *gtm=level->GetGtm();
			LevelPos3DFrom2D(pos3D,pos,gtm);
			pos3D.y+=htCast;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LevelUtil_CalcTargetDir(CLevelObj *loSrc,LevelSkillTarget &target,LevelPos&dir)
{
	CLevel *level=loSrc->GetLevel();
	LevelPos posMe=loSrc->GetFramePos();
	LevelPos posTarget;
	if (FALSE==LevelUtil_CalcTargetPos(level,target,posTarget))
		return FALSE;

	dir=posTarget-posMe;
	dir.safe_normalize();
	return TRUE;
}


//检查talks里面Active的node,如果这个node对应的player已经不存在了,则Deactive这个node
void LevelUtil_VerifyTalksPlayer(CLevel *level,CLevelTalks *talks)
{
	if (!talks)
		return;
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		CLevelPlayer *player=level->GetPlayer((LevelPlayerID)i);
		if (!player)
		{
			if (talks->GetState((LevelPlayerID)i)!=LevelTalk_None)
				talks->ClearActive((LevelPlayerID)i);
		}
	}
	
}

BOOL LevelUtil_ConvertSkillTarget(CLevel *level,LevelSkillTarget &target,LevelSkillTarget::Type tp)
{
	if (tp==LevelSkillTarget::Target_Aim)
	{
		if (target.tp==LevelSkillTarget::Target_DefObj)
		{
			CLevelObj*loTarget=level->GetIDs()->LoFromID(target.ObjID());
			if (loTarget)
			{
				target.SetAim(loTarget->GetFramePos());
				return TRUE;
			}
		}
	}

	if (tp==LevelSkillTarget::Target_Pos)
	{
		if (target.tp==LevelSkillTarget::Target_DefObj)
		{
			CLevelObj*loTarget=level->GetIDs()->LoFromID(target.ObjID());
			if (loTarget)
			{
				target.SetPos(loTarget->GetFramePos());
				return TRUE;
			}
		}
	}

	if (tp==LevelSkillTarget::Target_Pos3D)
	{
		if (target.tp==LevelSkillTarget::Target_DefObj)
		{
			CLevelObj*loTarget=level->GetIDs()->LoFromID(target.ObjID());
			if (loTarget)
			{
				target.SetPos3D(loTarget->GetFramePos3D());
				return TRUE;
			}
		}
	}

	return FALSE;
}


void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting)
{
	CLevelBuffs *buffs=lo->GetBuffs();
	if (buffs)
	{
		float ias=buffs->GetIAS();
		tCasting+=(AnimTick)(((float)dt)*ias);
	}
	else
		tCasting+=dt;
}

CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo)
{
	if(!lo)
		return NULL;
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		if (driver->IsSkillCasting())
			return driver->GetSkill();
	}
	return NULL;
}


void LevelUtil_BuildPathKeyset(KeySet &ks,std::vector<LevelPos>&sites,float speed)
{
	KeySet_Define(&ks,KT_Floatx2,0);
	ks.SetKeyCount(sites.size());

	AnimTick t=0;

	for (int i=0;i<sites.size();i++)
	{
		Key_2f *k=(Key_2f *)ks.GetKey(i);
		k->v=sites[i];
		k->t=t;

		if (i+1<sites.size())
			t+=ANIMTICK_FROM_SECOND(sites[i+1].getDistanceFrom(sites[i])/speed);
	}
}

BOOL LevelUtil_BuildPathKeyset(CLevel *level,KeySet &ksPos,KeySet &ksFace,AnimTick &dur,std::vector<LevelPathesEvent> *&events,LevelXfm &xfmBase,float scale,RecordID idPathRes)
{
	CLevelResources *res=level->GetResources();
	if (!res)
		return FALSE;

	LevelPath *path=res->FindPath(idPathRes);//Find the default path of idPathRes
	if (!path)
		return FALSE;

	i_math::matrix43f matBase;
	if (TRUE)
	{
		i_math::xformf xfm;
		xfm.pos.x=xfmBase.pos.x;
		xfm.pos.y=0.0f;
		xfm.pos.z=xfmBase.pos.y;

		i_math::vector3df euler;
		euler.x=LevelFaceToEuler(xfmBase.face);
		xfm.rot.fromEuler(euler);

		xfm.getMatrix(matBase);
	}

	ksPos.CopyFrom(path->ksPos);

	DWORD c=ksPos.GetKeyCount();
	for (int i=0;i<c;i++)
	{
		Key_2f *k=(Key_2f *)ksPos.GetKey(i);
		i_math::vector3df pos;
		pos.x=k->v.x*scale;
		pos.z=k->v.y*scale;
		pos.y=0.0f;
		matBase.transformVect(pos,pos);
		k->v=pos.getXZ();
	}

	ksFace.CopyFrom(path->ksFace);
	c=ksFace.GetKeyCount();
	for (int i=0;i<c;i++)
	{
		Key_f *k=(Key_f *)ksFace.GetKey(i);

		k->v=xfmBase.face+k->v-i_math::Pi/2.0f;
	}

	dur=path->dur;

	LevelPathes *pathes=res->FindPathes(idPathRes);
	if (pathes)
		events=&pathes->events;

	return TRUE;
}


float LevelUtil_GetMountHeight(CLevelObj *lo)
{
	if (lo->GetType()==LevelObjType_Unit)
	{
		LevelRecordUnit *rec=((CLoUnit *)lo)->GetRec();
		if (rec)
			return rec->htMount;
	}
	return 0.0f;
}

LevelRecordUnit *LevelUtil_GetUnitRecord(CLevelObj *lo)
{
	if (lo->GetType()==LevelObjType_Unit)
		return ((CLoUnit*)lo)->GetRec();
	return NULL;
}

int LevelUtil_CalcSkillStack(LevelSkillType &tpSkill,CLevelObj *owner)
{
	LevelRecordSkill *recSkill=LevelUtil_GetSkillRecord(owner,tpSkill);
	if (!recSkill)
		return 0;

	if (!owner)
		return 0;

	LevelCost *cost=&recSkill->cost;

	LevelAttr_Base *attrBase=owner->GetAttr_Base();
	LevelAttr_Resource *attrResource=owner->GetAttr_Resource();

	if (!attrBase)
		return 0;

	if (!attrResource)
		return 0;

	int nStack=MAX_LEVEL_SKILL_STACK;
	int n;
	float spCost=(float)cost->sp;
	float spMaxCost=0.0f;
	LevelUtil_ModSPCost(owner,spCost,spMaxCost);
	if (spCost>0.0f)
	{
		n=(int)(((float)attrBase->sp.GetCur_Int())/spCost);

		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	n=MAX_LEVEL_SKILL_STACK;
	if (cost->gold>0)
	{
		n=attrResource->Get(LevelResource_Gold)->GetCur_Int()/cost->gold;
		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	if (cost->gem>0)
	{
		n=attrResource->Get(LevelResource_Gem)->GetCur_Int()/cost->gem;
		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	if (cost->soul>0)
	{
		n=attrResource->Get(LevelResource_Soul)->GetCur_Int()/cost->soul;
		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	if (cost->crystal_>0)
	{
		n=attrResource->Get(LevelResource_Crystal)->GetCur_Int()/cost->crystal_;
		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	if (cost->mp>0)
	{
		DWORD nMP=0;
		CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)LevelUtil_GetActiveAbility(owner,LevelAbilityType_MagicRing);
		if (ability)
			nMP=ability->GetMP();
		n=nMP/cost->mp;
		if (n<nStack)
			nStack=n;
	}
	if (nStack<=0)
		return 0;

	return nStack;
}


BOOL LevelUtil_TestSkillCost(LevelSkillType &tpSkill,CLevelObj *owner)
{
	return LevelUtil_CalcSkillStack(tpSkill,owner)>0;
}

BOOL LevelUtil_CheckNeedAI(CLevelObj *lo)
{
	if (!lo)
		return FALSE;
// 	if (LevelUtil_CheckDead(lo))
// 		return FALSE;//已死
	if (lo->IsPlayer())
		return FALSE;//为主角

	return TRUE;
}

//把地图中的一个NPC变成某个Player的Retinue NPC
BOOL LevelUtil_SwitchNPCRetinue(CLevelPlayer *player,RecordID idNPC)
{
	CLevel *level=player->GetLevel();
	CLevelNPCs *npcsLevel=level->GetNPCs(player->GetPlayerID());
	if (!npcsLevel)
		return FALSE;

	CLevelNPCs *npcsPlayer=player->GetRtnuNPCs();
	if (!npcsPlayer)
		return FALSE;

	CLevelNPC *npc=npcsLevel->FetchNPC(idNPC);
	if (!npc)
		return FALSE;

	npcsPlayer->AddNPC(npc);
	npc->SwitchState(NPCState_Retinue);

	return TRUE;
}

float LevelUtil_GetSpeed(CLevelObj *lo)
{
	CLevelObjMove *move=lo->GetMove();
	if (!move)
		return 0.0f;
	return move->GetUnitSpeed();

}


BOOL LevelUtil_AddCoSkillCharge(CLevelObj *lo,LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target)
{
	if (lo->GetType()!=LevelObjType_Unit)
		return FALSE;
	CLoUnit *loUnit=(CLoUnit *)lo;

	CLevelCoSkill *coskill=loUnit->ObtainCoSkill();
	if (!coskill)
		return FALSE;

	return coskill->AddCharge(recSkill->GetID(),grd,target);
}

BOOL LevelUtil_CanCancelSkill(CLevelObj *lo)
{
	CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
	if (!skill)
		return FALSE;
	return skill->CanCancel();
}


LevelSkillID LevelUtil_CancelSkill(CLevelObj *lo,BOOL bStopAct)
{
	LevelSkillID idSkill=LevelSkillID_Invalid;

	CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
	if (!skill)
		return LevelSkillID_Invalid;

	skill->Cancel();

	if (bStopAct)
		LevelUtil_StopMove(lo);

	if (TRUE)
	{
		LevelOp_CancelSkill *op=lo->NewOp<LevelOp_CancelSkill>(LevelOpLink());
		op->idSkill=skill->GetID();
		op->bStopAct=bStopAct;
		lo->AddOp(op);
	}

	return skill->GetID();
}

LevelRelation LevelUtil_CalcPlayerRelation(LevelRelationMatrix *matRelation,LevelPlayerID idPlayer1,LevelPlayerID idPlayer2)
{
	if ((idPlayer1==LevelPlayerID_NeutralWild)||(idPlayer2==LevelPlayerID_NeutralWild))
		return LevelRelation_Neutral;
	if (idPlayer1==idPlayer2)
		return LevelRelation_Native;

	if (!matRelation)
		return LevelRelation_Neutral;

	if (matRelation->GetEnemies(idPlayer1)&(1<<idPlayer2))
		return LevelRelation_Enemy;

	if (matRelation->GetAllies(idPlayer1)&(1<<idPlayer2))
		return LevelRelation_Ally;

	return LevelRelation_Neutral;
}


BOOL LevelUtil_CheckDay(LevelPlayerID idPlayer,CLoAgent *loAgent)
{
	if (!loAgent)
		return FALSE;
	CLevel *level=loAgent->GetLevel();
	if (!level)
		return FALSE;
	LevelPlayerStates *lps=level->GetLPS(idPlayer);
	if (!lps)
		return FALSE;
	if (lps)
	{
		CLevelObjParam*lop=loAgent->GetLop();
		if (lop)
		{
			LevelGUID guid=lop->GetGUID_();

			if (guid!=LevelGUID_Invalid)
			{
				RecordID idMap=level->GetMapID();
				LevelPersistEntry_AgentS entry;
				DWORD iDayOld;
				if (entry.mem_.GetValue(LevelSimpleVarName_CheckDay,iDayOld))
				{
					if (iDayOld==lps->base.iDay)
						return TRUE;//没变
				}
				entry.mem_.SetValue(LevelSimpleVarName_CheckDay,lps->base.iDay);
				LPS_SetPersistEntry_AgentS(lps,idMap,guid,entry);

				return TRUE;
			}
		}
	}

	return FALSE;

}

BOOL LevelUtil_DeactivateAbility(CLevelPlayer *player,LevelAbilityType tp)
{
	if (!player)
		return FALSE;
	CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tp);
	if (ability)
	{
		ability->SetInactive();
//		ability->Clear();
		return TRUE;
	}
	return FALSE;
}


BOOL LevelUtil_ActivateAbility(CLevelAbilities *abilities,LevelAbilityType tp,CLevelRecords *records)
{
	if (abilities)
	{
		CLevelAbility *ability=abilities->GetAbility(tp);
		if (ability)
		{
			if (ability->IsActive())
				return TRUE;
			ability->SetActive();
			if (records)
			{
				CRecords *recordsUpgrade=records->GetRecords_Upgrade();
				DWORD c;
				RecordID *ids=recordsUpgrade->GetRecords(c);
				for (int i=0;i<c;i++)
				{
					LevelRecordUpgrade *rec=(LevelRecordUpgrade *)recordsUpgrade->GetSafeRecord(ids[i]);
					if (rec)
					{
						if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
						{
							CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
							if (upgrade->GetAbilityType()==tp)
							{
								if (upgrade->Init(ability))
									break;
							}
						}
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}


BOOL LevelUtil_ActivateAbility(CLevelPlayer *player,LevelAbilityType tp)
{
	if (!player)
		return FALSE;

	CLevelRecords *records=NULL;
	if (player->GetLevel())
		records=player->GetLevel()->GetRecords();

	if (LevelUtil_ActivateAbility(&player->GetAbilities(),tp,records))
		LPS_SaveAbilities(player->GetLPS(),&player->GetAbilities());

	return FALSE;
}

CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp)
{
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
		return player->GetAbilities().GetActiveAbility(tp);
	return NULL;
}

CLevelAbility *LevelUtil_GetActiveAbility(CLevelPlayer *player,LevelAbilityType tp)
{
	if (player)
		return player->GetAbilities().GetActiveAbility(tp);
	return NULL;
}


void LevelUtil_SaveAbilities(CLevelObj *lo)
{
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
	{
		LevelPlayerStates *lps=LevelUtil_GetLPS(lo);
		if (lps)
			LPS_SaveAbilities(lps,&player->GetAbilities());
	}
}

EquipPart LevelUtil_GetItemEquipPart(CLevelRecords *records,RecordID idItem)
{
	if (records)
	{
		LevelRecordItemClass *recItemClass=records->GetItemClassOfItem(idItem);
		if (recItemClass)
			return (EquipPart)recItemClass->part;
	}
	return EquipPart_Invalid;
}


EquipPart LevelUtil_GetItemEquipPart(CLevel *level,RecordID idItem)
{
	if (level)
		return LevelUtil_GetItemEquipPart(level->GetRecords(),idItem);

	return EquipPart_Invalid;
}

RecordID LevelUtil_ItemFromAbilityType(CLevelRecords *records,LevelAbilityType tp)
{
	if (!records)
		return RecordID_Invalid;

	static RecordID caches[LevelAbilityType_Max];
	static BOOL bCacheInit=FALSE;
	if (!bCacheInit)
	{
		memset(caches,0xff,sizeof(caches));
		bCacheInit=TRUE;
	}

	if (tp>=LevelAbilityType_Max)
		return RecordID_Invalid;
	if (caches[tp]==0xffffffff)
	{
		caches[tp]=RecordID_Invalid;

		CRecords *recordsItem=records->GetRecords_Item();
		DWORD c;
		RecordID *ids=recordsItem->GetRecords(c);
		for (int i=0;i<c;i++)
		{
			LevelRecordItem *recItem=(LevelRecordItem *)recordsItem->GetRecord(ids[i]);
			if (recItem)
			{
				if (recItem->tpAbility==tp)
				{
					caches[tp]=ids[i];
					break;
				}
			}
		}
	}

	return caches[tp];
}

LevelArtifactType LevelUtil_ArtifactTypeFromAbilityType(CLevelRecords *records,LevelAbilityType tp)
{
	RecordID idItem=LevelUtil_ItemFromAbilityType(records,tp);
	if (idItem!=RecordID_Invalid)
	{
		LevelRecordItem *rec=records->GetItem(idItem);
		if (rec)
			return rec->tpArtifact;
	}
	return LevelArtifact_None;
}




BOOL LevelUtil_RemoveArtifact(CLevelPlayer *player,EquipPart part)
{
	if (player)
	{
		CLevelRecords *records=player->GetLevel()->GetRecords();
		if(records)
		{
			LevelPlayerStates *lps=player->GetLPS();
			if (lps)
			{
				if (part<ARRAYSIZE(lps->equip.parts))
				{
					if (lps->equip.parts[part].IsValid())
					{
						LevelRecordItem *recItem=records->GetItem(lps->equip.parts[part].tid);
						if (recItem)
						{
							if (recItem->tpAbility!=LevelAbilityType_None)
								LevelUtil_DeactivateAbility(player,recItem->tpAbility);

							LPS_RemoveEquipment(lps,part);
							return TRUE;
						}
					}
				}
			}
		}
	}

	return FALSE;
}

BOOL LevelUtil_RemoveEquip(CLevelPlayer *player,RecordID idItem)
{
	LevelPlayerStates *lps=player->GetLPS();
	if (lps)
	{
		EquipPart part=LPS_EquipPartFromItem(lps,idItem);
		if (part!=EquipPart_Invalid)
			return LevelUtil_RemoveArtifact(player,part);
	}
	return FALSE;
}


BOOL LevelUtil_AddArtifact(CLevelPlayer *player,RecordID idItem,int nStack)
{
	if (player->GetLevel())
	{
		CLevelRecords *records=player->GetLevel()->GetRecords();
		if(records)
		{
			if (player->GetLPS())
			{
				LevelPlayerStates *lps=player->GetLPS();
				LevelRecordItem *rec=records->GetItem(idItem);
				if (rec)
				{
					if (rec->tpArtifact!=LevelArtifact_None)
					{
						extern BOOL LPS_AddArtifact(LevelPlayerStates *lps,CLevelRecords *records,RecordID idItem,int nStack);
						LPS_AddArtifact(lps,records,idItem,nStack);

						
						if (rec->tpAbility!=LevelAbilityType_None)
						{
							//激活Ability,如果需要的话
							LevelUtil_ActivateAbility(player,rec->tpAbility);

						}

						//更新角色的外观
						if (player->GetLoUnit())
							player->GetLoUnit()->UpdateExprEquips(lps);

						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

void LevelUtil_RemoveArtifact(CLevelPlayer *player,LevelArtifactType tpArtifact)
{
	if (player->GetLevel())
	{
		CLevelRecords *records=player->GetLevel()->GetRecords();
		if(records)
		{
			if (player->GetLPS())
			{
				LevelAbilityType tpAbility=LevelUtil_AbilityFromArtifact(player,tpArtifact);

				if (tpAbility!=LevelAbilityType_None)
				{
					//激活Ability,如果需要的话
					LevelUtil_DeactivateAbility(player,tpAbility);
				}

				LevelPlayerStates *lps=player->GetLPS();
				extern BOOL LPS_RemoveArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp);
				LPS_RemoveArtifact(lps,records,tpArtifact);

				//更新角色的外观
				if (player->GetLoUnit())
					player->GetLoUnit()->UpdateExprEquips(lps);
			}
		}
	}
}


LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill)
{
	if (!lo)
		return NULL;
	CLevel *level=lo->GetLevel();
	if (!level)
		return NULL;

	if (tpSkill.tpAbility_!=LevelAbilityType_None)
	{
		CLevelPlayer *player=level->GetPlayer(lo->GetPlayerID());
		if (player)
		{
			CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpSkill.tpAbility_);
			if (ability)
				return ability->GetSkillRecordRT(tpSkill.actionAbility);
		}
		return NULL;
	}

	return level->GetRecords()->GetSkill(tpSkill.idSkill);
}

LevelSkillGrade LevelUtil_GetAbilitySkillGrade(CLevelObj *lo,LevelAbilityType tpAbility)
{
	if (!lo)
		return LevelSkillGrade_Invalid;
	CLevel *level=lo->GetLevel();
	if (!level)
		return LevelSkillGrade_Invalid;

	if (tpAbility!=LevelAbilityType_None)
	{
		CLevelPlayer *player=level->GetPlayer(lo->GetPlayerID());
		if (player)
		{
			CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
			if (ability)
				return ability->GetSkillGradeRT();
		}
		return LevelSkillGrade_Invalid;
	}

	return LevelSkillGrade_Invalid;

}

BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit)
{
	i_math::aabbox3df aabb;
	aabb.reset(center.x,center.y+height/2.0f,center.z);
	aabb.inflate(radius,height/2.0f+fall,radius);

	if (aabb.isPointInside(line.start))
	{//嵌在里面
		vHit=line.start;
		return TRUE;
	}

	if (aabb.intersectsWithLine(line))
	{
		i_math::vector3df leave;
		if (aabb.calcIntersectionWithLine(line.start,line.getVector(),vHit,leave))
			return TRUE;
	}
	return FALSE;
}

BOOL LevelUtil_ShieldAmuletHitTest(i_math::line3df &line,float radius,CLevelObj *loUnit,i_math::vector3df &vHit)
{
	if (!loUnit->IsPlayer())
		return FALSE;

	CLevelAbility_ShieldAmulet *ability=(CLevelAbility_ShieldAmulet *)LevelUtil_GetActiveAbility(loUnit,LevelAbilityType_ShieldAmulet);
	if (!ability)
		return FALSE;
	return ability->HitTest(line,radius,vHit);
}


BOOL LevelUtil_AddEventSrc(LevelOSB &osb,CLevelObj *target,LevelEventType tp)
{
	CLevelEventSrc *src=target->GetEventSrc();
	if (src)
	{
		CLevel *level=osb.GetLevel();
		CLevelObj *owner=osb.GetOwner();
		if (owner)
		{
			src->Add(tp,osb.GetRootOwnerID(),owner->GetT());
			return TRUE;
		}
	}

	return FALSE;
}

BOOL LevelUtil_AddStunSrc(CLevelBuff *buff)
{
	CLevel *level=buff->GetLevel();
	CLevelObj *lo=buff->GetOwner();

	CLevelEventSrc *src=lo->GetEventSrc();
	if (src)
	{
		AnimTick t=level->GetT_();
		RecordID idBrokenSkill=RecordID_Invalid;
		StringID idBrokenSkillStage=StringID_Invalid;
		CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
		if (skill)
		{
			idBrokenSkill=skill->GetRecID();
			if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
				idBrokenSkillStage=((Skill_GeneralAdvS *)skill)->GetStageNameID();
		}
		src->AddStun(buff,idBrokenSkill,idBrokenSkillStage,t);
		return TRUE;
	}
	return FALSE;
}

BOOL LevelUtil_NotififyStunSrc_Finish(CLevelBuff *buff)
{
	CLevel *level=buff->GetLevel();
	CLevelObj *lo=buff->GetOwner();

	CLevelEventSrc *src=lo->GetEventSrc();
	if (src)
	{
		AnimTick t=level->GetT_();
		src->NofityStunFinish(buff,t);
		return TRUE;
	}
	return FALSE;
}

//接管弱点
void LevelUtil_TakeOverWeaksOverride(CLevelBuff *buff)
{
	CLevel *level=buff->GetLevel();
	CLevelObj *lo=buff->GetOwner();

	LevelWeaksPack wkpkOverride;
	LevelAttr_WeaksMod *attrWeaksMod=NULL;
	if (lo)
	{
		attrWeaksMod=lo->GetAttr_WeaksMod();
		if (attrWeaksMod)
		{
			if (attrWeaksMod->overrideCur.bValid&&attrWeaksMod->overrideCur.bCanTakeOver)
			{
				LevelWeaksPack wkpkOverride=attrWeaksMod->overrideCur.wkpk;
				attrWeaksMod->SetOverride(wkpkOverride,buff,level->GetT_(),TRUE);
			}
			else
			{
				if (!attrWeaksMod->overrideCur.bValid)
				{
					if (attrWeaksMod->overrideLast.bValid&&attrWeaksMod->overrideLast.bCanTakeOver)
					{
						if (attrWeaksMod->overrideLast.tFinish==level->GetT_())
						{
							LevelWeaksPack wkpkLastOverride=attrWeaksMod->overrideLast.wkpk;
							attrWeaksMod->SetOverride(wkpkLastOverride,buff,level->GetT_(),TRUE);
						}
					}
				}
			}
		}
	}
}

void LevelUtil_ClearWeaksOverride(CLevelBuff *buff)
{
	CLevel *level=buff->GetLevel();
	CLevelObj *lo=buff->GetOwner();

	LevelWeaksPack wkpkOverride;
	LevelAttr_WeaksMod *attrWeaksMod=NULL;
	if (lo)
	{
		attrWeaksMod=lo->GetAttr_WeaksMod();
		if (attrWeaksMod)
			attrWeaksMod->ClearOverride(buff,level->GetT_());
	}
}

void LevelUtil_SetWeaksFilter(WeaksEx &weaks,CLevelBgn *bgn)
{
	CLevelObj *lo=bgn->GetLo();
	if (lo)
	{
		LevelAttr_WeaksMod *mod=lo->GetAttr_WeaksMod();
		if (mod)
		{
			LevelWeaksPack wkpk;
			weaks.ToWeakPack(wkpk);
			mod->SetFilter(wkpk,bgn);
		}
	}
}

void LevelUtil_ClearWeaksFilter(CLevelBgn *bgn)
{
	CLevelObj *lo=bgn->GetLo();
	if (lo)
	{
		LevelAttr_WeaksMod *mod=lo->GetAttr_WeaksMod();
		if (mod)
			mod->ClearFilter(bgn);
	}
}


LevelPlayerID LevelUtil_GetTalkingPlayer(LevelBehaviorContext *ctx,CLevelTalks *talks)
{
	if (talks->IsExclusiveMode())
		return talks->GetFirstActive();
	return ctx->idPlayerTalk;
}

LevelPos LevelUtil_CalcPredictedPos(CLevelObj *loSrc,CLevelObj *loTarget,float dtPredict)
{
	if ((!loSrc)||(!loTarget))
		return LevelPos(0,0);

	LevelPos posTarget=loTarget->GetFramePos();
	LevelPos posSrc=loSrc->GetFramePos();
	LevelPos dirToTarget=posTarget-posSrc;

	LevelPos posPredict;

	CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
	if (player)
	{
		CLevelPlayerMove &move=player->GetMove();
		LevelMoveStep step;
		move.GetRecentMoveStep(loTarget->GetLevel()->GetT_(),ANIMTICK_FROM_SECOND(dtPredict),step);
		LevelPos posCur=loTarget->GetFramePos();
		posPredict=posCur+posCur-step.pos;
	}
	else
	{
		CLevelObjMove *move=loTarget->GetMove();
		if (move)
		{
			LevelPos vel=move->GetVel();
			vel*=dtPredict;
			posPredict=posTarget+vel;
		}
	}

	loTarget->GetLevel()->GetDbgDraw().DrawCircle(posTarget,0.2f,RGB(0,255,0),2.0f);
	loTarget->GetLevel()->GetDbgDraw().DrawCircle(posPredict,0.2f,RGB(255,0,0),2.0f);

	if (dirToTarget.getLengthSQ()>0.0001f)
	{

		LevelPos dirToPredict;
		dirToPredict=posPredict-posTarget;

		LevelFace faceSrcToTarget=LevelFaceFromDir(dirToTarget);
		LevelFace faceTargetToPredict=LevelFaceFromDir(dirToPredict);

		float gap=i_math::get_radian_dist(faceSrcToTarget,faceTargetToPredict);

		float lo=90.0f*i_math::GRAD_PI2;
		float hi=135.0f*i_math::GRAD_PI2;

		float rate=i_math::clamp_f((gap-lo)/(hi-lo),0.0f,1.0f);
		rate=1.0f-rate;

		dirToPredict*=rate;

		posPredict=posTarget+dirToPredict;
	}

	loTarget->GetLevel()->GetDbgDraw().DrawCircle(posPredict,0.2f,RGB(0,0,255),2.0f);


	return posPredict;
}

//根据起点和瞄准点,产生若干个散射的方向,为normalized过的
void LevelUtil_ScatterDirs(LevelPos &src,LevelPos &target,LevelPos *dirs,float degFov,DWORD c)
{
	LevelPos dir=target-src;
	if (c==1)
	{
		dir.safe_normalize();
		dirs[0]=dir;
		return;
	}

	float MaxRange=90.0f*(float)i_math::GRAD_PI2;
	float MinRange=10.0f*(float)i_math::GRAD_PI2;
	float MaxDist=12.0f;

	if (TRUE)
	{
		float dist=dir.getLength();
		float rad=0.0f;
		if (dist>0.01f)
			rad=atan2f(dir.y,dir.x);
		else
			rad=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);

		float range=degFov*i_math::GRAD_PI2/2.0f;
		float step=range*2.0f/(float)(c-1);

		float radFrom=rad-range;

		for (int i=0;i<c;i++)
		{
			float r=radFrom+step*(float)i;
			dirs[i].x=cosf(r);
			dirs[i].y=sinf(r);
		}
	}
}

float LevelUtil_GenRandomFace()
{
	return CSysRandom::RandRange<float>(0.0f,i_math::Pi*2.0f);//随机角度
}

void LevelUtil_RandomOffsetTargetPos2D(i_math::vector3df &posTarget,float radiusMin,float radiusMax)
{
	float rad=LevelUtil_GenRandomFace();
	float radius=CSysRandom::RandRange(radiusMin,radiusMax);

	posTarget.x+=cosf(rad)*radius;
	posTarget.z+=sinf(rad)*radius;
}

UnitFindPathType LevelUtil_GetFindPathType(CLevelObj *lo)
{
	CUnit *unit=lo->GetUnit();
	if (unit)
		return unit->GetPathFindType();

	return UnitFindPath_Walkable;//缺省值
}

BOOL LevelUtil_CheckPathValidity(CLevelObj *lo,RecordID idPath,LevelXfm &xfmBase)
{
	if (!lo)
		return FALSE;
	CLevel *level=lo->GetLevel();
	if (!level)
		return FALSE;

	CLevelResources *resources=level->GetResources();
	LevelLoResoPath *path=resources->FindLoResoPath(idPath);
	if (!path)
		return FALSE;

	i_math::matrix43f mat;
	if (TRUE)
	{
		i_math::vector3df euler;
		i_math::xformf xfm;
		xfm.pos.set(xfmBase.pos.x,0.0f,xfmBase.pos.y);
		euler.x=LevelFaceToEuler(xfmBase.face);
		xfm.rot.fromEuler(euler);

		xfm.getMatrix(mat);
	}


	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
	extern UnitFindPathType LevelUtil_GetFindPathType(CLevelObj *lo);
	UnitFindPathType tpFindPath=LevelUtil_GetFindPathType(lo);

	float scale=lo->GetModelScale();

	DWORD c=path->ks.GetKeyCount();
	LevelPos pos,posLast;
	int i=0;
	for (i=0;i<c;i++)
	{
		if (TRUE)
		{
			Key_2f *k=(Key_2f *)path->ks.GetKey(i);

			i_math::vector3df v;
			v.set(k->v.x,0.0,k->v.y);
			v*=scale;

			mat.transformVect(v,v);

			pos=v.getXZ();
		}

		if (!unitmgr->IsWalkable(tpFindPath,pos))
			break;

		if (i>0)
		{
			if (unitmgr->StaticObstacleTest(tpFindPath,posLast,pos))
				break;
		}

		posLast=pos;
	}

	if (i<c)
		return FALSE;

	return TRUE;
}

CLevelObj *LevelUtil_GetThreat(CLevelObj *lo)
{
	if (lo)
	{
		CLevelSensor*sensor=lo->GetSensor();
		if (sensor)
		{
			CLevelObj *loThreat=sensor->GetThreat();
			if (loThreat)
			{
				if (loThreat->IsAlive())
					return loThreat;
			}
			return NULL;
		}
	}
	return NULL;
}

void LevelUtil_UpdateAwardExpendable(LevelPlayerStates *lps,LevelAward *award)
{
	if (lps&&award)
	{
		award->bExpendable=0;
		if(lps)
		{
			if (award->tp==LevelAward::Resource)
			{
				if (award->tpRes==LevelResource_Gold)
				{
					if (lps->base.gold_>=award->count)
						award->bExpendable=1;
				}
				if (award->tpRes==LevelResource_Gem)
				{
					if (lps->base.gem_>=award->count)
						award->bExpendable=1;
				}
				if (award->tpRes==LevelResource_Crystal)
				{
					if (lps->base.crystal>=award->count)
						award->bExpendable=1;
				}
			}
		}
	}
}


void LevelUtil_UpdateAwardPriceAffordable(LevelPlayerStates *lps,LevelAwardPrice *price)
{
	if (lps&&price)
	{
		price->bAffordable=1;
		if(lps)
		{
			if (price->tpRes==LevelResource_Gold)
			{
				if (lps->base.gold_<price->count)
					price->bAffordable=0;
			}
			if (price->tpRes==LevelResource_Gem)
			{
				if (lps->base.gem_<price->count)
					price->bAffordable=0;
			}
			if (price->tpRes==LevelResource_Crystal)
			{
				if (lps->base.crystal<price->count)
					price->bAffordable=0;
			}
		}
	}
}

void LevelUtil_ApplyAwardPrice(CLevelPlayer *player,LevelAwardPrice *price)
{
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps&&price)
		{
			if (player->GetLevel())
			{
				CLevelObj *lo=(CLevelObj *)player->GetLoUnit();
				if (lo)
					player->GetLevel()->GetDecider()->MakeResModify(lo,(LevelResourceType)price->tpRes,-(int)price->count);
			}

			lps->base.SetDirtyDB_High();
		}
	}
}

BOOL LevelUtil_ApplyAward(CLevelPlayer *player,LevelAward *award,LevelAwardPrice *price)
{
	if (price)
	{
		if (!price->bAffordable)
			return FALSE;
	}
	if (!player)
		return FALSE;
	CLevel *level=player->GetLevel();
	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return FALSE;
	if (!level)
		return FALSE;

	switch(award->tp)
	{
		case LevelAward::Resource:
		{
			CLevelObj *lo=(CLevelObj*)player->GetLoUnit();
			if (lo)
				level->GetDecider()->MakeResModify(lo,award->tpRes,award->count);
			break;
		}
		case LevelAward::Item:
		{
			if (award->idRec!=RecordID_Invalid)
			{
				LevelRecordItem *recItem=level->GetRecords()->GetItem(award->idRec);
				if (recItem)
				{
					if (recItem->tpArtifact!=LevelArtifact_None)
					{
						extern BOOL LevelUtil_AddArtifact(CLevelPlayer *player,RecordID idItem,int nStack);
						LevelUtil_AddArtifact(player,award->idRec,award->count);
					}
					else
					{
						if (recItem->skill!=RecordID_Invalid)
						{
							extern BOOL LPS_AddSkillItem(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkillItem,DWORD nStack);

							LPS_AddSkillItem(lps,level->GetRecords(),award->idRec,award->count);
						}
					}
				}
			}
			break;
		}
		case LevelAward::Upgrade:
		{
			if (award->idRec!=RecordID_Invalid)
			{
				LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(award->idRec);
				if (rec)
				{
					if (rec->upgrade)
					{
						if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
						{
							CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
							player->UpgradeAbility(*upgrade,award->seed);
						}
						else
						{
							//xxxxxxxxxxxxxx
						}
					}
				}
			}

			break;
		}
		case LevelAward::LevelUp_Weapon:
		{
			if (award->idRec!=RecordID_Invalid)
			{
				LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(award->idRec);
				if (rec)
				{
					if (rec->upgrade)
					{
						if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
						{
							CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
							extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);
							if (upgrade->GetAbilityType()==LevelUtil_GetEquipingAbility((CLevelObj*)player->GetLoUnit()))
								player->UpgradeAbility(*upgrade,award->seed);
						}
					}
				}
			}
			break;
		}

	}

	//apply the price
	if (price)
	{
		extern void LevelUtil_ApplyAwardPrice(CLevelPlayer *player,LevelAwardPrice *price);
		LevelUtil_ApplyAwardPrice(player,price);
	}

	return TRUE;
}

BOOL LevelUtil_ExpendAward(CLevelPlayer *player,LevelAward *award)
{
	if (!award)
		return FALSE;
	if (award)
	{
		if (!award->bExpendable)
			return FALSE;
	}
	if (!player)
		return FALSE;
	CLevel *level=player->GetLevel();
	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return FALSE;
	if (!level)
		return FALSE;

	switch(award->tp)
	{
		case LevelAward::Resource:
		{
			CLevelObj *lo=(CLevelObj*)player->GetLoUnit();
			if (lo)
				level->GetDecider()->MakeResModify(lo,award->tpRes,-(int)award->count);
			break;
		}
	}

	return TRUE;
}


BOOL LevelUtil_UpdateExprEquips(ExprEquips *equips,LevelPlayerStates *lps,CLevelRecords *records)
{
	if (!equips)
		return FALSE;
	if (!lps)
		return FALSE;
	BOOL bMod=FALSE;

	RecordID items[EquipPart_MaxExpress];

	for (int i=0;i<EquipPart_MaxExpress;i++)
		items[i]=lps->equip.parts[i].tid;

	if (!lps->fasts.skillSel.IsEmpty())
	{
		LevelFastTarget &target=lps->fasts.skillSel;
		if ((target.mode==LevelFastTarget::AbilityMelee)||(target.mode==LevelFastTarget::AbilityMissile))
		{
			RecordID idItem=LevelUtil_ItemFromAbilityType(records,target.tp);
			if (idItem)
			{
				EquipPart part=LevelUtil_GetItemEquipPart(records,idItem);
				if (part<EquipPart_MaxExpress)
					items[part]=idItem;
			}
		}
	}

	for (int i=0;i<EquipPart_MaxExpress;i++)
	{
		if (equips->items[i]!=items[i])
		{
			bMod=TRUE;
			equips->items[i]=items[i];
		}
	}


	EquipPart wpnActive=lps->equip.GetActiveWpn();
	if (!lps->fasts.skillSel.IsEmpty())
	{
		LevelFastTarget &target=lps->fasts.skillSel;
		if (target.mode==LevelFastTarget::ShootArrow)
		{
			EquipPart part=LPS_FindEquipedBow(lps,records);
			if (part!=EquipPart_Invalid)
				wpnActive=part;
		}
	}

	if (wpnActive!=equips->wpnActive)
	{
		equips->wpnActive=wpnActive;
		bMod=TRUE;
	}

	return bMod;
}

RecordID LevelUtil_GetEquippingNonWeapon(CLevelObj *lo,EquipPart part)
{
	if (!lo)
		return RecordID_Invalid;
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (!player)
		return RecordID_Invalid;

	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return RecordID_Invalid;

	CLevel *level=lo->GetLevel();
	if (!level)
		return RecordID_Invalid;
	CLevelRecords *records=level->GetRecords();
	if (!records)
		return RecordID_Invalid;

	ExprEquips equips;
	LevelUtil_UpdateExprEquips(&equips,lps,records);
	return equips.items[part];
}

RecordID LevelUtil_GetEquippingWeapon(CLevelObj *lo,RecordID *id2ndWpn)
{
	if (id2ndWpn)
		*id2ndWpn=RecordID_Invalid;

	if (!lo)
		return RecordID_Invalid;
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (!player)
		return RecordID_Invalid;

	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return RecordID_Invalid;

	CLevel *level=lo->GetLevel();
	if (!level)
		return RecordID_Invalid;
	CLevelRecords *records=level->GetRecords();
	if (!records)
		return RecordID_Invalid;

	ExprEquips equips;
	LevelUtil_UpdateExprEquips(&equips,lps,records);

	if ((equips.wpnActive!=EquipPart_Weapon)&&(equips.wpnActive!=EquipPart_Weapon2nd))
		return RecordID_Invalid;

	if (id2ndWpn)
	{
		EquipPart part2nd=EquipPart_Weapon;
		if (equips.wpnActive==EquipPart_Weapon)
			part2nd=EquipPart_Weapon2nd;
		(*id2ndWpn)=equips.items[part2nd];
	}
	return equips.items[equips.wpnActive];
}

LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo)
{
	RecordID idRec=LevelUtil_GetEquippingWeapon(lo);
	if (idRec!=RecordID_Invalid)
	{
		CLevel *level=lo->GetLevel();
		if (level)
		{
			CLevelRecords *records=level->GetRecords();
			if (records)
			{
				LevelRecordItem *rec=records->GetItem(idRec);
				if (rec)
					return rec->tpAbility;
			}
		}
	}
	return LevelAbilityType_None;
}

void LevelUtil_GetWeaponAbilities(CLevelObj *lo, LevelAbilityType &tpActive, LevelAbilityType&tpInactive)
{
    tpActive = LevelAbilityType_None;
    tpInactive = LevelAbilityType_None;
    if (!lo)
        return;
    CLevelPlayer *player = LevelUtil_PlayerFromLo(lo);
    if (!player)
        return;

    LevelPlayerStates *lps = player->GetLPS();
    if (!lps)
        return;

    if ((lps->equip.wpnActive != EquipPart_Weapon) && (lps->equip.wpnActive != EquipPart_Weapon2nd))
        return;

    CLevel *level = lo->GetLevel();
    if (!level)
        return;
    CLevelRecords *records = level->GetRecords();
    if (!records)
        return;

    RecordID ids[2];
    ids[0] = lps->equip.parts[lps->equip.wpnActive].tid;
    if (lps->equip.wpnActive == EquipPart_Weapon)
        ids[1] = lps->equip.parts[EquipPart_Weapon2nd].tid;
    else
        ids[1] = lps->equip.parts[EquipPart_Weapon].tid;

    LevelRecordItem *rec = records->GetItem(ids[0]);
    if (rec)
        tpActive=rec->tpAbility;
    rec = records->GetItem(ids[1]);
    if (rec)
        tpInactive = rec->tpAbility;
}



BOOL LevelUtil_CheckEquipingAbility(CLevelObj *lo,LevelAbilityType tp)
{
	return LevelUtil_GetEquipingAbility(lo)==tp;
}

void LevelUtil_SendEvent(CLevelObj *lo,LevelEvent &e)
{
	lo->HandleEvent(e);
	if (e.bHandled)
		return;

	if (TRUE)
	{
		CLevelObj *loRootOwner=lo;
		LevelObjID idRootOwner=lo->GetRootOwnerID();
		if (loRootOwner->GetID()!=idRootOwner)
			loRootOwner=LevelUtil_GetAliveLo(loRootOwner->GetLevel(),idRootOwner);
		CLevelPlayer *player=LevelUtil_PlayerFromLo(loRootOwner);
		if (player)
		{
			player->GetAbilities().HandleEvent(e);
			if (e.bHandled)
				return;
		}
	}
}


CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level)
{
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		CLevelPlayer *player=level->GetPlayer((LevelPlayerID)i);
		if (player)
			return player;
	}
	return NULL;
}

CLoUnit*LevelUtil_GetFirstPlayerLoUnit(CLevel *level)
{
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);
	if (player)
		return player->GetLoUnit();
	return NULL;

}


BOOL LevelUtil_CheckSlatesAutoReveal(CLevelPlayer *player)
{
	return FALSE;
}

int LevelUtil_GetResCount(CLevelPlayer *player,LevelResourceType tp)
{
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps)
		{
			DWORD *p=lps->base.GetRes(tp);
			return (int)(*p);
		}
	}
	return 0;
}

LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelPlayer *player,LevelArtifactType tp)
{
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps)
		{
			CLevel *level=player->GetLevel();
			if (level)
			{
				CLevelRecords *records=level->GetRecords();
				extern LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp);
				return LPS_FindArtifact(lps,records,tp);
			}
		}
	}

	return NULL;
}

LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelObj *lo,LevelArtifactType tp)
{
	return LevelUtil_GetRawArtifactItemState(LevelUtil_PlayerFromLo(lo),tp);
}



BOOL LevelUtil_ExistArtifact(CLevelPlayer *player,LevelArtifactType tp)
{
	return LevelUtil_GetRawArtifactItemState(player,tp)!=NULL;
}

BOOL LevelUtil_ExistArtifact(CLevelObj *lo,LevelArtifactType tp)
{
	return LevelUtil_ExistArtifact(LevelUtil_PlayerFromLo(lo),tp);
}

LevelAbilityType LevelUtil_AbilityFromArtifact(CLevelPlayer *player,LevelArtifactType tp)
{
	LevelItemState *state=LevelUtil_GetRawArtifactItemState(player,tp);
	if (state)
	{
		CLevel *level = player->GetLevel();
		if (level)
		{
			CLevelRecords *records = level->GetRecords();
			if (records)
			{
				LevelRecordItem *recItem=records->GetItem(state->tid);
				if (recItem)
					return recItem->tpAbility;
			}
		}
	}
	return LevelAbilityType_None;
}


WORD LevelUtil_GetStrength(CLevelObj *lo)
{
	if (!lo)
		return 0;
	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
		return attr->str;
	return (LevelGrade)0;
}

WORD LevelUtil_GetMagic(CLevelObj *lo)
{
	if (!lo)
		return 0;
	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
		return attr->magic;
	return (LevelGrade)0;
}

WORD LevelUtil_GetHonor(CLevelObj *lo)
{
	if (!lo)
		return 0;
	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
		return (WORD)attr->hnr;
	return (LevelGrade)0;
}


int LevelUtil_GetArrowCountAddOn(CLevelObj *lo)
{
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
	{
		LevelItemState *is=LevelUtil_GetRawArtifactItemState(player,LevelArtifact_HunterRing);
		if (is)
			return is->nStack;
	}
	return 0;
}

BOOL LevelUtil_CheckAbilityToggledOn(CLevelObj *lo,LevelAbilityType tpAbility)
{
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
	{
		CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
		if (ability)
		{
			if (ability->SupportToggle())
			{
				if (ability->CheckToggledOn())
					return TRUE;
			}
		}
	}
	return FALSE;
}

void LevelUtil_HandleSacredArrowFired(CLevelObj *lo)
{
	CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps)
		{
			CLevel *level=player->GetLevel();
			if (level)
			{
				CLevelRecords *records=level->GetRecords();
				if (records)
				{
					extern int LPS_DecArtifactStack(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tpArtifact,int nStack);

					int nStack=LPS_DecArtifactStack(lps,records,LevelArtifact_SacredArrow,1);

					LevelItemState *state=LevelUtil_GetRawArtifactItemState(player,LevelArtifact_SacredArrow);
					if (state)
					{
						if (state->nStack<=0)
						{
							CLevelAbility*ability=player->GetAbilities().GetActiveAbility(LevelAbilityType_SacredArrow);
							if (ability)
							{
								if (ability->SupportToggle())
								{
									if (ability->CheckToggledOn())
										ability->Toggle(FALSE);
								}
							}
						}
					}
				}
			}
		}
	}
}

//返回实际减掉的Stack数量
int LevelUtil_DecArtifactStack(CLevelObj *lo,LevelArtifactType tpArtifact,int nStack)
{
	LevelPlayerStates *lps=LevelUtil_GetLPS(lo);
	if (lps)
	{
		CLevel *level=lo->GetLevel();
		if (level)
			return LPS_DecArtifactStack(lps,level->GetRecords(),tpArtifact,nStack);
	}
	return 0;
}

LevelEoqPower LevelUtil_CalcEoqPower(CLevelObj *lo)
{
	if (!LevelUtil_ExistArtifact(lo,LevelArtifact_EyeOfQueen))
		return LevelEoqPower_Invalid;
	return 10.0f;//临时值
}

CLevelRtnus *LevelUtil_GetOwnerRtnus(CLevelObj *lo)
{
	if (lo)
	{
		if (lo->GetLevel())
		{
			CLevelPlayer *player=lo->GetLevel()->GetPlayer(lo->GetPlayerID());
			if (player)
				return player->GetRtnus();
		}
	}
	return NULL;
}

CLevelRtnu *LevelUtil_RtnuFromLo(CLevelObj *lo)
{
	if (lo)
	{
		if (lo->GetType()==LevelObjType_Unit)
			return ((CLoUnit*)lo)->GetRtnu();
	}
	return NULL;
}

void LevelUtil_BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,CLevelObj*owner)
{
	extern void BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,void *owner);

	BuildShapeUnits(units,unitmgr,shape,mat,owner);

	for (int i=0;i<units.size();i++)
	{
		CUnit *unit=units[i];
		if (unit)
		{
			RvoUnit *unitRvo=unit->GetMirror();
			if (unitRvo)
				unitRvo->setPlayerID(owner->GetPlayerID());
		}
	}


}

void LevelUtil_CalcLoMat(CLevelObj *lo,i_math::matrix43f &mat)
{
	mat.makeIdentity();
	if (lo)
	{
		LevelPos3D pos=lo->GetFramePos3D();
		LevelFace face=lo->GetFrameFace();
		LevelPos3D zAxis;
		zAxis.setXZ(LevelFaceToDir(face));

		LevelPos3D yAxis(0,1,0);
		LevelPos3D xAxis;
		xAxis=yAxis.crossProduct(zAxis);
		xAxis.normalize();

		mat.buildMatrixLH(xAxis,yAxis,zAxis,pos);
	}
}

LevelFace LevelUtil_CalcTargetFacing(LevelFace faceInitial,CLevelObj *lo,LevelSkillTarget &target,LevelSkillTargetFacingMode mode,float angleMaxAdjust)
{
	if (!lo)
		return 0.0f;
	if (!lo->IsAlive())
		return 0.0f;

	LevelFace face=faceInitial;

	switch(mode)
	{
		case LevelSkillTargetFacingMode_FaceTarget:
		{
			LevelPos pos;
			if (LevelUtil_CalcTargetPos(lo->GetLevel(),target,pos))
			{
				LevelPos dir=pos-lo->GetFramePos();
				i_math::rotate_limited(face,LevelFaceFromDir(dir),angleMaxAdjust*i_math::GRAD_PI2);
			}
			break;
		}
		case LevelSkillTargetFacingMode_FaceTargetFixedPos:
		{
			if (target.tp==LevelSkillTarget::Target_FixPosAndObj)
			{
				LevelPos pos=target.Pos();
				LevelPos dir=pos-lo->GetFramePos();
				i_math::rotate_limited(face,LevelFaceFromDir(dir),angleMaxAdjust*i_math::GRAD_PI2);
			}
			break;
		}
	}

	return face;
}

LevelFace LevelUtil_CalcTargetFacing(CLevelObj *lo,LevelObjID idTarget)
{
	LevelSkillTarget target;
	target.SetObjID(idTarget);
	return LevelUtil_CalcTargetFacing(0.0f,lo,target,LevelSkillTargetFacingMode_FaceTarget,360.0f);
}



LevelPlayerMask LevelUtil_GetActualPlayerMask(CLevelObj *lo)
{
	if (lo->IsServerOnly())
		return 0;

	LevelPlayerMask maskActual=0;
	if (lo->IsGlobalSight())
		maskActual=0xffff;
	else
	{
		CLevel *level=lo->GetLevel();
		if (level)
		{
			CLevelAovMap *aovmap=level->GetAovMap();
			if (aovmap)
				maskActual=aovmap->GetPlayerMask(lo->GetFramePos());
		}
	}
	if (TRUE)
	{
		LevelPlayerID idVisibleTo=lo->GetOnlyVisible();
		if (idVisibleTo!=LevelPlayerID_Invalid)
			maskActual&=(1<<idVisibleTo);
	}

	return maskActual;
}

LevelObjID LevelUtil_GetLevelObjIDFromVar(CLevelObj *owner,StringID nm)
{
	LevelObjID id=LevelObjID_Invalid;
	if (owner)
	{
		if (nm!=StringID_Invalid)
		{
			CLevelBehavior *bhv=owner->GetBehaviorAI();
			if (bhv)
			{
				CBehaviorMem* mem=bhv->GetMem(0);
				if (mem)
					mem->GetID(nm,BehaviorMemType_ObjID,id);
			}
		}
	}
	return id;
}

CLevelService *LevelUtil_GetService(CLevelObj *lo,LevelServiceType tp)
{
	if (tp!=StringID_Invalid)
	{
		if (lo)
			return lo->GetLevel()->GetService_(tp);
	}

	return NULL;
}

CLevelService *LevelUtil_ObtainService(CLevelObj *lo,LevelServiceType tp)
{
	if (tp!=StringID_Invalid)
	{
		if (lo)
			return lo->GetLevel()->ObtainService(tp);
	}

	return NULL;
}

LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,LevelPos &pos,LevelFace face,LevelPlayerID idPlayer)
{
	CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	lo->PostCreate(idPlayer,NULL,idUnit,1,NULL,EquipSetPick_None,pos,face);

	level->AddToActives(lo);

	return lo->GetID();
}

LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,LevelPos3D &pos3D,LevelFace face,LevelPlayerID idPlayer)
{
	CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	lo->PostCreate(idPlayer,NULL,idUnit,1,NULL,EquipSetPick_None,pos3D,face);

	level->AddToActives(lo);

	return lo->GetID();
}

LevelObjID LevelUtil_CreateUnit(CLevel *level,RecordID idUnit,i_math::matrix43f &mat,LevelPlayerID idPlayer)
{
	i_math::xformf xfm;
	xfm.fromMatrix(mat);
	LevelFace face=LevelFaceFromQuat(xfm.rot);
	return LevelUtil_CreateUnit(level,idUnit,xfm.pos,face,idPlayer);
}

void LevelUtil_DestroyLo(CLevel *level,LevelObjID id)
{
	CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
	if (lo)
	{
		lo->AddRef();
		lo->Destroy();
	}
}

void LevelUtil_DeferDestroyLo(CLevel *level,LevelObjID id)
{
	CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
	if (lo)
		lo->DeferDestroy();
}

float LevelUtil_GetExaustedSP(CLevelObj *lo)
{
	float spExausted=0.0f;
	if (lo->GetType()==LevelObjType_Unit)
	{
		LevelRecordUnit *rec=((CLoUnit*)lo)->GetRec();
		if (rec)
			spExausted=rec->ExhaustedSP;
	}
	return spExausted;
}

const char *LevelUtil_GetArtifactName(LevelArtifactType tp)
{
	static std::vector<std::string> names;
	if (names.empty())
		SplitStringBy(",",std::string(LevelArtifactConstraintStr),&names);
	
	if (tp<names.size())
	{
		return names[tp].c_str();
	}

	return "n/a";

}

DWORD LevelUtil_PlayerIDToUnitCollideAlly(LevelPlayerID idPlayer)
{
	if(LevelPlayerID_PlayerWild!=idPlayer)
		return idPlayer;
	else
		return UnitCollide_AllyToAllPlayer;
}

BOOL LevelUtil_IsBow(RecordID idItem,CLevelRecords *records)
{
	LevelRecordPosture *recPosture=records->GetPostureOfItem(idItem);
	if (recPosture)
	{
		if ((recPosture->tp==LevelPosture_Bow)||(recPosture->tp==LevelPosture_Crossbow))
			return TRUE;
	}
	return FALSE;
}

//返回是否Reachable
BOOL LevelUtil_AddPathToSpline(CUnitMgr *unitmgr, CCubicSpline &spline,LevelPos3D &posSrc,LevelPos3D &posTarget,BOOL bResample)
{
	if (posSrc.getDistanceFrom(posTarget)<0.05f)
	{
		spline.AddNode(posTarget,i_math::quatf());
		return TRUE;
	}
	LevelPos3D pos;

	BOOL bAdded=FALSE;
	if (unitmgr)
	{
		std::vector<LevelPos> path;
		BOOL bEscape;
		if (unitmgr->FindPath(UnitFindPath_Walkable,posSrc.getXZ(),posTarget.getXZ(),0.0f,path,bEscape))
		{
			if (path.size()<2)
			{
				spline.AddNode(posTarget,i_math::quatf());
				return TRUE;
			}
			if (path.size()==2)
			{
				if (path[0].getDistanceFrom(path[1])<0.05f)
				{
					spline.AddNode(posTarget,i_math::quatf());
					return TRUE;
				}
			}
			if (path.size()>2)
			{
				int iStart=0;
				int iEnd=2;
				int c=1;
				while(1)
				{
					if (unitmgr->StaticObstacleTest(UnitFindPath_Walkable,path[iStart],path[iEnd]))
					{
						path[c]=path[iEnd-1];
						c++;
						iStart=iEnd-1;
						iEnd++;
					}
					else
						iEnd++;

					if (iEnd>=path.size())
					{
						path[c]=path[iEnd-1];
						c++;
						break;
					}
				}
				path.resize(c);
			}

			if (bResample)
			{
				KeySet ks;
				KeySet_Define(&ks,KT_Floatx2);
				ks.SetKeyCount(path.size());
				AnimTick length=0;
				for (int i=0;i<path.size();i++)
				{
					Key_2f *k=(Key_2f *)ks.GetKey(i);
					k->v=path[i];
					k->t=length;
					if (i<path.size()-1)
					{
						float dist=path[i].getDistanceFrom(path[i+1]);
						length+=ANIMTICK_FROM_SECOND(dist);
					}
				}

				float gap=3.0f;

				AnimTick dist=0;
				path.clear();
				while(dist<length)
				{
					Key_2f k;
					ks.CalcKey(dist,&k);
					path.push_back(k.v);
					dist+=ANIMTICK_FROM_SECOND(gap);
					if (dist+ANIMTICK_FROM_SECOND(0.1f)>length)
					{
						dist=length;
						ks.CalcKey(dist,&k);
						path.push_back(k.v);
						break;
					}
				}
			}

			float distFull=0.0f;
			for (int i=0;i<path.size()-1;i++)
				distFull+=path[i].getDistanceFrom(path[i+1]);

			float dist=0.0f;
			for (int i=0;i<path.size();i++)
			{
				float r=dist/distFull;
				pos.setXZ(path[i]);
				pos.y=i_math::lerp(posSrc.y,posTarget.y,r);
				spline.AddNode(pos,i_math::quatf());

				if (i<path.size()-1)
					dist+=path[i].getDistanceFrom(path[i+1]);
			}
			bAdded=TRUE;
		}
	}

	if (!bAdded)
	{
		spline.AddNode(posSrc,i_math::quatf());
		spline.AddNode(posTarget,i_math::quatf());
		return FALSE;
	}
	return TRUE;
}

BOOL LevelUtil_AddPathToSpline(CUnitMgr *unitmgr, CCubicSpline &spline,LevelPos &posSrc,LevelPos &posTarget,BOOL bResample)
{
	LevelPos3D posSrc2,posTarget2;
	posSrc2.setXZ(posSrc);
	posTarget2.setXZ(posTarget);
	return LevelUtil_AddPathToSpline(unitmgr,spline,posSrc2,posTarget2,bResample);
}


BOOL LevelUtil_FindNearbyPos(CLevel *level,LevelPos &posCenter,float radiusMin,float radiusMax,BOOL bWalkable,BOOL bReachable,DWORD nTry,LevelPos &posResult,FindNearbyPosCallBack dlgt)
{
	if (bReachable)
		bWalkable=TRUE;
	if (level)
	{
		CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
		if (unitmgr)
		{
			LevelPos pos;
			BOOL bFound=FALSE;
			for (int i=0;i<nTry;i++)
			{
				float radian=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
				float radius=CSysRandom::RandRange(radiusMin,radiusMax);
				pos.x=posCenter.x+radius*cosf(radian);
				pos.y=posCenter.y+radius*sinf(radian);

				if (dlgt)
				{
					if (!dlgt(pos))
						continue;
				}

				if (bWalkable)
				{
					if (unitmgr->IsWalkable(UnitFindPath_Walkable,pos))
					{
						if (!bReachable)
						{
							posResult=pos;
							return TRUE;
						}
						else
						{
							if (unitmgr->IsReachable(UnitFindPath_Walkable,posCenter,pos))
							{
								posResult=pos;
								return TRUE;
							}
						}
					}
				}
				else
				{
					posResult=pos;
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL LevelUtil_FindNearbyPos(CLevel *level,LevelPos &posCenter,float radius,BOOL bWalkable,BOOL bReachable,DWORD nTry,LevelPos &posResult,FindNearbyPosCallBack dlgt)
{
	return LevelUtil_FindNearbyPos(level,posCenter,radius,radius,bWalkable,bReachable,nTry,posResult,dlgt);
}


void LevelUtil_ModSPCost(CLevelObj *lo,float &sp,float &spCost)
{
	if (!lo->IsPlayer())
		return ;//目前只支持player

	float rate;
	LeModSPCost::Send(lo,rate);
	sp*=rate;
	spCost*=rate;
}

void LevelUtil_ChangeInSlatesBuff(CLevelPlayer *player,LevelObjID idSlates,BOOL bAddOrRemove)
{
	CLevel *level=player->GetLevel();
	if (level)
	{
		if (level->GetDecider())
		{
			CLoUnit *loUnit=player->GetLoUnit();
			if (loUnit)
			{
				CLevelRecords *records=level->GetRecords();
				if (records)
				{
					LevelRecordGlobal *rec=records->GetGlobal();
					if (rec)
					{
						if (rec->idDefBuff_InSlates!=RecordID_Invalid)
						{
							if (bAddOrRemove)
							{
								BuffArg_InSlates arg;
								arg.idSlates=idSlates;
								level->GetDecider()->MakeBuff(loUnit,rec->idDefBuff_InSlates,ANIMTICK_INFINITE,&arg,TRUE);
							}
							else
							{
								CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loUnit,rec->idDefBuff_InSlates);
								level->GetDecider()->RemoveBuff(LevelOSB(loUnit),loUnit,buff,LevelOpLink());
							}
						}
					}
				}
			}
		}
	}
}


CLoSlatesA *LevelUtil_GetSlatesAFromEmbed(CLevelObj *lo)
{
	if (!lo)
		return NULL;

	CLevel *level=lo->GetLevel();
	DWORD c;
	CLevelObj **buf=level->GetActiveObjs(c);
	for (int i=0;i<c;i++)
	{
		if (buf[i])
		{
			if (buf[i]->IsAlive())
			{
				if (buf[i]->GetClass()->IsSameWith(Class_Ptr2(CLoSlatesA)))
				{
					CLoSlatesA* loSlatesA=(CLoSlatesA*)buf[i];
					if (loSlatesA->FindEmbedID(lo->GetID())!=LevelSlateIdx_Invalid)
						return loSlatesA;
				}
			}
		}
	}
	return NULL;
}

BOOL LevelUtil_CheckAwardAvailable(CLevelPlayer *player,RecordID idItem)
{
	if (!player)
		return FALSE;
	CLevel *level=player->GetLevel();
	if (!level)
		return FALSE;
	if (idItem==RecordID_Invalid)
		return FALSE;
	LevelRecordItem *rec=level->GetRecords()->GetItem(idItem);
	if (rec)
	{
		if (!rec->bAllowStack)
		{
			if (rec->tpAbility!=LevelAbilityType_None)
			{
				CLevelAbility *ability=player->GetAbilities().GetActiveAbility(rec->tpAbility);
				if (ability)
					return FALSE;//已经有这个Ability了
			}
		}
	}
	return TRUE;
}

void LevelUtil_AddTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelAgentBrief::TreasureInfos::Entry &info)
{
	if (!player)
		return;
	CLevel *level=player->GetLevel();
	if (!level)
		return;
	CJjWorld *world=level->GetWorld();
	if (!world)
		return;
	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return;
	if (guidAgent==LevelGUID_Invalid)
		return;

	BOOL bModified=FALSE;
	LevelAgentBrief *brief=LPS_QueryPersistEntry_AgentBrief(lps,idMap,guidAgent);
	if (brief)
	{
		LevelAgentBrief::TreasureInfos &infos=brief->infosTreasure;

		for (int i=0;i<infos.nEntries;i++)
		{
			if (infos.entries[i].CheckCompatible(info))
			{
				if ((infos.entries[i].possibility==info.possibility)&&(infos.entries[i].count==info.count))
					return;//No change
				infos.entries[i].possibility=info.possibility;
				infos.entries[i].count=info.count;
				bModified=TRUE;
				break;
			}
		}

		if (!bModified)
		{
			if (infos.nEntries>=ARRAY_SIZE(infos.entries))
				return;
			infos.entries[infos.nEntries]=info;
			infos.nEntries++;
			bModified=TRUE;
		}
	}

	if(bModified)
	{
		LevelAgentGuid pending;
		pending.idMap=idMap;
		pending.guid=guidAgent;
		world->AddAgentBriefEntryToSend(player->GetPlayerID(),pending);
	}

}

void LevelUtil_RevealTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelResourceType tpRes,RecordID idItem)
{
	if (!player)
		return;
	CLevel *level=player->GetLevel();
	if (!level)
		return;
	CJjWorld *world=level->GetWorld();
	if (!world)
		return;
	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return;
	if (guidAgent==LevelGUID_Invalid)
		return;

	BOOL bModified=FALSE;
	LevelAgentBrief *brief=LPS_QueryPersistEntry_AgentBrief(lps,idMap,guidAgent);
	if (brief)
	{
		LevelAgentBrief::TreasureInfos &infos=brief->infosTreasure;

		for (int i=0;i<infos.nEntries;i++)
		{
			if (!infos.entries[i].bRevealed)
			{
				if ((infos.entries[i].tpRes==tpRes)&&(infos.entries[i].idItem==idItem))
				{
					infos.entries[i].bRevealed=1;
					bModified=TRUE;
				}
			}
		}
	}

	if(bModified)
	{
		LevelAgentGuid pending;
		pending.idMap=idMap;
		pending.guid=guidAgent;
		world->AddAgentBriefEntryToSend(player->GetPlayerID(),pending);
	}

}


float LevelUtil_CalcCurPain(LevelPain &pain,UnitPainInfo &infoPain,AnimTick tCur)
{
	float vPain=0.0f;
	if (TRUE)
	{
		BOOL bFullValue=FALSE;
		if (pain.v>(float)infoPain.full)
			bFullValue=TRUE;

		AnimTick durKeep=infoPain.durKeep;
		AnimTick tAge=ANIMTICK_SAFE_MINUS(tCur,pain.t);
		if (!bFullValue)
		{
			if (tAge<=durKeep)
				vPain=pain.v;
			else
			{
				vPain=pain.v-ANIMTICK_TO_SECOND(tAge-durKeep)*infoPain.speedDrop;
				if (vPain<0.0f)
					vPain=0.0f;
			}
		}
		else
		{
			const AnimTick durKeepFull=ANIMTICK_FROM_SECOND(1.0f);
			if (tAge<=durKeepFull)
				vPain=(float)infoPain.full;
			else
				vPain=0.0f;
		}
	}

	return vPain;
}

float LevelUtil_CalcCurPain(CLevelObj *lo)
{
	if (!lo)
		return 0.0f;
	if (lo->GetType()!=LevelObjType_Unit)
		return 0.0f;
	CLoUnit*loUnit=(CLoUnit*)lo;

	LevelAttr_Base *attrBase=loUnit->GetAttr_Base();
	return LevelUtil_CalcCurPain(attrBase->pain,loUnit->GetRec()->pain,loUnit->GetT());
}

float LevelUtil_CalcCurPainRatio(CLevelObj *lo)
{
	if (!lo)
		return 0.0f;
	if (lo->GetType()!=LevelObjType_Unit)
		return 0.0f;
	CLoUnit*loUnit=(CLoUnit*)lo;

	LevelAttr_Base *attrBase=loUnit->GetAttr_Base();
	float v=LevelUtil_CalcCurPain(attrBase->pain,loUnit->GetRec()->pain,loUnit->GetT());
	if (loUnit->GetRec()->pain.full>0.0f)
		return i_math::clamp_f(v/loUnit->GetRec()->pain.full,0.0f,1.0f);
	return 0.0f;
}

BOOL LevelUtil_CalcSkillCastingXfm(CLevelSkill *skill,i_math::xformf &xfm)
{
	if (skill->GetState()==SkillState_Casting)
	{
		skill->GetCastingPos3D(xfm.pos);
		LevelFace face=skill->GetCastingFace();
		LevelFaceToQuat(face,xfm.rot);
		CLevelObj *owner=skill->GetOwner();
		if (owner)
			xfm.scale_=owner->GetModelScale();
		else
			xfm.scale_=1.0f;

		return TRUE;
	}
	return FALSE;
}

AnimEventZone *LevelUtil_FindEZone(CLevel *level,SkillParam_GeneralAdvS *paramSkill,SkillParam_GeneralAdvS::Stage* paramSkillStage,StringID nmEvent)
{
	if (paramSkill&&nmEvent!=StringID_Invalid)
	{
		if (paramSkillStage)
		{
			if (paramSkillStage->idPathRes!=RecordID_Invalid)
			{
				LevelPathes *pathes=level->GetResources()->FindPathes(paramSkillStage->idPathRes);
				if (pathes)
				{
					LevelPathesEvent *e=pathes->FindEvent(nmEvent);
					if (e)
						return &e->zone;
				}
			}
		}
		else
		{
			for (int i=0;i<paramSkill->stages.size();i++)
			{
				SkillParam_GeneralAdvS::Stage &stage=paramSkill->stages[i];
				if (stage.idPathRes!=RecordID_Invalid)
				{
					LevelPathes *pathes=level->GetResources()->FindPathes(stage.idPathRes);
					if (pathes)
					{
						LevelPathesEvent *e=pathes->FindEvent(nmEvent);
						if (e)
							return &e->zone;
					}
				}
			}
		}
	}
	return NULL;
}

BOOL LevelUtil_InterruptCastingSkill(CLevelObj *lo)
{
	if (!LevelUtil_GetCastingSkill(lo))
		return TRUE;
	if (lo)
	{
		CLevel *level=lo->GetLevel();
		RecordID idRecBuff=level->GetRecords()->GetGlobal()->idDefBuff_SkillStun;
		if (idRecBuff!=RecordID_Invalid)
		{
			BuffArg_SkillStun arg;
			LevelBuffID idBuff=level->GetDecider()->MakeBuff(LevelOSB(lo),lo,idRecBuff,ANIMTICK_INFINITE,&arg,LevelOpLink());
			if (idBuff!=LevelBuffID_Invalid)
				return TRUE;
		}
	}
	return FALSE;
}

LevelObjID LevelUtil_CreateEnvEo(CLevelObj *loOwner,RecordID idEo)
{
	if (!loOwner)
		return LevelObjID_Invalid;

	LevelObjID idResult=LevelObjID_Invalid;
	CLevel *level=loOwner->GetLevel();
	LevelRecordEo *rec=level->GetRecords()->GetEo(idEo);
	if (rec)
	{
		CLoEffectObj *eo=(CLoEffectObj*)level->CreateObj(rec->param->GetEoClass());
		if (eo)
		{
			eo->SetHost(loOwner->GetID());
			eo->PostCreate(LevelPlayerID_Wild,rec,loOwner->GetFramePos(),LevelPos(0,1),1,LevelOSB(),LevelOpLink());
			level->AddToActives(eo);

			level->RegisterEoEnv(eo);

			idResult=eo->GetID();

			SAFE_RELEASE(eo);
		}
	}
	return idResult;
}

void LevelUtil_DestroyEnvEo(CLevel* level)
{
	CLevelObj *lo=level->GetEoEnv();
	if (lo)
	{
		if (lo->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
			((EoEnv*)lo)->RequestDestroy();
	}
}

BellySetting& LevelUtil_GetBellySetting(CLevel *level)
{
	if (level)
	{
		CLevelRecords *records=level->GetRecords();
		if (records)
		{
			LevelRecordGlobal *global=records->GetGlobal();
			if (global)
				return global->bellysetting;
		}
	}
	static BellySetting setting;
	return setting;
}
