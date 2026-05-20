#pragma once

#include "LoSlates.h"
#include "LevelSlateDefinesA.h"

#include "BgnGA_RollAwards.h"

#define CLASSUID_SlatesA 42

struct SlatesRandomPickEntryA:public SlatesRandomPickEntry
{
    BEGIN_GOBJ_PURE_UID(SlatesRandomPickEntryA,1);
		GELEM_VAR_INIT(LevelSlateType,tp,LevelSlateTypeA_Blank);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeA),"类型");
		GELEM_VAR_INIT(int,count,1);
			GELEM_EDITVAR("设置几个",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"设置几个");
		GELEM_VARVECTOR_INIT(LevelSlateType,tpsTarget,LevelSlateTypeA_Blank);
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeA),"类型");
    END_GOBJ();    
};



struct LopSlatesA:public LopSlates
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSlatesA,CLASSUID_SlatesA);

	BEGIN_GOBJ_PURE(LopSlatesA,1);	

		GELEM_VARVECTOR(LevelGUID,includes)
			GELEM_EDITVAR("包含石板",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"石板迷宫包含哪些石板");

		GELEM_OBJ(SlateSpaceDefine,space);
			GELEM_EDITOBJ("坐标系","坐标系");

		GELEM_OBJVECTOR(SlateGrpEntry,grps);
			GELEM_EDITOBJ("石板组","石板组");

		GELEM_VAR_INIT( StringID,bhvSetup,StringID_Invalid);	
			GELEM_EDITVAR( "设置过程图", GVT_U, GSem(GSem_StringID,"行为图名称"), "设置石板迷宫的行为图的名称" );

		GELEM_VARVECTOR(i_math::matrix43f,sitesTeleLeave)
			GELEM_EDITVAR("传送离开位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"传送离开位点");

		GELEM_OBJ(RollAwardParam,paramAwardTB);
			GELEM_EDITOBJ("宝箱奖励参数","宝箱奖励参数");

	END_GOBJ();

	std::vector<i_math::matrix43f> sitesTeleLeave;
	RollAwardParam paramAwardTB;

};

struct LosSlatesA:public LosSlates
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSlatesA,CLASSUID_SlatesA);

	BEGIN_GOBJ_PURE(LosSlatesA,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

	END_GOBJ();



};

struct LevelSlateInfo;
class CLevelSlateA
{
public:
	CLevelSlateA()
	{
		_info=NULL;
		_bSyncDirty=FALSE;
	}
	void Init(LevelSlateInfo *info);

	LevelPos &GetPos()	{		return _pos;	}

	BOOL CheckIn(LevelPos &pos);

	BOOL CheckCoverLocked();

	void SetSyncDirty()	{		_bSyncDirty=TRUE;	}
	void ResetSyncDirty()	{		_bSyncDirty=FALSE;	}

protected:
	LevelSlateA_Status _status;
	LevelSlateA_CoverStatus _statusCover;

	BOOL _bSyncDirty;

	LevelSlateInfo *_info;
	LevelPos _pos;

friend class CLoSlatesA;
};


struct LevelSlatesInfo;
class CLevelPlayer;
class CLoSlatesA:public CLoAgent
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoSlatesA,CLASSUID_SlatesA);

	CLoSlatesA()
	{
		Zero();
		_ver=_verSync=0;
	}
	void Zero()
	{
		_basis=NULL;
		_info=NULL;

		_reached=LevelSlateIdx_Invalid;
		_state=LevelSlatesState_None;

		_bhvProcess=NULL;
		_bInProcess=FALSE;

		_enter=LevelSlateIdx_Invalid;

		_tUpdate=0;
		_countTeleportCD=0;

		_nStars=0;
	}

	void Clear();

	virtual const char *GetShowName()	{		return "石板迷宫A";	}

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual void Update();
	virtual void HandleHook(LevelHook &hk);

	virtual CLevelSensor*GetSensor()	override{		return &_sensor;}
	virtual CLevelSkillDriver *GetSkillDriver() override {return &_skilldriver;}

	CLevelObj *GetThreat();

	void Setup_SetEntrance(StringID grp);
	void Setup_SetExit(StringID grp);
	void Setup_SetType(StringID grp,LevelSlateType tp,std::vector<LevelSlateIdx> *result);
	void Setup_SetType_RandomPick(StringID grp,SlatesRandomPickEntryA *entries,DWORD c,std::vector<LevelSlateIdx> *result);
	void Setup_SetMatchKey(std::vector<LevelSlateIdx> &indicesSlates,int matchkey);
	void Setup_SetEdgeLock(std::vector<LevelSlateIdx> &indicesSlates);
	void Setup_SetSwitch(std::vector<LevelSlateIdx> &indicesLocks,std::vector<LevelSlateIdx> &indicesSwitches,LevelSlateA_SwitchChannel channel);
	void Setup_SetSwitchPointer(std::vector<LevelSlateIdx> &indicesSwitchPointers,LevelSlateA_SwitchChannel channel);
	void Setup_SetButton(std::vector<LevelSlateIdx> &indicesLocks,std::vector<LevelSlateIdx> &indicesButtons,LevelSlateA_ButtonChannel channel);

	void FinishProcess()	{		_bInProcess=FALSE;	}
	BOOL CheckProcessed(LevelSlateIdx idxSlate);
	void RevealNearBy(LevelSlateIdx idxSlate,int radius);
	void RevealAll(LevelSlateIdx idxSlate);
	LevelSlateIdx FindTeleportTarget(LevelSlateIdx idxSlate);
	BOOL GetTeleLeavePos(LevelPos &pos);
	BOOL GetSlatePos(LevelSlateIdx idxSlate,LevelPos &pos);
	BOOL SpawnAgent(LevelSlateIdx idxSlate, RecordID idAgent,BOOL bFaceToReachedSlate);
	void OpenFenceWithSwitch(LevelSlateIdx idxSwitchSlate);
	BOOL CheckInTeleportCD()	{		return _countTeleportCD>0;	}
	void StartTeleportCD()	{		_countTeleportCD=2;	}
	int GetStarCount()	{		return _nStars;	}
	void ModRes(int mod);

	RollAwardParam *GetRollAwardParam();


	void RequestFlipFromClient(LevelPlayerID idPlayer,LevelSlateIdx idxSlate);
	void RequestIncSlateButtonChip(LevelPlayerID idPlayer,LevelSlateIdx idxSlate);

	LevelSlateIdx FindEmbedID(LevelObjID id);

protected:

	CLevelPlayer *_GetCurPlayer();

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	void _WriteInitials(CBitPacket *bp);
	void _WriteStates(CBitPacket *bp);
	void _WriteEmbeds(CBitPacket *bp);

	BOOL _CheckValidSlateIdx(LevelSlateIdx idxSlate)
	{
		if ((idxSlate>=_info->iStartSlate))
		{
			if(idxSlate-_info->iStartSlate<_bufSlate.size())
				return TRUE;
		}
		return FALSE;
	}

	CLevelSlateA &_GetSlate(LevelSlateIdx idxSlate)
	{
		assert(_CheckValidSlateIdx(idxSlate));
		return _bufSlate[idxSlate-_info->iStartSlate];
	}

	LevelSlatesGrpHandle _FindGrp(StringID nmGrp);
	CLevelSlateA &_GetGrpSlate(LevelSlatesGrpHandle hGrp,int idx)
	{
		assert((DWORD)idx<hGrp.count);
		assert(_info);
		assert(((DWORD)hGrp.iStart)+(DWORD)idx<_info->indicesGrp.size());
		LevelSlateIdx idxSlate=_info->indicesGrp[((DWORD)hGrp.iStart)+idx];
		return _GetSlate(idxSlate);
	}

	void _UpdateState();
	void _RefreshSensor();

	void _ApplyReach(CLevelSlateA &slateReach,CLevelPlayer *player);
	void _TriggerSwitch(CLevelSlateA &slateSwitch);

	void _SwitchState_Process(CLevelSlateA &slate);
	void _SwitchState_Idle(CLevelSlateA &slate);
	void _SwitchState_NotInSlate();

	void _RevealSlate(CLevelSlateA &slate);
	BOOL _FlipSlate(CLevelSlateA &slate);

	void _ValidateEdgeLocks(CLevelSlateA &slate);
	BOOL _CanLockEdge(CLevelSlateA &slate);

	void _RefreshTeleportLink();
	void _AddTeleportLink(LevelSlateIdx idxFrom,LevelSlateIdx idxTo);


	void _RefreshSwitchPointer();

	void _RefreshButtonLocks();
	void _AnnealButtons();

	void _FlushStar();
	LevelResourceType _GetResType()	{		return LevelResource_Gem;	}

	CLevelSkillDriver _skilldriver;
	CLevelSensor_Slates _sensor;

	void _IncVer()	{		_ver++;	}
	DWORD _ver;
	DWORD _verSync;

	LevelTick _tUpdate;

	LevelSlatesInfo *_info;
	CLevelSlatesBasis *_basis;

	LevelSlateIdx _enter;//从哪里进入这个Slates

	//States
	std::vector<CLevelSlateA> _bufSlate;
	std::vector<LevelSlateIdx> _reachables;
	std::vector<LevelSlateA_TeleportLink> _linksTeleport;
	LevelSlateIdx _reached;
	LevelSlatesState _state;
	int _countTeleportCD;
	int _nStars;

	//Process
	BOOL _bInProcess;
	CLevelBehavior *_bhvProcess;

	//Embeds
	LevelObjID _FindEmbed(LevelSlateIdx idx);
	void _AddEmbed(LevelSlateIdx idx,LevelObjID idLo);
	void _FlushEmbeds();
	void _DestroyEmbeds();
	std::unordered_map<LevelSlateIdx,LevelObjID> _embeds;

	//
	std::vector<WORD> _indicesTemp;
	std::vector<LevelSlateIdx> _revealsTemp;

};
