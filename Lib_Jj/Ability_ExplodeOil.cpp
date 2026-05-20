
#include "stdh.h"

#include "Ability_ExplodeOil.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeExplodeOil_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeExplodeOil_Init);
BOOL CUpgradeExplodeOil_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_ExplodeOil *ability=(CLevelAbility_ExplodeOil *)ability_;

	return TRUE;
}

// BOOL CUpgradeExplodeOil_Init::Init(LevelItemState *state)
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
//CLevelAbility_ExplodeOil

void CLevelAbility_ExplodeOil::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_ExplodeOil::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_ExplodeOil::_OnUpdate(LevelTick dt)
{
}


void CLevelAbility_ExplodeOil::_OnEvent(LevelEvent &e0)
{
    if (e0.GetType() == LET_Hit)
    {
        LeHit &e = (LeHit &)e0;
        if (e.osbSrc)
        {
            if (e.osbSrc->GetOwner() == _owner)
            {
//                 if (e.strike)
//                 {
//                     DamageAttrMask attr = (e.strike->maskDmg);
//                     if (attr&(DamageAttrType_Pierce | DamageAttrType_Crush))
//                     {
//                         CUpgradeExplodeOil_Init *upgradeInitial = _GetInitialUpgrade<CUpgradeExplodeOil_Init>();
//                         if (upgradeInitial)
//                         {
//                             if (upgradeInitial->_dealWhenDamaged)
//                             {
//                                 if (_owner)
//                                 {
//                                     DealArg arg;
//                                     arg.link = e.link;
//                                     float rad = CSysRandom::RandRange(0.0f, i_math::Pi*2.0f);
//                                     arg.dir.x = cosf(rad);
//                                     arg.dir.z = sinf(rad);
// 
//                                     upgradeInitial->_dealWhenDamaged->Make(LevelOSB(_owner), _owner->GetFramePos3D(), arg);
//                                 }
//                             }
//                         }
//                     }
//                }
            }
        }
    }
}

void CLevelAbility_ExplodeOil::_OnBuildArtifactState(LevelItemState &state)
{
}
