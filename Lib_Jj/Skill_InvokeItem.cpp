
#include "stdh.h"


#include "Skill_InvokeItem.h"

#include "LevelRecordSkill.h"

#include "Level.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_InvokeAgent
BIND_SKILLPARAM(Skill_InvokeItem,SkillParam_InvokeItem);


void Skill_InvokeItem::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	if (_target.tp==LevelSkillTarget::Target_DefObj)
	{
		CLevelObj *lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
		if (lo)
		{
			if (lo->GetType()==LevelObjType_Item)
			{
				CLoItem *loItem=(CLoItem *)lo;

				LevelPlayerID idPlayer=_owner->GetPlayerID();
				CLevel *lvl=GetLevel();
				CLevelPlayer *player=lvl->GetPlayer(idPlayer);
				if (player)
				{
					if (player->HandleInvokeItem(loItem,_target.arg))
						player->SetLPSDirty();
				}
			}
		}
	}

	_SetState(SkillState_Casted);
}
