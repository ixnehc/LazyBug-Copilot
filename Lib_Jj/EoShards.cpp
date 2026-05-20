
#include "stdh.h"

#include "Level.h"
#include "LevelTroops.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "EoShards.h"


BIND_EOPARAM(EoShards,EoParamShards);

void EoShards::_OnPostCreate()
{
	EoParamShards *param=GetParam<EoParamShards>();

	CLevelObj *owner=_GetOwner();
	if (owner)
	{
		CLevelTroop *troop=owner->GetTroop();
		if (troop)
			troop->AddUnit(LevelTroopRank_Minion,GetID());
	}

}

void EoShards::_OnDetroy()
{
}

void EoShards::NotifyReady()
{
	_stage=ShardsStage_Ready;
	_tStageStart=_level->GetT_();
}

void EoShards::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bit_Write(_bSyncDirty);
	if (_bSyncDirty)
	{
		bp->Bits_Write(_stage,3);
		bp->Bits_Write(_faction,3);
		bContent=TRUE;
	}
}

void EoShards::_OnPostWriteSync()
{
	_bSyncDirty=FALSE;
}

void EoShards::_OnUpdate()
{
	EoParamShards *param=GetParam<EoParamShards>();

	if (_stage==ShardsStage_Fire)
	{

	}

	if (_stage==ShardsStage_Ready)
	{
		DWORD c;
		CLevelObj **buf=_DetectRange(_GetInitialPos(),param->radiusMonitor,c);

		BOOL bPlayer=FALSE;
		LevelPlayerID idPlayer=LevelPlayerID_Invalid;
		for (int i=0;i<c;i++)
		{
			CLevelObj *lo=buf[i];
			if (lo->GetType()!=LevelObjType_Unit)
				continue;
			if(lo->IsPlayer())
			{
				bPlayer=TRUE;
				idPlayer=lo->GetPlayerID();
				break;
			}
		}
		if (idPlayer==LevelPlayerID_Invalid)
		{
			CLevelPlayer *player=LevelUtil_GetFirstPlayer(_level);
			if (player)
				idPlayer=player->GetPlayerID();
		}

		BOOL bEnemy=FALSE;
		for (int i=0;i<c;i++)
		{
			CLevelObj *lo=buf[i];
			if (lo->GetType()!=LevelObjType_Unit)
				continue;
			if (LevelUtil_CheckDead(lo))
				continue;
			if(!lo->IsPlayer())
			{
				LevelRelation relation=LevelUtil_CalcPlayerRelation(_level->GetRelationMatrix(),idPlayer,lo->GetPlayerID());
				if (relation==LevelRelation_Enemy)
				{
					bEnemy=TRUE;
					break;
				}
			}
		}

		int faction=0;
		if (bPlayer&&(!bEnemy))
			faction=1;	
		if (bEnemy&&(!bPlayer))
			faction=2;
		if (faction==0)
			faction=_faction;

		if (faction!=_faction)
			_bSyncDirty=TRUE;
		_faction=faction;

		if (_level->GetT_()>=_tStageStart+param->durReady)
		{
			if (faction==0)
				_stage=ShardsStage_PostFire;
			else
				_stage=ShardsStage_PreFire;
			_tStageStart=_level->GetT_();

			if (_stage==ShardsStage_PostFire)
			{
				if (_troop)
					_troop->DetachUnit(GetID());
			}
			_bSyncDirty=TRUE;
		}
	}

	if (_stage==ShardsStage_PreFire)
	{
		CLevelObj *loPlayer=LevelUtil_DetectPlayer(this,param->radiusFire);
		if (loPlayer)
		{
			_posFireLock=loPlayer->GetFramePos3D();
			_posFireLock.y+=loPlayer->GetAimHeight();
			_idFireLock=loPlayer->GetID();

			_stage=ShardsStage_Fire;
			_tStageStart=_level->GetT_();

			_bSyncDirty=TRUE;
		}
	}

	if (_stage==ShardsStage_Fire)
	{
		//更新FireLock
		if (TRUE)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idFireLock);
			if (lo)
			{
				_posFireLock=lo->GetFramePos3D();
				_posFireLock.y+=lo->GetAimHeight();
			}
		}

		AnimTick tStage=ANIMTICK_SAFE_MINUS(_level->GetT_(),_tStageStart);
		int nToFire=tStage/param->durFireCycle;
		RecordID idEo=RecordID_Invalid;
		if (_faction==1)
			idEo=param->idFriendlyBullet;
		if (_faction==2)
			idEo=param->idEnemyBullet;

		while(nToFire>_nFired)
		{
			LevelPos3D pos;
			pos=_GetInitialPos3D();
			pos.y+=param->htCore;

			LevelPos3D dir=_posFireLock-pos;
			dir.y=0.0f;
			dir.normalize();

			pos+=dir*param->radiusCore;

			if (idEo!=RecordID_Invalid)
			{
				LevelRecordEo *rec=_level->GetRecords()->GetEo(idEo);
				CLoEffectObj *eo=NULL;
				if (rec)
				{
					eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
					if (eo)
					{
						eo->SetHost(_idFireLock);
						eo->PostCreate(GetPlayerID(),rec,pos,dir,1,LevelOSB(this),LevelOpLink());
						_level->AddToActives(eo);
					}
				}
			}

			_nFired++;
		}

		if (_nFired>=param->count)
		{
			_stage=ShardsStage_PostFire;
			_tStageStart=_level->GetT_();
			_bSyncDirty=TRUE;

			if (_troop)
				_troop->DetachUnit(GetID());
		}
	}

	if (_stage==ShardsStage_PostFire)
	{
		AnimTick tStage=ANIMTICK_SAFE_MINUS(_level->GetT_(),_tStageStart);
		if (tStage>ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}

}
