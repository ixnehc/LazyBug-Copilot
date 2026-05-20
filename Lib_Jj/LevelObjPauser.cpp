
#include "stdh.h"
#include "Level.h"

#include "LevelObjPauser.h"
#include "LevelObjMove.h"
#include "LevelSkillDriver.h"

#include "LevelPlayer.h"



//////////////////////////////////////////////////////////////////////////
//LevelObjPauserCore
void LevelObjPauserCore::Init(CLevelObj *lo)	
{		
	_lo=lo;	
	_driver=lo->GetSkillDriver();
}

LevelSkillID LevelObjPauserCore::Pause()
{
	if (!_driver)
		return LevelSkillID_Invalid;

	LevelSkillID idBroken=LevelSkillID_Invalid;

	//立即暂停技能
	if (_driver)
		idBroken=_driver->PauseSkill();

	//隔一段时间后再暂停移动
	_tPauseMove=_lo->GetT()+GetDelay();
	_bMovePaused=FALSE;

	_bWorking=TRUE;

	return idBroken;
}

LevelSkillID LevelObjPauserCore::PauseNoDelay()
{
	if (!_driver)
		return LevelSkillID_Invalid;

	LevelSkillID idBroken=LevelSkillID_Invalid;

	if (_driver)
	{
		idBroken=_driver->PauseSkill();
		_driver->Pause();
		_driver->ClearWorking();

		_driver->Continue();
	}

	return idBroken;
}


LevelSkillID LevelObjPauserCore::Teleport(LevelPos &posTeleport,float faceTeleport,LevelTeleportID idTeleport)
{
	LevelSkillID idBroken=Pause();
	if (_bWorking)
	{
		_bTeleport=1;
		_methodToSwitch=LevelMoveMethod_None;

		_posTeleport.set(posTeleport.x,0.0f,posTeleport.y);
		_faceTeleport=faceTeleport;
		_idTeleport=idTeleport;
	}

	return idBroken;
}

LevelSkillID LevelObjPauserCore::TeleportResided(LevelPos3D &posReside,LevelTeleportID idTeleport)
{
	LevelSkillID idBroken=Pause();
	if (_bWorking)
	{
		_bTeleport=1;
		_methodToSwitch=LevelMoveMethod_Resided_;

		_posTeleport=posReside;
		_idTeleport=idTeleport;
	}

	return idBroken;
}

LevelSkillID LevelObjPauserCore::TeleportMount(LevelObjID idMountTarget,float htMount,LevelTeleportID idTeleport)
{
	LevelSkillID idBroken=Pause();
	if (_bWorking)
	{
		_bTeleport=1;
		_methodToSwitch=LevelMoveMethod_Mount;

		_idMountTarget=idMountTarget;
		_htMount=htMount;
		_idTeleport=idTeleport;
	}

	return idBroken;
}


LevelSkillID LevelObjPauserCore::TeleportGround(LevelPos &posGround,float face,LevelTeleportID idTeleport)
{
	LevelSkillID idBroken=Pause();
	if (_bWorking)
	{
		_bTeleport=1;
		_methodToSwitch=LevelMoveMethod_Ground;

		_posTeleport.set(posGround.x,0.0f,posGround.y);
		_faceTeleport=face;
		_idTeleport=idTeleport;
	}

	return idBroken;
}

LevelSkillID LevelObjPauserCore::TeleportFlying(LevelPos3D &posFlying,LevelTeleportID idTeleport)
{
	LevelSkillID idBroken=Pause();
	if (_bWorking)
	{
		_bTeleport=1;
		_methodToSwitch=LevelMoveMethod_Flying;

		_posTeleport=posFlying;
		_faceTeleport=0.0f;
		_idTeleport=idTeleport;
	}

	return idBroken;
}


void LevelObjPauserCore::Update()
{
	if (!_bWorking)
		return;

	if (!_bMovePaused)
	{
		if (_lo->GetT()>=_tPauseMove)
		{
			CLevelSkillDriver *driver=_lo->GetSkillDriver();
			if (driver)
			{
				driver->Pause();
				driver->ClearWorking();

				if (_bTeleport)
				{
					CLevelObjMove *move=_lo->GetMove();
					if (move)
					{
						switch(_methodToSwitch)
						{
							case LevelMoveMethod_None:
								move->Teleport(_idTeleport,LevelPos(_posTeleport.x,_posTeleport.z),_faceTeleport);
								break;
							case LevelMoveMethod_Resided_:
								move->SwitchResided(_posTeleport,_idTeleport);
								break;
							case LevelMoveMethod_Ground:
								move->SwitchGround(LevelPos(_posTeleport.x,_posTeleport.z),_faceTeleport,_idTeleport);
								break;
							case LevelMoveMethod_Flying:
								move->SwitchFlying(_posTeleport,_faceTeleport,_idTeleport);
								break;
							case LevelMoveMethod_Mount:
								move->SwitchMount(_idMountTarget,_htMount,_idTeleport);
								break;
						}
					}
				}


				if (_lo->IsPlayer())
				{
					CLevelPlayer *player=_lo->GetLevel()->GetPlayer(_lo->GetPlayerID());
					if (player)
					{
						player->GetMove().PauseMove();
						if(_bTeleport)
							player->GetMove().AuthorizeTeleport(_idTeleport,LevelPos(_posTeleport.x,_posTeleport.z));
					}
				}
			}
			_bMovePaused=TRUE;
		}
	}
	else
	{
		CLevelBuffs *buffs=_lo->GetBuffs();
		BOOL bPausing=FALSE;
		if (buffs)
			bPausing=buffs->TestFlag(BuffFlag_Pausing);

		if (!bPausing)
		{
			_driver->Continue();

			//结束工作
			_bWorking=FALSE;
			_bTeleport=FALSE;
			_bMovePaused=FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//CLevelObjPauser
void CLevelObjPauser::Init(CLevelObj *owner)
{
	_owner=owner;
}

void CLevelObjPauser::Clear()
{
	Safe_Class_Delete(_core);

}

void CLevelObjPauser::_EnsureCore()
{
	if (_core)
		return;
	_core=Class_New2(LevelObjPauserCore);
	_core->Init(_owner);
}

