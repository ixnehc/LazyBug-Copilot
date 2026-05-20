#pragma once

#include "class/class.h"
#include "anim/animdefines.h"

#include "LevelDefines.h"
#include "LevelRtnuDefines.h"


class CLevelObj;
inline void lo_verify(CLevelObj*c) {}


#define DEFINE_LEVELOBJ_CLASS(clss,uid)													\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelObj;															\
		instance._uid=uid;																					\
		{clss *p=NULL;lo_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


class CLevelObjSrc;
class CLevelObjParam;
class CLevelMsgBuf;
class CLevel;
class CBitPacket;
class CUnit;
class CUnit3D;
class CRtnuUnit;
class CLevelSkillCDs;
class CLevelSkillDriver;
class CLevelBuffs;
class CLevelObjPauser;
class CLevelObjMove;
class CLevelTalks;
class CLevelOp;
class CLevelOps;
struct LevelAttr_Base;
struct LevelAttr_Resists;
struct LevelAttr_Evade;
struct LevelAttr_Weaks;
struct LevelAttr_WeaksMod;
struct LevelAttr_DefendMods;
struct LevelAttr_AttackMods;
struct LevelAttr_Drop;
struct LevelAttr_Resource;
struct LevelAttr_Temple;
struct LevelAttr_MagicBoard;
struct LoMiscFlags;
struct LevelRecordSkill;
struct LevelObjTile;
struct LevelEventQueue;
class CLevelObjResidable;
class CLevelBehavior;
class CLevelCoSkill;
class CLevelCounter;
struct ExprEquips;
class CLevelTroops;
class CLevelEventSrc;
struct LevelAICmd;
class CLevelTroop;
class CLevelSensor;
class CLevelSkill;
struct LevelAIContext;
class CLevelBlocking;
class CLevelObj
{
public:
	IMPLEMENT_REFCOUNT_OVERRIDE;
	void OnRelease();

	virtual CClass *GetClass()=0;
	virtual const char *GetShowName()	{		return "未知类型对象";	}

	CLevelObj()
	{
		_level=NULL;
		_src=NULL;
		_param=NULL;
		_bits=0;
		_id=LevelObjID_Invalid;
		_maskPlayer=0;
		_idPlayer=LevelPlayerID_Wild;
		_idOnlyVisible=LevelPlayerID_Invalid;
		_tile=NULL;
	}


	BOOL IsAlive()	{		return (_bAlive==1)?TRUE:FALSE;	}
	BOOL IsActive()	{		return _bActive==1?TRUE:FALSE;	}
	BOOL IsDeferDestroy()	{		return _bDeferDestroy==1?TRUE:FALSE;	}
	BOOL IsPlayer()	{		return _bPlayer==1?TRUE:FALSE;	}
	BOOL IsRetinue()	
	{		
		if (!_bPlayer)
		{
			if (_idPlayer<LEVEL_MAX_PLAYER)
				return TRUE;
		}
		return FALSE;
	}
	void SetEnum(BOOL bEnum)	{		_bEnum=bEnum?1:0;	}
	BOOL IsEnum()	{		return _bEnum==1?TRUE:FALSE;	}

	CLevel *GetLevel()	{		return _level;	}
	CLevelObjSrc *GetLos()	{		return _src;	}
	template<typename T>
	T *GetLos()
	{
		if (_src)
		{
			if (_src->GetClass()->IsSameWith(Class_Ptr2(T)))
				return (T*)_src;
		}
		return NULL;
	}
	CLevelObjParam *GetLop()	{		return _param;	}
	template<typename T>
	T *GetLop()
	{
		if (_param)
		{
			if (_param->GetClass()->IsSameWith(Class_Ptr2(T)))
				return (T*)_param;
		}
		return NULL;
	}

	template <typename T>
	T *ToPtr()
	{
		if (GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)this;
		return NULL;
	}

	BOOL Create();//加引用计数
	void Destroy();//减引用计数
	void DeferDestroy()//注意:不减引用计数
	{
		if (_bAlive)
			_bDeferDestroy=1;
	}

	BOOL Activate();
	void Deactivate();

	void HandleEvent(LevelEvent &e);

	LevelObjID GetID()	{		return _id;	}
	LevelPlayerID GetPlayerID()	{		return _idPlayer;	}
	void SetPlayerID(LevelPlayerID idPlayer)	
	{		
		if (_idPlayer!=idPlayer)
		{
			_idPlayer=idPlayer;	
			_bSyncPlayerID=1;
			OnPlayerIDChanged();
		}
	}
	DWORD GetPlayerIDMask()	{		return 1<<_idPlayer;	}

	LevelPlayerID GetOnlyVisible()	{		return _idOnlyVisible;	}
	void SetOnlyVisible(LevelPlayerID idPlayer)	{		_idOnlyVisible=idPlayer;	}
	void SetVisibleToAll()	{		_idOnlyVisible=LevelPlayerID_Invalid;	}

	LevelPlayerMask GetPlayerMask()	{		return _maskPlayer;	}
	void SetPlayerMask(LevelPlayerMask mask)	{		_maskPlayer=mask;	}
	void AddPlayerMask(LevelPlayerMask mask)	{		_maskPlayer|=mask;	}
	void RemovePlayerMask(LevelPlayerMask mask)	{		_maskPlayer&=~mask;	}
	void ClearPlayerMask()	{		_maskPlayer=0;	}

	void SetCollide_Ghost(BOOL bGhost);

	LevelMoveSession MoveCmd_RequestNoTarget();
	LevelMoveSession MoveCmd_RequestTarget(LevelPos &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange);
	LevelMoveSession MoveCmd_RequestTarget(LevelPos3D &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange);
	LevelMoveSession MoveCmd_RequestTarget(CLevelObj*lo,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange,BOOL b3DFollow);
	LevelMoveSession MoveCmd_RequestFacing(float range,float rad);
	void MoveCmd_ResetIdle();

	void BreakTalk(LevelPlayerID idPlayer);

	BOOL TestBuffFlag(DWORD flags);//BuffFlag


	void StartCD(LevelRecordSkill *rec);

	void AddEvent(LevelEvent *e);

	CLevelOp *NewOp(CClass *clss,LevelOpLink &link);
	template <typename T>
	T *NewOp(LevelOpLink &link)
	{
		return (T*)NewOp(Class_Ptr2(T),link);
	}

	void FillOpDesc(LevelOpDesc &desc,CClass *clssOp,LevelOpLink &link)
	{
		desc.uid=(WORD)clssOp->GetUID();
		desc.tpOwner=LevelOpDesc::Obj;
		desc.idOwner=GetID();
		desc.link=link;
	}

	LevelMoveMethodMask GetMoveMethodMask()
	{
		LevelMoveMethod method=GetMoveMethod();
		if (method>0)
			return 1<<(method-1);
		return 0;
	}


	virtual LevelObjType GetType()=0;
	virtual LevelTick GetT();
	virtual ServerSecond GetServerSecond();
	virtual LevelObjID GetRootOwnerID()=0;
	virtual CLevelSkill *GetRootSkill()	{		return NULL;	}
	virtual LevelGUID GetGUID()	{		return LevelGUID_Invalid;	}


	virtual BOOL IsServerOnly()	{		return FALSE;	}
	virtual BOOL IsGlobalSight()	{		return FALSE;	}//是否全局视野(全局游戏对象),即玩家在任何地方都能看到它

	virtual void PostCreate()	{	}
	virtual void OnDestroy()		{}
	virtual BOOL OnCreate()		{return TRUE;		}

	virtual BOOL OnActivate()		{ return TRUE;	}
	virtual void OnDeactivate()		{}

	virtual void OnPlayerIDChanged(){}

	virtual void OnEvent(LevelEvent &e){}
	virtual void HandleHook(LevelHook &hk)	{		return;	}

	virtual LevelGrade GetGrade()	{		return 1;	}
	virtual LevelAttr_Base*GetAttr_Base()	{		return NULL;	}
	virtual LevelAttr_Resists*GetAttr_Resists()	{		return NULL;	}
	virtual LevelAttr_Weaks*GetAttr_Weaks()	{		return NULL;	}
	virtual LevelAttr_WeaksMod*GetAttr_WeaksMod()	{		return NULL;	}
	virtual LevelAttr_Evade*GetAttr_Evade()	{		return NULL;	}
	virtual LevelAttr_DefendMods*GetAttr_DefendMods()	{		return NULL;	}
	virtual LevelAttr_AttackMods*GetAttr_AttackMods()	{		return NULL;	}
	virtual LevelAttr_Drop*GetAttr_Drop()	{		return NULL;	}
	virtual LevelAttr_Resource*GetAttr_Resource()	{		return NULL;	}
	virtual LevelAttr_Temple*GetAttr_Temple()	{		return NULL;	}
	virtual LevelAttr_MagicBoard*GetAttr_MagicBoard()	{		return NULL;	}

	virtual LoMiscFlags*GetMiscFlags()	{		return NULL;	}
// 	virtual LevelAttack GetAttack(LevelAttackType tp);
// 	virtual LevelDefence GetDefence(LevelAttackType tp);

	virtual LevelObjShapeType GetShapeType()	{		return LevelObjShape_SingleCircle;	}//外形模拟方式

	virtual LevelPos GetFramePos()	{		return LevelPos_Invalid;	}//得到当前帧的位置
	virtual LevelFace GetFrameFace()	{		return 0.0f;	}//得到当前帧的朝向
	virtual LevelPos3D GetFramePos3D();
	virtual float GetRadius_()	{		return 0.0f;	}
	virtual float GetHeight() {		return 0.0f;	}
	virtual float GetAimHeight()	{		return 0.0f;	}//作为瞄准对象时的点的高度
	virtual float GetCastHeight()	{		return 0.0f;	}//发射时的点的高度
	virtual float GetModelScale()	{	return 1.0f;	}
	virtual LevelObjCircle *GetShapeCircles(DWORD &count){		count=0;return 0;	}//得到所有的描述Shape的圆
	virtual CUnit *GetUnit()	{		return NULL;	}
	virtual CUnit3D *GetUnit3D()	{		return NULL;	}
	virtual CRtnuUnit *GetRtnuUnit()	{		return NULL;	}
	virtual CLevelObjMove *GetMove()	{		return NULL;}
	virtual LevelMoveMethod GetMoveMethod()	{		return LevelMoveMethod_Ground;}
	virtual CLevelTalks*GetTalks()	{		return NULL;}
	virtual CLevelSensor*GetSensor()	{		return NULL;}
	virtual CLevelSkillCDs *GetSkillCDs()	{		return NULL;	}
	virtual CLevelSkillDriver *GetSkillDriver()	{		return NULL;	}
	virtual CLevelBuffs *GetBuffs()	{		return NULL;	}
	virtual CLevelOps *GetOps()	{		return NULL;	}
	virtual CLevelObjPauser *GetPauser()	{		return NULL;	}
	virtual ExprEquips *GetExprEquips()	{		return NULL;	}
	virtual LevelEventQueue *GetEventQueue(){return NULL;}
	virtual CLevelBehavior*GetBehaviorAI()	{		return NULL;	}
	virtual CLevelObjResidable*GetResidable()	{		return NULL;	}
	virtual CLevelCoSkill *ObtainCoSkill()	{		return NULL;	}
	virtual CLevelCounter *GetCounter()	{		return NULL;	}
	virtual CLevelCounter *ObtainCounter()	{		return NULL;	}
	virtual CLevelTroops *ObtainTroops()	{		return NULL;	}
	virtual CLevelTroops *GetTroops()	{		return NULL;	}
	virtual CLevelEventSrc *GetEventSrc()	{		return NULL;	}
	virtual CLevelBlocking *GetBlocking()	{		return NULL;	}
	virtual void SetTroop(CLevelTroop *troop)	{	}
	virtual CLevelTroop *GetTroop()	{		return NULL;	}
	virtual LevelRtnuRank GetRtnuRank()	{		return LevelRtnuRank_None;	}


	virtual LevelAIContext *ObtainAIContext()	{		return NULL;	}
	virtual LevelAIContext *GetAIContext()	{		return NULL;	}
	virtual void SetAICmd(StringID idCmd)	{	}
	virtual StringID GetAICmd()	{	 return NULL; }


	virtual LevelTeleportID GenTeleportID()	{		return LevelTeleportID_Invalid;	}
	virtual void AddOp(CLevelOp *op);

	virtual void Update()	{	}
	virtual void UpdateSubframe()	{	}

	//写入同步信息,FirstSync表示是第一次同步(第一次同步需要传送一些初始的信息,后续的同步只需要在之前的信息基础上
	//作修改)
	virtual void WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)	{	}
	//高频同步信息(几乎每帧都会需要更新的信息),H代表High freq
	virtual void WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)	{	}
	//低频同步信息(隔一段时间(3秒以上)更新的信息),L代表Low freq
	virtual void WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)	{	}
	virtual void PostWriteSync()	{	}//这个函数在调过WriteSyncX(..)后一定要调,切记


	virtual DWORD GetBuffID_Dead()	{		return 0;	}//RecordID
	virtual DWORD GetBuffID_Stun()	{		return 0;	}//RecordID
	virtual DWORD GetBuffID_KB()	{		return 0;	}//RecordID
	virtual DWORD GetBuffID_Bleed()	{		return 0;	}//RecordID
	virtual DWORD GetBuffID_Ash()	{		return 0;	}//RecordID
	virtual StringID GetBuffHandler_Jink()	{		return StringID_Invalid;	}
	virtual StringID GetBuffHandler_SkillStun()	{		return StringID_Invalid;	}

protected:
	BOOL _Create();
	void _Destroy();

	void _RegisterLevelHook(int tp,DWORD priority);

	void _WriteSync_PlayerID(CBitPacket *bp,BOOL &bContent);
	void _PostWriteSync_PlayerID();

	union
	{
		struct
		{
			WORD _bAlive:1;
			WORD _bActive:1;
			WORD _bDeferDestroy:1;//需要被删除(会在下一次Update时删除)
													//之所以不直接删除的原因是,如果直接删除,我们在CLevel::Update()的时候将无法找出那些因为
													//被删除而离开角色视野的LevelObj,所以我们做一个DeferDestroy的标记,在CLevel::Update()作
													//真正的删除
			WORD _bEnum:1;
			WORD _bAffect:1;
			WORD _bPlayer:1;//是否是玩家单位(主角)
			WORD _bSyncPlayerID:1;//表示_idPlayer发生了变化,需要同步给客户端
		};
		WORD _bits;
	};
	LevelPlayerID _idPlayer;//主人是谁,如果_bPlayer为1,则这个值为这个Player的id(自己的PlayerID)
	LevelPlayerID _idOnlyVisible;//仅对哪个Player可见,如果为PlayerID_Invalid,对所有Player可见
	LevelObjID _id;
	CLevel *_level;

	CLevelObjSrc *_src;
	CLevelObjParam *_param;

	LevelObjTile *_tile;//属于哪个tile

	LevelPlayerMask _maskPlayer;//标记这个obj哪些Player的视野范围内

	friend class CLevelInactives;
	friend class CLevel;
	friend class CLevelIDs;
	friend class CLevelObjMap;
};

