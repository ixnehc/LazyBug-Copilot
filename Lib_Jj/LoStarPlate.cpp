
#include "stdh.h"

#include "Level.h"

#include "LoStarPlate.h"

#include "Random/Random.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "LevelUtil.h"
#include "LoUnit.h"
#include "LoBelly.h"

#include "LevelTroops.h"

#include "LevelOSB.h"

extern int GenPrimeStep();

//BOOL g_nDebugMaxSites=-1;
BOOL g_nDebugMaxSites=2;

StarPlateSetting &CLoStarPlate::_GetSetting()
{
	LevelRecordGlobal *recGlobal=_level->GetRecords()->GetGlobal();
	if (recGlobal)
		return recGlobal->starplatesetting;

	static StarPlateSetting setting;
	return setting;
}


BOOL CLoStarPlate::_BuildChain(Chain &chain,LevelPos &posStart,LevelPos &posEnd,int levelConstraint)
{
	LosStarPlate *los=(LosStarPlate *)_src;
	StarPlateSetting &setting=_GetSetting();
	LopStarPlate *lop=(LopStarPlate *)_param;


	LevelPos posCur=posStart;

	int count=CSysRandom::RandRangeInt(setting.countBridgeMin,setting.countBridgeMax+1);
	for(int j=0;j<count;j++)
	{
		int idx=CSysRandom::RandRangeInt<int>(0,lop->links.size());
		int step=GenPrimeStep();

		BOOL bFound=FALSE;
		for (int k=0;k<lop->links.size();k++)
		{
			LevelPos posTest=lop->links[idx].getXZ();
			int idxTest=idx;
			idx=(idx+step)%lop->links.size();

			if (chain.Exist(idxTest))
				continue;

			float dist=posTest.getDistanceFrom(posCur);
			if (levelConstraint>0)
			{
				if (dist<setting.distBridgeMin)
					continue;
			}
			if (levelConstraint>1)
			{
				if (dist>setting.distBridgeMax)
					continue;
			}
			if (levelConstraint>2)
			{
				if (j==count-1)
				{
					dist=posTest.getDistanceFrom(posEnd);
					if ((dist<setting.distBridgeMin)||(dist>setting.distBridgeMax))
						continue;
				}
			}

			Link link;
			link.idxLink=idxTest;
			chain.links.push_back(link);
			posCur=posTest;
			bFound=TRUE;
			break;
		}
		if (!bFound)
			break;
	}

	if (chain.links.size()<setting.countBridgeMin)
	{
		chain.Clear();
		return FALSE;
	}
	return TRUE;
}


void CLoStarPlate::_BuildChains()
{
	LosStarPlate *los=(LosStarPlate *)_src;
	StarPlateSetting &setting=_GetSetting();
	LopStarPlate *lop=(LopStarPlate *)_param;

	_chains.clear();
	_chains.resize(lop->sites.size());

	for (int i=0;i<lop->sites.size();i++)
	{
		Chain &chain=_chains[i];

		LevelPos posStart=lop->sites[i].getTranslation().getXZ();

		int iNext=(i+1)<lop->sites.size()?(i+1):0;
		LevelPos posEnd=lop->sites[iNext].getTranslation().getXZ();

		BOOL bFound=FALSE;
		for (int levelConstraint=3;levelConstraint>=0;levelConstraint--)
		{
			for (int j=0;j<10;j++)//每一种限制等级尝试10次
			{
				if (_BuildChain(chain,posStart,posEnd,levelConstraint))
				{
					bFound=TRUE;
					break;
				}
			}
			if (bFound)
				break;
		}
		if (!bFound)
		{
			_chains.clear();
			return;
		}
	}
}

BOOL CLoStarPlate::_IsChainsBuilt()
{
	LosStarPlate *los=(LosStarPlate *)_src;
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (lop->sites.size()>0)
	{
		if (lop->sites.size()==_chains.size())
			return TRUE;
	}
	return FALSE;
}

void CLoStarPlate::Clear()
{
	_lookupSites.clear();
	_lookupSites2.clear();

	_hilightsLink.clear();
	
}


void CLoStarPlate::OnDestroy()
{
	Clear();

	_level->UnregisterUniqueObj(LevelUniqueObj_StarPlate,this);
}

void CLoStarPlate::ResetState()
{
	_iStartSite=StarPlateSiteIndex_Invalid;
	_iActiveSite=StarPlateSiteIndex_Invalid;
	_tSiteActivated=ANIMTICK_INFINITE;
	_iNextLink=-1;
	_flagsObeliskStars=0;
	_flagsObeliskBulbs=0;
	_nEnemySpawned=0;
	_nEnemyEliminated=0;
}


BOOL CLoStarPlate::OnActivate()
{
	_level->RegisterUniqueObj(LevelUniqueObj_StarPlate,this);
	_BuildChains();

	_tStart=GetT();
	return TRUE;
}

void CLoStarPlate::_BuildLookup()
{
	if (_lookupSites.size()>0)
		return;

	LosStarPlate *los=(LosStarPlate *)_src;
	StarPlateSetting &setting=_GetSetting();
	LopStarPlate *lop=(LopStarPlate *)_param;

	_lookupSites2.resize(lop->sites.size());
	VEC_SET(_lookupSites2,0);
	for(int i=0;i<lop->sites.size();i++)
	{
		_lookupSites2[i]=LevelObjID_Invalid;

		LevelPos pos=lop->sites[i].getTranslationP()->getXZ();

		if (TRUE)
		{
			CLevelObj *lo=LevelUtil_DetectClosestAgent(_level,pos,1.0f,NULL,setting.idAgentSite);
			if (lo)
			{
				_lookupSites[lo->GetID()]=i;
				_lookupSites2[i]=lo->GetID();
			}
		}

	}

	if (TRUE)
	{
		CLevelObj *lo=LevelUtil_DetectClosestAgent(_level,GetFramePos(),8.0f,NULL,setting.idAgentObelisk);
		if (lo)
			_idObelisk=lo->GetID();
	}

}

StarPlateSiteIndex CLoStarPlate::_FindSite(LevelObjID id)
{
	std::unordered_map<LevelObjID,StarPlateSiteIndex>::iterator it=_lookupSites.find(id);
	if (it==_lookupSites.end())
		return StarPlateSiteIndex_Invalid;
	return (*it).second;
}


BOOL CLoStarPlate::CanActivateSite(LevelObjID idSite)
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (CheckFullActivated())
		return FALSE;

	StarPlateSiteIndex iSite=_FindSite(idSite);
	if (iSite<0)
		return FALSE;

	if ((_iStartSite==StarPlateSiteIndex_Invalid)&&(_iActiveSite==StarPlateSiteIndex_Invalid))
	{
		if (iSite==0)
			return TRUE;
	}

	StarPlateSiteIndex iNextSite=(_iActiveSite+1)%lop->sites.size();

	if (iNextSite==iSite)
	{
		Chain &chain=_chains[_iActiveSite];
		if (chain.links.size()<=_iNextLink)
			return TRUE;
	}
	return FALSE;
}

BOOL CLoStarPlate::GetNextSitePosToActivate(LevelPos &pos)
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (_iActiveSite<0)
		return FALSE;

	StarPlateSiteIndex iNextSite=(_iActiveSite+1)%lop->sites.size();
	if (iNextSite==_iStartSite)
		return FALSE;

	CLevelObj *loSite=LevelUtil_GetAliveLo(_level,_lookupSites2[iNextSite]);
	if (loSite)
	{
		pos=loSite->GetFramePos();
		return TRUE;
	}

	pos=lop->sites[iNextSite].getTranslation().getXZ();

	return TRUE;

}

BOOL CLoStarPlate::CheckFullActivated()
{
	LopStarPlate *lop=(LopStarPlate *)_param;
	if ((_iStartSite!=StarPlateSiteIndex_Invalid)&&(_iActiveSite!=StarPlateSiteIndex_Invalid))
	{
		if (g_nDebugMaxSites>=0)
		{
			if (_iActiveSite==_iStartSite+g_nDebugMaxSites-1)
				return TRUE;
		}
		if ((_iActiveSite+1)%lop->sites.size()==_iStartSite)
			return TRUE;
	}
	return FALSE;
}


void CLoStarPlate::ActivateSite(LevelObjID idSite)
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (!CanActivateSite(idSite))
		return;

	_bStateSyncDirty=1;

	StarPlateSiteIndex iSite=_FindSite(idSite);

	if (_iStartSite==StarPlateSiteIndex_Invalid)
	{
		_iStartSite=iSite;
		_iActiveSite=iSite;
		_iNextLink=0;

		_tSiteActivated=_level->GetT_();

		_flagsObeliskStars|=(1<<iSite);
		_flagsObeliskBulbs|=(1<<iSite);

		return;
	}

	StarPlateSiteIndex iNextSite=(_iActiveSite+1)%lop->sites.size();

	_iActiveSite=iSite;
	_iNextLink=0;
	_tSiteActivated=_level->GetT_();

	_flagsObeliskStars|=(1<<iSite);
	_flagsObeliskBulbs|=(1<<iSite);

}

BOOL CLoStarPlate::CheckAnySiteActivated()
{
	return _iActiveSite!=StarPlateSiteIndex_Invalid;
}

BOOL CLoStarPlate::_CheckSiteActivated(StarPlateSiteIndex idxSite)
{
	if (idxSite==StarPlateSiteIndex_Invalid)
		return FALSE;
	if (_iActiveSite>=_iStartSite)
	{
		if ((idxSite>=_iStartSite)&&(idxSite<=_iActiveSite))
			return TRUE;
	}
	else
	{
		if (idxSite>=_iStartSite)
			return TRUE;
		if (idxSite<=_iActiveSite)
			return TRUE;
	}
	return FALSE;
}

BOOL CLoStarPlate::CheckSiteActivated(LevelObjID idSite)
{
	StarPlateSiteIndex iSite=_FindSite(idSite);
	return _CheckSiteActivated(iSite);
}


LevelObjID CLoStarPlate::FindClosestSite(LevelPos pos)
{
	LevelObjID idClosest=LevelObjID_Invalid;
	float distMin=10000000.0f;

	std::unordered_map<LevelObjID,StarPlateSiteIndex>::iterator it;
	for (it=_lookupSites.begin();it!=_lookupSites.end();it++)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,(*it).first);
		if (lo)
		{
			float dist=lo->GetFramePos().getDistanceFrom(pos);
			if (dist<distMin)
			{
				distMin=dist;
				idClosest=lo->GetID();
			}
		}
	}

	return idClosest;
}

BOOL CLoStarPlate::GetNextSitePos(LevelObjID idSite,LevelPos &posNext)
{
	StarPlateSiteIndex iSite=_FindSite(idSite);
	if (iSite==StarPlateSiteIndex_Invalid)
		return FALSE;

	LopStarPlate *lop=(LopStarPlate *)_param;

	StarPlateSiteIndex iNextSite=(iSite+1)%lop->sites.size();

	CLevelObj *loSite=LevelUtil_GetAliveLo(_level,_lookupSites2[iNextSite]);
	if (loSite)
	{
		posNext=loSite->GetFramePos();
		return TRUE;
	}

	posNext=lop->sites[iNextSite].getTranslation().getXZ();

	return TRUE;
}


void CLoStarPlate::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (!_bStateSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Data_WriteSimple(_iActiveSite);
		if (_iNextLink>0)
		{
			bp->Bit_Write_1();
			bp->Data_WriteSimple(_chains[_iActiveSite].links[_iNextLink-1].idxLink);
			if (_iNextLink>=_chains[_iActiveSite].links.size())
				bp->Bit_Write_1();
			else
				bp->Bit_Write_0();
		}
		else
			bp->Bit_Write_0();
	}

	if (!_bObeliskSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Data_WriteSimple(_flagsObeliskStars);
		bp->Data_WriteSimple(_flagsObeliskBulbs);
		BYTE nEnemyToSpawn=(BYTE)_GetEnemyToSpawn();
		bp->Data_WriteSimple(nEnemyToSpawn);
		bp->Data_WriteSimple(_nEnemySpawned);
		bp->Data_WriteSimple(_nEnemyEliminated);
	}

	if (!_bHilightsSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();
		bp->Data_NextWord()=_hilightsLink.size();
		std::unordered_set<short>::iterator it;
		for (it=_hilightsLink.begin();it!=_hilightsLink.end();it++)
			bp->Data_WriteSimple(*it);
	}
}
void CLoStarPlate::_OnPostWriteSync()
{
	_bStateSyncDirty=FALSE;
	_bHilightsSyncDirty=FALSE;
	_bObeliskSyncDirty=FALSE;
}


BOOL CLoStarPlate::_GetPlayerPos(LevelPos &pos)
{
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(_level);
	if (player)
	{
		CLoUnit *lo=player->GetLoUnit();
		if (lo)
		{
			pos=lo->GetFramePos();
			return TRUE;
		}
	}
	return FALSE;
}

void CLoStarPlate::_UpdateActive()
{
	LosStarPlate *los=(LosStarPlate *)_src;
	StarPlateSetting &setting=_GetSetting();
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (CheckFullActivated())
		return;

	AnimTick t=GetT();

	if (_iActiveSite>=0)
	{
		BOOL bTimeUp=FALSE;
		if (TRUE)
		{
			if (t>_tSiteActivated+ANIMTICK_FROM_SECOND(setting.durCDSite))
				bTimeUp=TRUE;
		}

		if (bTimeUp)
		{
			ResetState();

			_bStateSyncDirty=TRUE;
			_bObeliskSyncDirty=TRUE;
			return;
		}

		Chain &chain=_chains[_iActiveSite];
		if (_iNextLink>=0)
		{
			if (_iNextLink<chain.links.size())
			{
				int idxLink=chain.links[_iNextLink].idxLink;
				i_math::vector3df posSite=lop->links[idxLink];

				LevelPos posPlayer;
				if (_GetPlayerPos(posPlayer))
				{
					if (posPlayer.getDistanceFrom(posSite.getXZ())<setting.radiusSense)
					{
	// 					if (setting.idBuff_SacredOrb!=RecordID_Invalid)
	// 					{
	// 						CLevelPlayer *player=LevelUtil_GetFirstPlayer(_level);
	// 						if (player)
	// 						{
	// 							CLoUnit *lo=player->GetLoUnit();
	// 							if (lo)
	// 							{
	// 								if (!LevelUtil_FindBuffByRecordID(lo,setting.idBuff_SacredOrb))
	// 									_level->GetDecider()->MakeBuff(lo,setting.idBuff_SacredOrb,ANIMTICK_INFINITE,NULL,TRUE);
	// 							}
	// 						}
	// 					}

						_iNextLink++;

// 						if (chain.links.size()<=_iNextLink)
// 							_flagsObeliskStars|=(1<<_iActiveSite);

						_bStateSyncDirty=TRUE;
					}
				}
			}
		}
	}
}


void CLoStarPlate::_UpdateHilights()
{
	LosStarPlate *los=(LosStarPlate *)_src;
	StarPlateSetting &setting=_GetSetting();
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (CheckFullActivated())
		return;

	LevelPos posPlayer(-10000.0f,-10000.0f);
	_GetPlayerPos(posPlayer);

	std::unordered_set<short>::iterator it=_hilightsLink.begin();
	while(it!=_hilightsLink.end())
	{
		std::unordered_set<short>::iterator itCur=it;
		it++;
		LevelPos posTest=lop->links[*itCur].getXZ();
		if (posTest.getDistanceFrom(posPlayer)>=setting.radiusSense)
		{
			_hilightsLink.erase(itCur);
			_bHilightsSyncDirty=TRUE;
		}
	}

	for (int k=0;k<lop->links.size();k++)
	{
		LevelPos posTest=lop->links[k].getXZ();
		if (posTest.getDistanceFrom(posPlayer)<setting.radiusSense)
		{
			if (_hilightsLink.find(k)==_hilightsLink.end())
			{
				_hilightsLink.insert(k);
				_bHilightsSyncDirty=TRUE;
			}
		}
	}

}

void CLoStarPlate::_UpdateKillingKing()
{
	if (_bKingKilled)
		return;

	if (!CheckFullActivated())
		return;

	StarPlateSetting &setting=_GetSetting();
	CLevel *level=GetLevel();

	if (_tSiteActivated+ANIMTICK_FROM_SECOND(setting.durKillingKingDelay)>level->GetT_())
		return;

	CLevelObj *loKing=NULL;
	if (TRUE)
	{
		CLoBelly *loBelly=(CLoBelly *)level->GetUniqueObj(LevelUniqueObj_Belly);
		if (loBelly)
			loKing=LevelUtil_GetAliveLo(level,loBelly->GetObjID_King());
	}

	if (loKing)
	{
		if (!LevelUtil_CheckDead(level,loKing->GetID()))
		{
			DealArg arg;
			MakeDeals(setting.dealsKillingKing,LevelOSB(this),loKing,arg,NULL);
		}
	}

	_bKingKilled=TRUE;


}


void CLoStarPlate::Update()
{
	if (_tStart<ANIMTICK_INFINITE)
	{
		if (GetT()>_tStart+ANIMTICK_FROM_SECOND(0.2f))
			_BuildLookup();
	}

	_UpdateActive();		
	_UpdateKillingKing();
	_UpdateEnemy();
	_UpdateSacredOrb();

	_UpdateHilights();

}

BOOL CLoStarPlate::GetCenterCircle(i_math::circlef &circle)
{
	LopStarPlate *lop=(LopStarPlate *)_param;
	if (lop->center.size()>0)
	{
		circle.center=lop->center[0].center.getXZ();
		circle.radius=lop->center[0].radius;
		return TRUE;
	}

	return FALSE;
}

BOOL CLoStarPlate::CheckNeedSpawnEnemy()
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (CheckFullActivated())
		return FALSE;

	if (_GetEnemyToSpawn()>_nEnemySpawned)
		return TRUE;

	return FALSE;
}

int CLoStarPlate::_GetEnemyToSpawn()
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	int nEnemyToSpawn=0;
	if (_iStartSite>=0)
	{
		nEnemyToSpawn=(int)(_iActiveSite-_iStartSite);
		if (nEnemyToSpawn<0)
			nEnemyToSpawn+=lop->sites.size();
		nEnemyToSpawn++;
		if (nEnemyToSpawn>lop->sites.size()-1)
			nEnemyToSpawn=lop->sites.size()-1;

		if (g_nDebugMaxSites>=0)
		{
			if (nEnemyToSpawn>g_nDebugMaxSites-1)
				nEnemyToSpawn=g_nDebugMaxSites-1;
		}

	}
	return nEnemyToSpawn;
}

void CLoStarPlate::NotifyEnemySpawned()
{
	_nEnemySpawned=_GetEnemyToSpawn();
	_bObeliskSyncDirty=1;
}


BOOL CLoStarPlate::_CheckAllEnemyEliminated()
{
	if (_idObelisk==LevelObjID_Invalid)
		return FALSE;

	CLevelObj *loObelisk=LevelUtil_GetAliveLo(_level,_idObelisk);
	if (loObelisk)
	{
		CLevelTroops *troops=loObelisk->GetTroops();
		if (troops)
		{
			if (CLevelTroop *troop=troops->GetFirst())
			{
				if (!troop->IsAllDead(LevelTroopRankFlag_All))
					return FALSE;
			}
		}
	}

	return TRUE;
}

void CLoStarPlate::_UpdateEnemy()
{
	if (_nEnemySpawned>_nEnemyEliminated)
	{
		if (_CheckAllEnemyEliminated())
		{
			_nEnemyEliminated=_nEnemySpawned;
			_bObeliskSyncDirty=1;
		}
	}
}

BOOL CLoStarPlate::CheckSacredOrbCharged()
{
	if (_flagsObeliskStars>0)
	{
		BYTE nEnemyToSpawn=(BYTE)_GetEnemyToSpawn();
		if (_nEnemyEliminated>=nEnemyToSpawn)
		{
			if (_iActiveSite>=0)
			{
				Chain &chain=_chains[_iActiveSite];
				if (chain.links.size()<=_iNextLink)
					return TRUE;
			}
		}
	}
	return FALSE;
}


void CLoStarPlate::_UpdateSacredOrb()
{
	StarPlateSetting &setting=_GetSetting();

	if (setting.idBuff_SacredOrb==RecordID_Invalid)
		return;

	CLevelPlayer *player=LevelUtil_GetFirstPlayer(_level);
	if (player)
	{
		CLoUnit *loPlayer=player->GetLoUnit();
		if (loPlayer)
		{
			CLevelBuff *buff=LevelUtil_FindBuffByRecordID(loPlayer,setting.idBuff_SacredOrb);
			if (!buff)
				_level->GetDecider()->MakeBuff(loPlayer,setting.idBuff_SacredOrb,ANIMTICK_INFINITE,NULL,TRUE);
		}
	}

}

void CLoStarPlate::NotifySacredOrbFire()
{
	LopStarPlate *lop=(LopStarPlate *)_param;

	if (_flagsObeliskStars>0)
	{
		for (int i=lop->sites.size()-1;i>=0;i--)
		{
			if (_flagsObeliskStars&(1<<i))
			{
				_flagsObeliskStars&=~(WORD)(1<<i);
				_bObeliskSyncDirty=1;
				break;
			}
		}
	}
}
