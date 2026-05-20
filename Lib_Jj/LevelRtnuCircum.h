#pragma once

#include "unitmgr/UnitMgr.h"

class CLevel;

class RvoUnit;
class CLevelPlayer;

//派生自CUnit只是为了使用里面的一些基础信息(pos,face等),CRtnuUnit本身并不是一个正常的CUnit,不会加到CUnitMgr里面
//其实完全可以不必派生自CUnit
//它的核心是一个RvoUnit
class CRtnuUnit:public CUnit
{
public:
	DEFINE_CLASS(CRtnuUnit);

	CRtnuUnit()
	{
		Zero();
	}

	void Zero()
	{
		_unitRvo=NULL;
		_rank=LevelRtnuRank_None;

		_faceCur=0.0f;
		_bRotateOnSpot=FALSE;
		_faceRotateOnSpot=0.0f;
		_status=Status_None;
		_durGathering=0.0f;
		_durFollowing=0.0f;

		_wtFaceOwner=0.0f;
		_faceOwner=0.0f;
	}
	void Clear();

	RvoUnit *GetRvoUnit()	{		return _unitRvo;	}
	CUnit *GetMirrorUnit();

	virtual float GetFace()	{		return _faceCur;	}
	virtual UnitStage GetStage();

	void StartRotateOnSpot(float faceTarget)
	{
		_bRotateOnSpot=TRUE;
		_faceRotateOnSpot=faceTarget;
	}

	void StopRotateOnSpot()	{		_bRotateOnSpot=FALSE;	}

	BOOL IsAvoiding();

	BOOL AllowSlideMoving()
	{
		return _rank==LevelRtnuRank_Knight||_rank==LevelRtnuRank_Archer||_rank==LevelRtnuRank_Monk;
	}

protected:

	enum Status
	{
		Status_None,
		Status_Accompanying,
		Status_Gathering,
		Status_Following,
		Status_GatheringAvoiding,
	};
	void _StartGathering(CRtnuUnit *unitRtnu,float speedGathering);

	RvoUnit *_unitRvo;
	LevelRtnuRank _rank;

	Status _status;
	float _durGathering;
	float _durFollowing;

	//状态
	float _faceCur;
	BOOL _bRotateOnSpot;
	float _faceRotateOnSpot;//RotateOnSpot的目的地

	float _faceOwner;
	float _wtFaceOwner;

	friend class CLevelRtnuCircum;
};

class CLevelRtnuCircum
{
public:
	DEFINE_CLASS(CLevelRtnuCircum);

	CLevelRtnuCircum()
	{
		Zero();
	}

	void Zero()
	{
		_unitRvoOwner=NULL;
		_owner=NULL;
		_level=NULL;
		_unitmgr=NULL;
	}

	void Init(CLevelPlayer *player);
	void Clear();

	CRtnuUnit *Register(LevelRtnuRank rank,CLevelObj *lo,LevelPos &pos,LevelFace face);
	void Unregister(CRtnuUnit *unitRtnu);

	void PreSimulate();//填充用来Simulate的数据

	void PostSimulate();//处理Simulate的结果

protected:

	void _StartGathering(CRtnuUnit *unitRtnu,LevelPos &posOwner,float speedGathering,float dt);

	void _UpdateThreats();
	void _UpdateUnit_Knight(CRtnuUnit *unit,LevelMoveStep &stepOwnerMove,float dt);

	LevelPos _CalcKnightVel(LevelPos &pos,LevelPos &posOwner,LevelPos &dirOwner,float speedOwner);

	RvoUnit *_unitRvoOwner;


	std::vector<CRtnuUnit*> _units[LevelRtnuRank_Max];

	LevelMoveStep _stepOwnerMove;

	CLevelPlayer *_owner;
	CLevel *_level;
	CUnitMgr *_unitmgr;//要使用其中的Navmesh相关的功能比如Pathfinding
};

