
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoSwingBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


void EoSwingBulletBase::_CalcSwing(EoParamSwingBase *param,AnimTick tAge,LevelPos &pos,LevelFace &face,float &radius)
{
	CLevelObj *owner=_GetOwner();
	assert(owner);

	face=owner->GetFrameFace();
	pos=owner->GetFramePos();

	float r=((float)tAge)/((float)_dur);

	radius=param->radius.GetFloat(ANIMTICK_FROM_SECOND(r));//+owner->GetRadius_();
	float yaw=param->yaw.GetFloat(ANIMTICK_FROM_SECOND(r))*i_math::GRAD_PI2;

	face+=-yaw;
}

void EoSwingBulletBase::_ClampSwing(EoParamSwingBase *param,LevelFace &faceSwing)
{
	float faceOff=i_math::normalize_radian(faceSwing-_faceSwing);

	if (param->bLeft)
	{
		if (faceOff<0.0f)
			faceOff=0.0f;
	}
	else
	{
		if (faceOff>0.0f)
			faceOff=0.0f;
	}
	faceSwing=_faceSwing+faceOff;
}



void EoSwingBulletBase::_OnPostCreate()
{
	_tStart=_GetSkillCastingTime();
	_tAge=0;

	EoParamSwingBullet *param=GetParam<EoParamSwingBullet>();
	CLevelObj *owner=_GetOwner();

	_dur=ANIMTICK_FROM_SECOND(1.0f);
	if (!param->bUseEndEvent)
		_dur=param->dur;
	else
	{
		AnimTick tEnd=_GetSkillCastingEventTime(param->nmEndEvent);
		assert(tEnd!=ANIMTICK_INFINITE);
		if (tEnd!=ANIMTICK_INFINITE)
		{
			if (tEnd>_tStart)
				_dur=tEnd-_tStart;
			else
				_dur=1;
		}
	}

	if (param&&owner)
		_CalcSwing(param,_tAge,_centerSwing,_faceSwing,_radiusSwing);
}


void EoSwingBulletBase::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LevelSkillID idSkill=LevelSkillID_Invalid;
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
		idSkill=skill->GetID();

	bp->Data_WriteSimple(idSkill);
	bContent=TRUE;
}

//////////////////////////////////////////////////////////////////////////
//EoSwingBullet

BIND_EOPARAM(EoSwingBullet,EoParamSwingBullet);

void EoSwingBullet::_OnPostCreate()
{
	__super::_OnPostCreate();

	EoParamSwingBullet *param=GetParam<EoParamSwingBullet>();
	if (param->nBullet==1)
		_timesBullet.push_back(_dur/2);
	else
	{
		AnimTick t=0;
		for (int i=0;i<param->nBullet;i++)
		{
			_timesBullet.push_back(t);
			t+=_dur/(param->nBullet-1);
			if (t>_dur)
				t=_dur;
		}
	}
}


void EoSwingBullet::_OnUpdate()
{
	EoParamSwingBullet *param=GetParam<EoParamSwingBullet>();
	if (!param)
		return;

	CLevelObj *owner=_GetOwner();
	CLevelSkill *ownerSkill=_GetOwnerSkill();
	if (ownerSkill&&owner)
	{
		CLevel *level=owner->GetLevel();
		AnimTick tCur=_GetSkillCastingTime();
		if (tCur!=ANIMTICK_INFINITE)
		{
			tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

			if (tCur>_tAge)
			{
				LevelPos centerSwing;
				LevelFace faceSwing;
				float radiusSwing;

				_CalcSwing(param,tCur,centerSwing,faceSwing,radiusSwing);

				_ClampSwing(param,faceSwing);

				while(_timesBullet.size()>0)
				{
					if (_timesBullet[0]>tCur)
						break;

					AnimTick tBullet=_timesBullet[0];
					_timesBullet.pop_front();

					float r=((float)(tBullet-_tAge))/((float)(tCur-_tAge));

					LevelPos pos;
					LevelFace face;
					float radius;

					pos=_centerSwing.getInterpolated(centerSwing,r);
					radius=i_math::lerp(_radiusSwing,radiusSwing,r);
					face=LevelFaceLerp(_faceSwing,faceSwing,r);

					LevelPos dir=LevelFaceToDir(face);
					pos=pos+=dir*radius;

					if (_rec->deals.size()<=0)
					{
						LOG_DUMP_1P("EoSwing",Log_Error,"Eo(%s)里结算列表为空",_rec->Name.c_str());
					}
					DealArg arg;
					arg.dir.setXZ(dir);
					arg.grd=1;
					arg.link.id=level->GenOpLinkID();
					arg.link.t=tCur;

					extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
					LevelPos3D pos3D=LevelUtil_GetGroundHeight(level,pos.x,pos.y,TRUE);
					_MakeDeals(pos3D,arg);

					_centerSwing=centerSwing;
					_radiusSwing=radiusSwing;
					_faceSwing=faceSwing;
				}
				_tAge=tCur;
			}

			if (_timesBullet.size()>0)
				return;//还有Bullet
		}
	}
	DeferDestroy();
	return;
}

