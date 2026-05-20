/********************************************************************
	created:	2022/07/02 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordBuff.h"
#include "LevelRtnus.h"
#include "LoUnit.h"
#include "LevelUtil.h"

#include "BgnFireFly.h"

#include "Buff_FireFly.h"

//////////////////////////////////////////////////////////////////////////
//CBgp_FireFly_CheckFleeing
void CBgp_FireFly_CheckFleeing::FillDesc(std::string &s,FillDescAssist *assist)
{
	s="(FireFly)检测自己是否在Flee";
}


////////////////////////////////////////////////////////////////////////
//CBgn_FireFly_CheckFleeing

BIND_BGN_CLASS(CBgn_FireFly_CheckFleeing,CBgp_FireFly_CheckFleeing);

void CBgn_FireFly_CheckFleeing::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FireFly_CheckFleeing*pad=_GetPad<CBgp_FireFly_CheckFleeing>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	Buff_FireFly *buff=(Buff_FireFly *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_FireFly));
	if (buff)
	{
		if (buff->IsFleeing())
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}



////////////////////////////////////////////////////////////////////////
//CBgn_FireFly_GetFleePos

BIND_BGN_CLASS(CBgn_FireFly_GetFleePos,CBgp_FireFly_GetFleePos);

void CBgn_FireFly_GetFleePos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FireFly_GetFleePos*pad=_GetPad<CBgp_FireFly_GetFleePos>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if(pad->_varPos!=StringID_Invalid)
	{
		Buff_FireFly *buff=(Buff_FireFly *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_FireFly));
		if (buff)
		{
			LevelPos posFlee;
			if (buff->GetCurFleePos(posFlee))
			{
				_SetPos(pad->_varPos,posFlee);

				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}




////////////////////////////////////////////////////////////////////////
//CBgnGA_AbsorbFireFly

BIND_BGN_CLASS(CBgnGA_AbsorbFireFly,CBgpGA_AbsorbFireFly);

void CBgnGA_AbsorbFireFly::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_AbsorbFireFly*pad=_GetPad<CBgpGA_AbsorbFireFly>();
	CLevelObj *loMe=_GetLo();

	CLevelPlayer *player=_GetTalkPlayer();
	if (player)
	{
		CLevelRtnus *rtnus=player->GetRtnus();
		if (rtnus)
		{
			DWORD c;
			CLevelRtnu **buf=rtnus->GetValidRetinues(c);
			for (int i=0;i<c;i++)
			{
				CLevelObj *lo=(CLevelObj *)buf[i]->GetLo();
				if (lo)
				{
					Buff_FireFly *buff=(Buff_FireFly *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_FireFly));
					if (buff)
					{
						CLevelObjSrc *los=loMe->GetLos();
						if (los)
						{
							if (buff->EnterTorch(loMe->GetID(),pad->guides,los->GetMat()))
							{
								_OutputOk(outputs,1,"成功");
								return;
							}
						}
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}


////////////////////////////////////////////////////////////////////////
//CBgnGA_FireFlyTorchSpawn

BIND_BGN_CLASS(CBgnGA_FireFlyTorchSpawn,CBgpGA_FireFlyTorchSpawn);

void CBgnGA_FireFlyTorchSpawn::_Spawn()
{
	CBgpGA_FireFlyTorchSpawn*pad=_GetPad<CBgpGA_FireFlyTorchSpawn>();
	CLevelObj *loMe=_GetLo();
	CLevel *level=_GetLevel();

	if (pad->idUnit!=RecordID_Invalid&&pad->idBuff!=RecordID_Invalid)
	{
		CLoUnit* lo=NULL;
		lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
		lo->PostCreate(LevelPlayerID_PlayerWild,NULL,pad->idUnit,1,NULL,EquipSetPick_None,loMe->GetFramePos(),0.0f);
		level->AddToActives(lo);

		LevelBuffID idBuff=level->GetDecider()->MakeBuff(lo,pad->idBuff,ANIMTICK_INFINITE,NULL,FALSE);
		CLevelBuff *buff=LevelUtil_FindBuffByID(lo,idBuff);
		if (buff)
		{
			if (buff->GetClass()->IsSameWith(Class_Ptr2(Buff_FireFly)))
			{
				CLevelObjSrc *los=loMe->GetLos();
				if (los)
					((Buff_FireFly*)buff)->SetInTorch(pad->guides,los->GetMat());

			}
		}

		_idCur=lo->GetID();
		SAFE_RELEASE(lo);
	}

}


void CBgnGA_FireFlyTorchSpawn::Start(DWORD iStb,BGNOutputs &outputs)
{
	_Spawn();
}

//是否有flash fly在torch里
BOOL CBgnGA_FireFlyTorchSpawn::_CheckExistInTorch()
{
	if (_idCur!=LevelObjID_Invalid)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_GetLevel(),_idCur);
		if (lo)
		{
			Buff_FireFly *buff=(Buff_FireFly *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_FireFly));
			if (buff)
			{
				if (buff->IsInTorch())
					return TRUE;
			}
		}
	}

	return FALSE;
}


void CBgnGA_FireFlyTorchSpawn::Update(BGNOutputs &outputs)
{
	if (!_CheckExistInTorch())
		_Spawn();
}
