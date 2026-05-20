/********************************************************************
	created:	2012/11/26 
	author:		cxi
	
	purpose:	NPC
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "LevelNPCs.h"

#include "LevelPlayerStates.h"

#include "behaviorgraph/BehaviorMem.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordNPC.h"

#include "Log/LogDump.h"


void CLevelNPC::SwitchState(NPCState stateNew)
{
	if (_state!=stateNew)
	{
		_state=stateNew;
		_bStateDirty=1;

		if (_loUnit)
		{
			CLevelNPCs *npcs=GetNPCs();
			LevelPlayerID idPlayer=npcs->GetPlayerID();
			if (idPlayer!=LevelPlayerID_Invalid)
			{
				if (_state==NPCState_Retinue)
				{
						_loUnit->SetPlayerID(idPlayer);
						_loUnit->SetVisibleToAll();
				}
				if (_state==NPCState_Default)
				{
					_loUnit->SetPlayerID(LevelPlayerID_NeutralWild);
					_loUnit->SetOnlyVisible(idPlayer);
				}
			}
		}
	}

}


void CLevelNPC::Update()
{
	if (TRUE)
	{
		_bInUpdate=1;

		if (_loUnit)
		{
			//判断_loUnit是否还活着
			if (_loUnit->IsAlive())
			{
				extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
				if (LevelUtil_CheckDead(_loUnit))
				{
					SwitchState(NPCState_RetinueDead);
					if(_loUnit)
						_loUnit->DeferDestroy();
					SAFE_RELEASE(_loUnit);
				}
			}
			else
			{
				SAFE_RELEASE(_loUnit);
			}

		}

		_bInUpdate=0;

		LPSNpcData *dataNPC=NULL;

		if (_bStateDirty)
		{
			dataNPC=LPS_QueryNpcData(_npcs->GetLPS(),_rec->GetID());
			if (dataNPC)
				dataNPC->state=_state;
			_bStateDirty=FALSE;
		}

		if (_loUnit)
		{
			CLevelBehavior *bhv=_loUnit->GetBehaviorAI();
			if (bhv)
			{
				CBehaviorMem *mem=bhv->GetMem(0);
				if (mem)
				{
					if (mem->IsPersistDirty())
					{
						if (!dataNPC)
							dataNPC=LPS_QueryNpcData(_npcs->GetLPS(),_rec->GetID());

						if (dataNPC)
						{
							CDataPacket dp;
							DP_BeginSave(dp,dataNPC->dataBhv);
							mem->SavePersist(&dp);
							DP_EndSave();

							mem->ClearPersistDirty();
						}
					}
				}
			}
		}
	}
}

LPSNpcData *CLevelNPC::GetNpcData()
{
	if (_npcs&&_rec)
		return LPS_FindNpcData(_npcs->GetLPS(),_rec->GetID());
	return NULL;
}


// void CLevelNPC::_CreateBehavior(CLevel *level,LPSNpcData *dataNPC)
// {
// 	if (_loUnit)
// 	{
// 		if (_rec->nmBG!=StringID_Invalid)
// 		{
// 			LevelBehaviorContext ctx;
// 			ctx.idPlayerLock=_npcs->GetPlayerID();
// 			ctx.lo=_loUnit;
// 			ctx.npc=this;
// 			_bhv=level->CreateBehavior(_rec->nmBG,ctx);
// 			if (_bhv)
// 			{
// 				CBehaviorMem *mem=_bhv->GetMem(0);
// 				if (mem)
// 				{
// 					if (dataNPC)
// 					{
// 						if (dataNPC->dataBhv.size()>0)
// 						{
// 							CDataPacket dp;
// 							dp.SetDataBufferPointer(&dataNPC->dataBhv[0]);
// 
// 							mem->LoadPersist(&dp);
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// }


BOOL CLevelNPC::Create(LevelPos &pos,LPSNpcData *dataNPC)
{
	_state=NPCState_Default;
	if (dataNPC)
		_state=dataNPC->state;

	CLevel *level=_npcs->GetLevel();
	LevelPlayerID idPlayer=_npcs->GetPlayerID();
	if (_state!=NPCState_RetinueDead)
	{
		CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
		if (!lo)
			return FALSE;
		if (_state==NPCState_Retinue)
			lo->PostCreate(idPlayer,NULL,_rec->idUnit,_rec->grdBase,NULL,EquipSetPick_None,pos);
		else
		{
			lo->PostCreate(LevelPlayerID_NeutralWild,NULL,_rec->idUnit,_rec->grdBase,NULL,EquipSetPick_None,pos);
			lo->SetOnlyVisible(idPlayer);
		}

		lo->EquipNpc(this);
		level->AddToActives(lo);

		_loUnit=lo;
	}

	return TRUE;
}

BOOL CLevelNPC::CreateTeleport(CLevel *level,LevelPos &pos,CLevelNPC *npcOrg)
{
	_state=npcOrg->_state;
	_bStateDirty=0;
	_bInUpdate=0;

	if (npcOrg->_loUnit)
	{
		_loUnit=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
		_loUnit->PostCreate_Teleport(npcOrg->_loUnit,pos);
		_loUnit->EquipNpc(this);
		level->AddToActives(_loUnit);
	}

	return TRUE;
}


void CLevelNPC::Destroy()
{
	if (_loUnit)
		_loUnit->DeferDestroy();
	SAFE_RELEASE(_loUnit);

	Zero();
}

RecordID CLevelNPC::GetRecID()
{		
	return _rec?_rec->GetID():RecordID_Invalid;	
}

void CLevelNPC::AddCoSkillCharge(LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target)
{
	if (_loUnit)
	{
		extern BOOL LevelUtil_AddCoSkillCharge(CLevelObj *lo,LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target);
		if (LevelUtil_AddCoSkillCharge(_loUnit,recSkill,grd,target))
		{
// 			if (_bhv)
// 				_bhv->Update();
		}
	}
}


////////////////////////////////////////////////////////////////////////
//CLevelNPCs
BOOL CLevelNPCs::Create(CLevel *level,LevelPlayerID idPlayer,CNPCPendings::Pendings *pendings)
{
	CLevelPlayer *player=level->GetPlayer(idPlayer);
	if (!player)
		return FALSE;

	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return FALSE;

	_level=level;
	_idPlayer=idPlayer;
	_lps=lps;

	if (pendings)
	{
		CLevelRecords *records=level->GetRecords();

		BOOL bPlayerPos=FALSE;
		LevelPos posPlayer;
		if (TRUE)
		{
			CLoUnit *loUnit=player->GetLoUnit();
			if (loUnit)
			{
				posPlayer=loUnit->GetFramePos();
				bPlayerPos=TRUE;
			}
		}

		_npcs.reserve(pendings->buf.size());


		for (int i=0;i<pendings->buf.size();i++)
		{
			CNPCPendings::Pending *pending=&pendings->buf[i];

			LPSNpcData *dataNPC=pending->dataNPC;
			if (!dataNPC)
				dataNPC=LPS_FindNpcData(lps,pending->idNPC);

			BOOL bRetinue=FALSE;
			if (dataNPC)
			{
				if ((dataNPC->state==NPCState_Retinue)||(dataNPC->state==NPCState_RetinueDead))
					bRetinue=TRUE;
			}

			LevelPos pos=pending->pos;
			if (bRetinue)
			{
				if (bPlayerPos)
					pos=posPlayer;
				else
					continue;//无法得到Player的位置,所以无法创建Retinue的NPC
			}

			LevelRecordNPC *recNPC=records->GetNPC(pending->idNPC);
			if (!recNPC)
				continue;
			if (recNPC->idUnit==RecordID_Invalid)
			{
				LOG_DUMP_1P("CLevelNPCs",Log_Error,"NPC表格项(%s)中未填写正确的单位id!",recNPC->Name.c_str());
				continue;
			}
		
			CLevelNPC *npc=Class_New2(CLevelNPC);
			npc->Init(this,recNPC);
			if (FALSE==npc->Create(pos,dataNPC))
			{
				Safe_Class_Delete(npc);
				continue;
			}

			_npcs.push_back(npc);
		}
	}

	return TRUE;
}

BOOL CLevelNPCs::CreateTeleport(CLevel *level,LevelPlayerID idPlayer,CLevelNPCs *npcsOrg)
{
	CLevelPlayer *player=level->GetPlayer(idPlayer);
	if (!player)
		return FALSE;

	LevelPlayerStates *lps=player->GetLPS();
	if (!lps)
		return FALSE;

	_level=level;
	_idPlayer=idPlayer;
	_lps=lps;

	LevelPos posPlayer;
	if (TRUE)
	{
		CLoUnit *loUnit=player->GetLoUnit();
		if (loUnit)
			posPlayer=loUnit->GetFramePos();
		else
			return FALSE;
	}


	_npcs.reserve(npcsOrg->_npcs.size());

	for (int i=0;i<npcsOrg->_npcs.size();i++)
	{
		CLevelNPC *npcOrg=npcsOrg->_npcs[i];

		CLevelNPC *npc=Class_New2(CLevelNPC);
		npc->Init(this,npcOrg->GetRec());

		npc->CreateTeleport(_level,posPlayer,npcOrg);

		_npcs.push_back(npc);
	}
	return TRUE;
}

void CLevelNPCs::Destroy(RecordID idNPC)
{
	int c=0;
	for (int i=0;i<_npcs.size();i++)
	{
		if (_npcs[i])
		{
			if ((idNPC==RecordID_Invalid)||(_npcs[i]->GetRecID()==idNPC))
			{
				_npcs[i]->Destroy();
				Safe_Class_Delete(_npcs[i]);
				continue;
			}
			_npcs[c]=_npcs[i];
			c++;
		}
	}
	_npcs.clear();
	Zero();

}

void CLevelNPCs::Update()
{
	for (int i=0;i<_npcs.size();i++)
	{
		if (_npcs[i])
			_npcs[i]->Update();
	}


}

CLevelNPC *CLevelNPCs::FetchNPC(RecordID idNPC)
{
	for (int i=0;i<_npcs.size();i++)
	{
		if (_npcs[i])
		{
			LevelRecordNPC *rec=_npcs[i]->GetRec();
			if (rec)
			{
				if (rec->GetID()==idNPC)
				{
					CLevelNPC *ret=_npcs[i];
					if (_npcs.size()>0)
					{
						_npcs[i]=_npcs[_npcs.size()-1];
						_npcs.pop_back();
					}
					return ret;
				}
			}
		}
	}

	return NULL;
}

void CLevelNPCs::AddNPC(CLevelNPC *npc)
{
	_npcs.push_back(npc);
}

void CLevelNPCs::AddCoSkillCharge(LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target)
{
	for (int i=0;i<_npcs.size();i++)
	{
		if (_npcs[i])
			_npcs[i]->AddCoSkillCharge(recSkill,grd,target);
	}
}
