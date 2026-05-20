
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoBellyEelString.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Skill_GeneralAdvS.h"

#include "LoMagicCircuit.h"



//////////////////////////////////////////////////////////////////////////
//EoBellyEelString
BIND_EOPARAM(EoBellyEelString,EoParamBellyEelString);


void EoBellyEelString::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	EoParamBellyEelString*param=GetParam<EoParamBellyEelString>();

	bp->Data_WriteSimple(_idOwner);
	bp->Data_WriteSimple(_idSrc);
	bContent=TRUE;
}

void EoBellyEelString::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_bSyncDirty)
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimple(_idTarget);
		bContent=TRUE;
	}
	else
		bp->Bit_Write_0();
}

void EoBellyEelString::_OnPostWriteSync()
{
	_bSyncDirty=0;
}


void EoBellyEelString::_OnDetroy()
{
	EoParamBellyEelString*param=GetParam<EoParamBellyEelString>();


	CLoEffectObj::_OnDetroy();
}


void EoBellyEelString::_OnPostCreate()
{
	CLoEffectObj::_OnPostCreate();

	EoParamBellyEelString*param=GetParam<EoParamBellyEelString>();

	CLevelObj *lo=NULL;
	_idOwner=GetRootOwnerID();

	for (int i=0;i<param->_candidatesAttachAgent.size();i++)
	{
		CLevelObj *loAgent=LevelUtil_DetectClosestAgent(this,0.5f,NULL,param->_candidatesAttachAgent[i]);
		if (loAgent)
		{
			_idSrc=loAgent->GetID();
			break;
		}
	}


	_SetStage(Stage_Connecting);
}


void EoBellyEelString::_OnUpdate()
{
	EoParamBellyEelString*param=GetParam<EoParamBellyEelString>();

	if (_idTarget == LevelObjID_Invalid)
	{
		Skill_GeneralAdvS* skill = (Skill_GeneralAdvS*)_GetOwnerSkill();
		if (skill)
		{
			StringID nmStage = skill->GetStageNameID();
			if ((strcmp(StrLib_GetStr(nmStage), "EnterCoil") == 0) ||
				(strcmp(StrLib_GetStr(nmStage), "Coil") == 0))
			{
				for (int i = 0;i < param->_candidatesAttachAgent.size();i++)
				{
					CLevelObj* loAgent = LevelUtil_DetectClosestAgent(_GetOwner(), 1.0f, NULL, param->_candidatesAttachAgent[i]);
					if (loAgent)
					{
						if (loAgent->GetID() != LevelObjID_Invalid)
						{
							_idTarget = loAgent->GetID();
							_bSyncDirty = 1;

							CLoMagicCircuit* loMagicCircuit = (CLoMagicCircuit*)_level->GetUniqueObj(LevelUniqueObj_MagicCircuit);
							if (loMagicCircuit)
								loMagicCircuit->GetEelRoadNetwork().Repair(_idSrc, _idTarget);

							_SetStage(Stage_Connected);

							break;
						}
					}
				}
			}
		}
	}

	if (_msg.tp==CBEelString::Touch)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_msg.idTarget);
		if (lo)
		{
			DealArg arg;
			MakeDeals(param->_dealsTouch,LevelOSB(this),lo,arg,NULL);
		}
	}

	if (_msg.tp == CBEelString::Broken)
	{
		if (_stage != Stage_Broken)
		{
			CLoMagicCircuit* loMagicCircuit = (CLoMagicCircuit*)_level->GetUniqueObj(LevelUniqueObj_MagicCircuit);
			if (loMagicCircuit)
				loMagicCircuit->GetEelRoadNetwork().Break(_idSrc, _idTarget);
			_SetStage(Stage_Broken);
		}
	}

	if (_stage == Stage_Broken)
	{
		if (_GetT()>_tStageStart+ANIMTICK_FROM_SECOND(2.0f))
			DeferDestroy();
	}


}


void EoBellyEelString::_SetStage(Stage stage)
{
	_stage=stage;
	_tStageStart = _GetT();
}


