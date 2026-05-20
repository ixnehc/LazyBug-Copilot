#pragma once

#include "LevelDefines.h"

#include "records/recordsdefine.h"

enum LevelSkillDriverMode
{
	LevelSkillDriverMode_None,
	LevelSkillDriverMode_Default,
	LevelSkillDriverMode_Follow,
	LevelSkillDriverMode_Approach,
	LevelSkillDriverMode_Cast,
};

struct LevelRecordSkill;
struct LevelSkillArg;
struct PendingSkill
{
	DEFINE_CLASS(PendingSkill);
	PendingSkill()
	{
		Zero();
	}
	void Zero()
	{
		mode=LevelSkillDriverMode_None;
		grd=LevelSkillGrade_Invalid;
		arg=NULL;
		rangeFollowOverride=-1.0f;
	}
	LevelSkillDriverMode mode;
	LevelSkillType tpSkill;

	LevelSkillTarget target;
	LevelSkillArg *arg;
	LevelSkillGrade grd;
	float rangeFollowOverride;
};

class CLevelSkill;
class CLevelObj;
class CLevelSkillDriver
{
public:
	DEFINE_CLASS(CLevelSkillDriver);
	CLevelSkillDriver()
	{
		Zero();
	}
	~CLevelSkillDriver()
	{
		Clear();
	}


	enum PauseState
	{
		NotPaused,
		SkillPaused,//不允许使用技能,但允许移动
		Paused,//什么都不允许
	};
	void Zero()
	{
		ZeroWorking();
		_owner=NULL;
		_pause=NotPaused;
		_pending=NULL;
		_verCast=0;
	}
	void ZeroWorking()
	{
		_bWorking=0;
		_mode=LevelSkillDriverMode_None;
		_skill=NULL;
		_rec=NULL;
		_skillIntend=NULL;
		_targetObj=NULL;
		_bPassive=FALSE;
		_bContinuous=FALSE;
		_bNoFollow=FALSE;
		_bClosestFollow=0;
		_bFailFollow=0;
		_idFailFollow=LevelObjID_Invalid;
		_rangeFollow=0.0f;
		_grd=LevelSkillGrade_Invalid;
		_arg=NULL;
		_sessionFollow=LevelMoveSession_Invalid;
	}
	void Init(CLevelObj *owner)
	{
		_owner=owner;
	}
	void Clear()
	{
		ClearWorking();
		ClearPending();
	}

	BOOL IsWorking()	{		return _bWorking;	}
	BOOL IsFailFollow()	{		return _bFailFollow;	}
	BOOL IsFailReach()	{		return _bFailFollow&&_bFailFollowCannotReach;	}
	LevelObjID GetFailTarget()	{		return _idFailFollow;	}//IsFailReach()为TRUE时有效
	BYTE GetCastVer()	{		return _verCast;	}

	//bPassive表示,这个技能是否是由客户端启动的,Server的SkillDriver处于被动响应的地位
	//返回是否成功
	BOOL Start(LevelSkillType &tpSkill,LevelSkillTarget &target,BOOL bPassive,ClientSkillID idClient,LevelSkillGrade grd,LevelSkillArg *arg,float rangeFollowOverride=-1.0f);
	BOOL StartFollow(LevelSkillTarget &target,float rangeFollowOverride=-1.0f,BOOL bClosestFollow=TRUE);
	BOOL StartApproach(LevelSkillType &tpSkill,LevelSkillTarget &target,float rangeFollowOverride=-1.0f);//
	BOOL StartCast(LevelSkillType &tpSkill,LevelSkillTarget &target,LevelSkillGrade grd,LevelSkillArg *arg,LevelOpLink *link);//忽略时空条件的Cast
	BOOL CheckStartCast(LevelSkillType &tpSkill,LevelSkillTarget &target,BOOL bIgnoreFaceCheck);
	BOOL StopMove();
	BOOL Combine(LevelSkillTarget &target,ClientSkillID idClient);
	void ClearWorking();

	void ClearPending();

	//暂停/继续
	PauseState GetPauseState()	{		return _pause;	}
	LevelSkillID PauseSkill();//暂停技能的使用,会取消当前正在使用的Skill,返回取消的Skill的id
	void Pause();
	BOOL Continue();

	//返回是否成功
	BOOL Update(AnimTick dt);

	CLevelObj *GetOwner()	{		return _owner;	}
	LevelSkillTarget *GetTarget()	{		return &_target;	}
	LevelSkillArg *GetArg()	{		return _arg;	}
	LevelRecordSkill *GetRec()	{		return _rec;	}
	CLevelSkill *GetSkill()	{		return _skill;	}

	BOOL CheckInRange();//判断_target是否在范围内了
	BOOL CheckInDir(BOOL bIgnoreTol);//判断_target是否在朝向范围内了
	BOOL IsSkillCasting()	{		return _IsSkillCasting();	}
	void StopCast(AnimTick tCasting);

protected:

	BOOL _CanFollow();
	float _CalcCastRange();
	float _CalcCastRange(LevelRecordSkill *rec,CLevelSkill *skill,LevelSkillTarget &target);
	BOOL _CheckCastSpacetime(LevelSkillDriverMode mode,LevelRecordSkill *rec,CLevelObj *loTarget,LevelSkillTarget &target,float range,BOOL bIgnoreFaceCheck,BOOL &bInRange,BOOL &bInDir);
	BOOL _CheckCastSpacetime(BOOL &bInRange,BOOL &bInDir);//检查Cast的时间空间条件(是否在距离范围内,在朝向范围内,CD是否结束). bInRange返回是否在范围之内
	BOOL _CheckInRange(CLevelObj *loTarget,LevelSkillTarget &target,float range);
	BOOL _CheckInDir(BOOL bIgnoreTol,LevelRecordSkill *rec,CLevelObj *loTarget,LevelSkillTarget &target);
	BOOL _DoCastSkill(ClientSkillID idClient,LevelOpLink *link);
	void _DoIgnoreCastSkill();
	void _DoFollow();

	BOOL _CheckLostTarget();
	BOOL _CheckLostTarget(LevelSkillTarget &target,LevelRecordSkill *rec,CLevelObj *loTarget,LevelSkillDriverMode mode);

	BOOL _IsSkillCasting();

	BOOL _Continue();

	BOOL _CheckOwnerCanStartSkill();//检查owner是否因为某种限制不能开始一个技能
	void _SetPending_Default(LevelSkillType &tpSkill,LevelSkillTarget &target,LevelSkillGrade grd,LevelSkillArg *arg,float rangeFollowOverride);
	void _SetPending_Follow(LevelSkillTarget &target,float rangeFollowOverride);
	void _SetPending_Approach(LevelSkillType &tpSkill,LevelSkillTarget &target,float rangeFollowOverride);

	float _CalcDirTol(LevelRecordSkill *rec);

	BOOL _CalcDirTargetPos(LevelPos &pos,CLevelObj *loTarget,LevelSkillTarget &target);

	CLevelObj*_owner;
	PendingSkill *_pending;//等待开始的技能

	LevelRecordSkill *_rec;//如果为NULL,表示我们正在使用一个Dummy技能(Dummy为一个不做任何事情,永远施放失败的技能,主要用在单位的移动上)
	CLevelSkill *_skillIntend;//将要施放的技能,只是一个CLevelSkill的指针,没有初始化,这个指针与_skill不能同时非NULL,
	LevelSkillTarget _target;
	LevelSkillArg *_arg;

	WORD _mode:4;//工作模式
	WORD _bWorking:1;
	WORD _bPassive:1;
	WORD _bContinuous:1;
	WORD _bNoFollow:1;
	WORD _bFailFollow:1;//这个标志表示在最近一次执行技能时的Follow过程中,无法Follow(或者说,跟丢了)
	WORD _bFailFollowCannotReach:1;//_bFailFollow为TRUE时有效,表示无法Follow的原因是因为无法到达目标点
	WORD _bClosestFollow:1;
	LevelSkillGrade _grd;
	BYTE _verCast;//每_DoCastSkill()一次,这个值加1,注意这个值不会被_ClearWorking()归零
	CLevelObj *_targetObj;
	float _rangeFollow;
	LevelObjID _idFailFollow;

	LevelMoveSession _sessionFollow;


	CLevelSkill *_skill;//当前正在施放的技能,如果为空,表示正在Follow中
	PauseState _pause;

};
