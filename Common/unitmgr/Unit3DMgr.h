#pragma once

#include "class/class.h"

#include "UnitGesture.h"


struct UnitFly
{
	UnitFly()
	{
		speed=0.0f;
		blendSpeed=0.0f;
		speedFace=0.0f;
		hang=0.0f;
		hangVary=0.0f;
	}
	float GetHangLow()
	{
		return i_math::clampup_f(hang-hangVary,0.0f);
	}
	float GetHangHi()
	{
		return hang+hangVary;
	}
	float speed;
	float blendSpeed;
	float speedFace;
	float hang;
	float hangVary;
};

class GameTileMap;
class CUnit;
class CUnitPath;
struct UnitFly;
class CUnit3D
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CUnit3D);

	CUnit3D()
	{
		Zero();
	}

	~CUnit3D()
	{
		Clear();
	}

	void Zero()
	{
		_fly=NULL;

		_target=NULL;
		_target3D=NULL;
		_rangeFollow=0.0f;
		_htTargetCenter=0.0;

		_speedScale=0.0f;

		_state=Idle;

		_data=NULL;
			
		_toMe=NULL;

		_tpTarget=Target_None;

		_gtm=NULL;

		_infoGesture.Zero();
	}

	enum State
	{
		Idle=0,
		PreFollow,
		Follow,
		PostFollow,

	};

	enum TargetType
	{
		Target_None,
		Target_Unit,//以某个unit上空为目标
		Target_GroundUnit,//以某个unit的地表位置为目标
		Target_Unit3D,
		Target_Pos,//以某点的上空为目标
		Target_Pos3D,
	};


	BOOL IsAlive()
	{
		return _bAlive!=0?TRUE:FALSE;
	}

	void Init(i_math::vector3df &pos,float face,i_math::vector3df &vel,GameTileMap *gtm,UnitFly *fly);
	void Clear();
	void Destroy()
	{
		Clear();
		Release();
	}

	void SetGesture(CUnitGesture *gesture)	{		_infoGesture.SetGesture(gesture);	ResetIdle();}
	CUnitGesture *GetGesture()	{		return _infoGesture.GetGesture();	}
	CUnitGesture *FetchGesture()	{		return _infoGesture.FetchGesture();	}
	UnitGestureUID GetGestureUID()	{		return _infoGesture.GetGestureUID();	}
	float GetGestureAge()	{		return _infoGesture.GetGestureAge();}

 	void SetSpeed(float speed);
	float GetSpeed()	{		return _fly?_fly->speed*_speedScale:0.0f;	}


	State GetState()	{		return (State)_state;	}

	void SetTarget_Unit3D(CUnit3D *target,float range=0.0f);
	void SetTarget_Unit(CUnit *target,float range=0.0f);
	void SetTarget_GroundUnit(CUnit *target,float htCenter,float range);
	void SetTarget_Pos3D(i_math::vector3df &pos,float range=0.0f);
	void SetTarget_Pos(i_math::vector2df &pos,float range=0.0f);

	void Reset(i_math::vector3df &pos,float face);
	void ResetIdle();

	i_math::vector3df &GetPos()	{		return _pos;	}
	float GetFace()	{		return _face;	}
	i_math::vector3df &GetVel()	{		return _vel;	}

	void UpdateState(float dt);

	GameTileMap *GetGTM()	{		return _gtm;	}

	void SetData(void *data)	{		_data=data;	}
	void *GetData()	{		return _data;	}

	void AccumVelPos(i_math::vector3df &vel,float blend,float dt);
	void AccumVelPosWithFace(i_math::vector3df &vel,float blend,float face,float dt);

public:

	void _ClampGround(i_math::vector3df &pos);

	void _ClearTarget();

	UnitFly *_fly;
	i_math::vector3df _pos;
	float _face;
	i_math::vector3df _vel;//移动的速度

	float _speedScale;

	TargetType _tpTarget;
	CUnit3D *_target3D;
	CUnit *_target;
	i_math::vector3df _targetPos3D;
	i_math::vector2df _targetPos;

	float _htTargetCenter;//目标点的中心高度
	float _rangeFollow;//当Follow一个unit时有效,表示走到这个target的多大范围以内就算到达目标了

	UnitGestureInfo _infoGesture;

	void *_data;//外部使用的数据

	GameTileMap *_gtm;

	CUnitPath *_toMe;

	union
	{
		struct
		{
			State _state:8;
			DWORD _bAlive:1;
			DWORD _bReaching:1;//正在走向终点
			DWORD _bReached:1;//_bReaching为1时有效,表示是否已经走到
			DWORD _bInRange:1;//在_state为PostFollow时有效,表示是否跟随到了范围之内(或者不在范围之内,放弃了)
		};
		DWORD _flags;
	};

};


class CUnit3DMgr
{
public:
	CUnit3DMgr()
	{
		Zero();
	}
	void Zero()
	{
		_gtm=NULL;
		_bNeedFlushDead=FALSE;
		_idxGC=0;
	}
	void Init(GameTileMap *gtm);
	void Clear();

	CUnit3D *CreateUnit3D(i_math::vector3df &pos,float face,UnitFly *fly);

	void Update(float dt);

protected:

	void _GarbageCollect();

	GameTileMap *_gtm;

	std::vector<CUnit3D *>_units;

	DWORD _idxGC;

	BOOL _bNeedFlushDead;

	friend class CUnitGesture;

};