#pragma once

#include <unordered_set>
#include <deque>

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"

#include "math/circle.h"
#include "math/sphere.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "EelRoadNetwork.h"

#include "Random/Random.h"

#define CLASSUID_MagicCircuit 75

typedef WORD MagicCuicuitTailOrbID;
#define MagicCuicuitTailOrbID_Invalid (65535)

#define MAGICCIRCUIT_MAX_TAILORB_COUNT (4)
#define MAGICCIRCUIT_TAILORBS_REACHING_DURATION (ANIMTICK_FROM_SECOND(1.0f))
#define MAGICCIRCUIT_TAILORB_MAX_FALL_DURATION (ANIMTICK_FROM_SECOND(1.0f))

#define IDX_CRYSTALTARGET (3)
#define IDX_RELAYBIRDHOME (2)
#define IDX_TAILORBSHOME (4)
#define IDX_SPOTHOME (1)
#define IDX_SWITCHHOME (0)


class CMagicCircuitTail
{
public:
	CMagicCircuitTail()
	{
	}

	void Clear();
	void UpdateHead(i_math::vector3df &pos);
	BOOL Sample(float dist, i_math::vector3df &pos,i_math::vector3df *normal);
	BOOL Sample(int idx, i_math::vector3df &pos,i_math::vector3df *normal);

public:
	struct Node
	{
		i_math::vector3df pos;
		i_math::vector3df dir;//到上一个Node到这个Node的方向
		float length;//到上一个Node的距离
	};
	std::deque<Node> _nodes;
	
};


struct LopMagicCircuit:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopMagicCircuit,CLASSUID_MagicCircuit);

	BEGIN_GOBJ_PURE(LopMagicCircuit,1);

		GELEM_VARVECTOR(i_math::vector3df,pipes);GELEM_UID(1);
			GELEM_EDITVAR("管道点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"管道点");
		GELEM_VARVECTOR(i_math::vector3df,relays);GELEM_UID(2);
			GELEM_EDITVAR("中继点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"中继点");
		GELEM_VARVECTOR(i_math::vector3df,bollards);GELEM_UID(9);
			GELEM_EDITVAR("栓柱点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"栓柱点");
		GELEM_VARVECTOR(i_math::vector3df,networksEel);GELEM_UID(10);
			GELEM_EDITVAR("Eel的网络连线",GVT_Fx3,GSem(GSem_Unknown,"MatSet,Route"),"Eel的网络连线");
		GELEM_VARVECTOR(i_math::vector3df,crystals);GELEM_UID(3);
			GELEM_EDITVAR("水晶点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"水晶点");
		GELEM_VARVECTOR(i_math::vector3df,perches);GELEM_UID(4);
			GELEM_EDITVAR("栖息点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"栖息点");
		GELEM_VARVECTOR(i_math::vector3df,focuses);GELEM_UID(5);
			GELEM_EDITVAR("聚焦点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"聚焦点");
		GELEM_VARVECTOR(i_math::vector3df,centerFocus);GELEM_UID(6);
			GELEM_EDITVAR("聚焦点中心",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"聚焦点中心");
		GELEM_VARVECTOR(i_math::vector3df,centerTailOrbSlots);GELEM_UID(7);
			GELEM_EDITVAR("TailOrbSlots中心",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"TailOrbSlots中心");
		GELEM_VARVECTOR(i_math::spheref,spots);GELEM_UID(8);
			GELEM_EDITVAR("Spot点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"Spot点");
		GELEM_VARVECTOR(i_math::spheref, startsTeleport);GELEM_UID(11);
			GELEM_EDITVAR("传送起始点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"传送起始点");
		GELEM_VARVECTOR(i_math::vector3df, endsTeleport);GELEM_UID(12);
			GELEM_EDITVAR("传送终止点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"传送终止点");
		GELEM_VARVECTOR(i_math::spheref, railguardsSpawn);GELEM_UID(14);
			GELEM_EDITVAR("RailGuards出生区域",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"RailGuards出生区域");
		GELEM_VARVECTOR(i_math::spheref, railguardsTeleport);GELEM_UID(13);
			GELEM_EDITVAR("RailGuards活动区域",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"RailGuards活动区域");
		GELEM_VARVECTOR(i_math::vector3df, rail);GELEM_UID(15);
			GELEM_EDITVAR("Rail点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"Rail点");

		//Next GElem UID: 16
	END_GOBJ();

	std::vector<i_math::vector3df> relays;
	std::vector<i_math::vector3df> bollards;
	std::vector<i_math::vector3df> networksEel;
	std::vector<i_math::vector3df> pipes;
	std::vector<i_math::vector3df> crystals;
	std::vector<i_math::vector3df> perches;
	std::vector<i_math::vector3df> focuses;
	std::vector<i_math::vector3df> centerFocus;
	std::vector<i_math::vector3df> centerTailOrbSlots;
	std::vector<i_math::spheref> spots;
	std::vector<i_math::spheref> startsTeleport;
	std::vector<i_math::vector3df> endsTeleport;
	std::vector<i_math::spheref> railguardsTeleport;
	std::vector<i_math::spheref> railguardsSpawn;
	std::vector<i_math::vector3df> rail;
};

struct LosMagicCircuit:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosMagicCircuit,CLASSUID_MagicCircuit);

	BEGIN_GOBJ_PURE(LosMagicCircuit,1);

		GELEM_AGENTRECORD();


	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};

struct MagicCircuitSetting;

class CLoMagicCircuit:public CLoAgent
{
public:
	CLoMagicCircuit()
	{
		Zero();
	}
	DEFINE_LEVELOBJ_CLASS(CLoMagicCircuit,CLASSUID_MagicCircuit);

	virtual const char *GetShowName()	{		return "魔能回路";	}

	void Zero();
	void Clear();

	void OnDestroy() override;



	BOOL OnActivate() override;
	void Update() override;

	BOOL IsServerOnly()override	{		return FALSE;	}

	void ActivateRelay(LevelObjID idRelay,BOOL bActivate);

	BOOL CanActivateFocus();
	void ActivateFocus();
	void CommitFocus();
	BOOL CheckFocus()	{		return _idxFocus>=0;	}

	LevelObjID FindTargetRelayForEel(LevelPos posEel);
	LevelObjID FindEelNetworkNodeID(i_math::vector2df& pos);
	CEelRoadNetwork& GetEelRoadNetwork();

	BOOL CanSpawnCrystal();
	LevelObjID SpawnCrystal();
	LevelObjID GetCrystalTarget();

	LevelObjID SpawnRelayBird();
	BOOL GetRelayBirdTargetPos(LevelPos& posRelayBird,LevelPos &posTarget);
	BOOL CheckRelayBirdAtHome(LevelPos& posRelayBird);
	BOOL CheckRelayBirdAtRest(LevelPos& posRelayBird);

	LevelObjID GetTailOrbsHome();
	BOOL CheckConnected_TailOrbsHome();
	BOOL CanTailOrbsReach();
	void StartTailOrbsReach();
	BOOL CheckTailOrbsReached();

	void SpawnRailGuards();
	void DespawnRailGuards();

	LevelPos3D FindClosestRailPoint(i_math::vector3df& pos);

public:

	struct RelayInfo
	{
		RelayInfo()
		{
			memset(this,0,sizeof(*this));
		}
		LevelObjID idRelay;
	};

	MagicCircuitSetting &_GetSetting();

	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void _OnPostWriteSync() override;

	void _BuildRelayInfo();
	std::vector<RelayInfo> _infosRelay;

	void _BuildBollardInfo();
	std::vector<LevelObjID> _infosBollard;

	//RoadNetwork for eel
	void _BuildEelNetwork();
	CEelRoadNetwork _networkEel;

	AnimTick _tStart;

	CLevelBehavior *_bhv;

	//Relay states
	BOOL _CheckRelayConnected(int idxRelay);
	void _UpdateRelayHits();
	WORD _activesRelay;
	BOOL _bRelayActiveSyncDirty;
	WORD _hitsRelay;
	BOOL _bRelayHitSyncDirty;


	//Spots states
	void _UpdateSpotsState();
	struct SpotsState
	{
		SpotsState()
		{
			Zero();
		}
		void Zero()
		{
			memset(this,0,sizeof(*this));
		}

		BOOL IsAllOff()		{			return states[0]==0&&states[1]==0&&states[2]==0&&states[3]==0;		}
		BOOL IsAllOn()		{			return states[0]==2&&states[1]==2&&states[2]==2&&states[3]==2;		}
		BOOL Equals(SpotsState &other)
		{
			return states[0]==other.states[0]&&states[1]==other.states[1]&&states[2]==other.states[2]&&states[3]==other.states[3];
		}
		BYTE states[4];//0: off, 1: flash, 2:on
	};
	SpotsState _statesSpot;
	BOOL _bSpotsSyncDirty;


	//Focus states
	enum FocusState
	{
		Focus_None,
		Focus_Activated,
		Focus_Hit,
	};
	FocusState _stateFocus;
	short _idxFocus;
	LevelTick _tFocusStateStart;
	MagicCuicuitTailOrbID _idTailOrbOfFocus;
	BOOL _bFocusSyncDirty;

	//TailOrbs states
	struct TailOrb
	{
		MagicCuicuitTailOrbID id;
		LevelTick tBirth;
	};
	void _SpawnTailOrb(MagicCuicuitTailOrbID id);
	MagicCuicuitTailOrbID _GenTailOrbID();
	int _GetMaxTailOrbsCount();
	void _UpdateTail();
	CMagicCircuitTail _tail;
	std::vector<TailOrb> _tailorbs;
	LevelTick _tTailOrbsReachingStart;
	BOOL _bTailOrbsSyncDirty;

	//Teleport
	void _UpdateTeleport();

	//Rail guards
	std::vector<LevelObjID> _idsRailGuards;


};
