
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoShootBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//EoShootBullet

BIND_EOPARAM(EoShootBullet,EoParamShootBullet);

void EoShootBullet::_OnPostCreate()
{
	CLevelSkill *skill=_GetOwnerSkill();

	EoParamShootBullet *param=_rec->GetParam<EoParamShootBullet>();

	if (param)
	{
		LevelPos3D posInitial0;
		LevelPos3D dirInitial0;

		posInitial0=_GetInitialPos3D();
		dirInitial0=_GetInitialDir3D();
		AnimEventZone::KeyFan kFan;
		LevelPos3D posTarget=posInitial0;
		LevelPos3D posAim=posTarget;

		LevelPos3D posOwner;
		CLevelObj *owner=_GetOwner();
		posOwner=owner->GetFramePos3D();

		if (skill)
		{
			LevelSkillTarget &target=skill->GetTarget();

			float fov;
			_CalcEZoneInfo(kFan,posInitial0,dirInitial0,fov);
		
// 			AnimEventZone::KeyFan kFan;
// 			if (_eZone)
// 			{
// 				i_math::xformf xfm;
// 				if (_GetSkillCastingXfm(xfm))
// 				{
// 					if (_eZone->CalcKeyFan(_eZone->t,kFan))
// 					{
// 						kFan.xfmCenter.ApplyBase(xfm);
// 
// 						posInitial0=kFan.xfmCenter.pos;
// 
// 						float rad=i_math::lerp_angle(kFan.radianFrom,kFan.radianTo,0.5f);
// 						dirInitial0.set(cosf(rad),0.0f,sinf(rad));
// 						dirInitial0=kFan.xfmCenter.rot*dirInitial0;
// 						dirInitial0.normalize();
// 					}
// 				}
// 			}
			extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
			LevelUtil_CalcTargetPos3D(_level,target,posTarget);

			extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
			CLevelObj *lo=LevelUtil_GetTargetObj(_level,target);
			if (lo)
				posAim.y+=lo->GetAimHeight();
		}

		LevelPos3D dir0;
		switch(param->modeDir)
		{
			case EoParamShootBullet::OriginalDir:
			{
				dir0=dirInitial0;
				break;
			}
			case EoParamShootBullet::OwnerPosToTargetPos:
			{
				dir0=posTarget-posOwner;
				break;
			}
			case EoParamShootBullet::ShootPosToTargetPosKeepPitch:
			{
				LevelPos3D dirInitial=dirInitial0;
				dir0=posTarget-posInitial0;
				dir0.y=0.0f;
				dir0.normalize();
				dir0*=dirInitial.getXZ().getLength();
				dir0.y=dirInitial.y;
				break;
			}
			case EoParamShootBullet::ShootPosToAimPosKeepPitch:
			{
				LevelPos3D dirInitial=dirInitial0;
				dir0=posAim-posInitial0;
				dir0.y=0.0f;
				dir0.normalize();
				dir0*=dirInitial.getXZ().getLength();
				dir0.y=dirInitial.y;
				break;
			}

			case EoParamShootBullet::ShootPosToTargetPos:
			{
				dir0=posTarget-posInitial0;
				break;
			}
		}
		dir0.normalize();

		DWORD c=1;
		if (param->modeDirRandom!=EoParamShootBullet::DirRandomMode_NoRandom)
			c=param->countRandom;

		for (int i=0;i<c;i++)
		{
			LevelPos3D dir=dir0;

			if (param->modeDirRandom==EoParamShootBullet::DirRandomMode_3D)
			{
				i_math::vector3df axis1,axis2;
				dir.findBestAxis(axis1,axis2);
				i_math::quatf rot;
				rot.fromAngleAxis(CSysRandom::RandRange(0.0f,i_math::Pi*2.0f),dir);
				axis1=rot*axis1;
				axis1.normalize();
				rot.fromAngleAxis(CSysRandom::RandRange(0.0f,param->angleDirOff*i_math::GRAD_PI2),axis1);
				dir=rot*dir;
				dir.normalize();
			}
			if (param->modeDirRandom==EoParamShootBullet::DirRandomMode_2D)
			{
				i_math::quatf rot;
				rot.fromAngleAxis(CSysRandom::RandRange(-param->angleDirOff*i_math::GRAD_PI2,param->angleDirOff*i_math::GRAD_PI2),i_math::vector3df(0,1,0));
				dir=rot*dir;
				dir.normalize();
			}
			if (param->modeDirRandom==EoParamShootBullet::DirRandomMode_InEZone)
			{
				float rad=i_math::lerp_angle(kFan.radianFrom,kFan.radianTo,CSysRandom::RandRange(0.0f,1.0f));
				dir.set(cosf(rad),0.0f,sinf(rad));
				dir=kFan.xfmCenter.rot*dir;
				dir.normalize();
			}

			DealArg arg;
			if (_opBirth)
				arg.link=_opBirth->GetDesc().link;
			else
				arg.link.id=_level->GenOpLinkID();
			arg.dir=dir;

			if (skill)
				_MakeDeals(LevelOSB(skill),posInitial0,arg);
			else
				_MakeDeals(LevelOSB(_GetOwner()),posInitial0,arg);
		}
	}

	if (_rec->idEffect==0)//如果有effect的话,我们晚一点再destroy(如果立即destroy的话,这个eo不会被同步到client端)
		DeferDestroy();

}


void EoShootBullet::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_NextDword()=_idSrcOwner;
}

void EoShootBullet::_OnUpdate()
{
	if (_GetAge()>ANIMTICK_FROM_SECOND(0.1f))
		DeferDestroy();
}
