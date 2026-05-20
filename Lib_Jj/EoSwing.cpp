
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoSwing.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoSwing,EoParamSwing);

void EoSwing::_CalcSwing(EoParamSwing *param,AnimTick tAge,LevelPos &pos,LevelFace &face,float &radius)
{
	CLevelObj *owner=_GetOwner();
	assert(owner);

	face=owner->GetFrameFace();
	pos=owner->GetFramePos();

	float r=((float)tAge)/((float)_dur);

	radius=param->radius.GetFloat(ANIMTICK_FROM_SECOND(r))+owner->GetRadius_();
	LevelFaceYaw yaw=param->yaw.GetFloat(ANIMTICK_FROM_SECOND(r))*i_math::GRAD_PI2;

	LevelFaceApplyYaw(face,yaw);
}

void EoSwing::_OnDetroy()
{
	_handled.clear();
}



void EoSwing::_OnPostCreate()
{
	_tStart=_GetSkillCastingTime();
	_tAge=0;

	EoParamSwing *param=GetParam<EoParamSwing>();
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


LevelAttr_AttackMods*EoSwing::GetAttr_AttackMods()
{
	if (_GetOwner())
		return _GetOwner()->GetAttr_AttackMods();
	return NULL;
}



void EoSwing::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LevelSkillID idSkill=LevelSkillID_Invalid;
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
		idSkill=skill->GetID();

	bp->Data_WriteSimple(idSkill);
	bContent=TRUE;
}



void EoSwing::_OnUpdate()
{
	EoParamSwing *param=GetParam<EoParamSwing>();
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

			if (tCur<_dur)
			{
				if (tCur>_tAge)
				{
					_tAge=tCur;

					LevelPos centerSwing;
					LevelFace faceSwing;
					float radiusSwing;

					_CalcSwing(param,tCur,centerSwing,faceSwing,radiusSwing);

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
					float faceMiddle=_faceSwing+faceOff/2.0f;
					LevelPos dirMiddle=LevelFaceToDir(faceMiddle);

					i_math::line2df from,to,move;

					from.start=_centerSwing;
					from.end=_centerSwing+LevelFaceToDir(_faceSwing)*_radiusSwing;

					to.start=centerSwing;
					to.end=centerSwing+LevelFaceToDir(faceSwing)*radiusSwing;

					move.start=_centerSwing;
					move.end=centerSwing;

					_centerSwing=centerSwing;
					_faceSwing=faceSwing;

					float radiusDetect=radiusSwing;
					if (to.start.getDistanceSQFrom(from.end)>radiusDetect*radiusDetect)
						radiusDetect=to.start.getDistanceFrom(from.end);

					DWORD c;
					CLevelObj **los=_DetectRange(to.start,radiusDetect,c);
					for (int i=0;i<c;i++)
					{
						CLevelObj *loTarget=los[i];
						if (_handled.find(loTarget->GetID())!=_handled.end())
							continue;//已经处理过了
						float radiusTarget=loTarget->GetRadius_();
						LevelPos posTarget=loTarget->GetFramePos();

						i_math::circlef ccl;
						ccl.setCenter(posTarget);
						ccl.setRadius(radiusTarget);

						//判断是否在两条扫描线之间
						if (!ccl.isIntersectingWithLine(from))
						{
							if(!ccl.isIntersectingWithLine(to))
							{
								if (param->bLeft)
								{
									if (from.classifyPoint(posTarget)>0)
										continue;
									if (to.classifyPoint(posTarget)<0)
										continue;
								}
								else
								{
									if (from.classifyPoint(posTarget)<0)
										continue;
									if (to.classifyPoint(posTarget)>0)
										continue;
								}
							}
						}

						LevelPos posMiddle=(from.start+to.start)*0.5f;

						if (posTarget.getDistanceSQFrom(posMiddle)>=radiusSwing*radiusSwing)
							continue;

						if (dirMiddle.dotProduct(posTarget-posMiddle)<=0.0f)
							continue;

						if (_rec->deals.size()<=0)
						{
							LOG_DUMP_1P("EoSwing",Log_Error,"Eo(%s)里结算列表为空",_rec->Name.c_str());
						}
						DealArg arg;
						arg.dir.setXZ(dirMiddle);
						arg.grd=1;
						arg.link.id=level->GenOpLinkID();
						arg.link.t=tCur;
						_MakeDeals(loTarget,arg);

						_handled.insert(loTarget->GetID());
					}
				}

				return;
			}
		}
	}
	DeferDestroy();
	return;
}

