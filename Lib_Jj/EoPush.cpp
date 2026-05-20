
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoPush.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Skill_GeneralS.h"


BIND_EOPARAM(EoPush,EoParamPush);

void EoPush::_CalcPush(EoParamPush *param,AnimTick tAge,i_math::line2df &left,i_math::line2df &right,float &radius)
{
	CLevelObj *owner=_GetOwner();
	assert(owner);


	LevelFace face=0.0f;
	LevelPos pos;
	if (TRUE)
	{
		BOOL bContent=FALSE;

		//尝试从owner skill里拿到最精确的位置/旋转信息
		CLevelSkill *skill=_GetOwnerSkill();
		if (skill)
		{
			if (IsClass2(skill,Skill_GeneralS))
			{
				((Skill_GeneralS *)skill)->GetCastingPos(pos);
				face=((Skill_GeneralS *)skill)->GetCastingFace();
				bContent=TRUE;
			}
		}

		if (!bContent)
		{
			face=owner->GetFrameFace();
			pos=owner->GetFramePos();
		}
	}

	float radiusOwner=owner->GetRadius_();

	float r=((float)tAge)/((float)param->dur);

	radius=param->radius.GetFloat(ANIMTICK_FROM_SECOND(r))+radiusOwner;
	LevelFaceYaw yaw=param->yaw.GetFloat(ANIMTICK_FROM_SECOND(r))*i_math::GRAD_PI2;

	LevelFaceApplyYaw(face,yaw);

	float fov=param->fov.GetFloat(ANIMTICK_FROM_SECOND(r))*i_math::GRAD_PI2;

	LevelFace faceLeft,faceRight;
	if (TRUE)
	{
		LevelFaceYaw yaw;
		yaw=-fov/2.0f;

		faceLeft=face;
		LevelFaceApplyYaw(faceLeft,yaw);
		faceRight=face;
		LevelFaceApplyYaw(faceRight,-yaw);
	}

	if (TRUE)
	{
		LevelFaceYaw yaw;
		yaw=-i_math::Pi/2.0f;

		LevelFace face2=face;
		LevelFaceApplyYaw(face2,yaw);

		left.start=pos+LevelFaceToDir(face2)*radiusOwner;
		right.start=pos-LevelFaceToDir(face2)*radiusOwner;
	}

	left.end=left.start+LevelFaceToDir(faceLeft)*radius;
	right.end=right.start+LevelFaceToDir(faceRight)*radius;

}

void EoPush::_OnDetroy()
{
	_handled.clear();
}



void EoPush::_OnPostCreate()
{
	_tStart=_GetSkillCastingTime();
	_tAge=0;

	EoParamPush *param=GetParam<EoParamPush>();
	CLevelObj *owner=_GetOwner();

	_level->RegisterSubframeUpdate(this);

	_bFirstUpdate=TRUE;
//	_OnUpdate();//强制执行一次
}


LevelAttr_AttackMods*EoPush::GetAttr_AttackMods()
{
	if (_GetOwner())
		return _GetOwner()->GetAttr_AttackMods();
	return NULL;
}



void EoPush::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LevelSkillID idSkill=LevelSkillID_Invalid;
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
		idSkill=skill->GetID();

	bp->Data_WriteSimple(idSkill);
	bContent=TRUE;
}


void EoPush::_OnUpdate()
{
	EoParamPush *param=GetParam<EoParamPush>();
	if (!param)
		return;

	CLevelObj *owner=_GetOwner();
	CLevelSkill *skillOwner=_GetOwnerSkill();
	if (skillOwner&&owner)
	{
		CLevel *level=owner->GetLevel();
		AnimTick tCur=_GetSkillCastingTime();
		if (tCur!=ANIMTICK_INFINITE)
		{
			tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

			if ((tCur<=_tAge)&&(!_bFirstUpdate))
				return;

			if (((tCur<param->dur)&&(tCur>_tAge))||_bFirstUpdate)
			{
				_bFirstUpdate=FALSE;

				_tAge=tCur;

				i_math::line2df left,right,base;
				float radiusPush;

				_CalcPush(param,tCur,left,right,radiusPush);
				base.start=left.start;
				base.end=right.start;

				LevelPos posMiddle=base.getMiddle();

//				level->GetDbgDraw().DrawCircle(posMiddle,0.1f+0.04f*(float)_iUpdate,RGB(255,0,0),5.0);
				_iUpdate++;
				
				DWORD c=0;
				CLevelObj **los=NULL;
				if (param->bSkillTargetOnly)
				{
					extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
					_temp=LevelUtil_GetTargetObj(level,skillOwner->GetTarget());
					if (_temp)
					{
						los=&_temp;
						c=1;
					}
				}
				else
					los=_DetectRange(posMiddle,radiusPush,c);

				LevelPos3D posOwnver3D=owner->GetFramePos3D();
				for (int i=0;i<c;i++)
				{
					CLevelObj *loTarget=los[i];
					if (_handled.find(loTarget->GetID())!=_handled.end())
						continue;//已经处理过了
					float radiusTarget=loTarget->GetRadius_();
					LevelPos posTarget=loTarget->GetFramePos();

					if (posTarget.getDistanceSQFrom(posMiddle)>=(radiusPush+radiusTarget)*(radiusPush+radiusTarget))
						continue;
					if (posTarget.getDistanceSQFrom(posMiddle)<=0.0001f)
						continue;//太近了

					//竖直方向上的范围检测
					if (owner->GetType()==LevelObjType_Unit)
					{
						if (loTarget->GetType()==LevelObjType_Unit)
						{
							LevelPos3D posTarget3D=loTarget->GetFramePos3D();
							if (owner->GetHeight()+posOwnver3D.y<posTarget3D.y)
								continue;//太高了
						}
					}

					i_math::circlef ccl;
					ccl.setCenter(posTarget);
					ccl.setRadius(radiusTarget);

					//判断是否在3条扫描线之间
// 						if (!ccl.isIntersectingWithLine(left))
// 						{
// 							if(!ccl.isIntersectingWithLine(right))
// 							{
// 								if(!ccl.isIntersectingWithLine(base))
							{
								if (left.classifyPoint(posTarget)<0)
									continue;
								if (base.classifyPoint(posTarget)>0)
									continue;
								if (right.classifyPoint(posTarget)>0)
									continue;
							}
// 							}
// 						}


					if (_rec->deals.size()<=0)
					{
						LOG_DUMP_1P("EoPush",Log_Error,"Eo(%s)里结算列表为空",_rec->Name.c_str());
					}
					DealArg arg;
					arg.dir.setXZ((posTarget-posMiddle).safe_normalize());
					arg.grd=1;
					arg.link.id=level->GenOpLinkID();
					arg.link.t=tCur;
					_MakeDeals(loTarget,arg);

					_handled.insert(loTarget->GetID());
				}

				return;
			}
		}
	}

	if (_iUpdate<=1)
	{
		AnimTick tCur=_GetSkillCastingTime();
		if (tCur!=ANIMTICK_INFINITE)
		{
			tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);
		}

	}
	DeferDestroy();
	return;
}


void EoPush::UpdateSubframe()
{
	_OnUpdate();
}
