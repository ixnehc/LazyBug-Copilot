#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"

#include "LevelObj.h"



struct LevelObjPauserCore
{
public:
	DEFINE_CLASS(LevelObjPauserCore);
	LevelObjPauserCore()
	{
		_lo=NULL;
		_driver=NULL;
		_bWorking=0;
		_bMovePaused=0;
		_bTeleport=0;
		_methodToSwitch=LevelMoveMethod_None;
		_tPauseMove=0;
		_idTeleport=LevelTeleportID_Invalid;
		_faceTeleport=0.0f;
		_idMountTarget=LevelObjID_Invalid;
		_htMount=0.0f;
	}

	void Init(CLevelObj *lo);
	LevelSkillID Pause();
	LevelSkillID PauseNoDelay();
	LevelSkillID Teleport(LevelPos &posTeleport,float face,LevelTeleportID idTeleport);
	LevelSkillID TeleportResided(LevelPos3D &posReside,LevelTeleportID idTeleport);
	LevelSkillID TeleportMount(LevelObjID idMountTarget,float ht,LevelTeleportID idTeleport);
	LevelSkillID TeleportGround(LevelPos &posGround,float face,LevelTeleportID idTeleport);
	LevelSkillID TeleportFlying(LevelPos3D &posFlying,LevelTeleportID idTeleport);
	BOOL IsWorking()	{		return _bWorking;	}
	BOOL IsMovePaused()	{		return _bMovePaused;	}
// 	BOOL IsTeleport()	{		return _bWorking?_bTeleport:0;	}
// 	LevelPos &GetTeleportPos()	{		return _posTeleport;	}
// 	LevelTeleportID GetTeleportID()	
// 	{		
// 		if (_bWorking&&_bTeleport)
// 			return _idTeleport;
// 		return LevelTeleportID_Invalid;
// 	}

	void Update();
	LevelTick GetDelay()	{		return LEVEL_FRAME_TICK;	}

protected:
	CLevelObj *_lo;
	CLevelSkillDriver *_driver;
	DWORD _bWorking:1;
	DWORD _bMovePaused:1;
	DWORD _bTeleport:1;
	LevelMoveMethod _methodToSwitch;//是否要切换method
	LevelPos3D _posTeleport;
	float _faceTeleport;
	LevelObjID _idMountTarget;
	float _htMount;
	LevelTeleportID _idTeleport;
	LevelTick _tPauseMove;

};

class CLevelObjPauser
{
public:
	CLevelObjPauser()
	{
		_core=NULL;
		_owner=NULL;
	}
	void Init(CLevelObj *owner);
	void Clear();

	LevelSkillID Pause()
	{
		_EnsureCore();
		return _core->Pause();
	}
	LevelSkillID PauseNoDelay()
	{
		_EnsureCore();
		return _core->PauseNoDelay();
	}

	LevelSkillID Teleport(LevelPos &posTeleport,float face,LevelTeleportID idTeleport)
	{
		_EnsureCore();
		return _core->Teleport(posTeleport,face,idTeleport);
	}
	LevelSkillID TeleportResided(LevelPos3D &posReside,LevelTeleportID idTeleport)//瞬移并进入驻留状态
	{
		_EnsureCore();
		return _core->TeleportResided(posReside,idTeleport);
	}
	LevelSkillID TeleportGround(LevelPos &posGround,float face,LevelTeleportID idTeleport)//瞬移并进入地面行走状态
	{
		_EnsureCore();
		return _core->TeleportGround(posGround,face,idTeleport);
	}
	LevelSkillID TeleportFlying(LevelPos3D &posFlying,LevelTeleportID idTeleport)//瞬移并进入飞行状态
	{
		_EnsureCore();
		return _core->TeleportFlying(posFlying,idTeleport);
	}
	LevelSkillID TeleportMount(LevelObjID idMountTarget,float ht,LevelTeleportID idTeleport)//瞬移并进入骑行状态
	{
		_EnsureCore();
		return _core->TeleportMount(idMountTarget,ht,idTeleport);
	}
	void Update()
	{
		if (_core)
		{
			_core->Update();
			if (!_core->IsWorking())
				Safe_Class_Delete(_core);
		}
	}
	BOOL IsWorking()	{		return _core?_core->IsWorking():FALSE;	}
	BOOL IsMovePaused()	{		return _core?_core->IsMovePaused():FALSE;	}
// 	BOOL IsTeleport()	{		return _core?_core->IsTeleport():FALSE;	}
// 	LevelPos GetTeleportPos()	{		return _core?_core->GetTeleportPos():LevelPos_Invalid;	}
// 	LevelTeleportID GetTeleportID()	{		return _core?_core->GetTeleportID():LevelTeleportID_Invalid;	}

	LevelTick GetDelay()	{		return LEVEL_FRAME_TICK;	}

protected:
	void _EnsureCore();
	LevelObjPauserCore *_core;
	CLevelObj *_owner;
};

