
#include "stdh.h"

#include "LevelSkillDriver.h"
#include "LevelRecordBuff.h"

#include "LevelAttrs_Weak.h"


#include "Buff_SkillStun.h"
#include "LevelObjPauser.h"

#include "datapacket/BitPacket.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_SkillStun
BIND_BUFFPARAM(Buff_SkillStun,BuffParam_SkillStun,BuffArg_SkillStun);

void Buff_SkillStun::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_SkillStun *arg=(BuffArg_SkillStun *)arg0;

	extern BOOL LevelUtil_AddStunSrc(CLevelBuff *buff);
	LevelUtil_AddStunSrc(this);

	CLevelObjPauser *pauser=_GetOwner()->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idBroken=pauser->PauseNoDelay();
	}


}

void Buff_SkillStun::_OnDestroy()
{
}


void Buff_SkillStun::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimple(_idBroken);
}
