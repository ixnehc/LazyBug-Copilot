#pragma once

#include <unordered_set>

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"

#include "math/circle.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#define CLASSUID_StarPlate 72

typedef short StarPlateSiteIndex;
#define StarPlateSiteIndex_Invalid (-1)


struct LopStarPlate:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopStarPlate,CLASSUID_StarPlate);

	BEGIN_GOBJ_PURE(LopStarPlate,1);

	GELEM_VARVECTOR(i_math::spheref,center);GELEM_UID(1);
		GELEM_EDITVAR("中心区域",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"中心区域");
		GELEM_VARVECTOR(i_math::vector3df,links);GELEM_UID(2);
			GELEM_EDITVAR("衔接点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"衔接点");
		GELEM_VARVECTOR(i_math::matrix43f,sites);GELEM_UID(3);
			GELEM_EDITVAR("开始点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"开始点");

	END_GOBJ();

	std::vector<i_math::spheref> center;
	std::vector<i_math::vector3df> links;
	std::vector<i_math::matrix43f> sites;


};

struct LosStarPlate:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosStarPlate,CLASSUID_StarPlate);

	BEGIN_GOBJ_PURE(LosStarPlate,1);

		GELEM_AGENTRECORD();


	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};

struct StarPlateSetting;

class CLoStarPlate:public CLoAgent
{
public:
	CLoStarPlate()
	{
		_tStart=ANIMTICK_INFINITE;

		_idObelisk=LevelObjID_Invalid;

		_bStateSyncDirty=FALSE;
		_bObeliskSyncDirty=FALSE;
		_bHilightsSyncDirty=FALSE;

		_bKingKilled=FALSE;

		ResetState();
	}
	DEFINE_LEVELOBJ_CLASS(CLoStarPlate,CLASSUID_StarPlate);

	virtual const char *GetShowName()	{		return "星盘";	}

	void Clear();

	void OnDestroy() override;

	void ResetState();


	BOOL OnActivate() override;
	void Update() override;

	BOOL IsServerOnly()override	{		return FALSE;	}

	BOOL CheckFullActivated();
	BOOL CanActivateSite(LevelObjID idSite);
	void ActivateSite(LevelObjID idSite);
	BOOL CheckAnySiteActivated();
	BOOL CheckSiteActivated(LevelObjID idSite);
	BOOL CheckNeedSpawnEnemy();
	void NotifyEnemySpawned();
	void NotifySacredOrbFire();
	BOOL CheckSacredOrbCharged();

	LevelObjID FindClosestSite(LevelPos pos);
	BOOL GetNextSitePos(LevelObjID idSite,LevelPos &posNext);
	BOOL GetNextSitePosToActivate(LevelPos &pos);

	BOOL GetCenterCircle(i_math::circlef &center);

public:

	void _LoadPersistS(LevelPlayerID idPlayer);
	void _SavePersistS(LevelPlayerID idPlayer);

	struct Link
	{
		int idxLink;
	};

	struct Chain
	{
		BOOL Exist(int idxLink)
		{
			for (int i=0;i<links.size();i++)
			{
				if (links[i].idxLink==idxLink)
					return TRUE;
			}
			return FALSE;
		}
		void Clear()
		{
			links.clear();
		}
		std::vector<Link> links;
	};

	BOOL _IsChainsBuilt();
	void _BuildChains();
	BOOL _BuildChain(Chain &chain,LevelPos &posStart,LevelPos &posEnd,int levelConstraint);
	std::vector<Chain> _chains;

	StarPlateSetting &_GetSetting();

	void _UpdateSacredOrb();

	void _UpdateEnemy();
	int _GetEnemyToSpawn();
	BOOL _CheckAllEnemyEliminated();

	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void _OnPostWriteSync() override;

	BOOL _GetPlayerPos(LevelPos &pos);

	void _BuildLookup();
	StarPlateSiteIndex _FindSite(LevelObjID id);
	BOOL _CheckSiteActivated(StarPlateSiteIndex idxSite);
	std::unordered_map<LevelObjID,StarPlateSiteIndex> _lookupSites;
	std::vector<LevelObjID> _lookupSites2;
	LevelObjID _idObelisk;

	AnimTick _tStart;

	void _UpdateKillingKing();
	BOOL _bKingKilled;

	//State
	void _UpdateActive();
	StarPlateSiteIndex _iStartSite;
	StarPlateSiteIndex _iActiveSite;
	AnimTick _tSiteActivated;
	short _iNextLink;
	BOOL _bStateSyncDirty;

	//Obelisk states
	WORD _flagsObeliskStars;
	WORD _flagsObeliskBulbs;
	BYTE _nEnemySpawned;
	BYTE _nEnemyEliminated;
	BOOL _bObeliskSyncDirty;


	//Hilights
	void _UpdateHilights();
	std::unordered_set<short>_hilightsLink;
	BOOL _bHilightsSyncDirty;


};
