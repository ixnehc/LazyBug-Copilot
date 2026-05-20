#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "LevelPlayerStates.h"

#include "LevelIDs.h"
#include "LevelHooks.h"

#include "LevelObj.h"
#include "LevelItem.h"
#include "LevelInactives.h"
#include "LevelAovMap.h"
#include "LevelTileMap.h"
#include "LevelSkills.h"
#include "unitmgr/UnitMgrNavMesh.h"
#include "unitmgr/Unit3DMgr.h"
#include "rvo2/RvoSimulator.h"
#include "LevelObjMap.h"
#include "LevelEventMap.h"

#include "LevelDecider.h"
#include "LevelChancer.h"

#include "LevelLockers.h"

#include "LevelPlayer.h"
#include "LevelNPCs.h"

#include "LevelAIs.h"

#include "LevelData.h"


#include "LevelMsgBuf.h"

#include "LevelBuff.h"

#include "LevelRelationMatrix.h"

#include "LevelResPiles.h"

#include "LevelDebugDraw.h"

struct NetMsgSC;
struct NetProxy
{
	virtual void SendMsg(LevelPlayerID idPlayer,NetMsgSC *msg)=0;
};

struct LevelTeleportQuest
{
	DEFINE_CLASS(LevelTeleportQuest);

	LevelTeleportQuest()
	{
		Zero();
	}

	void Zero()
	{
		bAccept=FALSE;
		loPlayer=NULL;
		tDoTeleport=ANIMTICK_INFINITE;
		idMap=RecordID_Invalid;
		idSite=StringID_Invalid;
		bSitePos=FALSE;
	}
	void Clear();

	BOOL bAccept;//客户端是否已经确认这次Teleport

	CLevelObj *loPlayer;
	LevelTick tDoTeleport;//在什么时候进行实际的Teleport

	RecordID idMap;
	StringID idSite;
	BOOL bSitePos;
	LevelPos posSite;
	

};

class CJjWorld;
class CLevelData;
class CLevelSheets;
class CLevelRecords;
class CLevelResources;
class CLevelDropper;
class CLevelBGs;
class CLevelBehavior;
struct LevelBehaviorContext;
class CBehaviorPersist;
struct PlayerMoveReply;
struct CBTalkOp;
struct MagicBoardInvoke;
class CLevel
{
public:
	DEFINE_CLASS(CLevel);

	CLevel()
	{
		Zero();
	}

	void Zero()
	{
		_world=NULL;

		_idMap=RecordID_Invalid;
		_secondServer=-1.0f;
		_t=0;
		_iSubFrame=0;
		_data=NULL;
		_records=NULL;
		_resources=NULL;
		_dropper=NULL;
		_bgs=NULL;
		_msgbuf=NULL;

		_nPlayers=0;

		_seedSkillID=1;
		_seedOpLinkID=1;

		_bRecordAffect=FALSE;

		memset(_uos,0,sizeof(_uos));

		_eoEnv=NULL;
		_seedEnvLichen=1;
		_seedEnvSpore=1;
	}

	void Create(CLevelData *data,RecordID idMap,CLevelRecords *records,CLevelResources *resources,CLevelDropper *dropper,CLevelBGs *bgs,CJjWorld *world);
	void Destroy();

	CJjWorld *GetWorld()	{		return _world;	}
	RecordID GetMapID()	{		return _idMap;	}
	i_math::recti &GetMapRect()	{		return _rc;	}

	const char*GetNavDataPath();
	CLevelRecords*GetRecords()	{		return _records;	}
	CLevelResources*GetResources()	{		return _resources;	}
	CLevelDropper*GetDropper()	{		return _dropper;	}
	CLevelBGs*GetBGs()	{		return _bgs;	}
	CLevelData *GetData()	{		return _data;	}

	ServerSecond GetServerSecond()	{		return _secondServer;	}
	LevelTick GetT_()	{		return _t;	}

	CLevelIDs *GetIDs()	{		return &_ids;	}
	CLevelHooks *GetHooks()	{		return &_hooks;	}

	CLevelAovMap *GetAovMap()	{		return &_aovmap;	}
	CLevelTileMap *GetTileMap()	{		return &_tilemap;	}
	CLevelSkills *GetSkills()	{		return &_skills;	}

	CLevelDecider*GetDecider()	{		return &_decider;	}

	CUnitMgrNavMesh *GetUnitMgr()	{		return &_unitmgr;	}
	CUnit3DMgr *GetUnit3DMgr()	{		return &_unit3dmgr;	}
	CLevelObjMap *GetObjMap()	{		return &_mpObj;	}
	CLevelEventMap *GetEventMap()	{		return &_mpEvent;	}

	RvoSimulator &GetRvoSim()	{		return _simRvo;	}

	CLevelLockers *GetLockers()	{		return &_lockers;	}

	GameTileMap*GetGtm()	{		return _data->GetGtm();	}
	CGameTrisMap*GetGtr()	{		return _data->GetGtr();	}
	CGameRgnGrids*GetRgnGrids()	{		return _data->GetRgnGrids();	}

	CLevelResPiles &GetResPiles()	{		return _pilesRes;	}

	CLevelDebugDraw &GetDbgDraw()	{		return _dbgdraw;	}

	LevelRelationMatrix *GetRelationMatrix();

	CLevelObj *CreateObj(ClassUID uid);//带一个引用计数
	CLevelObj *CreateObj(CLevelObjSrc *src,CLevelObjParam *param);//带一个引用计数
	CLevelObj *CreateObj(CClass *clss);//带一个引用计数

	CLevelItem *CreateItem(ClassUID uid);//带一个引用计数
	CLevelItem *CreateItem(CClass *clss);//带一个引用计数

	CLevelBehavior *CreateBehavior(StringID nmBG,LevelBehaviorContext &ctx);

	void AddToActives(CLevelObj *obj);

	void CreatePlayer(LevelPlayerID id,LevelPos&center,LevelPlayerStates *lps);
	void CreatePlayer(LevelPlayerID id,CLevelObj *lo,LevelPlayerStates *lps);
	void DestroyPlayer(LevelPlayerID id);
	CLevelPlayer *GetPlayer(LevelPlayerID id);
	LevelPlayerStates *GetLPS(LevelPlayerID id);
	LevelPlayerID *GetPlayerIDs(DWORD &c)	{		c=_nPlayers;		return _idPlayers;	}
	LevelPlayerMask GetPlayerMask()	{		return _maskPlayerID;	}

	CLevelNPCs *GetNPCs(LevelPlayerID id);
	void DestroyNPCs(LevelPlayerID id,RecordID idNPC);

// 	CLevelNPC *NPCFromLo(CLevelObj *lo)	{		return _npcs.NPCFromLo(lo);	}

	void AddTeleportQuest(LevelTeleportQuest *questTP);

	void SendNetMsg(LevelPlayerID idPlayer,NetMsgSC *msg);
	void SendNetMsg(NetMsgSC *msg);

	//处理网络消息的函数
	void HandlePlayerMove(LevelPlayerID id,ServerSecond t,PlayerMove &move,PlayerMoveReply &reply);
	BOOL HandlePlayerSkill(LevelPlayerID id,PlayerSkill &skill,LevelSkillArg *arg);
	BOOL HandlePlayerSkillCasted(ClientSkillID idSkill);
	BOOL HandlePlayerSkillCombine(ClientSkillID idSkill,LevelSkillTarget &target);
	BOOL HandlePlayerSkillStopCasting(LevelPlayerID id,ClientSkillID idSkill,AnimTick tCasting);
	BOOL HandleTalkOp(LevelPlayerID idPlayer,CBTalkOp &msg);
	BOOL HandleGatherItem(LevelPlayerID idPlayer,LevelObjID idItem);
	BOOL HandleGatherResPile(LevelPlayerID idPlayer,CBGatherResPile &msg);
	BOOL HandleInvokeMagicBoard(LevelPlayerID idPlayer,MagicBoardInvoke&invoke);
	BOOL HandleFlipSlate(LevelPlayerID idPlayer,LevelObjID idSlates,LevelSlateIdx idxSlate);
	BOOL HandleIncSlateButtonChip(LevelPlayerID idPlayer,LevelObjID idSlates,LevelSlateIdx idxSlate);

	BOOL CheckNeedUpdate(ServerSecond second,BOOL &bSubFrame);//判断是否需要更新,如果要更新的话,是否为一个SubFrame
	void UpdateFrame(ServerSecond second,CLevelMsgBuf *msgbuf,std::vector<CLevelObj *>&unsyncs);//注意unsyncs里返回指针有引用计数
	void UpdateSubFrame();

	//以下为一些测试用函数
	CLevelObj **GetActiveObjs(DWORD &sz)
	{
		sz=_actives.size();
		return _actives.data();
	}

	void EnumLo(LevelPos &center,float radius);
	CLevelObj **GetEnumLo(DWORD &c)
	{
		c=_enums.size();
		return _enums.data();
	}

	void BeginRecordAffects()	
	{		
		_bRecordAffect=TRUE;	
	}
	void EndRecordAffects()
	{
		_ClearAffects();
		_bRecordAffect=FALSE;

	}
	void AddAffect(CLevelObj *lo)
	{
		if (_bRecordAffect)
		{
			if (lo->IsAlive()&&(!lo->_bAffect))
			{
				_affects.push_back(lo);
				lo->_bAffect=1;
			}
		}
	}
	CLevelObj **GetAffects(DWORD &c)
	{
		c=_affects.size();
		return _affects.data();
	}

	LevelSkillID GenSkillID()
	{
		_seedSkillID++;
		if (_seedSkillID>60000)
			_seedSkillID=1;
		return _seedSkillID;
	}

	LevelOpLinkID_ GenOpLinkID()
	{
		_seedOpLinkID++;
		if (_seedOpLinkID>0xfff0)
			_seedOpLinkID=1;
		return _seedOpLinkID;
	}
	CLevelBuffIDPool* GetBuffIDPool()	{		return &_poolBuffID;	}

	void RegisterUniqueObj(LevelUniqueObjType tp,CLevelObj *lo);
	void UnregisterUniqueObj(LevelUniqueObjType tp,CLevelObj *lo);
	CLevelObj *GetUniqueObj(LevelUniqueObjType tp);

	void RegisterCentipede(CLevelObj *lo);
	void UnRegisterCentipede(CLevelObj *lo);
	CLevelObj *Get1stCentipede();

	void RegisterEoEnv(CLevelObj *eo);
	void UnRegisterEoEnv(CLevelObj *eo);
	CLevelObj *GetEoEnv()	{		return _eoEnv;	}
	DWORD _GenEnvLichenHandle()	{		return _seedEnvLichen++;	}
	DWORD _GenEnvSporeHandle()	{		return _seedEnvSpore++;	}
	DWORD _seedEnvLichen;
	DWORD _seedEnvSpore;

	void RegisterSubframeUpdate(CLevelObj *lo);

	CLevelService *ObtainService(LevelServiceType tp);
	CLevelService *GetService_(LevelServiceType tp);


protected:

	CLevelObj *_CreateObj(CLevelObj *lo);//带一个引用计数


	CJjWorld *_world;//属于哪个world

	RecordID _idMap;

	CLevelPlayer _players[LEVEL_MAX_PLAYER];
	void _RefreshPlayerIDs();
	LevelPlayerID _idPlayers[LEVEL_MAX_PLAYER];
	DWORD _nPlayers;
	void _RefreshPlayerIDMask();
	LevelPlayerMask _maskPlayerID;//Level里有哪些Player

	CLevelNPCs _npcs[LEVEL_MAX_PLAYER];

	CLevelMsgBuf*_msgbuf;

	CLevelHooks _hooks;

	std::vector<CLevelObj*> _actives;//所有激活的objs
	std::vector<CLevelObj*> _activesSubframe;//需要调用UpdateSubFrme的object

	CLevelInactives _inactives;//所有未激活的objs

	i_math::recti _rc;//地图的范围,以米为单位

	CLevelAovMap _aovmap;

	CLevelTileMap _tilemap;

	CLevelDecider _decider;

	CLevelSkills _skills;

	CUnitMgrNavMesh _unitmgr;
	CUnit3DMgr _unit3dmgr;

	RvoSimulator _simRvo;

	CLevelObjMap _mpObj;
	CLevelEventMap _mpEvent;

	CLevelChancer _chancer;

	CLevelAIs _ais;

	CLevelLockers _lockers;

	CLevelData* _data;
	CLevelRecords *_records;
	CLevelResources*_resources;
	CLevelDropper*_dropper;
	CLevelBGs *_bgs;

	CLevelIDs _ids;

	CLevelObj *_uos[LevelUniqueObj_Max];
	std::set<CLevelObj *> _loCentipedes;

	CLevelResPiles _pilesRes;

	CLevelObj *_eoEnv;

	CLevelDebugDraw _dbgdraw;

	//LevelServices
	std::unordered_map<LevelServiceType,CLevelService *> _services;

	ServerSecond _secondServer;//服务器时间
	//_t是这个level自己的时间,以0为基准
	LevelTick _t;
	DWORD _iSubFrame;

	LevelSkillID _seedSkillID;
	LevelOpLinkID_ _seedOpLinkID;
	CLevelBuffIDPool _poolBuffID;

	std::vector<LevelTeleportQuest*> _questsTP;

	std::vector<CLevelObj*> _enums;

	void _ClearAffects();
	BOOL _bRecordAffect;
	std::vector<CLevelObj*> _affects;//处理各种网络消息后,立即影响到的LevelObj(它们需要立即与客户端同步)

	std::deque<CLevelObj*> _destroys;

	friend class CJjWorld;

};

struct LevelPlayerStates;
class CWorldData;
class CJjWorld
{
public:
	struct PlayerEntry
	{
		PlayerEntry()
		{
			states=NULL;
			level=NULL;
		}
		void Clear()
		{
			states=NULL;
			level=NULL;
			pendingsNPC.Clear();
			emsToSend.clear();
			emsSent.clear();
		}

		LevelPlayerStates *states;
		CLevel *level;//在哪个level
		std::deque<RecordID>emsToSend;
		std::unordered_set<RecordID>emsSent;//记录所有已经发送给Client ExploreMap信息的地图
		std::vector<LevelAgentGuid> pendingAgentBrief;//需要更新给客户端AgentBrief的guid
		CNPCPendings pendingsNPC;
	};	
	CJjWorld()
	{
		Zero();
		_data=NULL;
	}
	void Zero()
	{
		_net=NULL;
		_data=NULL;
		_records=NULL;
		_resources=NULL;
		_dropper=NULL;
		_bgs=NULL;
		_seedLo=0;
		_idUnique=0;
	}
	void Create(NetProxy* net,CWorldData *data,CLevelRecords *records,CLevelResources *resources,CLevelDropper *dropper,CLevelBGs *bgs);
	void Destroy();

	LevelObjID NewLevelObjID()	{		_seedLo++;		return _seedLo;	}

	NetProxy *GetNetProxy()	{		return _net;	}

	CWorldData *GetData()	{		return _data;	}
	CLevelRecords *GetRecords()	{		return _records;	}

	LevelRelationMatrix *GetRelationMatrix()	{		return &_matRelation;	}

	LevelPlayerID EnterPlayer(LevelPlayerStates *lps);
	void StartDay();
	BOOL EndDay(BOOL bBreak);//bBreak表示这天是不是中断了(没有正常结束)
	void LeavePlayer(LevelPlayerID idPlayer);
	LevelPlayerID GetPlayerIDFromLPS(LevelPlayerStates *lps);

	BOOL ExistPlayer(LevelPlayerID idPlayer);
	CLevel *LevelFromPlayer(LevelPlayerID idPlayer);
	CLevelPlayer *LevelPlayerFromID(LevelPlayerID idPlayer);
	LevelPlayerStates *GetLPS(LevelPlayerID idPlayer);
	CNPCPendings *GetNPCPendings(LevelPlayerID idPlayer);
	void DestroyNPC(LevelPlayerID idPlayer,RecordID idNPC);

	DWORD GetUniqueID()	{		return _idUnique;	}

	void AcceptTeleport(LevelPlayerID idPlayer);
	void FlushTeleportQuest();

	CLevel *GetFirstLevel()
	{
		if (_levels2.size()<=0)
			return NULL;
		return _levels2[0];
	}

	CLevel *FindLevel(RecordID idMap);

	CLevel **GetLevels(DWORD &c)
	{
		c=_levels2.size();
		return _levels2.data();
	}

	RecordID FetchExploreMapToSend(LevelPlayerID idPlayer);
	void AddExploreMapToSend(LevelPlayerID idPlayer,RecordID idMap);
	BOOL IsExploreMapSent(LevelPlayerID idPlayer,RecordID idMap);
	void SetExploreMapSent(LevelPlayerID idPlayer,RecordID idMap);
	void AddAgentBriefEntryToSend(LevelPlayerID idPlayer,LevelAgentGuid &entry);
	void FetchPendingAgentBriefEntry(LevelPlayerID idPlayer,std::vector<LevelAgentGuid> &pendings);

	void RegisterTeleportTarget(LevelTeleportTarget target)
	{
		_targetsTeleport[target.tp]=target;
	}
	void GetTeleportTarget(LevelTeleportTarget::Type tp,LevelTeleportTarget &target)
	{
		target=_targetsTeleport[tp];
	}


protected:
	CLevel *_ObtainLevel(RecordID idMap);
	void _ClearAllLevel();

	NetProxy*_net;

	CWorldData *_data;
	CLevelRecords *_records;
	CLevelResources *_resources;
	CLevelDropper *_dropper;
	CLevelBGs *_bgs;

	std::unordered_map<RecordID,CLevel *>_levels;
	std::vector<CLevel*>_levels2;//用于快速遍历的buffer,永远与_levels同步

	DWORD _seedLo;

	PlayerEntry _players[LEVEL_MAX_PLAYER];

	LevelRelationMatrix _matRelation;

	LevelTeleportTarget _targetsTeleport[LevelTeleportTarget::MaxType];

	std::unordered_set<LevelGUID> _treasurepicks;

	DWORD _idUnique;//这个世界的独一无二的id

};