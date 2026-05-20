
#pragma once

#include "avtrstates_defines.h"
#include "avtrstatesanchor.h"

#include "../strlib/strlib.h"
#include "../anim/animbase.h"



struct AvtrCtrlData
{
	AvtrCtrlData()
	{
		Zero();
	};
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}

	StringID posture;//姿态
	DWORD pstver;//每更换一个posture,这个值加1

	DWORD jmpver;//每次跳一下,这个值加1

	StringID acttype;//act类型
	short actsub;//act子动作
	DWORD actver;//act版本号,每执行一个新的act时,这个值加1

	float size;//角色的大小
	float scale;//角色的缩放

	float speedMove;//移动时的绝对速度
	i_math::vector3df speedEuler;//旋转速度
	float speedJump;//起跳的瞬间速度

	DWORD bMoveToNeverStop:1;//是否为NeverStop 模式的move to,所谓NeverStop模式,是指,即使走到了目的地,仍然要尝试着走(不停),不切换到NotMove状态
	DWORD bShiftMove:1;//移动的时候不改变旋转
	DWORD bSlideMove:1;//移动的时候不播放移动的动画,也不旋转到目的地

	i_math::vector3df posTarget;//移动到的目的点
	i_math::vector3df eulerTarget;//旋转到的角度
	i_math::vector3df eulerShift;//朝什么方向上进行shift move

	AnimTick tFlyUp;
	AnimTick tFlyDown;
	AnimTick tJumpLand;//开始tJumpLand的时间
	AnimTick tUnSit;//开始UnSit的时间
	AnimTick tUnKO;//开始UnKO的时间
	AnimTick tActEnd;//当前Act要持续到什么时候

};

struct AvtrStateData
{
	AvtrStateData()
	{
		Zero();
	}
	void Zero()
	{
		speedoff=0.0f;
		timerate=1.0f;
		stage_=AvtrMoveStage_None;
		verStage=0;
		rotoffStart=0.0f;
		rotoffMove=0.0f;
		rotoffOnSpot=0.0f;
		footstep=AvtrFootStep_None;
	}

	float speedoff;//移动方向相对于朝向的夹角
	float timerate;
	AvtrMoveStage stage_;
	DWORD verStage;
	AvtrFootStep footstep;
	float rotoffStart;//转身角度(起步)
	float rotoffMove;//转身角度(移动)
	float rotoffOnSpot;//转身角度(原地转身)
};

class CAvtrStates
{
public:
	DEFINE_CLASS(CAvtrStates);
	IMPLEMENT_REFCOUNT_C
	CAvtrStates()
	{
		Zero();
		_bModified=FALSE;
		_cmds.Zero();
	}

	~CAvtrStates()
	{
		Clear();
	}

	void Zero()
	{
		_states.Reset();
		_statesOld=_states;
	};


	void Clear()
	{
		_anchor.Clear();
		_cdata.Zero();
		_data.Zero();
		Zero();
	}


	//命令
	BOOL cmd_Reset(BOOL bFlying);
	BOOL cmd_SetMoveSpeed(float speed);
	BOOL cmd_SetShift(BOOL bShift,i_math::vector3df &euler);
	BOOL cmd_Move()	{		return _cmd_Move(NULL);	}
	BOOL cmd_MoveTo(i_math::vector3df &pos,BOOL bNeverStop=FALSE,BOOL bSlide=FALSE)	{		return _cmd_MoveTo(pos,bNeverStop,bSlide);	}
	BOOL cmd_MoveTeleport(i_math::vector3df &pos);
	BOOL cmd_StopMove();
	BOOL cmd_MoveReached();
	BOOL cmd_SetTurnSpeed(i_math::vector3df &euler);
	BOOL cmd_Turn();//speed的单位是(弧度/s),以顺时针为正
	BOOL cmd_TurnTo(i_math::vector3df &euler);
	BOOL cmd_StopTurn();
	BOOL cmd_TurnReached();

	BOOL cmd_StartFly(AnimTick t);
	BOOL cmd_FliedUp();//起飞结束
	BOOL cmd_EndFly();
	BOOL cmd_FlyDescended(AnimTick t);//着陆前的下落过程结束了
	BOOL cmd_FliedDown();//着陆结束

	BOOL cmd_SetJumpSpeed(float speedUp);
	BOOL cmd_Jump();
	BOOL cmd_JumpPreLand(AnimTick t);
	BOOL cmd_JumpLanded();//物理着地

	BOOL cmd_Act(StringID acttype,int actsub,AnimTick tActEnd);
	BOOL cmd_EndAct(AnimTick tActEnd);
	BOOL cmd_StopAct();
	BOOL cmd_BreakAct();

	BOOL cmd_SetPosture(StringID pst);

	BOOL cmd_Mount();
	BOOL cmd_StopMount();

	BOOL cmd_Impel();
	BOOL cmd_StopImpel();

	BOOL cmd_Reside();
	BOOL cmd_StopReside();

	BOOL IsAct()	{		return _IsAct();	}
	BOOL IsMoving()	{		return _IsMoving();	}

	AvtrCtrlData*GetCData()	{		return &_cdata;	}
	AvtrStateData*GetData()	{		return &_data;	}
	void BeginCmd()
	{		
		_statesOld=_states;
	}
	void EndCmd()	
	{		
		if (_bModified)
		{
			StateChange change;
			change.old=_statesOld;
			change.cur=_states;
			change.avs=this;
			change.cmds=_cmds;
			_anchor.NotifyChange(change);
			_bModified=FALSE;
			_cmds.Zero();
		}

	}

	AvtrStateMask &GetStatesMask()	{		return _states;	}
	AvtrStatesAnchor *GetAnchor()	{		return &_anchor;	}


protected:
	BOOL _TestOccupy(AvtrState state,AvtrStateMask statesCur);
	AvtrStateMask _GetDiscards(AvtrState state);


	BOOL _IsMoving()
	{
		return !_states.Test(AVS_NotMove);
	}
	BOOL _IsRotating()
	{
		return !_states.Test(AVS_NotTurn);
	}
	BOOL _IsTargetMoving()	
	{		
		return _states.Test(AVS_MoveTo);
	}
	BOOL _IsTargetRotating()	
	{
		return _states.Test(AVS_TurnTo);
	}


	BOOL _IsAct()
	{
		return _states.Test(AVS_Act_0,AVS_Act_1,AVS_Act_2,AVS_Act_3);
	}
	BOOL _IsFlying()
	{
		return !_states.Test(AVS_NotFly);
	}

	//正在起飞或者降落
	BOOL _IsSwitchingFly()
	{
		return (_states.Test(AVS_FlyUp)||
			_states.Test(AVS_FlyDown)||
			_states.Test(AVS_FlyDescend));
	}


	BOOL _cmd_Move(i_math::vector3df *speed);
	BOOL _cmd_MoveTo(i_math::vector3df &pos,BOOL bNeverStop,BOOL bSlide);


	//core _states
	AvtrStateMask _states;
	AvtrCtrlData _cdata;
	AvtrStateData _data;

	//for change track
	BOOL _bModified;
	AvtrCmdFlags _cmds;//跟踪调用了哪些命令
	AvtrStateMask _statesOld;
	AvtrStatesAnchor _anchor;
};

