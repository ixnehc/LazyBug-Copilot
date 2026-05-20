
#include "stdh.h"

#include "Ability_WhirlWind.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeWhirlWind_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeWhirlWind_Init);
BOOL CUpgradeWhirlWind_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_WhirlWind *ability=(CLevelAbility_WhirlWind *)ability_;

	return TRUE;
}

// BOOL CUpgradeWhirlWind_Init::Init(LevelItemState *state)
// {
// 	ItemBuff buff;
// 	buff.Set_AddMaxHP(_deltaMaxHP);
// 	state->AddItemBuff(buff);
// 
// 	buff.Set_AddMaxSP(_deltaMaxSP);
// 	state->AddItemBuff(buff);
// 
// 	buff.Set_AddPhysDef(_deltaPhysDef);	
// 	state->AddItemBuff(buff);
// 
// 	buff.Set_AddMoveSpeed(_ims);
// 	state->AddItemBuff(buff);
// 
// 
// 	return TRUE;
// }


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_WhirlWind

void CLevelAbility_WhirlWind::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_WhirlWind::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_WhirlWind::_OnUpdate(LevelTick dt)
{
}


void CLevelAbility_WhirlWind::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_Damage)
	{
		LeDamage &e=(LeDamage &)e0;
		if (e.loTarget==_owner)
		{
			if (e.strike)
			{
				DamageAttrMask attr=(e.strike->maskDmg);//XXXXX: More DamageAttrType
				if (attr&(DamageAttrType_Pierce|DamageAttrType_Crush))
				{
					CUpgradeWhirlWind_Init *upgradeInitial=_GetInitialUpgrade<CUpgradeWhirlWind_Init>();
					if (upgradeInitial)
					{
						if (upgradeInitial->_dealWhenDamaged)
						{
							if (_owner)
							{
								DealArg arg;
								arg.link=e.link;
								float rad=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
								arg.dir.x=cosf(rad);
								arg.dir.z=sinf(rad);

								upgradeInitial->_dealWhenDamaged->Make(LevelOSB(_owner),_owner->GetFramePos3D(),arg,NULL);
							}
						}
					}
				}
			}
		}
	}
}

void CLevelAbility_WhirlWind::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeWhirlWind_Init*upgrade=_GetInitialUpgrade<CUpgradeWhirlWind_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);


	}

}
