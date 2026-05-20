#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "LevelObj.h"

#include "LevelSkillDriver.h"

#include "LevelOp.h"

#include "LevelRecordSkill.h"

class CLevelSkill;
inline void buff_verify(CLevelSkill*c) {}

#define DEFINE_SKILL_CLASS(clss,uid)															\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelSkill;																	\
		instance._uid=uid;																					\
		{clss *p=NULL;buff_verify(p);}																\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


#define DEFINE_SKILLPARAM_CLASS(clss)											\
	DEFINE_CLASS(clss);																	\
	virtual CClass*GetSkillClass()																				\
	{																																\
		extern CClass *GetSkillClass_##clss();																				\
		return GetSkillClass_##clss();																				\
	}

#define BIND_SKILLPARAM(clssSkill,clssSkillParam)													\
	CClass *GetSkillClass_##clssSkillParam()																				\
	{																													\
		return Class_Ptr2(clssSkill);																\
	}																													\
	CClass *GetClass_##clssSkillParam()														\
	{																													\
		return Class_Ptr2(clssSkillParam);															\
	}



struct LevelSkillParam
{
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual CClass*GetSkillClass()=0;
};

class CLevelSkillCasting
{
public:
	CLevelSkillCasting()
	{
		_skill=NULL;
		_tCasting=0;
		_tFire=0;
		_bNeedFire=0;
		_bNeedCasted=0;
		_bNeedFinished=0;
		_bFired=0;
	}

	void Init(CLevelSkill *skill)
	{
		_skill=skill;
	}

	void UpdateToCasted(AnimTick dt);//Casting正常结束后,变成Casted
	void UpdateToFinished(AnimTick dt);//Casting正常结束后,变成Finished

	BOOL NeedFire()	{		return _bNeedFire;	}
	BOOL NeedCasted()	{		return _bNeedCasted;	}
	BOOL NeedFinished()	{		return _bNeedFinished;	}

	BOOL IsFired()	{		return _bFired;	}
	AnimTick GetFireTime()	{		return _tFire;	}
	AnimTick GetCastTime()	{		return _tCasting;	}


protected:
	AnimTick _tCasting;
	AnimTick _tFire;
	BYTE _bNeedFire:1;//这一次更新后,要启动Fire
	BYTE _bNeedCasted:1;//这一次更新后,要把Skill切换成Casted状态
	BYTE _bNeedFinished:1;//这一次更新后,要把Skill切换成Finished状态
	BYTE _bFired:1;

	CLevelSkill *_skill;

};

class CLevelSkill;

struct LevelRecordSkill;
class CLevelSkillDriver;
class CLoUnit;
struct DealArg;
class CLevelSkill
{
public:
	IMPLEMENT_REFCOUNT_C
	CLevelSkill()
	{
		Zero();
	}
	~CLevelSkill()
	{
		Clear();
	}

	enum CastMoving
	{
		CastMoving_None,//casting状态下,不能移动
		CastMoving_Move,//casting状态下,可以移动,为普通的移动,技能本身不负责移动位置
		CastMoving_Control,//casting状态下,可以移动,移动方式完全由技能控制
	};

	virtual CClass *GetClass()=0;

	void Zero()
	{
		_state=SkillState_None;
		_id=LevelSkillID_Invalid;
		_idClient=ClientSkillID_Invalid;
		_owner=NULL;
		_rec=NULL;
		_bPlayer=FALSE;
		_tBirth=0;
		_tUpdate=0;
		_param=NULL;
		_grd=LevelSkillGrade_Invalid;
		_arg=NULL;
		_bBroken=0;
	}

	virtual BOOL PreInitStartCheck(CLevelObj *owner,LevelRecordSkill *rec,LevelSkillTarget &target)//在初始化前检查一下是否可以开始
	{
		return TRUE;
	}

	void Init(CLevelSkillDriver *driver,LevelSkillID id,ClientSkillID idClient,LevelSkillGrade grd);
	void Clear();

	LevelSkillID GetID()	{		return _id;	}
	ClientSkillID GetClientID()	{		return _idClient;	}

	CLevel *GetLevel()	{		return _owner->GetLevel();	}
	BOOL CheckOwnerAlive()
	{
		if (!_owner)
			return FALSE;
		return _owner->IsAlive();
	}
	CLevelObj *GetOwner()	{		return _owner;	}
	LevelTick GetBirthTick()	{		return _tBirth;	}
	LevelRecordSkill *GetRec()	{		return _rec;	}
	RecordID GetRecID();

	virtual LevelSkillTarget::TypeMask GetTargetTypes()=0;

	virtual BOOL NeedLockPick()	{		return TRUE;	}

	virtual CastMoving GetCastMoving()	{		return CastMoving_None;	}//技能在释放时,如何移动
	virtual AnimTick GetCastingTime()	{		return ANIMTICK_INFINITE;	}//返回经过IAS修正的casting time
	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return FALSE;	}
	virtual AnimTick GetCastingEventTime(StringID nmEvent)	{		return ANIMTICK_INFINITE;	}

	virtual void GetCastingPos(LevelPos &pos)	{		pos=_owner->GetFramePos();	}
	virtual void GetCastingPos3D(LevelPos3D &pos)	{		pos=_owner->GetFramePos3D();	}
	virtual LevelFace GetCastingFace()	{		return _owner->GetFrameFace();	}

	virtual BOOL IsInvokingAgent()	{		return FALSE;	}

	void Start();
	void Start(LevelOpLink &link);
	void Update();
	void Break()	
	{
		_bBroken=1;
		_OnBreak();
	}

	virtual void NotifyCasted(){}//由Client发起的Casted
	virtual void StopCast(AnimTick tStop)	{	}

	virtual BOOL Combine(LevelSkillTarget &target)	{		return FALSE;	}
	virtual BOOL IsImmune()	{		return FALSE;	}
	virtual BOOL CanCancel()	{		return TRUE;	}
	virtual void Cancel()	{	Break();	}


	void Finish()
	{
		_OnFinish();
		SAFE_RELEASE(_owner);
	}

	LevelSkillTarget &GetTarget()	{		return _target;	}
	LevelSkillState GetState()	{		return _state;	}

	CLevelOp *NewOp(CClass *clss,LevelOpLink &link);
	template <typename T>
	T *NewOp(LevelOpLink &link)
	{
		return (T*)NewOp(Class_Ptr2(T),link);
	}

	void FillOpDesc(LevelOpDesc &desc,CClass *clssOp,LevelOpLink &link)
	{
		desc.uid=(WORD)clssOp->GetUID();
		desc.idOwner=_id;
		desc.tpOwner=LevelOpDesc::Skill;
		desc.link=link;
	}



protected:

	void _FillRndSeed();

	void _SetParam(LevelSkillParam*param)	{		_param=param;	}

	virtual void _OnStart(){}
	virtual void _OnStart(LevelOpLink &link)	{		_OnStart();	}
	virtual void _OnFinish(){}
	virtual void _OnBreak(){}
	virtual void _OnUpdate(AnimTick dt){}

	virtual void _MakeTransferTarget(LevelSkillTarget &target)	{	}

	virtual BOOL _WriteSyncData(CBitPacket *bp)	{ return FALSE;	}

	LevelPos _CalcAimDir(LevelSkillTarget &target);

	void _SetState(LevelSkillState state)	{		_state=state;	}

	void _AddStartOp();
	void _AddStartOp(LevelOpLink &link);
	void _AddCombineOp(LevelSkillTarget &target);
	void _AddSyncDataOp();

	AnimTick _GetAge()	{		return ANIMTICK_SAFE_MINUS(_tUpdate,_tBirth);	}

	BOOL _IsPlayer()	{		return _bPlayer;	}

	void _MakeDeals(LevelPos3D &pos,DealArg&arg);
	void _MakeDeals(CLevelObj *loTarget,DealArg&arg);

	void _MakeFanDeals(float angleFov,float range);

	void _DoTeleport(LevelPos &posTeleport,LevelFace faceTeleport,DWORD flag=0);


	LevelSkillState _state;
	LevelSkillTarget _target;
	LevelSkillArg*_arg;
	CLevelObj *_owner;
	LevelSkillParam *_param;
	LevelRecordSkill *_rec;
	LevelSkillID _id;
	LevelSkillGrade _grd;
	ClientSkillID _idClient;//Client提供的ID,如果为ClientSkillID_Invalid,表示这个技能不是由Client启动的
	DWORD _bPlayer:1;//是不是player的技能
	DWORD _bBroken:1;

	LevelTick _tUpdate;
	LevelTick _tBirth;

	friend class CLevelSkillCasting;
};
