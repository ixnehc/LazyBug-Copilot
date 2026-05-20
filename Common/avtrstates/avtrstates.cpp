/********************************************************************
	created:	06/11/2009
	filename: 	avtrstates
	author:		chenxi
	
	purpose:	avatar states
*********************************************************************/
#include "stdh.h"
#include "avtrstates.h"

#include "stringparser/stringparser.h"

#include "../enums/enums.h"
#include "../Log/LogDump.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CCharStates




BOOL CAvtrStates::cmd_Reset(BOOL bFlying)
{
	Clear();
	_states.Add(AVS_NotMove);
	_states.Add(AVS_NotTurn);
	_states.Add(AVS_NotJump);
	_states.Add(AVS_NotAct_0);
	_states.Add(AVS_NotAct_1);
	_states.Add(AVS_NotAct_2);
	_states.Add(AVS_NotAct_3);
	_states.Add(AVS_NotFly);
	_states.Add(AVS_NotMount);
	_states.Add(AVS_NotImpel);
	_states.Add(AVS_NotReside);
	if (bFlying)
	{
		_states.Remove(AVS_NotFly);
		_states.Add(AVS_Flying);
	}

	_cdata.speedMove=3.0f;
	_cdata.speedEuler.set(3.14f,1.57f,0.0f);//每秒180度

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_SetShift(BOOL bShift,i_math::vector3df &euler)
{
	_cdata.bShiftMove=bShift;
	if (bShift)
		_cdata.eulerShift=euler;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_SetMoveSpeed(float speed)
{
	_cdata.speedMove=speed;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::_cmd_Move(i_math::vector3df *speed)
{
	_states.Remove(AVS_NotMove,AVS_MoveTo,AVS_MoveTeleport);
	_states.Add(AVS_Move);

	_bModified=TRUE;
	_cmds.bMove=1;
	return TRUE;
}


BOOL CAvtrStates::_cmd_MoveTo(i_math::vector3df &pos,BOOL bNeverStop,BOOL bSlide)
{
	if (!bSlide)
		_states.Remove(AVS_NotMove,AVS_Move,AVS_TurnTo,AVS_MoveTeleport);
	else
		_states.Remove(AVS_NotMove,AVS_Move,AVS_MoveTeleport);

	_states.Add(AVS_MoveTo);

	_cdata.posTarget=pos;
	_cdata.bMoveToNeverStop=bNeverStop;
	_cdata.bSlideMove=bSlide;

	_bModified=TRUE;
	_cmds.bMoveTo=1;
	return TRUE;
}


BOOL CAvtrStates::cmd_StopMove()
{
	_states.Remove(AVS_Move,AVS_MoveTo,AVS_MoveTeleport);
	_states.Add(AVS_NotMove);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_MoveTeleport(i_math::vector3df &pos)
{
	_states.Remove(AVS_Move,AVS_MoveTo,AVS_NotMove);
	_states.Add(AVS_MoveTeleport);

	_cdata.posTarget=pos;

	_bModified=TRUE;
	return TRUE;

}


BOOL CAvtrStates::cmd_MoveReached()
{
	_states.Remove(AVS_Move,AVS_MoveTo);
	_states.Add(AVS_NotMove);

	_bModified=TRUE;
	return TRUE;
}



BOOL CAvtrStates::cmd_SetTurnSpeed(i_math::vector3df &euler)
{
	_cdata.speedEuler=euler;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_Turn()
{
	_states.Remove(AVS_NotTurn,AVS_TurnTo);
	_states.Add(AVS_Turn);

	_bModified=TRUE;
	_cmds.bTurn=1;
	return TRUE;
}

BOOL CAvtrStates::cmd_TurnTo(i_math::vector3df &euler)
{
	_states.Remove(AVS_NotTurn,AVS_Turn);
	_states.Add(AVS_TurnTo);

	_cdata.eulerTarget=euler;

	_bModified=TRUE;
	_cmds.bTurnTo=1;
	return TRUE;
}

BOOL CAvtrStates::cmd_StopTurn()
{
	_states.Remove(AVS_Turn,AVS_TurnTo);
	_states.Add(AVS_NotTurn);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_TurnReached()
{
	_states.Remove(AVS_Turn,AVS_TurnTo);
	_states.Add(AVS_NotTurn);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_SetJumpSpeed(float speedUp)
{
	_cdata.speedJump=speedUp;
	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_Jump()
{
	_states.Remove(AVS_NotJump,AVS_JumpLand);
	_states.Add(AVS_Jump);
	_cdata.jmpver++;

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_JumpPreLand(AnimTick t) 
{
	_states.Remove(AVS_NotJump,AVS_Jump);
	_states.Add(AVS_JumpLand);

	_cdata.tJumpLand=t;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_JumpLanded()
{
	_states.Remove(AVS_JumpLand,AVS_Jump);
	_states.Add(AVS_NotJump);

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_Act(StringID acttype,int actsub,AnimTick tActEnd)
{
	_states.Remove((AvtrState)(AVS_NotAct_0));
	_states.Add((AvtrState)(AVS_Act_0));

// 	if (_cdata.acttype==29229440)
// 	{
// 		int v=0;
// 		v++;
// 	}

	_cdata.acttype=acttype;
	_cdata.actsub=actsub;
	_cdata.tActEnd=tActEnd;//记录结束时间
	_cdata.actver++;//版本号加1

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_EndAct(AnimTick tActEnd)
{
	_cdata.tActEnd=tActEnd;
	_bModified=TRUE;
	return TRUE;
}



BOOL CAvtrStates::cmd_StopAct()
{
	_states.Remove((AvtrState)(AVS_Act_0));	
	_states.Add((AvtrState)(AVS_NotAct_0));

// 	if (_cdata.acttype==29229440)
// 	{
// 		int v=0;
// 		v++;
// 	}

	_cdata.acttype=StringID_Invalid;
	_cdata.actsub=-1;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_BreakAct()
{
	_states.Remove((AvtrState)(AVS_Act_0));
	_states.Add((AvtrState)(AVS_NotAct_0));

// 	if (_cdata.acttype==29229440)
// 	{
// 		int v=0;
// 		v++;
// 	}

	_cdata.acttype=StringID_Invalid;
	_cdata.actsub=-1;
	_cdata.actver++;//版本号加1

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_SetPosture(StringID pst)
{
	if (_cdata.posture==pst)
		return TRUE;
	_cdata.posture=pst;
	_cdata.pstver++;
	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_StartFly(AnimTick t)
{
	_states.Remove(AVS_NotFly,AVS_Flying,AVS_FlyDescend,AVS_FlyDown);
	_states.Add(AVS_FlyUp);

	_cdata.tFlyUp=t;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_FliedUp()
{
	_states.Remove(AVS_NotFly,AVS_FlyDescend,AVS_FlyDown,AVS_FlyUp);
	_states.Add(AVS_Flying);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_EndFly()
{
	_states.Remove(AVS_NotFly,AVS_Flying,AVS_FlyDown,AVS_FlyUp);
	_states.Add(AVS_FlyDescend);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_FlyDescended(AnimTick t)
{
	_states.Remove(AVS_NotFly,AVS_Flying,AVS_FlyDescend,AVS_FlyUp);
	_states.Add(AVS_FlyDown);
	_cdata.tFlyDown=t;

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_FliedDown()
{
	_states.Remove(AVS_Flying,AVS_FlyDescend,AVS_FlyDown,AVS_FlyUp);
	_states.Add(AVS_NotFly);

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_Mount()
{
	_states.Remove(AVS_NotMount);
	_states.Add(AVS_Mounting);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_StopMount()
{
	_states.Remove(AVS_Mounting);
	_states.Add(AVS_NotMount);

	_bModified=TRUE;
	return TRUE;
}


BOOL CAvtrStates::cmd_Impel()
{
	_states.Remove(AVS_NotImpel);
	_states.Add(AVS_Impel);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_StopImpel()
{
	_states.Remove(AVS_Impel);
	_states.Add(AVS_NotImpel);

	_bModified=TRUE;
	return TRUE;
}

BOOL CAvtrStates::cmd_Reside()
{
	_states.Remove(AVS_NotReside);
	_states.Add(AVS_Reside);

	_bModified=TRUE;
	return TRUE;

}
BOOL CAvtrStates::cmd_StopReside()
{
	_states.Remove(AVS_Reside);
	_states.Add(AVS_NotReside);

	_bModified=TRUE;
	return TRUE;
}
