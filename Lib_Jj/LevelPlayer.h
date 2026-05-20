#pragma once

#include "class/class.h"

#include "anim/animdefines.h"
#include "anim/KeySet.h"

#include "LevelDefines.h"

#include "LevelObj.h"
#include "LevelInactives.h"
#include "LevelAovMap.h"
#include "LevelDecider.h"
#include "unitmgr/UnitMgrNavMesh.h"

#include "bitset/bitset.h"

#include "LevelPlayerStates.h"

#include "LevelService.h"

#include "LevelAbility.h"

#include "LevelRtnuCmds.h"

#include "Protocal.h"



//CLevelSight代表一个可以被某个Player看到的LevelObj
class CLevelObj;
class CLevelSight
{
public:
	CLevelSight()
	{
		obj=NULL;
		bFirstSync=FALSE;
	}
	CLevelObj *obj;
	BOOL bFirstSync;
};

struct PlayerMove;
struct PlayerMoveReply;
struct PlayerSkill;
class CLoUnit;
class CBitPacket;
class CLevelRtnuCircum;

class CLevelPlayerMove
{
public:
	CLevelPlayerMove()
	{
		Zero();
	}
	void Zero()
	{
		_lo=NULL;
		_bReaching=FALSE;
		_idTeleport=LevelTeleportID_Invalid;
		_idAuthTeleport=LevelTeleportID_Invalid;

		_tClient=ANIMTICK_INFINITE;
		_tLevel=ANIMTICK_INFINITE;

		_bPauseMove=FALSE;
	}

	void Init(CLoUnit* lo)
	{
		_lo=lo;
		KeySet_Define(&_ks,KT_Floatx2);
		KeySet_Define(&_ksFace,KT_Float);

		_posExpect=((CLevelObj*)lo)->GetFramePos();

	}
	void Clear()
	{
		_ks.Clean();
		Zero();
	}

	void HandleMove(PlayerMove &move,ServerSecond second,PlayerMoveReply &reply);//返回处理这个消息后,需要同步的Obj
	void UpdatePosAndFace(ServerSecond second);
	void PauseMove()	{		_bPauseMove=TRUE;	}
	void AuthorizeTeleport(LevelTeleportID idTeleport,LevelPos &posTeleport);//授权Teleport,(允许客户端以指定的TeleportID将位置移动到指定位置)

	LevelPos GetExpectPos()	{		return _posExpect;	}

	LevelPos CalcPos(LevelTick tLocal);

	BOOL IsReaching(LevelTick tLocal);
	void GetRecentMoveStep(LevelTick tLocal,LevelTick durStep,LevelMoveStep &step);

protected:
	CLoUnit*_lo;

	//角色移动同步控制数据
	LevelTick _ToLocalT(ServerSecond second);
	LevelTick _ToClientT(LevelTick tLevel)	//把Server的时间,转换成Client的时间
	{
		return _tClient+(tLevel-_tLevel);
	}
	BOOL _NeedPauseMove();
	KeySet _ks;//以Client时间为key的KeySet
	KeySet _ksFace;//以Client时间为key的KeySet
	LevelTick _tClient;
	LevelTick _tLevel;//
	BOOL _bReaching;//表示_ks走完后,是否走到了
	BOOL _bPauseMove;//是否暂停移动
	LevelTeleportID _idTeleport;//路点(_ks)的teleport id
	LevelPos _posTeleport;//_idTeleport对应的位置
	LevelTeleportID _idAuthTeleport;//得到授权的TeleportID
	LevelPos _posAuthTeleport;//得到授权的Teleport位置

	LevelPos _posExpect;//目的位置

};

struct LevelPlayerSkill
{
	LevelPlayerSkill()
	{
		idSkill=RecordID_Invalid;
		grd=LevelSkillGrade_Invalid;
		stack=0;
	}
	RecordID idSkill;
	LevelSkillGrade grd;//技能等级
	DWORD stack;
};
class CLevelRecords;
struct LevelRecordUnit;
class CLevelPlayerSkills
{
public:
	CLevelPlayerSkills()
	{
		Zero();
	}

	void Zero()
	{
		_verDB=LPSVer_Invalid;
	}
	void Clear();
	void Refresh(LevelPlayerStates *lps,LevelRecordUnit *recUnit,CLevelRecords *records);

	LevelSkillGrade GetSkillGrade(RecordID idSkill);
	DWORD GetSkillStack(RecordID idSkill);

protected:
	void _Build(LevelPlayerStates *lps);
	void _AddSkill(RecordID idSkill,LevelSkillGrade grd,DWORD stack);
	LPSVer _verDB;
	std::unordered_map<RecordID,LevelPlayerSkill>_skills;
};

struct LevelRtnuCmdEx:public LevelRtnuCmd
{
	LevelTick t;
};


class CLoItem;
struct DecideInfo_Equip;
class CLevelRtnu;
class CLevelRtnus;
class CLevelNPCs;
struct LevelSkillArg;
class CLevelAbilityUpgrade;
class CLevelPlayer
{
public:
	CLevelPlayer()
	{
		Zero();
	}

	void Zero()
	{
		_centerAov=AovCenter_Invalid;
		_centerAoa=AoaCenter_Invalid;
		_id=LevelPlayerID_Invalid;
		_mask=0;
		_lo=NULL;
		_level=NULL;

		_lps=NULL;
		_diEquip.Zero();

		_verEquip=0;

		_posExplore.set(-10000,-10000);

		_posLastFrame=LevelPos_Invalid;

		_npcsRtnu=NULL;

		_tExhaustedStart=ANIMTICK_INFINITE;

		_idSlates=LevelObjID_Invalid;

		_circumRtnu=NULL;
		_rtnus=NULL;
	}

	void Init(CLevel *level,LevelPlayerID id,CLoUnit*unit,LevelPlayerStates *lps);
	void Clear();

	void OnEnterLevel();
	void OnLeaveLevel();


	CLevel *GetLevel()	{		return _level;	}

	BOOL IsEmpty()	{		return _id==LevelPlayerID_Invalid;	}


	void SetPlayerID(LevelPlayerID id)
	{
		_id=id;
		_mask=1<<id;
	}

	LevelPlayerID GetPlayerID()	{		return _id;	}
	LevelPlayerMask GetPlayerMask()	{		return _mask;	}

	LevelPlayerStates *GetLPS()	{		return _lps;	}

	CLoUnit *GetLoUnit()	{		return _lo;	}

	CLevelPlayerMove &GetMove()	{		return _move;	}

	CLevelRtnuCircum *GetRtnuCircum()	{		return _circumRtnu;	}
	CLevelRtnus *GetRtnus()	{		return _rtnus;	}
	DWORD GetRecentRtnuCmds(float dur,LevelRtnuCmd **buf,DWORD szBuf);

	CLevelAbilities &GetAbilities()	{		return _abilities;	}//注意返回的abilities只能读,不能修改

	CLevelService*GetService(LevelServiceType tp);

	AovCenter &GetAovCenter()	{	return _centerAov;	}
	AoaCenter &GetAoaCenter()	{	return _centerAoa;	}

	DWORD AddSightEnter(CLevelObj *obj,BOOL bFirstSync=TRUE);


	void CreateRtnuNPCs();
	void CreateRtnuNPCs_Teleport(CLevelNPCs *npcsOrg);
	CLevelNPCs *FetchRtnuNPCs()
	{
		CLevelNPCs *ret=_npcsRtnu;
		_npcsRtnu=NULL;
		return ret;
	}
	CLevelNPCs *GetRtnuNPCs()	{		return _npcsRtnu;	}

	//写入离开sight的obj的信息
	void WriteFrameLeaveSight(CBitPacket *bp);

	//写入视野里的obj的帧同步信息
	void WriteFrameSync(CBitPacket *bp);

	BOOL HandleSkill(PlayerSkill &skill,LevelSkillArg *arg);

	//处理把装备栏的道具卸下,转移到PickUp的请求
	//如果返回TRUE,bAffectBag0表示这次道具装备,有没有影响到主背包(某些道具被扔进了主背包)
	BOOL HandleUnEquipToPickUp(EquipPart part,BOOL &bAffectBag0);

	//处理把PickUp的道具,装备到装备栏的请求
	//如果返回TRUE,bAffectBag0表示这次道具装备,有没有影响到主背包(某些道具被扔进了主背包)
	BOOL HandleEquipFromPickUp(EquipPart part,BOOL &bAffectBag0);

	//处理把Bag内的道具,装备到装备栏的请求
	//如果返回TRUE,bAffectBag0表示这次道具装备,有没有影响到主背包(某些道具被扔进了主背包)
	BOOL HandleEquipFromBag(DWORD iBag,i_math::rect_c &rc,BOOL &bAffectBag0);

	//处理把装备栏的道具卸下,转移到某个Bag的请求
	//如果返回TRUE,bAffectBag0表示这次操作,有没有影响到主背包(某些道具被扔进了主背包)
	BOOL HandleUnEquipToBag(EquipPart part,DWORD iBag,i_math::rect_c &rc,BOOL &bAffectBag0);

	BOOL HandleDiscardPickUp(LevelPos &pos);

	BOOL HandleInvokeItem(CLoItem *item,BOOL bToBag);

	BOOL HandleRtnuCmd(LevelRtnuCmd &cmd);

	BOOL HandleRtnuHint(LevelRtnuHint &hint);

	BOOL HandleSwitchWpn(EquipPart wpnActive);

	void HandleStartDay();
	void HandleEndDay();

	BOOL HandleToeStoneThrust(EquipPart wpnActive);


	void SetLPSDirty();//调用这个函数将会确保LPS被尽快的更新给Client

	DecideInfo_Equip *GetDecideInfo_Equip();

	void UpdateExlporeMap();

	void AddPendingAgentBriefEntry(LevelGUID guid);
	void FetchPendingAgentBriefEntry(std::vector<LevelAgentGuid> &pendings);

	BOOL UpgradeAbility(CLevelAbilityUpgrade &upgrade,LevelAwardSeed &seed);

	void UpdateAbilities();

	void UpdateHPSP(float dt);
	void UpdateHPSP_New(float dt);

	void UpdateRtnus();
	void UpdateRtnuCmds();


protected:

	CLevel *_level;

	LevelPlayerStates *_lps;

	LevelPlayerMask _mask;//自己这个Player的Mask
	LevelPlayerID _id;

	AovCenter _centerAov;
	AoaCenter _centerAoa;

	//保存将要进入/离开视野的LevelObj的临时buffer
	std::vector<CLevelObj*> _leaves;

	std::deque<CLevelSight> _sights;
	std::deque<WORD>_freelistSights;

	CLevelNPCs *_npcsRtnu;//随从的NPC

	CLoUnit*_lo;

	CLevelPlayerMove _move;
//	CLevelPlayerSkills _skills;
	CLevelAbilities _abilities;
	BCAbility _msgAbilities[LevelAbilityType_Max];

	//装备的DecideInfo
	DecideInfo_Equip _diEquip;
	LPSVer _verEquip;

	i_math::pos2di _posExplore;

	std::deque<LevelRtnuCmdEx> _cmdsRtnu;

	CLevelRtnuCircum *_circumRtnu;
	CLevelRtnus *_rtnus;

	CLevelServiceCureHP _serviceCureHP;

	LevelPos _posLastFrame;

	LevelTick _tExhaustedStart;

	LevelObjID _idSlates;//当前在哪个Slates里

	friend class CLevel;


};
