/********************************************************************
	created:	2012/11/26 
	author:		cxi
	
	purpose:	NPC
*********************************************************************/
#include "stdh.h"

#include "LevelBasis.h"

#include "Level.h"
#include "LevelNPCs.h"

#include "LevelRecords.h"

#include "LevelRecordNPC.h"

#include "Random/Random.h"

#include "LoNPCLoc.h"



////////////////////////////////////////////////////////////////////////
//CWorldPlayerNPCs

CNPCPendings::Pendings *CNPCPendings::_ObtainMapPendings(RecordID idMap)
{
	std::unordered_map<RecordID,Pendings>::iterator it=_mps.find(idMap);
	if (it!=_mps.end())
		return &(*it).second;

	Pendings *p=&_mps[idMap];
	return p;
}

CNPCPendings::Pendings *CNPCPendings::_GetMapPendings(RecordID idMap)
{
	std::unordered_map<RecordID,Pendings>::iterator it=_mps.find(idMap);
	if (it!=_mps.end())
		return &(*it).second;

	return NULL;
}

void CNPCPendings::_EraseMapPendings(RecordID idMap)
{
	std::unordered_map<RecordID,Pendings>::iterator it=_mps.find(idMap);
	if (it!=_mps.end())
		_mps.erase(it);
}

void CNPCPendings::ClearRtnuPendings()
{
	_rtnus.buf.clear();
}



void CNPCPendings::Init(CJjWorld *world,LevelPlayerID idPlayer)
{
	Clear();

	_world=world;
	_idPlayer=idPlayer;
	_lps=_world->GetLPS(_idPlayer);

	NPCBasis *basis=NULL;
	if (TRUE)
	{
		CWorldData *data=world->GetData();
		if (data)
		{
			CWorldBasis *basisWorld=data->GetBasis();
			if (basisWorld)
				basis=basisWorld->GetNPCBasis();
		}
	}

	CLevelRecords *records=world->GetRecords();

	if (basis)
	{
		for (int i=0;i<basis->distribs.size();i++)
		{
			NPCDistribute*distrib=&basis->distribs[i];

			LevelRecordNPC *rec=records->GetNPC(distrib->idNPC);
			if (!rec)
				continue;

			if (rec->bFreePos)
			{//这个NPC的位置有可能特殊指定,或者有可能是随从,我们需要得到这个NPC的状态,来决定在哪里创建它
				if (_lps)
				{
					LPSNpcData *dataNPC=LPS_FindNpcData(_lps,distrib->idNPC);

					if (dataNPC)
					{
						if (dataNPC->state==NPCState_Default)
						{//不是随从
							if (dataNPC->idMap!=RecordID_Invalid)
							{//这个NPC指定了一个出现位置,我们使用这个位置来创建这个NPC
								Pendings *pendingMP=_ObtainMapPendings(dataNPC->idMap);
								assert(pendingMP);
								Pending pending;
								pending.idNPC=distrib->idNPC;
								pending.pos=dataNPC->pos;
								pending.dataNPC=dataNPC;
								pendingMP->buf.push_back(pending);
								continue;
							}
						}
						else
						{//是随从,我们作为随从NPC来创建
							Pending pending;
							pending.idNPC=distrib->idNPC;
							pending.dataNPC=dataNPC;
							_rtnus.buf.push_back(pending);
							continue;
						}
					}
				}
			}

			for (int i=0;i<distrib->entries.size();i++)
			{
				NPCDistribute::Entry *e=&distrib->entries[i];

				Pendings *pendingMP=_ObtainMapPendings(e->idMap);
				assert(pendingMP);

				Pending pending;
				pending.idNPC=distrib->idNPC;
				assert(e->loc);
				pending.pos=e->loc->GetPos();
				pending.dataNPC=NULL;
				pendingMP->buf.push_back(pending);
			}
			
		}
	}

	//对于同一个地图里面,某个npc有多个分布位置的情况,要在多个位置中随机选择一个
	if (TRUE)
	{
		std::unordered_map<RecordID,int> counts;
		std::unordered_map<RecordID,int> selects;

		std::unordered_map<RecordID,Pendings>::iterator it;
		for (it=_mps.begin();it!=_mps.end();it++)
		{
			Pendings &pendings=(*it).second;

			counts.clear();
			selects.clear();

			for (int i=0;i<pendings.buf.size();i++)
			{
				Pending& pending=pendings.buf[i];
				if (counts.find(pending.idNPC)==counts.end())
					counts[pending.idNPC]=0;
				else
					counts[pending.idNPC]++;
			}

			if (TRUE)
			{
				selects.swap(counts);
				std::unordered_map<RecordID,int>::iterator it;
				for (it=selects.begin();it!=selects.end();it++)
					(*it).second=CSysRandom::RandRangeInt(0,(*it).second);
			}

			//标记未选中的为待删除
			if (TRUE)
			{
				std::unordered_map<RecordID,int>::iterator it;
				for (it=selects.begin();it!=selects.end();it++)
				{
					RecordID idNPC=(*it).first;
					int iSel=(*it).second;

					int idx=0;
					for (int i=0;i<pendings.buf.size();i++)
					{
						Pending& pending=pendings.buf[i];
						if (pending.idNPC==idNPC)
						{
							if(idx!=iSel)
								pending.idNPC=RecordID_Invalid;//标记为待删除
							idx++;
						}
					}
				}
			}

			if(TRUE)
			{
				int c=0;
				for (int i=0;i<pendings.buf.size();i++)
				{
					if (pendings.buf[i].idNPC==RecordID_Invalid)
						continue;
					pendings.buf[c]=pendings.buf[i];
					c++;
				}
				pendings.buf.resize(c);
			}
		}

	}
}

void CNPCPendings::Clear()
{
	_mps.clear();
	_rtnus.buf.clear();
	Zero();
}

