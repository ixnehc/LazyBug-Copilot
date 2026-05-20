
#include "stdh.h"

#include "LevelSkillDriver.h"
#include "LevelRecordBuff.h"

#include "LevelAttrs_Weak.h"


#include "Buff_Stun.h"
#include "LevelObjPauser.h"

#include "datapacket/BitPacket.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Stun
BIND_BUFFPARAM(Buff_Stun,BuffParam_Stun,BuffArg_Stun);

void Buff_Stun::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Stun *arg=(BuffArg_Stun *)arg0;
	_strike=arg->strike;

	extern BOOL LevelUtil_AddStunSrc(CLevelBuff *buff);
	LevelUtil_AddStunSrc(this);

	CLevelObjPauser *pauser=_GetOwner()->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idBroken=pauser->Pause();
	}

	extern float LevelUtil_CalcCurPainRatio(CLevelObj *lo);
	if(LevelUtil_CalcCurPainRatio(_GetOwner())<1.0f)
	{
		//继承旧的弱点
		extern void LevelUtil_TakeOverWeaksOverride(CLevelBuff *buff);
		LevelUtil_TakeOverWeaksOverride(this);
	}

}

void Buff_Stun::_OnDestroy()
{
	extern BOOL LevelUtil_NotififyStunSrc_Finish(CLevelBuff *buff);
	LevelUtil_NotififyStunSrc_Finish(this);

	extern void LevelUtil_ClearWeaksOverride(CLevelBuff *buff);
	LevelUtil_ClearWeaksOverride(this);

}


void Buff_Stun::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimple(_idBroken);
	_strike.Save(dp);
}
