
#include "stdh.h"


#include "Skill_MIssileSP.h"

#include "LevelRecordSkill.h"

#include "Level.h"

#include "LevelDecider.h"

#include "LoSPStone.h"

#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_MissileSP
BIND_SKILLPARAM(Skill_MissileSP,SkillParam_MissileSP);


void Skill_MissileSP::Start()
{
	SkillParam_MissileSP *param=(SkillParam_MissileSP*)_param;

	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	if (_target.tp==LevelSkillTarget::Target_DefObj)
	{
		CLevelObj *lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
		if (lo)
		{
			LevelPos pos=lo->GetFramePos();
			LevelPos posMe=_owner->GetFramePos();

			float dist=(pos-posMe).getLength();

			_dur=ANIMTICK_FROM_SECOND(dist/param->Speed);
			_SetState(SkillState_Casted);
		}
	}
}

void Skill_MissileSP::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casted)
	{
		_dur=ANIMTICK_SAFE_MINUS(_dur,dt);
		if (_dur<=0)
		{
			CLevelObj *lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());

			if (lo)
			{
				if (IsClass2(_owner,CLoSPStone))
				{
					CLoSPStone *owner=((CLoSPStone*)_owner);
					if (!owner->IsGathered())
					{
						LevelOpLink link;
						link.id=GetLevel()->GenOpLinkID();
						GetLevel()->GetDecider()->CommitSPMod((float)owner->GetSPAmount(),
											LevelOSB(this),lo,link,FALSE);
						owner->SetGathered();
					}
				}
			}
			_SetState(SkillState_Finished);
		}
	}

}
