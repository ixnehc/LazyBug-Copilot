
#include "stdh.h"

#include "LevelService.h"

#include "LevelObj.h"
#include "LoUnit.h"
#include "Level.h"

const char *GetServiceName(LevelServiceType tp)
{
	if (tp==StringID_Invalid)
		return "";
	return StrLib_GetStr(tp);
}


void CLevelService::Init(CLevel *level)
{
	_level=level;
}

void CLevelService::Clear()
{
	_clients.clear();
	_servers.clear();
}

BOOL CLevelService::AddServer(LevelObjID idServer,DWORD nQuota,AnimTick tTimeOut)
{
	std::unordered_map<LevelObjID,ServerEntry>::iterator it=_servers.find(idServer);
	if (it!=_servers.end())
		return FALSE;

	ServerEntry e;
	e.nQuota=nQuota;
	e.nPreserved=0;
	e.tTimeOut=tTimeOut;
	_servers[idServer]=e;
	return TRUE;
}

BOOL CLevelService::AddUniqueServer(LevelObjID idServer,DWORD nQuota)
{
	_GarbageCollect();
	if (_servers.size()>0)
		return FALSE;//不Unique

	return AddServer(idServer,nQuota);
}

LevelObjID CLevelService::GetUniqueServer()
{
	_GarbageCollect();
	if (_servers.size()!=1)
		return LevelObjID_Invalid;
	std::unordered_map<LevelObjID,ServerEntry>::iterator it=_servers.begin();
	return (*it).first;
}

DWORD CLevelService::GetServerCount()
{
	return _servers.size();
}

void CLevelService::_RemoveServer(std::unordered_map<LevelObjID,ServerEntry>::iterator it)
{
	ServerEntry *e=&(*it).second;
	if (e->nPreserved>0)
	{
		std::unordered_map<LevelObjID,ClientEntry>::iterator it2;
		for (it2=_clients.begin();it2!=_clients.end();it2++)
		{
			ClientEntry *e2=&(*it2).second;
			if (e2->idServer==(*it).first)
				e2->idServer=LevelObjID_Invalid;
		}
	}

	_servers.erase(it);
}


void CLevelService::RemoveServer(LevelObjID idServer)
{
	std::unordered_map<LevelObjID,ServerEntry>::iterator it=_servers.find(idServer);
	if (it==_servers.end())
		return;

	_RemoveServer(it);
}

DWORD CLevelService::GetServerQuota(LevelObjID idServer)
{
	std::unordered_map<LevelObjID,ServerEntry>::iterator it=_servers.find(idServer);
	if (it==_servers.end())
		return 0;

	ServerEntry *e=&(*it).second;

	return e->nQuota;
}

LevelObjID CLevelService::Get1stPreserveClient(LevelObjID idServer)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it;
	for(it=_clients.begin();it!=_clients.end();it++)
	{
		ClientEntry *e=&(*it).second;
		if (e->bValid)
		{
			if (e->idServer==idServer)
				return (*it).first;
		}
	}
	return LevelObjID_Invalid;
}



BOOL CLevelService::AddClient(LevelObjID idClient)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it!=_clients.end())
	{
		ClientEntry *e=&(*it).second;
		e->bValid=TRUE;
		return TRUE;
	}

	ClientEntry e;
	e.bValid=TRUE;
	_clients[idClient]=e;
	return TRUE;
}

void CLevelService::_RemoveClient(std::unordered_map<LevelObjID,ClientEntry>::iterator it,BOOL bErase)
{
	LevelObjID idServer=(*it).second.idServer;
	if (idServer!=LevelObjID_Invalid)
	{
		std::unordered_map<LevelObjID,ServerEntry>::iterator it2=_servers.find(idServer);
		if (it2!=_servers.end())
		{
			ServerEntry *e=&(*it2).second;
			if (e->nPreserved>0)
				e->nPreserved--;
		}
	}

	if (bErase)
		_clients.erase(it);
	else
	{
		(*it).second.idServer=LevelObjID_Invalid;
		(*it).second.bValid=FALSE;
	}
}


void CLevelService::RemoveClient(LevelObjID idClient)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it==_clients.end())
		return;

	_RemoveClient(it,FALSE);
}



LevelObjID CLevelService::QueryClient(LevelObjID idClient)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it==_clients.end())
		return LevelObjID_Invalid;

	return (*it).second.idServer;
}

void CLevelService::Acknowledge(LevelObjID idClient)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it==_clients.end())
		return;

	LevelObjID idServer=(*it).second.idServer;
	if (idServer!=LevelObjID_Invalid)
	{
		std::unordered_map<LevelObjID,ServerEntry>::iterator it2=_servers.find(idServer);
		if (it2!=_servers.end())
		{
			ServerEntry *e=&(*it2).second;
			if (e->nPreserved>0)
				e->nPreserved--;
			if (e->nQuota)
				e->nQuota--;
		}
	}
	(*it).second.idServer=LevelObjID_Invalid;
	(*it).second.bValid=FALSE;
}

void CLevelService::SetClientCD(LevelObjID idClient,AnimTick durCD)
{
	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it==_clients.end())
		return;

	(*it).second.tCDEnd=_level->GetT_()+durCD;
}

extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
void CLevelService::_GarbageCollect()
{
	AnimTick tCur=_level->GetT_();

	if (TRUE)
	{
		std::unordered_map<LevelObjID,ServerEntry>::iterator it,itCur;
		it=_servers.begin();
		while(it!=_servers.end())
		{
			itCur=it;
			it++;

			if ((*itCur).second.tTimeOut<tCur)
			{//过期了
				if ((*itCur).second.nPreserved<=0)
				{
					_RemoveServer(itCur);
					continue;
				}
			}

			if ((*itCur).second.nQuota<=0)
			{//没有配额了
				_RemoveServer(itCur);
				continue;
			}
			LevelObjID idServer=(*itCur).first;
			CLevelObj *lo=_level->GetIDs()->LoFromID(idServer);
			if (!lo)
			{
				_RemoveServer(itCur);
				continue;
			}

			if (LevelUtil_CheckDead(lo))
			{
				_RemoveServer(itCur);
				continue;
			}
		}
	}

	if (TRUE)
	{
		std::unordered_map<LevelObjID,ClientEntry>::iterator it,itCur;
		it=_clients.begin();
		while(it!=_clients.end())
		{
			itCur=it;
			it++;

			LevelObjID idClient=(*itCur).first;
			CLevelObj *lo=_level->GetIDs()->LoFromID(idClient);
			if (!lo)
			{
				_RemoveClient(itCur,TRUE);
				continue;
			}

			if (LevelUtil_CheckDead(lo))
			{
				_RemoveClient(itCur,TRUE);
				continue;
			}
		}
	}
}


extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
void CLevelService::Update()
{
	_GarbageCollect();

	AnimTick tCur=_level->GetT_();
	std::unordered_map<LevelObjID,ServerEntry>::iterator it;
	for (it=_servers.begin();it!=_servers.end();it++)
	{
		LevelObjID idServer=(*it).first;
		CLevelObj *loServer=_level->GetIDs()->LoFromID(idServer);
		if (!loServer)
			continue;
		if (LevelUtil_CheckDead(loServer))
			continue;
		ServerEntry *eServer=&(*it).second;
		if (eServer->nQuota>eServer->nPreserved)
		{//有可以分配的配额

			//有多少剩余的配额
			DWORD nQuota=eServer->nQuota-eServer->nPreserved;

			//记录下所有需要分配的Clients
			float wts[64];
			std::unordered_map<LevelObjID,ClientEntry>::iterator itClient[64];
			DWORD nClients=0;
			if (TRUE)
			{
				std::unordered_map<LevelObjID,ClientEntry>::iterator it2;
				for (it2=_clients.begin();it2!=_clients.end();it2++)
				{
					ClientEntry *eClient=&(*it2).second;
					if (!eClient->bValid)
						continue;
					if (eClient->tCDEnd>tCur)
						continue;
					if (eClient->idServer==LevelObjID_Invalid)
					{
						if (nClients<ARRAY_SIZE(itClient))
						{
							CLevelObj *loClient=_level->GetIDs()->LoFromID((*it2).first);
							if (!loClient)
								continue;
							if (LevelUtil_CheckDead(loClient))
								continue;
							float wt=_CalcPriority(loServer,loClient);
							if (wt>0.0f)
							{
								wts[nClients]=wt;
								itClient[nClients]=it2;
								nClients++;
							}
						}
					}
				}
			}

			//按照权重优先分配
			while (nQuota>0)
			{
				float wtMax=0.0f;
				int idx=-1;

				for (int i=0;i<nClients;i++)
				{
					if (wts[i]>wtMax)
					{
						wtMax=wts[i];
						idx=i;
					}
				}

				if (idx==-1)
					break;

				//分配
				std::unordered_map<LevelObjID,ClientEntry>::iterator it2=itClient[idx];
				ClientEntry *eClient=&(*it2).second;
				eClient->idServer=idServer;
				eServer->nPreserved++;
				nQuota--;

				Swap(itClient[nClients-1],itClient[idx]);
				Swap(wts[nClients-1],wts[idx]);

				nClients--;
			}
		}
	}
}

LevelObjID CLevelService::FindClosestServer(LevelPos &pos,float radius,DWORD nMinAvailableQuota)
{
	_GarbageCollect();

	float dist2Min=100000.0f;
	LevelObjID idClosestServer=LevelObjID_Invalid;
	std::unordered_map<LevelObjID,ServerEntry>::iterator it;
	for (it=_servers.begin();it!=_servers.end();it++)
	{
		LevelObjID idServer=(*it).first;
		CLevelObj *loServer=_level->GetIDs()->LoFromID(idServer);
		if (!loServer)
			continue;
		ServerEntry *eServer=&(*it).second;
		if (eServer->nQuota>=eServer->nPreserved+nMinAvailableQuota)
		{
			CLevelObj *loServer=_level->GetIDs()->LoFromID(idServer);
			assert(loServer);

			LevelPos posServer=loServer->GetFramePos();
			float dist2=pos.getDistanceSQFrom(posServer);
			if (dist2<radius*radius)
			{
				if (dist2<dist2Min)
				{
					dist2Min=dist2;
					idClosestServer=idServer;
				}
			}
		}
	}

	return idClosestServer;
}

BOOL CLevelService::CanPreserve(LevelObjID idClient,LevelObjID idServer)
{
	_GarbageCollect();

	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it!=_clients.end())
	{
		ClientEntry *e=&(*it).second;
		if (e->idServer!=LevelObjID_Invalid)
		{
			if (e->idServer!=idServer)
				return FALSE;//已经占用了其它Server的额度
		}
	}

	std::unordered_map<LevelObjID,ServerEntry>::iterator itServer=_servers.find(idServer);
	if (itServer!=_servers.end())
	{
		ServerEntry *eServer=&(*itServer).second;
		if (eServer->nQuota<=eServer->nPreserved)
			return FALSE;//没有剩余的额度了
	}

	return TRUE;
}


BOOL CLevelService::Preserve(LevelObjID idClient,LevelObjID idServer)
{
	_GarbageCollect();

	if (FALSE==AddClient(idClient))
		return FALSE;

	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it!=_clients.end())
	{
		ClientEntry *e=&(*it).second;
		if (e->idServer!=LevelObjID_Invalid)
		{
			if (e->idServer==idServer)
				return TRUE;
			return FALSE;
		}

		std::unordered_map<LevelObjID,ServerEntry>::iterator itServer=_servers.find(idServer);
		if (itServer!=_servers.end())
		{
			ServerEntry *eServer=&(*itServer).second;
			if (eServer->nQuota>eServer->nPreserved)
			{
				eServer->nPreserved++;
				e->idServer=idServer;
				return TRUE;
			}
		}
	}

	return FALSE;
}

LevelObjID CLevelService::GetPreserve(LevelObjID idClient)
{
	_GarbageCollect();

	std::unordered_map<LevelObjID,ClientEntry>::iterator it=_clients.find(idClient);
	if (it!=_clients.end())
	{
		ClientEntry *e=&(*it).second;
		return e->idServer;
	}
	return LevelObjID_Invalid;
}


BOOL CLevelService::CheckPreserve(LevelObjID idClient,LevelObjID idServer)
{
	LevelObjID idPreserveServer=GetPreserve(idClient);
	if (idPreserveServer==idServer)
		return TRUE;

	return FALSE;
}

BOOL CLevelService::CheckPreserve(LevelObjID idClient)
{
	LevelObjID idPreserveServer=GetPreserve(idClient);
	if (idPreserveServer!=LevelObjID_Invalid)
		return TRUE;

	return FALSE;
}

float CLevelService::_CalcPriority(CLevelObj *loServer,CLevelObj *loClient)
{
	if (_methodPriority==ClientPriority_Equal)
		return 1.0f;

	if (_methodPriority==ClientPriority_Closer)
	{
		float dist=loServer->GetFramePos().getDistanceFrom(loClient->GetFramePos());
		return i_math::clampup_f(100.0f-dist,0.0f);
	}

	return 1.0f;
}




////////////////////////////////////////////////////////////////////////
//CLevelServiceCureHP

float CLevelServiceCureHP::_CalcPriority(CLevelObj *loServer,CLevelObj *loClient)
{
	if (loClient->GetType()==LevelObjType_Unit)
	{
		LevelAttr_Base *attr=((CLoUnit *)loClient)->GetAttr_Base();
		float rate=attr->hp.GetRatio();
		if (rate<0.33f)
		{
			float dist2=loServer->GetFramePos().getDistanceSQFrom(loClient->GetFramePos());
			if (dist2<10.0f*10.0f)
				return 1.0f-rate;
		}
	}
	return 0.0f;
}
