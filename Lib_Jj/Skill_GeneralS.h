#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

#include "Skill_GeneralC.h"

struct SkillParam_GeneralS:public SkillParam_General
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_GeneralS);

	BEGIN_GOBJ_PURE(SkillParam_GeneralS,1); DERIVE_GOBJ(SkillParam_GeneralS,SkillParam_General)

	END_GOBJ();


};


class CUnitMgrNavMesh;
class Skill_GeneralS;
class CLevel;
class CSkillGesture_PathS:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CSkillGesture_PathS);

	CSkillGesture_PathS()
	{
		Zero();
	}

	void Zero()
	{
		_level=NULL;
		_bAllowFlying=FALSE;
		_bFinished=FALSE;
		_ht=0.0f;
		_face=0.0f;
	}

	void Create(CLevel *level,BOOL bAllowFlying);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt);
	virtual BOOL IsFinished()	{		return _bFinished;	}

	void Stop()	{		_bFinished=TRUE;	_level=NULL;}

	void ResetLoc(LevelPos &pos,float ht,float face);
	void UpdateLoc(LevelPos &pos,float ht,float face,AnimTick dt);

protected:

	CLevel *_level;
	BOOL _bAllowFlying;

	DWORD _bFinished;

	LevelPos3D _pos3D;
	LevelPos _pos;
	float _ht;
	LevelFace _face;

	i_math::vector3df _vel;

	friend class Skill_GeneralS;
	friend class Skill_GeneralAdvS;
};

struct LevelPathesEvent;
struct LevelPath;
class Skill_GeneralS:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_GeneralS,34);

	Skill_GeneralS()
	{
		_ges=NULL;
		_tCasting=0;

		_path=NULL;
		_tXfmAnim=0;

		_ht=0.0f;
		_face=0.0f;
		_facePath=0.0f;

		_modeFacing=LevelSkillTargetFacingMode_None;
		_modePathFacing=LevelSkillTargetFacingMode_None;

		_methodObstacle=SkillParam_GeneralS::ObstacleMethod_NotCheck;

		_scaleZAxis=1.0f;
		_scaleFaceCurRotate=1.0f;
		_scaleHt=1.0f;

		_bAllowCancel=FALSE;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_None)|(1<<LevelSkillTarget::Target_DefObj)|(1<<LevelSkillTarget::Target_FixPosAndObj)|(1<<LevelSkillTarget::Target_Pos);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}
	virtual AnimTick GetCastingTime()	{		return _tCasting;	}//返回经过IAS修正的casting time

	virtual BOOL CheckCastingEvent(StringID nmEvent)	{		return _events.CheckCastingEvent(nmEvent);}
	virtual AnimTick GetCastingEventTime(StringID nmEvent);

	void GetCastingPos(LevelPos &pos) override
	{
		if (_ges)
			pos=_ges->_pos;
		else
			pos=_owner->GetFramePos();
	}
	LevelFace GetCastingFace() override
	{
		if (_ges)
			return _ges->_face;
		return _owner->GetFrameFace();
	}

	void OnOp(GeneralSkillOpEntry &entryOp);

	BOOL CheckEventWindow(StringID nmOpen,StringID nmClose);

protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual BOOL _WriteSyncData(CBitPacket *bp) override;

	virtual void _OnFinish()	{		_Finish();	}

	virtual BOOL CanCancel()	 override{		return _bAllowCancel;	}
	virtual void Cancel() override;


	void _CalcAnimXfm(i_math::xformf &xfm,AnimTick t);
	LevelFace _AdjustFacing(LevelFace face,LevelSkillTargetFacingMode mode,float speedMaxAdjust,AnimTick dt);

	void _Finish();

	AnimTick _tCasting;

	LevelPath *_path;

	AnimTick _tXfmAnim;
	i_math::xformf _xfmAnim;
	LevelPos _pos;
	float _ht;//相对于地表的高度,不是世界空间里的绝对高度
	LevelFace _face;
	LevelFace _facePath;//路径播放时面向哪个方向(这个方向可能和单位的朝向不一致,比如单位在播放技能时朝向会锁定某个敌人,但移动路径会朝向另一个方向)

	LevelSkillTargetFacingMode _modeFacing;
	LevelSkillTargetFacingMode _modePathFacing;
	SkillParam_GeneralS::ObstacleMethod _methodObstacle;


	CSkillGesture_PathS *_ges;

	AnimTick _dur;

	CSkillGeneralEvents _events;

	BOOL _bAllowCancel;

	float _scaleZAxis;
	float _scaleFaceCurRotate;
	float _scaleHt;

};

