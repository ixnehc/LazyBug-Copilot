
#include "stdh.h"

#include <unordered_map>

#include "LevelBasis.h"

#include "LevelRecords.h"

#include "LoNPCLoc.h"
#include "LoLoc.h"
#include "LoRoute.h"
#include "LoTeleportSite.h"

#include "Log/LogDump.h"

void CLevelBasis::Build(CLevelSources *srces,const char *pathSrces,CLevelRecords *records)
{
	_srces=srces;
	if (TRUE)
	{
		int sz=srces->_buf.size();
		for (int i=0;i<sz;i++)
		{
			CLevelObjSrc *src=srces->_buf[i].src;
			CLevelObjParam *param=srces->_buf[i].param;

			if (src)
			{
				if (src->IsDisable())
					continue;
			}

			if (param)
			{
				if (param->IsDisable())
					continue;
			}

			if (src->_idRec!=RecordID_Invalid)
				src->SetRecord(records->GetAgent(src->_idRec));

			//特殊处理
			if (src)
			{
				if (src->GetClass()->CheckName("LosStartLoc"))
				{
					LevelPos pos;
					pos.x=src->GetMat().getTranslationP()->x;
					pos.y=src->GetMat().getTranslationP()->z;
					LevelLoc cp;
					cp.pos=pos;
					cp.face=0.0f;

					_locs[StringID_Invalid]=cp;
					continue;
				}
			}
			if (param)
			{
				if (param->GetClass()->CheckName("LopLoc"))
				{
					LopLoc *lop=(LopLoc *)param;
					if (lop)
					{
						if (lop->nm!=StringID_Invalid)
						{
							LevelLoc loc;
							LevelPos pos;
							i_math::xformf xfm;
							xfm.fromMatrix(src->GetMat());

							pos.x=xfm.pos.x;
							pos.y=xfm.pos.z;
							loc.pos=pos;
							loc.face=LevelFaceFromQuat(xfm.rot);
							_locs[lop->nm]=loc;
						}
					}
					continue;
				}
			}
			if (param)
			{
				if (param->GetClass()->CheckName("LopRoute"))
				{
					LopRoute*lop=(LopRoute*)param;
					if (lop)
					{
						if (lop->nm!=StringID_Invalid)
						{
							LevelRoute *route=Class_New2(LevelRoute);
							route->nodes=lop->nodes;
							_routes[lop->nm]=route;
						}
					}
					continue;
				}
			}

			if (param&&src)
			{
				if (param->GetClass()->CheckName("LopTeleportSite"))
				{
					LopTeleportSite*lop=(LopTeleportSite*)param;
					LosTeleportSite *los=(LosTeleportSite *)src;
					if (lop)
					{
						if (lop->nm!=StringID_Invalid)
						{
							LevelTeleportSite*tpsite=Class_New2(LevelTeleportSite);
							tpsite->pos=LevelPos(los->GetMat().getTranslationP()->x,los->GetMat().getTranslationP()->z);
							tpsite->csites.AddRing(&lop->ring0[0],lop->ring0.size());
							tpsite->csites.AddRing(&lop->ring1[0],lop->ring1.size());
							tpsite->csites.AddRing(&lop->ring2[0],lop->ring2.size());
							tpsite->csites.AddRing(&lop->ring3[0],lop->ring3.size());

							_tpsites[lop->nm]=tpsite;
						}
					}
				}
			}
			if (src)
			{
				if (src->GetClass()->CheckName("LosNPCLoc"))
					continue;
			}

			if (src)
			{
				if (src->GetClass()->CheckName("LosSlate"))
					continue;
			}

			Src v;
			v.src=src;
			v.param=param;

			_buf.push_back(v);
		}
	}

	//初始化ChanceData
	_chancedata.Init();
	for (int i=0;i<_buf.size();i++)
	{
		if (_buf[i].param)
			_buf[i].param->RegisterChance(&_chancedata,records);
	}


	//一些检验
	if (TRUE)
	{
		if (_locs.find(StringID_Invalid)==_locs.end())
		{
			LOG_DUMP_1P("Level",Log_Error,"无法在地图\"%s\"里找到起始位置!",pathSrces);
		}
	}

	_csitesDef.Build(10,3.0f,2.0f);

	_basisSlates.Build(srces);

}

void CLevelBasis::Clear()
{
	_chancedata.Clear();
	_locs.clear();
	_csitesDef.Clear();

	if (TRUE)
	{
		std::unordered_map<StringID,LevelRoute*>::iterator it;
		for (it=_routes.begin();it!=_routes.end();it++)
		{
			Safe_Class_Delete((*it).second);
		}
		_routes.clear();
	}

	if (TRUE)
	{
		std::unordered_map<StringID,LevelTeleportSite*>::iterator it;
		for (it=_tpsites.begin();it!=_tpsites.end();it++)
		{
			Safe_Class_Delete((*it).second);
		}
		_tpsites.clear();
	}

	_basisSlates.Clear();

	_buf.clear();
	Zero();
}


LevelPos CLevelBasis::FindLocPos(StringID idCheckPoint)
{
	std::unordered_map<StringID,LevelLoc>::iterator it=_locs.find(idCheckPoint);

	if (it==_locs.end())
		return LevelPos_Invalid;

	return (*it).second.pos;
}

LevelLoc *CLevelBasis::FindLoc(StringID idCheckPoint)
{
	std::unordered_map<StringID,LevelLoc>::iterator it=_locs.find(idCheckPoint);

	if (it==_locs.end())
		return NULL;

	return &(*it).second;
}


LevelRoute *CLevelBasis::FindRoute(StringID nmRoute)
{
	std::unordered_map<StringID,LevelRoute *>::iterator it=_routes.find(nmRoute);
	if (it==_routes.end())
		return NULL;
	return (*it).second;
}

LevelTeleportSite*CLevelBasis::FindTeleportSite(StringID nm)
{
	std::unordered_map<StringID,LevelTeleportSite*>::iterator it=_tpsites.find(nm);
	if (it==_tpsites.end())
		return NULL;
	return (*it).second;
}


////////////////////////////////////////////////////////////////////////
//CWorldBasis
void CWorldBasis::Clear()
{
	_npcs.Clear();
}


void CWorldBasis::Build(CLevelSources **srcesAll,RecordID *idMaps,DWORD nSources,CLevelRecords *records)
{
	std::unordered_map<RecordID,int> lookupNPC;
	for (int k=0;k<nSources;k++)
	{
		CLevelSources *srces=srcesAll[k];
		int sz=srces->_buf.size();
		for (int i=0;i<sz;i++)
		{
			CLevelObjSrc *src=srces->_buf[i].src;
			CLevelObjParam *param=srces->_buf[i].param;

			if (src)
			{
				if (src->IsDisable())
					continue;
			}

			if (param)
			{
				if (param->IsDisable())
					continue;
			}

			if (src)
			{
				if (src->GetClass()->CheckName("LosNPCLoc"))
				{
					LosNPCLoc *los=(LosNPCLoc *)src;
					lookupNPC[los->idNPC]=-1;
					continue;
				}
			}
		}
	}

	if (TRUE)
	{
		_npcs.distribs.resize(lookupNPC.size());

		if (TRUE)
		{
			std::unordered_map<RecordID,int>::iterator it;
			DWORD idx=0;
			for (it=lookupNPC.begin();it!=lookupNPC.end();it++)
			{
				(*it).second=idx;
				_npcs.distribs[idx].idNPC=(*it).first;
				idx++;
			}
		}


		for (int k=0;k<nSources;k++)
		{
			CLevelSources *srces=srcesAll[k];
			int sz=srces->_buf.size();
			for (int i=0;i<sz;i++)
			{
				CLevelObjSrc *src=srces->_buf[i].src;
				if (src->GetClass()->CheckName("LosNPCLoc"))
				{
					LosNPCLoc *los=(LosNPCLoc *)src;

					std::unordered_map<RecordID,int>::iterator it=lookupNPC.find(los->idNPC);
					if (it!=lookupNPC.end())
					{
						DWORD idx=(*it).second;
						if (idx<_npcs.distribs.size())
						{
							NPCDistribute::Entry e;
							e.idMap=idMaps[k];
							e.loc=los;
							_npcs.distribs[idx].entries.push_back(e);
						}
					}
				}
			}
		}
	}

}

