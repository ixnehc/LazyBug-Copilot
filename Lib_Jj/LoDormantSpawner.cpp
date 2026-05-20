
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "LoDormantSpawner.h"

#include "Random/Random.h"

#include "LevelRecords.h"

#include "LevelRecordRegion.h"

#include "Buff_Dormant.h"


////////////////////////////////////////////////////////////////////////
//LopDormantSpawner

BOOL LopDormantSpawner::CheckCreateChance(CLevel *level,CLevelObjSrc *los)
{
	return TRUE;
	i_math::matrix43f *mat=&los->GetMat();

	extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
	AgentDistributeInfo*adi=LevelUtil_FindADI(level,los->GetRecID(),mat->getTranslationP()->x,mat->getTranslationP()->z);
	if (!adi)
		return FALSE;

	if (!CSysRandom::Roll(adi->rateAppear))
		return FALSE;

	return TRUE;

}


////////////////////////////////////////////////////////////////////////
//CLoDormantSpawner
void CLoDormantSpawner::_LoadPersist()
{
	LosDormantSpawner *los=(LosDormantSpawner*)_src;
	_bPersistValid=FALSE;

	if (!los->bPersist)
		return;

	CLevel *level=GetLevel();

	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);

	if (TRUE)
	{
		LevelGUID guid=_GetGUID();
		if (guid!=LevelGUID_Invalid)
		{
			if (player)
			{
				LevelPlayerStates *lps=player->GetLPS();
				extern LevelPersistEntry_Agent *LPS_FindPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
				LevelPersistEntry_Agent *entry=LPS_FindPersistEntry_Agent(lps,_level->GetMapID(),guid);
				if (entry)
				{
					if (entry->szData>0)
					{
						CDataPacket dp;
						dp.SetDataBufferPointer(entry->data);

						dp.Data_ReadSimple(_dormants);

						_bPersistValid=TRUE;
					}
				}
			}
		}
	}
}

void CLoDormantSpawner::_SavePersist()
{
	LosDormantSpawner *los=(LosDormantSpawner*)_src;
	if (!los->bPersist)
		return;

	if (!_bPersistValid)
		return;

	CLevel *level=GetLevel();

	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);

	if (TRUE)
	{
		LevelGUID guid=_GetGUID();
		if (guid!=LevelGUID_Invalid)
		{
			if (player)
			{
				LevelPlayerStates *lps=player->GetLPS();
				extern LevelPersistEntry_Agent *LPS_QueryPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
				LevelPersistEntry_Agent *entry=LPS_QueryPersistEntry_Agent(lps,_level->GetMapID(),guid);
				if (entry)
				{
					CDataPacket dp;
					dp.SetDataBufferPointer(entry->data);

					dp.Data_WriteSimple(_dormants);

					entry->szData=dp.GetDataSize();
					entry->ver=0;
				}
			}
		}
	}
}

void CLoDormantSpawner::_UpdateBuffEntries()
{
	LosDormantSpawner *los=(LosDormantSpawner*)_src;
	if (!los->bPersist)
		return;


	BOOL bDirty=FALSE;
	int i=0;
	while(i<_nBuffEntries)
	{
		BuffEntry &e=_entriesBuff[i];
		if (e.buff)
		{
			if (e.buff->IsAlive())
			{
				i++;
				continue;
			}
		}

		SAFE_RELEASE(e.buff);
		BuffEntry eT=e;
		e=_entriesBuff[_nBuffEntries-1];
		_entriesBuff[_nBuffEntries-1]=eT;
		_nBuffEntries--;

		bDirty=TRUE;
	}

	if (bDirty||(!_bPersistValid))
	{
		int c=0;
		for (int i=0;i<_nBuffEntries;i++)
		{
			if (_entriesBuff[i].iSite>=c)
				c=_entriesBuff[i].iSite+1;
		}

		_dormants.resize(c);
		_dormants.resetAll();
		for (int i=0;i<_nBuffEntries;i++)
			_dormants.set(_entriesBuff[i].iSite);

		_bPersistValid=TRUE;

		_SavePersist();
	}
}


void CLoDormantSpawner::OnDeactivate()
{
	for (int i=0;i<_nBuffEntries;i++)
	{
		SAFE_RELEASE(_entriesBuff[i].buff);
	}
	_nBuffEntries=0;
}


BOOL CLoDormantSpawner::OnActivate()
{
	LosDormantSpawner *los=(LosDormantSpawner*)_src;
	LopDormantSpawner *lop=(LopDormantSpawner*)_param;
	CLevel *level=GetLevel();

	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);

	i_math::matrix43f *mats=&_src->GetMat();
	DWORD nMats=1;
	if (lop->sites.size())
	{
		mats=&lop->sites[0];
		nMats=lop->sites.size();
	}

	_LoadPersist();

	std::vector<WORD>indices;
	int c=0;

	if (!_bPersistValid)
	{
		c=CSysRandom::RandRangeInt(lop->nMin,lop->nMax+1);
		if (c>nMats)
			c=nMats;
		CSysRandom::GenRandomIndices(indices,nMats);
	}
	else
	{
		c=0;
		for (int i=0;i<_dormants.size();i++)
		{
			if (_dormants.test(i))
			{
				indices.push_back(i);
				c++;
			}
		}
	}

	if (c>DORMANTSPAWNER_MAXSITE)
		c=DORMANTSPAWNER_MAXSITE;

	_nBuffEntries=0;

	for (int i=0;i<c;i++)
	{
		if (indices[i]>=nMats)
			continue;
		if (level->GetRecords()->GetUnit(los->idUnit))
		{
			CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

			i_math::matrix43f &mat=mats[indices[i]];
			LevelPos pos;
			LevelFace face;
			if (TRUE)
			{
				i_math::xformf xfm;
				xfm.fromMatrix(mat);
				pos.x=mat.getTranslationP()->x;
				pos.y=mat.getTranslationP()->z;
				face=LevelFaceFromQuat(xfm.rot);
			}

			LevelGrade grd=lop->grdBase;
			if (TRUE)
			{
				AgentDistributeInfo *adi=NULL;
				extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
				adi=LevelUtil_FindADI(_level,los->GetRecID(),pos.x,pos.y);
				if (adi)
					grd=adi->grdRef;
			}

			grd=(LevelGrade)CSysRandom::RandVaryUInt((int)grd,lop->grdVary);

			lo->PostCreate(LevelPlayerID_Wild,NULL,los->idUnit,grd,NULL,EquipSetPick_None,pos,face);

			level->AddToActives(lo);

			BuffArg_Dormant arg;
			arg.nReviveSites=lop->sitesRevive.size();
			arg.sitesRevive=&lop->sitesRevive[0];
			LevelBuffID idBuff=level->GetDecider()->MakeBuff(lo,los->idDormantBuff,
				ANIMTICK_FROM_SECOND(1.0f),&arg,FALSE);//1.0为随便填写的值,这个Buff持续的时间会根据record里决定

			if (los->bPersist)
			{
				CLevelBuff *buff=lo->GetBuffs()->FindBuffByID(idBuff);
				if (buff)
				{
					if (_nBuffEntries<ARRAY_SIZE(_entriesBuff))
					{
						_entriesBuff[_nBuffEntries].buff=buff;
						_entriesBuff[_nBuffEntries].buff->AddRef();
						_entriesBuff[_nBuffEntries].iSite=indices[i];
						_nBuffEntries++;
					}
				}
			}

			SAFE_RELEASE(lo);
		}
	}

	return TRUE;
}


void CLoDormantSpawner::Update()
{
	_UpdateBuffEntries();
}
