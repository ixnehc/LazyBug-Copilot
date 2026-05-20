#include "stdh.h"

#include "Deal_SummonUnit.h"

#include "LevelOSB.h"

#include "LevelRecordEO.h"
#include "LevelRecords.h"

#include "Log/LogDump.h"

#include "LoUnit.h"
#include "Level.h"
#include "LevelUtil.h"

#include "Random/Random.h"

#include "Level.h"

#include "LevelRtnus.h"
#include "LevelTroops.h"

BIND_DEAL(Deal_SummonUnit);
class CLoEffectObj;
void Deal_SummonUnit::Make(LevelOSB &osbSrc,LevelPos3D &pos3DTarget,DealArg&arg,DealResult *result)
{

	CLevel *level=osbSrc.GetLevel();
	CLevelDecider *decider=level->GetDecider();
	CLevelObj *owner=osbSrc.GetOwner();
	if (owner)
	{
		LevelPos posTarget=pos3DTarget.getXZ();
		LevelPos posSrc=owner->GetFramePos();

		LevelPos dirBase=arg.dir.getXZ();
		LevelFace faceDirBase=owner->GetFrameFace();
		if (dirBase.getLengthSQ()>0.0001f)
			faceDirBase=LevelFaceFromDir(dirBase);

		LevelPos posCenter;

		posCenter=posSrc;
		if (_mode==UsingOrgPos)
			posCenter=posSrc;
		if ((_mode==UsingTargetPos)||(_mode==UsingTargetPos3D))
			posCenter=posTarget;

		for (int j=0;j<_cats.size();j++)
		{
			SummonUnitCategory *cat=&_cats[j];
			for (int i=0;i<cat->count;i++)
			{
				float faceVary=cat->fov*i_math::GRAD_PI2/2.0f;

				LevelPos pos;
				LevelPos3D pos3D;
				LevelFace faceDir;
				if (TRUE)
				{
					faceDir=faceDirBase+CSysRandom::RandRange(-faceVary,faceVary);
					float radius=CSysRandom::RandRange(0.0f,1.0f)*cat->radius;
					LevelPos dir=LevelFaceToDir(faceDir);
					pos=dir*radius+posCenter;
				}

				if (_mode!=UsingTargetPos3D)
				{
					extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
					pos3D=LevelUtil_GetGroundHeight(level,pos.x,pos.y,TRUE);
					pos3D.y+=cat->ht;
				}
				else
				{
					pos3D.x=pos.x;
					pos3D.z=pos.y;
					pos3D.y=pos3DTarget.y;
				}

				LevelFace face;
				if (TRUE)
				{
					switch(cat->modeFacing)
					{
						case SummonUnitCategory::Facing_Random:
							face=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
							break;
						case SummonUnitCategory::Facing_FaceToOrg:
							face=faceDir+i_math::Pi;
							break;
						case SummonUnitCategory::Facing_BackToOrg:
							face=faceDir;
							break;
					}
				}

				CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

				CLevelPlayer *player=level->GetPlayer((LevelPlayerID)owner->GetPlayerID());
				if (cat->bRetinue)
				{
					if (cat->ht<=0.0f)
						lo->PostCreate(LevelPlayerID_Wild,NULL,cat->idUnit,arg.grd,NULL,EquipSetPick_None,pos,face);//使用技能的等级
					else
						lo->PostCreate(LevelPlayerID_Wild,NULL,cat->idUnit,arg.grd,NULL,EquipSetPick_None,pos3D,face);//使用技能的等级
					level->AddToActives(lo);

					if (player)
					{
						if (player->GetRtnus())
							player->GetRtnus()->Add_New(lo,TRUE);
					}
				}
				else
				{
					LevelPlayerID idPlayer=player?(LevelPlayerID)owner->GetPlayerID():LevelPlayerID_Wild;
					if (_mode==UsingTargetPos3D)
						lo->PostCreate(idPlayer,NULL,cat->idUnit,arg.grd,NULL,EquipSetPick_None,pos3D,face);//使用技能的等级
					else
					{
						if (cat->ht<=0.0f)
							lo->PostCreate(idPlayer,NULL,cat->idUnit,arg.grd,NULL,EquipSetPick_None,pos,face);//使用技能的等级
						else
							lo->PostCreate(idPlayer,NULL,cat->idUnit,arg.grd,NULL,EquipSetPick_None,pos3D,face);//使用技能的等级
					}
					level->AddToActives(lo);
				}

				if (result)
					result->idSummoned=lo->GetID();

				if (TRUE)
				{
					if (owner->GetTroop())
						owner->GetTroop()->AddUnit(LevelTroopRank_Minion,lo->GetID());
					else
					{
						LevelObjID idRootOwner=owner->GetRootOwnerID();
						CLevelObj *loRootOwner=LevelUtil_GetAliveLo(owner->GetLevel(),idRootOwner);
						if (loRootOwner)
						{
							if (loRootOwner->GetTroop())
								loRootOwner->GetTroop()->AddUnit(LevelTroopRank_Minion,lo->GetID());
						}
					}
				}

				if (arg.hpInitial>=0)
				{
					LevelAttr_Base *attrBase=lo->GetAttr_Base();
					if (attrBase)
						attrBase->hp.SetCur_Int(arg.hpInitial);
				}

				if (cat->idBirth)
				{
					decider->MakeBuff(osbSrc,lo,cat->idBirth,0,NULL,arg.link);
//					decider->MakeBirth(osbSrc,lo,cat->idBirth,arg.link);
				}

				if (cat->idBirthSkill)
				{
					extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
					CLevelSkill *skillCasting=LevelUtil_GetCastingSkill(owner);

					LevelOp_Dummy *op=osbSrc.NewOp<LevelOp_Dummy>(arg.link);
					if (skillCasting)//加一个dummy op,用来通过link连锁触发技能的StartOp
						op->t=skillCasting->GetCastingTime();
					lo->AddOp(op);


					CLevelSkillDriver *driver=lo->GetSkillDriver();
					if (driver)
					{
						LevelSkillTarget target;
						if (cat->tpBirthSkillTarget==1)
						{
							if (skillCasting)
								target=skillCasting->GetTarget();
						}
						if (FALSE==driver->StartCast(LevelSkillType(cat->idBirthSkill),target,1,NULL,&arg.link))
						{
							int v=0;
							v++;
						}
						BOOL bStartSkill=FALSE;
						CLevelOps *ops=lo->GetOps();
						for (int i=0;i<ops->_ops.size();i++)
						{
							CLevelOp*op=ops->_ops[i];
							if (op->ToPtr<LevelOp_StartSkill>())
							{
								bStartSkill=TRUE;
								break;
							}
						}
					}
				}

				SAFE_RELEASE(lo);
			}
		}
	}

}

void Deal_SummonUnit::Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	LevelPos3D pos=loTarget->GetFramePos3D();
	Make(osbSrc,pos,arg,result);
}
