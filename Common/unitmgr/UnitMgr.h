#pragma once

#include "class/class.h"

#include "datapacket/DataPacket.h"

#include "valueset/valueset.h"

#include "avtrstates/avtrstates_defines.h"

#include "UnitGesture.h"

#include "UnitMap.h"


//vRay是normalize过的
//返回小于0的值表示没有碰撞
//否则返回沿着vRay前进的距离
inline float intersectSphereBySphere(i_math::vector2df& vSrc,float rSrc,i_math::vector2df& vRay, i_math::vector2df& vTarget, float rTarget)
{	
	float dot=(vTarget-vSrc).dotProduct(vRay);//vTarget在vRay上的投影距离
	if (dot<=0.0f)
		return -1.0f;
	float distSQ=(float)(vSrc-vTarget).getLengthSQ();//圆心的距离
	float dR=rSrc+rTarget;//半径之和
	if (distSQ<dR*dR)
		return 0.0f;//两圆重叠
	float dropSQ=distSQ-dot*dot;
	if (dropSQ>=dR*dR)

		return -1.0f;
	return dot-sqrtf(dR*dR-dropSQ);
} 



#define UNIT_MAX_RADIUS (2.0f)

typedef DWORD UnitCollide;
#define UnitCollide_Empty (0)
#define UnitCollide_AllyToAllPlayer (0xff)

enum UnitFindPathType
{
	UnitFindPath_Walkable,
	UnitFindPath_AdvWalkable,

	UnitFindPathType_Max,
};

enum UnitAbortReason
{
	UnitAbortReason_None,
	UnitAbortReason_CannotReach,
	UnitAbortReason_OutOfFacing,
	UnitAbortReason_OutOfFacingRange,
	UnitAbortReason_LoseTarget,
};

enum UnitStage
{
	UnitStage_NotMove=0,
	UnitStage_RotateOnSpot,
	_UnitStage_Blocked,//内部使用的stage
	_UnitStage_BigTurnInterrupted,//内部使用的stage
	UnitStage_Faced,
	UnitStage_Reached,//到达目标停止
	UnitStage_Abort,//未到达目标停止
	UnitStage_StartFw,
	UnitStage_StartRot,
	UnitStage_Move,
	_UnitStage_Reaching,//内部使用的stage
	UnitStage_Stop,

	//XXXXX:More Move Stage
	UnitStage_ForceDword=0xffffffff,
};



class CUnitPath
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CUnitPath);
	CUnitPath()
	{
		Zero();
	}
	~CUnitPath()
	{
		Clear();
	}
	void Zero()
	{
		_flags=0;
		_next=NULL;
	}
	void Clear();
	void Destroy();
	BOOL IsAlive()
	{
		return _bAlive!=0?TRUE:FALSE;
	}
	void BuildDistsToGo();
	float GetDistToGo(i_math::vector2df &pos,int iPathNode);
public:
	i_math::vector2df _target;//用来计算路径的target(注意它的值不一定和_nodes[_nodes.size()-1]相等)
	std::vector<i_math::vector2df> _nodes;
	std::vector<float>_distsToGo;//每个node到终点的距离
	union
	{
		struct
		{
			DWORD _bAlive:1;
			DWORD _bDirectPath:1;//是否为直接到的路径(起点直接到终点,只有一段)
			DWORD _bOnUnwalkable:1;
		};
		DWORD _flags;
	};
	CUnitPath *_next;
};

class CUnitTargetPos
{
public:
	DEFINE_CLASS(CUnitTargetPos);
	CUnitTargetPos()
	{
		_toMe=NULL;
	}
	~CUnitTargetPos()
	{
		Clear();
	}

	void Clear();

public:
	i_math::vector2df _pos;
	CUnitPath *_toMe;
};

struct UnitPace
{
	UnitPace()
	{
		rotlimit=0.0f;
		rotlimitAvoid=0.0f;
		durROS=1.0f;
		durROSWait=0.2f;
		angleStartRot=45.0f;
		rotlimitBigTurn=0.0f;
		bSupportStart=FALSE;
		bSupportStop=FALSE;
	}

	BOOL bSupportStart;
	ValueSet startFw;
	ValueSet startRot;
	BOOL bSupportStop;
	ValueSet stop;

	float durROS;//RotateOnSpot的时长
	float durROSWait;//转到目的地后,停顿的时间

	float angleStartRot;//起步时偏转角度大于多少需要使用StartRot
	float rotlimit;//每秒最多旋转的弧度,小于等于0表示没有限制
	float rotlimitAvoid;//在Avoid时每秒最多旋转的弧度,小于等于0表示没有限制
	float rotlimitBigTurn;//一秒内转动角度大于多少度要停止移动,小于等于0表示不考虑
};

class CUnitTarget
{
public:
	DEFINE_CLASS(CUnitTarget);
	CUnitTarget()
	{
		Zero();
	}
	~CUnitTarget()
	{
		Clear();
	}
	void Zero()
	{
		_unit=NULL;
		_unit3D=NULL;
		_pos=NULL;
		_bCur=FALSE;
		_rangeFollow=0.0f;
		_bClosestFollow=0;
		_bNoStopMoveWhenInRange=0;
		_bFace=0;
		_rangeFace=0.0f;
		_radFace=0.0f;
		_bFindPathOnUnwalkable=0;
	}
	void Clear();
	BOOL IsEmpty()
	{
		return !(_unit||_unit3D||_pos||_bCur);
	}
	BOOL IsCur()	{		return _bCur;	}

	void VerifyAlive();
	BOOL CheckInRange(CUnit *unitSrc,BOOL bDisableClosestFollow);
	BOOL CheckInFaceRad(CUnit *unitSrc);
	BOOL CheckInFaceRange(CUnit *unitSrc);
	BOOL GetPos(i_math::vector2df &pos);
public:
	CUnit *_unit;
	CUnit3D *_unit3D;
	CUnitTargetPos *_pos;
	BOOL _bCur;//永远以当前位置朝向为目标,表示立即停下
	float _rangeFollow;//当Follow一个unit时有效,表示走到这个target的多大范围以内就算到达目标了
	float _rangeFace;//表示Face后,target在多大距离范围内,仍然有效
	float _radFace;//表示Face后,target在多大弧度范围内,仍然有效
	DWORD _bClosestFollow:1;//表示即便在FollowRange中,也要尽量移动到离目标点最近的位置(在FollowRange范围内,遇到障碍后才停止),目前不支持3D的Target
	DWORD _bNoStopMoveWhenInRange:1;//表示在到达时,如果在范围内,则不要进入Stop stage
	DWORD _bFace:1;//表示在到达后,还要朝向目标
	DWORD _bFindPathOnUnwalkable;

};

struct UnitRot
{
	DEFINE_CLASS(UnitRot)
	UnitRot()
	{
		from=to=speed=0.0f;
	}
	float from;
	float to;
	float speed;
};

typedef DWORD UnitSession;



class RvoUnit;
class RvoSimulator;
class CUnitMgr;
class CUnit3D;
class CUnit:public CUnitBase
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CUnit);

	CUnit()
	{
		Zero();
	}

	~CUnit()
	{
		_Clear();
	}

	void Zero()
	{
		_mgr=NULL;
		_flags=0;
		_face=0.0f;
		_faceOverriden=-1000.0f;
		_radius=0;
		_dist=0;
		_target=NULL;
		_targetPending=NULL;
		_path=NULL;
		_toMe=NULL;
		_radLastAvoid=-1.0f;

		_next=NULL;
		_ptUnitMapTile.set(-1,-1);
		_tile=NULL;

		_collide=UnitCollide_Empty;

		_htFloating=0.0f;

		_infoGesture.Zero();

		_pace=NULL;
		_stage=UnitStage_NotMove;
		_tInStage=0.0f;

		_rot=NULL;

		_session=0;
		_sessionPending=0;

		_data=NULL;

		_mirror=NULL;
	}

	void SetMgr(CUnitMgr *mgr)	{		_mgr=mgr;	}

	BOOL IsAlive()
	{
		return _bAlive!=0?TRUE:FALSE;
	}
	void SetAlive()	{		_bAlive=1;	}

	void Reset(i_math::vector2df &pos,float face);
	void Reset();

	BOOL IsFaceOverriden()	{		return _faceOverriden>-1000.0f;	}
	void OverrideFace(float face)	{		_faceOverriden=face;	}
	void ClearOverrideFace()
	{
		if (IsFaceOverriden())
		{
			_face=_faceOverriden;
			_faceOverriden=-1000.0f;
		}
	}

	void SetSpeed(float speed)	{		_speed=speed;	}
	float GetSpeed()	{		return _speed;	}


	float GetRadius()	{		return _radius;	}
	void SetRadius(float radius)	{		_radius=radius;	}


	void SetCollide(UnitCollide collide)	{		_collide=collide;	_UpdateMirrorCollide();}
	UnitCollide GetCollide()	{		return _collide;	}

	BOOL IsClosestFollow();

	BOOL IsStart()	{		return _stage==UnitStage_StartFw||_stage==UnitStage_StartRot;	}
	BOOL IsMoving()	{		return _stage>=UnitStage_StartFw;	}
	BOOL IsMovingOrRotating()	{		return IsMoving()||_stage==UnitStage_RotateOnSpot;	}
	BOOL IsEnd()	{		return (_stage>=UnitStage_Faced)&&(_stage<=UnitStage_Abort);	}
	BOOL IsSuccess()	{		return (_stage==UnitStage_Faced)||(_stage==UnitStage_Reached);	}
	BOOL IsFailure()	{		return _stage==UnitStage_Abort;	}
	UnitAbortReason GetAbortReason()	{		return _reasonAbort;	}
	BOOL IsStartFinished();
	BOOL IsStopFinished();

	virtual UnitStage GetStage()	{		return (UnitStage)_stage;	}
	void SetStage(UnitStage stage)	{		_SetStage(stage);	}

	void StopMove()	{		_StopMove();	}
	void FinalizeReach();

	UnitRot* GetRot()	{		return _rot;	}

	float GetLastAvoidRad()	{		return _radLastAvoid;	}
	void SetLastAvoidRad(float rad)	{		_radLastAvoid=rad;	}

	UnitSession GetPendingSession()	{		return _sessionPending;	}
	UnitSession GetSession()	{		return _session;	}


	UnitSession RequestNoTarget();
	UnitSession RequestTarget(CUnit *target,float range=0.0f,BOOL bClosestFollow=FALSE,BOOL _bNoStopMoveWhenInRange=FALSE);
	UnitSession RequestTarget3D(CUnit3D *target,float range=0.0f,BOOL _bNoStopMoveWhenInRange=FALSE);
	UnitSession RequestTargetPos(i_math::vector2df &pos,float range=0.0f,BOOL bClosestFollow=FALSE,BOOL _bNoStopMoveWhenInRange=FALSE,BOOL bFindPathOnUnwalkable=FALSE);
	UnitSession RequestFacing(float range,float rad);



	void ResetFloatingHeight(float ht)	{		_htFloating=ht;	}

	void SetGesture(CUnitGesture *gesture)	{		_infoGesture.SetGesture(gesture); Reset();	}
	CUnitGesture *GetGesture()	{		return _infoGesture.GetGesture();	}
	CUnitGesture *FetchGesture()	{		return _infoGesture.FetchGesture();	}
	UnitGestureUID GetGestureUID()	{		return _infoGesture.GetGestureUID();	}
	float GetGestureAge()	{		return _infoGesture.GetGestureAge();}

	void SetPathFindType(UnitFindPathType tp)	{		_tpPathFind=tp;	}
	UnitFindPathType GetPathFindType()	{		return _tpPathFind;	}

	i_math::vector2df &GetPos()	{		return _pos;	}
	virtual float GetFace()	{		return IsFaceOverriden()?_faceOverriden:_face;	}
	void SetFace(float face)	{		_face=face;	}
	float GetFloatingHeight()	{		return _htFloating;	}
	void UpdateStage(float dt);

	void Destroy();

	void SetData(void *data)	{		_data=data;	}
	void *GetData()	{		return _data;	}

	float GetDistToGo();//返回离终点还有多远的距离,返回-1表示目前这个值没有意义(单位没有在follow)

	CUnitTarget *GetTarget()	{		return _target;	}
	CUnitTarget *GetPendingTarget()	{		return _targetPending;	}

	BOOL IsPathOnUnwalkable()
	{
		if (_path)
			return _path->_bOnUnwalkable;
		return FALSE;
	}

	BOOL CanSetPace();
	BOOL SetPace(UnitPace *pace);

	i_math::vector2df& GetMoveDir()	{		return _dir;	}
	float GetMoveDist()	{		return _dist;	}

	RvoUnit *GetMirror()	{		return _mirror;	}

public:

	void _Clear();
	void _Destroy();

	void _SetTarget(CUnit *target,float range=0.0f,BOOL bClosestFollow=FALSE);
	void _SetTarget3D(CUnit3D *target,float range=0.0f);
	void _SetTargetPos(CUnitTargetPos *targetPos,float range=0.0f,BOOL bClosestFollow=FALSE);

	void _ResetIdle();

	BOOL _CanAcceptPendingTarget();

	void _ClearPath();
	BOOL _CheckInRange();
	BOOL _CheckInRange_NoClosestFollow();
	BOOL _CheckInFaceRad();
	BOOL _CheckInFaceRange();

	BOOL _MakePathToTarget();
	void _StartMove();
	void _StopMove();

	BOOL _SupportStartStage();
	BOOL _SupportStopStage();

	void _SetStage(UnitStage stage);

	void _UpdateMirrorCollide();

	CUnitMgr *_mgr;

	std::string _nm;

	UnitPace *_pace;
	float _radius;
	float _speed;//固有速度(或者说移动时的最大速度)
	float _face;//朝向,以弧度为单位
	float _faceOverriden;//朝向,以弧度为单位
	float _htFloating;//floating height
	i_math::vector2df _dir;//移动的方向(normalized)
	float _dist;//移动的距离
	float _radLastAvoid;//上一次绕道的朝向,小于0为无效值
	float _closest;//_bReaching为1时有效,表示曾经离目标的最近距离
	DWORD _tClosest;//_bReaching为1时有效,表示到达离目标的最近距离的时间,单位为ms
	float _tInStage;//当前stage已开始多长时间了

	CUnitTarget *_target;
	CUnitTarget *_targetPending;

	CUnitPath *_path;//如果这个指针为空,并且_stage不是NotMove,则表示直接可到

	CUnitPath *_toMe;

	UnitRot *_rot;

	UnitGestureInfo _infoGesture;

	UnitSession _session;
	UnitSession _sessionPending;

	UnitCollide _collide;

	void *_data;//外部使用的数据

	RvoUnit *_mirror;

	union
	{
		struct
		{
			UnitStage _stage:5;
			UnitFindPathType _tpPathFind:4;
			UnitAbortReason _reasonAbort:3;
			DWORD _iPathNode:8;//当_path不为NULL时有效,表示当前正在前往第几个node
			DWORD _bAlive:1;
			DWORD _expandflag:1;
			DWORD _sortflag:1;
			DWORD _solveflag:1;
			DWORD _bEscape:1;//正在从不能行走的地方走到可走的地方,此时忽略所有阻碍
			DWORD _nStucks:3;//连续被Stuck几次
		};
		DWORD _flags;
	};


	friend class CUnitMgr;

};



class CUnitMgr
{
public:
	CUnitMgr()
	{
		_expandflag=0;
		_sortflag=0;
		_solveflag=0;
		_t=0;
		_bNeedFlushDead=FALSE;
		_idxGC=0;
		_mirror=NULL;
	}

	struct CircumRanges
	{
	public:
		CircumRanges()
		{
			_nRanges=0;
		}
		struct Range
		{
			BOOL IsFull()
			{
				if (fabsf(hi)+fabsf(low)>=i_math::Pi*2.0f)
				{
					if(hi*low<0)
						return TRUE;
				}
				return FALSE;
			}
			float low,hi;
			CUnitBase *unitLow,*unitHi;
		};
		void Clear()
		{
			_nRanges=0;
		}
		void Add(float low,float hi,CUnitBase *unit);

		Range *FindRange(float v)
		{
			for (int i=0;i<_nRanges;i++)
			{
				if ((_ranges[i].low<=v)&&(_ranges[i].hi>=v))
					return &_ranges[i];
			}
			return NULL;
		}

	protected:
		Range _ranges[128];//big enough
		DWORD _nRanges;

	};


	void Create(i_math::recti &rcMap);//以米为单位
	void Destroy();

	void SetMirror(RvoSimulator *mirror)	{		_mirror=mirror;	}
	RvoSimulator *GetMirror()	{		return _mirror;	}

	CUnit* CreateUnit(i_math::vector2df &pos,float face,float radius,float speed,UnitPace *pace,BOOL bAllowMirror=TRUE,const char *nm="");//返回的对象带一个引用计数

	void UpdateUnitPos(CUnit *unit);

	float GetDt()	{		return _dt;	}
	void Update(float dt);
	void UpdateSingle(CUnit *unit,float dt);

	DWORD GetUnitCount()	{		return _units.size();	}
	CUnit *GetUnit(DWORD idx)	{		return _units[idx];	}
	CUnit *FindUnit(const char *nm)
	{
		for (int i=0;i<_units.size();i++)
		{
			if (_units[i]->_nm==nm)
				return _units[i];
		}
		return NULL;
	}

	CUnitMap *GetMap()	{		return &_mp;	}

	//以下函数为调试用
	void Dump(CDataPacket &dp);
	void Restore(CDataPacket &dp);
	void AddFailPath(i_math::vector2df &src,i_math::vector2df &target)
	{
		return;
		_failpath.push_back(src);
		_failpath.push_back(target);
	}
	i_math::vector2df *GetFailPathes(DWORD &c)
	{
		c=_failpath.size();
		return _failpath.data();
	}

	//测试unit到unitTarget之间是否可以走通,会考虑所有非移动的Unit的阻挡
	BOOL TestUnitPath(CUnit *unit,CUnit *unitTarget);

	//检测unit是否可以没有碰撞的到达posTarget,目前不检查静态障碍
	BOOL CheckUnitColliding(CUnit *unit,i_math::vector2df &posTarget,float rateShrink,i_math::vector2df *posColliding=NULL,CUnit **unitColliding=NULL);
	DWORD CheckUnitCollidings(CUnit *unit,i_math::vector2df &posTarget,float rateShrink,float *&bufDist,i_math::vector2df *&bufPos,CUnit **&bufUnits);//注意不是ThreadSafe的

	//路径搜索
	//bEscape如果为TRUE,表示起始位置为不可走的地方,所以路径的第一段为从不可走处走到可走处,我们称为Escape
	//toleranceEnd表示如果end在不可走区域,最大允许走到离它多远的地方
	virtual BOOL FindPath(UnitFindPathType tpFindPath,i_math::vector2df &start,i_math::vector2df& end,float toleranceEnd,std::vector<i_math::vector2df>&path,BOOL &bEscape)	{		return FALSE;	}
	virtual BOOL FindPathOnUnwalkable(UnitFindPathType tpFindPath,i_math::vector2df &start,i_math::vector2df& end,std::vector<i_math::vector2df>&path)	{		return FALSE;	}

	//返回两点间是否有静态障碍
	virtual BOOL StaticObstacleTest(UnitFindPathType tpFindPath,i_math::vector2df &posSrc,i_math::vector2df &posTarget)	{		return FALSE;	}

	virtual BOOL StaticRayCast(UnitFindPathType tpFindPath,i_math::vector2df &posSrc,i_math::vector2df &posTarget,i_math::vector2df &posHit)	{		return FALSE;	}

	virtual BOOL IsWalkable(UnitFindPathType tpFindPath,i_math::vector2df &pos)	{		return FALSE;	}
	virtual void SwitchWalkable(BOOL bOn,i_math::vector2df &pos,float radius=20.0f)	{	}

protected:

	void _UpdateUnitStage(CUnit *unit,float dt);
	void _UpdateUnit(CUnit *unit,float dt);

	void _FinalizeBlocks();
	void _FinalizeReaching(CUnit *unit);

	void _GarbageCollect();

	void _ClearUnits();

	DWORD _expandflag;
	DWORD _sortflag;
	DWORD _solveflag;


	std::vector<CUnit*>_units;

	CUnitMap _mp;

	std::vector<CUnit*>_stack;
	std::vector<CUnit*>_sorts;
	std::vector<CUnit*>_circums;
	std::vector<CUnit*>_blocks;
	CircumRanges _ranges;

	DWORD _t;//当前时刻,单位为ms
	float _dt;

	BOOL _bNeedFlushDead;

	DWORD _idxGC;

	RvoSimulator *_mirror;

	std::vector<i_math::vector2df> _pathTemp;

	std::vector<i_math::vector2df> _failpath;//每次找路径失败,就把失败的路径放在这里,用于调试

	std::vector<float> _collidingTempDists;
	std::vector<i_math::vector2df> _collidingTempPos;
	std::vector<CUnit *> _collidingTempUnits;


};
