
#include "stdh.h"

#include "Ability_StormEye.h"
#include "LevelAttrs.h"
#include "LevelItemState.h" 

#include "LevelEvents.h"

#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeStormEye_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeStormEye_Init);
BOOL CUpgradeStormEye_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_StormEye *ability=(CLevelAbility_StormEye *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_StormEye
void CLevelAbility_StormEye::_OnBuildRT()
{
	_BuildGradeRT();
	CUpgradeStormEye_Init *upgradeInitial=_upgradeInitial;
	_BuildSkillRT(upgradeInitial->settings);

}

void CLevelAbility_StormEye::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_StormEye::_OnUpdate(LevelTick dt)
{
	if(!_bActive)
	{
		CUpgradeStormEye_Init *upgrade=_GetInitialUpgrade<CUpgradeStormEye_Init>();
		_tAccum+=dt;
		if (_tAccum>upgrade->durAccum)
		{
			_tAccum=0;
			_bActive=TRUE;
		}
	}
}

void CLevelAbility_StormEye::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeStormEye_Init *upgrade=_GetInitialUpgrade<CUpgradeStormEye_Init>();
	if (upgrade)
	{

	}

}

void CLevelAbility_StormEye::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreBlocking)
	{
		LePreBlocking &e=(LePreBlocking &)e0;	
		if (e.loTarget==_owner)
		{
			CUpgradeStormEye_Init *upgrade=_GetInitialUpgrade<CUpgradeStormEye_Init>();
			e.idBuff=upgrade->idBuff;
			e.attrBlocking=upgrade->blocking.Get();
		}
	}

	if (e0.GetType()==LET_PostBlocking)
	{
		LePostBlocking &e=(LePostBlocking &)e0;	
		if (e.loBlocker==_owner)
		{
			CUpgradeStormEye_Init *upgrade=_GetInitialUpgrade<CUpgradeStormEye_Init>();

			_bActive=FALSE;
			_tAccum=0;

			BOOL bCrytical=FALSE;
			if (TRUE)
			{
				if (_owner->GetT()<e.tActivated+upgrade->durCrytical)
					bCrytical=TRUE;
			}

			if (e.tpBlock==DmgBlockType_Reflectible)
			{
				CLevelObj *loBlockee=e.osbBlockee->GetRootOwner();
				if (loBlockee->GetType()==LevelObjType_Unit)
				{
					DealArg arg;
					arg.dir.setXZ(e.strikeBack->GetDir());
					arg.link=e.link;

					std::vector<DealEntry> *deals;
					if (!bCrytical)
						deals=&upgrade->dealsBlocking;
					else
						deals=&upgrade->dealsCryticalBlocking;

					CLevelDeal *dealDmgBackup=NULL;
					CLevelDeal *dealDmg=NULL;
					if (deals->size()>0)
					{
						if ((*deals)[0].deal)
						{
							dealDmgBackup=(*deals)[0].deal;
							dealDmg=(*deals)[0].deal->Clone();
							(*deals)[0].deal=dealDmg;

							Deal_Dmg *dealDmgNew=dealDmg->ToPtr<Deal_Dmg>();
							if (dealDmgNew)
							{
								LevelAttr_Damages *attrDamages=dealDmgNew->_attacks.Get();
								if (attrDamages)
								{
									WORD dmgReflect=bCrytical?(WORD)(e.dmgSP*3.0f):(WORD)e.dmgSP;
									attrDamages->damages[DamageAttrType_Crush].lo=dmgReflect;
									attrDamages->damages[DamageAttrType_Crush].hi=dmgReflect;
								}
							}
						}
					}

					MakeDeals(*deals,LevelOSB(_owner),loBlockee,arg,NULL);

					Safe_Class_Delete(dealDmg);
					if (deals->size()>0)
						(*deals)[0].deal=dealDmgBackup;
				}
			}

		}
	}

}
