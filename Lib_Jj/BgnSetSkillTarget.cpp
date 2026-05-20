/********************************************************************
	created:	2020/01/19 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"


#include "BgnSetSkillTarget.h"

#include "LevelBGs.h"

#include "Skill_GeneralAdvS.h"

#include "LevelUtil.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetSkillTarget
BIND_BGN_CLASS(CBgnSetSkillTarget,CBgpSetSkillTarget);

void CBgnSetSkillTarget::_ClampPosDist(LevelPos &pos)
{
	CBgpSetSkillTarget*pad=_GetPad<CBgpSetSkillTarget>();

	if ((pad->distMin>0.0f)||(pad->distMax>0.0f))
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			LevelPos posMe=lo->GetFramePos();

			float dist=posMe.getDistanceFrom(pos);

			if (pad->distMin>0.0f)
			{
				if (dist<pad->distMin)
					dist=pad->distMin;
			}
			if (pad->distMax>0.0f)
			{
				if (dist>pad->distMax)
					dist=pad->distMax;
			}

			LevelPos dir=pos-posMe;
			dir.normalize();
			pos=posMe+dir*dist;
		}
	}

}

void CBgnSetSkillTarget::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpSetSkillTarget*pad=_GetPad<CBgpSetSkillTarget>();

	CLevelObj *lo=ctx->lo;

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
	if (skill)
	{
		LevelPos posSrc=lo->GetFramePos();

		if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
		{
			Skill_GeneralAdvS *skillG=(Skill_GeneralAdvS *)skill;

			if (pad->tp==CBgpSetSkillTarget::SpecifiedObjId)
			{
				LevelObjID idTarget;
				if (_GetID(pad->varObjId,BehaviorMemType_ObjID,idTarget))
					skillG->GetTarget().SetObjID(idTarget);
			}
			if (pad->tp==CBgpSetSkillTarget::SpecifiedPos)
			{
				LevelPos posTarget;
				if (_GetPos(pad->varPos,posTarget))
				{
					_ClampPosDist(posTarget);
					skillG->GetTarget().SetPos(posTarget);
				}
			}
			if (pad->tp==CBgpSetSkillTarget::SpecifiedObjIDAtSpecifiedPos)
			{
				LevelObjID idTarget;
				LevelPos posTarget;
				if (_GetID(pad->varObjId,BehaviorMemType_ObjID,idTarget))
				{
					if (_GetPos(pad->varPos,posTarget))
					{
						_ClampPosDist(posTarget);
						skillG->GetTarget().SetFixPosAndObj(posTarget,0.0f,idTarget);
					}
				}
			}
			if (pad->tp==CBgpSetSkillTarget::ThreatAtSpecifiedPos)
			{
				LevelPos posTarget;
				CLevelObj *target=_GetThreat();
				if (target)
				{
					if (_GetPos(pad->varPos,posTarget))
					{
						_ClampPosDist(posTarget);
						skillG->GetTarget().SetFixPosAndObj(posTarget,0.0f,target->GetID());
					}
				}
			}
			if (pad->tp==CBgpSetSkillTarget::Threat)
			{
				CLevelObj *target=_GetThreat();
				if (target)
					skillG->GetTarget().SetObjID(target->GetID());
			}
			if (pad->tp==CBgpSetSkillTarget::ThreatPos)
			{
				LevelPos posTarget;
				CLevelObj *target=_GetThreat();
				if (target)
				{
					posTarget=target->GetFramePos();
					_ClampPosDist(posTarget);
					skillG->GetTarget().SetPos(posTarget);
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

