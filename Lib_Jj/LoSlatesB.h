#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"
#include "LevelSlateDefinesB.h"

#include "LevelSlatesBasis.h"

#include "LevelChancer.h"

#include "LevelSensor.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#include "LevelAttrs.h"

#include "LevelBuff.h"

#include "LoSlates.h"

#include "LevelSkillDriver.h"

#define CLASSUID_SlatesB 61

struct SlatesRandomPickEntryB:public SlatesRandomPickEntry
{
    BEGIN_GOBJ_PURE_UID2(SlatesRandomPickEntryB,461,1);
		GELEM_VAR_INIT(LevelSlateType,tp,LevelSlateTypeB_Cross);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeB),"类型");
		GELEM_VAR_INIT(int,count,1);
			GELEM_EDITVAR("设置几个",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"设置几个");
		GELEM_VARVECTOR_INIT(LevelSlateType,tpsTarget,LevelSlateTypeB_Cross);
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeB),"类型");
    END_GOBJ();    
};

struct SlatesB_GenerateParam
{
    BEGIN_GOBJ_PURE_UID2(SlatesB_GenerateParam,463,1);
		GELEM_VAR_INIT(int,lvlDifficulty,0);
			GELEM_EDITVAR("难度",GVT_S,GSem(GSem_Interger,"Easy,Medium,Hard"),"难度");
    END_GOBJ();    

	int lvlDifficulty;
};




struct LopSlatesB:public LopSlates
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSlatesB,CLASSUID_SlatesB);

	BEGIN_GOBJ_PURE(LopSlatesB,1);	

		GELEM_VARVECTOR(LevelGUID,includes)
			GELEM_EDITVAR("包含石板",GVT_U,GSem(GSem_Unknown,"AssetUIDSet"),"石板迷宫包含哪些石板");

		GELEM_OBJ(SlateSpaceDefine,space);
			GELEM_EDITOBJ("坐标系","坐标系");

		GELEM_OBJVECTOR(SlateGrpEntry,grps);
			GELEM_EDITOBJ("石板组","石板组");

		GELEM_VAR_INIT( StringID,bhvSetup,StringID_Invalid);	
			GELEM_EDITVAR( "设置过程图", GVT_U, GSem(GSem_StringID,"行为图名称"), "设置石板迷宫的行为图的名称" );
	END_GOBJ();



};

struct LosSlatesB:public LosSlates
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSlatesB,CLASSUID_SlatesB);

	BEGIN_GOBJ_PURE(LosSlatesB,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

	END_GOBJ();

};

struct LevelSlateInfo;
class CLevelSlateB
{
public:
	CLevelSlateB()
	{
		_info=NULL;
		_bSyncDirty=FALSE;
	}
	void Init(LevelSlateInfo *info);

	LevelPos &GetPos()	{		return _pos;	}

	BOOL CheckIn(LevelPos &pos);

	void SetSyncDirty()	{		_bSyncDirty=TRUE;	}
	void ResetSyncDirty()	{		_bSyncDirty=FALSE;	}

	BOOL IsEntrance()	{		return _status.bEntrance;	}
	BOOL IsLock()
	{
		return (_status.tp>=LevelSlateTypeB_Rune01)&&(_status.tp<=LevelSlateTypeB_Rune06);
	}

protected:

	BOOL _bSyncDirty;

	LevelSlateB_Status _status;
	LevelSlateIdx _stamps[SLATESB_MAX_STAMP];

	LevelSlateInfo *_info;
	LevelPos _pos;

friend class CLoSlatesB;
};


struct LevelSlatesInfo;
class CLevelPlayer;
class CLoSlatesB:public CLoAgent
{
public:
	DEFINE_LEVELOBJ_CLASS(CLoSlatesB,CLASSUID_SlatesB);

	CLoSlatesB()
	{
		Zero();
		_ver=_verSync=0;
	}
	void Zero()
	{
		_basis=NULL;
		_info=NULL;

		_state=LevelSlatesState_None;

		_bhvProcess=NULL;
		_bInProcess=FALSE;

		_enter=LevelSlateIdx_Invalid;

		_tUpdate=0;
		_bUnlocked=FALSE;
	}

	void Clear();

	virtual const char *GetShowName()	{		return "石板迷宫B";	}

	virtual BOOL IsServerOnly()	{		return FALSE;	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual void Update();
	virtual void HandleHook(LevelHook &hk);

	virtual CLevelSensor*GetSensor()	override{		return &_sensor;}
	virtual CLevelSkillDriver *GetSkillDriver() override {return &_skilldriver;}

	CLevelObj *GetThreat();

	void Setup_SetExit(StringID grp);
	void Setup_Generate(SlatesB_GenerateParam &param);

	void FinishProcess()	{		_bInProcess=FALSE;	}
	void RevealNearBy(LevelSlateIdx idxSlate,int radius);
	void RevealAll(LevelSlateIdx idxSlate);
	BOOL GetSlatePos(LevelSlateIdx idxSlate,LevelPos &pos);

	void RequestFlipFromClient(LevelPlayerID idPlayer,LevelSlateIdx idxSlate);

protected:

	CLevelPlayer *_GetCurPlayer();

	void _LoadFromData(SlatesBData &data);

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void _OnPostWriteSync();

	void _WriteInitials(CBitPacket *bp);
	void _WriteStates(CBitPacket *bp);

	BOOL _CheckValidSlateIdx(LevelSlateIdx idxSlate)
	{
		if ((idxSlate>=_info->iStartSlate))
		{
			if(idxSlate-_info->iStartSlate<_bufSlate.size())
				return TRUE;
		}
		return FALSE;
	}

	CLevelSlateB &_GetSlate(LevelSlateIdx idxSlate)
	{
		assert(_CheckValidSlateIdx(idxSlate));
		return _bufSlate[idxSlate-_info->iStartSlate];
	}

	CLevelSlateB &_GetSlate(int x,int y)
	{
		return _GetSlate(_info->GetSlateAt(x,y));
	}


	LevelSlatesGrpHandle _FindGrp(StringID nmGrp);
	CLevelSlateB &_GetGrpSlate(LevelSlatesGrpHandle hGrp,int idx)
	{
		assert((DWORD)idx<hGrp.count);
		assert(_info);
		assert(((DWORD)hGrp.iStart)+(DWORD)idx<_info->indicesGrp.size());
		LevelSlateIdx idxSlate=_info->indicesGrp[((DWORD)hGrp.iStart)+idx];
		return _GetSlate(idxSlate);
	}

	void _UpdateState();
	void _RefreshSensor();


	void _SwitchState_Process(CLevelSlateB &slate);
	void _SwitchState_Idle(CLevelSlateB &slate);
	void _SwitchState_NotInSlate();

	void _ClearSlateReveal(CLevelSlateB &slate);
	void _RevealSlate(CLevelSlateB &slate,LevelSlateIdx stamp);
	void _UnRevealSlate(CLevelSlateB &slate);
	void _RevealNearbySlates(CLevelSlateB &slate,i_math::pos2di *offsets,DWORD nOffsets);
	void _RevealTouchingSlates(CLevelSlateB &slate);
	void _ApplyReach(CLevelSlateB &slateReach,CLevelPlayer *player);
	void _AddNearbyReachables(CLevelSlateB &slate);

	CLevelSkillDriver _skilldriver;
	CLevelSensor_Slates _sensor;

	void _IncVer()	{		_ver++;	}
	DWORD _ver;
	DWORD _verSync;

	LevelTick _tUpdate;

	LevelSlatesInfo *_info;
	CLevelSlatesBasis *_basis;

	LevelSlateIdx _enter;//从哪里进入这个Slates

	BOOL _bUnlocked;

	//States
	std::vector<CLevelSlateB> _bufSlate;
	std::vector<LevelSlateIdx> _reachables;
	LevelSlateIdx _reached;
	LevelSlatesState _state;

	//Process
	BOOL _bInProcess;
	CLevelBehavior *_bhvProcess;

	//
	std::vector<WORD> _indicesTemp;
	std::set<CLevelSlateB*> _revealsTemp;

};
