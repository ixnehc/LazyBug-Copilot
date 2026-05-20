/********************************************************************
	created:	2020/02/24
	file base:	Buff_CentipedeNode_Tendrils
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"

#include "LoCentipede.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "Buff_CentipedeNode_Tendrils.h"

#include "LevelPlayerStates.h"

#include "Deal_MakeBuff.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_CentipedeNode_Tendrils

BIND_BUFFPARAM(Buff_CentipedeNode_Tendrils,BuffParam_CentipedeNode_Tendrils,BuffArg_CentipedeNode_Tendrils);



void Buff_CentipedeNode_Tendrils::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_CentipedeNode_Tendrils *arg=(BuffArg_CentipedeNode_Tendrils *)arg0;

	CLevelObj *owner=_GetOwner();

	BuffParam_CentipedeNode_Tendrils*param=_rec->GetParam<BuffParam_CentipedeNode_Tendrils>();
	if (param)
	{
		extern LevelObjID LevelUtil_GetLevelObjIDFromVar(CLevelObj *owner,StringID nm);
		_idAgent=LevelUtil_GetLevelObjIDFromVar(owner,param->varCentipedeAgent);
	}

	if (_idAgent!=LevelObjID_Invalid)
	{
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *lo=LevelUtil_GetAliveLo(_GetLevel(),_idAgent);
		if (lo)
		{
			if (lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
			{
				CLoCentipede *loCentipede=(CLoCentipede *)lo;
				loCentipede->GetNodeIndex(_GetOwner()->GetID(),_iNode);
			}
		}
	}

	if (param->dealDisappear)
	{
		if (param->dealDisappear->GetClass()->IsSameWith(Class_Ptr2(Deal_MakeBuff)))
		{
			Deal_MakeBuff *dealMakeBuff=(Deal_MakeBuff *)param->dealDisappear;
			_idDisappearBuff=dealMakeBuff->idBuff;
		}
	}

}

void Buff_CentipedeNode_Tendrils::_OnDestroy()
{
}

void Buff_CentipedeNode_Tendrils::_OnUpdate(AnimTick dt)
{

}

void Buff_CentipedeNode_Tendrils::_WriteData(CBitPacket *bp)
{
	bp->Data_EncodeDword(_iNode);
}

void Buff_CentipedeNode_Tendrils::HandleEvent(LevelEvent &e0)
{
	BuffParam_CentipedeNode_Tendrils*param=_rec->GetParam<BuffParam_CentipedeNode_Tendrils>();
	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage *e=(LePreDamage*)&e0;
		if (e->loTarget==_GetOwner())
		{
			if (_idDisappearBuff!=RecordID_Invalid)
			{
				extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
				if (!LevelUtil_FindBuffByRecordID(e->loTarget,_idDisappearBuff))
				{
					if (!e->bStun)
					{
						e->bAbandon=TRUE;
						e->bHandled=TRUE;
					}
				}
			}
		}
		return;
	}

	if (e0.GetType()==LET_PostDamage)
	{
		LePostDamage *e=(LePostDamage *)&e0;

		if (e->loTarget==_GetOwner())
		{
			LevelAttr_Base *attr=e->loTarget->GetAttr_Base();
			if (attr)
			{
				if (attr->hp.GetRatio()<param->hpratioToDisappear)
				{
					DealArg arg;
					arg.link=e->link;
					param->dealDisappear->Make(LevelOSB(this),_GetOwner(),arg,NULL);
				}
			}
		}
		return;
	}

	__super::HandleEvent(e0);

}

void Buff_CentipedeNode_Tendrils::SetOwnerBroken()
{
	_bOwnerBroken=TRUE;
	_tOwnerBroken=_tAge;
}

BOOL Buff_CentipedeNode_Tendrils::IsDisappear()
{
	extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
	if (LevelUtil_FindBuffByRecordID(_GetOwner(),_idDisappearBuff))
		return TRUE;
	return FALSE;
}
