/********************************************************************
	created:	2020/01/31 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"

#include "behaviorgraph/BehaviorMem.h"

#include "Buff_CentipedeNode_Crouch.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_CentipedeNode_Crouch
BIND_BUFFPARAM(Buff_CentipedeNode_Crouch,BuffParam_CentipedeNode_Crouch,BuffArg_CentipedeNode_Crouch);

LevelBuffMask Buff_CentipedeNode_Crouch::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

void Buff_CentipedeNode_Crouch::_OnCreate(LevelBuffArg *arg0)
{
	BuffParam_CentipedeNode_Crouch*param=_rec->GetParam<BuffParam_CentipedeNode_Crouch>();

	extern LevelObjID LevelUtil_GetLevelObjIDFromVar(CLevelObj *owner,StringID nm);
	_idAgent=LevelUtil_GetLevelObjIDFromVar(_GetOwner(),param->varCentipedeAgent);

}

void Buff_CentipedeNode_Crouch::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idAgent);
}
