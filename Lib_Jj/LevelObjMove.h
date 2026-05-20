#pragma once

#include "LevelDefines.h"
#include "LevelRtnuDefines.h"

#include "attr/attr.h"


class CLevelObj;

struct ResidedUnit
{
	DEFINE_CLASS(ResidedUnit);
	LevelPos3D pos;
};

struct MountUnit
{
	DEFINE_CLASS(MountUnit);
	LevelObjID idMountTarget;//Mount在那个Obj上
};

struct SpeedMod
{
	DEFINE_CLASS(SpeedMod);

	void Clear()
	{
		speed.Clear();
		speedFlying.Clear();
		speedFinal.Clear();
		speedFlyingFinal.Clear();
	}

	AttrMod_Float speed;
	AttrMod_Float speedFlying;

	AttrMod_Float speedFinal;//强制的速度
	AttrMod_Float speedFlyingFinal;//强制的飞行速度
};


struct UnitPace;
struct UnitFly;
struct LevelObjMoveSetting
{
	LevelObjMoveSetting()
	{
		radius=0.0f;
		speed=0.0f;
		bAdvWalkable=0;
		layorCollide=0;
		pace=NULL;
		fly=NULL;
	}
	float radius;
	float speed;
	DWORD layorCollide:8;
	DWORD bAdvWalkable:1;
	UnitFly *fly;
	UnitPace *pace;

};

class CUnit;
class CUnit3D;
class CRtnuUnit;
class CUnitGesture;
struct LevelUnitPace;
class CLevelObjMove
{
public:

	CLevelObjMove()
	{
		Zero();
	}
	void Zero()
	{
		_method=LevelMoveMethod_None;
		_owner=NULL;
		
		_modSpeed=NULL;


		_unitGround=NULL;
		_unitGroundRtnu=NULL;
		_unitResided_=NULL;
		_unitFlying=NULL;
		_unitMount=NULL;
		_bReaching=0;
		_stageLast=LevelMoveStage_None;
		_faceNextLast=0;
		_methodLast=LevelMoveMethod_None;
		_idTeleport=_idTeleportLast=LevelTeleportID_Invalid;
		_speedLast=0.0f;
		_idSkill=_idSkillLast=LevelSkillID_Invalid;
		_face=0.0f;
		_ht=0.0f;
		_idMountTarget=LevelObjID_Invalid;

		_uidGestureLast=0;

		_tPos=0;

		_pacePending=NULL;
	}

	void Create(CLevelObj *owner,LevelObjMoveSetting &setting);
	void SwitchGround(LevelPos &pos,LevelFace face,LevelTeleportID idTeleport);//注意:强制切为非Rtnu模式
	void SwitchFloating(LevelPos &pos,LevelFace face,float htFloating,LevelTeleportID idTeleport);
	void SwitchResided(LevelPos3D &pos3D,LevelTeleportID idTeleport);
	void SwitchFlying(LevelPos3D &pos3D,LevelFace face,LevelTeleportID idTeleport);
	void SwitchMount(LevelObjID idMountTarget,float ht,LevelTeleportID idTeleport);
	void Destroy();

	LevelMoveMethod GetMethod()	{		return _method;	}

	float GetOrgSpeed()	{		return _setting.speed;	}

	SpeedMod *ObtainSpeedMod()
	{
		if (!_modSpeed)
			_modSpeed=Class_New2(SpeedMod);
		return _modSpeed;
	}
	SpeedMod *GetSpeedMod()
	{
		return _modSpeed;
	}

	BOOL IsMoving_();
	BOOL IsMovingOrRotating();
	BOOL IsMovingFailure();
	BOOL IsMovingEnd();
	BOOL IsMovingMethod()	{		return (_method!=LevelMoveMethod_Resided_)&&(_method!=LevelMoveMethod_Mount);	}

	CUnit *GetGroundUnit()	{		return _unitGround;	}
	CRtnuUnit *GetGroundRtnuUnit()	{		return _unitGroundRtnu;	}
	CUnit3D *GetFlyingUnit()	{		return _unitFlying;	}

	float CalcGroundSpeed();
	float GetUnitSpeed()	{		return _GetUnitSpeed();	}

	LevelPos &GetFramePos()	
	{		
		Update();
		return _pos;
	}
	float GetFrameFace()	
	{		
		Update();
		return _face;
	}

	BOOL GetFramePos3D(LevelPos3D &pos3D);

	LevelPos &GetVel()	{		return _vel;	}

	LevelMoveStage GetStage();

	void ResetIdle();
	LevelMoveSession RequestNoTarget();
	LevelMoveSession RequestTarget(LevelPos &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange);
	LevelMoveSession RequestTarget(LevelPos3D &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange);
	LevelMoveSession RequestTarget(CLevelObj *lo,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange,BOOL b3DFollow);
	LevelMoveSession RequestFacing(float range,float rad);

	void SetMove(LevelPos &pos,LevelFace face,BOOL bReached);

	void Teleport(LevelTeleportID idTeleport,LevelPos &pos,LevelFace face);

	void SetCollide_Ghost(BOOL bGhost);

	void SetUnitPace(LevelUnitPace *pace)	{		_pacePending=pace;	}


	void Update();

	void WriteFirstSync(CBitPacket *bp);
	void WriteSyncH(CBitPacket *bp,BOOL &bContent);
	void PostWriteSync();

	BOOL IsRtnuMode()	{		return _unitGroundRtnu?TRUE:FALSE;	}
	void SwitchRtnuMode(BOOL bRtnuMode,LevelRtnuRank rank);

protected:
	void _WritePos(CBitPacket *bp);
	void _ClearUnit();
	void _ClearSpeedMod();
	CUnitGesture *_FetchGesture();
	void _UpdateUnitSpeed();
	float _GetUnitSpeed();//从_unitXXXX里得到它们当前的速度

	LevelPos _GetMountPos();

	void _SwitchGroundUnit(BOOL bFloating,LevelPos &pos,LevelFace face,LevelTeleportID idTeleport);
	void _UpdateRtnuPlayerID();

	LevelTick _GetT();


protected:

	LevelPos &_GetUnitGroundPos();
	LevelFace _GetUnitGroundFace();

	CLevelObj *_owner;
	LevelObjMoveSetting _setting;
	SpeedMod *_modSpeed;

	LevelUnitPace *_pacePending;

	//位置/移动
	LevelMoveMethod _method;
	//_unitXXX里面的位置是下一帧时刻(_level->GetT()+LEVEL_UPDATE_INTERVAL)的位置
	CUnit *_unitGround;//Ground或者Floating
	CRtnuUnit *_unitGroundRtnu;//目前只在Ground时有效
	ResidedUnit *_unitResided_;
	CUnit3D *_unitFlying;
	MountUnit *_unitMount;

	AnimTick _tPos;//_pos,_ht的时间
	LevelPos _pos;//_tPos时刻的位置
	LevelFace _face;//_tPos时刻的face
	LevelPos _vel;//注意不是normalize的
	LevelHeight _ht;//_tPos时刻的高度,在_method为Ground/Floating时无效
	DWORD _bMountPosDirty:1;//在_method为Mount时有效,表示_pos需要重新计算

	LevelTeleportID _idTeleport;//位置每经过一次Teleport(以非行走移动的形式改变位置),会有一个新的TeleportID
	LevelSkillID _idSkill;//当前由哪个skill控制移动
	DWORD _bReaching:1;//表示下一帧时刻(_level->GetT()+LEVEL_UPDATE_INTERVAL)是否算到达了

	//同步的Cache
	LevelMoveMethod _methodLast;
	LevelPosInt _posNextLast;//上一次更新的位置
	LevelFaceInt _faceNextLast;//上一次更新的face
	LevelHeightVu _vuLast;//上一次更新的高度
	LevelPos3D _posNext3DLast;//上一次更新的3D位置
	DWORD _uidGestureLast;//UnitGestureUID
	LevelObjID _idMountTarget;//上一次更新的Mount Target
	float _tGestureAgeLast;
	DWORD _stageLast:LevelMoveStage_BitCount;
	LevelTeleportID _idTeleportLast;
	LevelSkillID _idSkillLast;
	float _speedLast;

};


