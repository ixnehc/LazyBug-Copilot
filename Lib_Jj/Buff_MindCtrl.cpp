/********************************************************************
	created:	2012/01/17
	file base:	Buff_Retinue
	author:		cxi
	
	purpose:	死亡的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_MindCtrl.h"

#include "Buff_KB.h"
#include "Buff_KD.h"

//////////////////////////////////////////////////////////////////////////
//Buff_Retinue

LevelBuffMask Buff_MindCtrl::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_MindCtrl)->GetUID());

	return mask;
}

void Buff_MindCtrl::_OnCreate(LevelBuffArg *param0)
{
	BuffArg_MindCtrl *param=(BuffArg_MindCtrl *)param0;

	CLevelObj *owner=_GetOwner();

	_dur=ANIMTICK_INFINITE;
	_data.idPlayer=param->idPlayer;
}

void Buff_MindCtrl::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}
