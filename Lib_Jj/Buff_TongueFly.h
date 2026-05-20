#pragma once

#include "LevelBuff.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"

#include "behaviorgraph/BehaviorParam.h"

#include "TonguePhys.h"
#include "EoEnv.h"

#include "rope/rope.h"


struct BuffParam_TongueFly;
class Buff_TongueFly;

struct BuffParam_TongueFly:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_TongueFly);

	BEGIN_GOBJ_PURE(BuffParam_TongueFly,1);

		GELEM_VAR_INIT(unsigned __int64,idTongue,0);
			GELEM_EDITVAR("舌Proto",GVT_Bx8,GSem_ProtoPath,"舌的Proto");
		GELEM_VAR_INIT(RecordID,idKnot,RecordID_Invalid);
			GELEM_EDITVAR("舌节点",GVT_U,GSem(GSem_RecordID,"units"),"");
		GELEM_OBJ(RopeProp,propBrokenTongue);
			GELEM_EDITOBJ("断舌物理参数","断舌物理参数");
		
	END_GOBJ();

	unsigned __int64 idTongue;
	RecordID idKnot;
	RopeProp propBrokenTongue;
};



struct BuffArg_TongueFly:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_TongueFly);
	BuffArg_TongueFly()
	{
	}

};

class CLevelGesture_TongueKnot_Move;
class Buff_TongueFly:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_TongueFly,48)

	enum WorkMode
	{
		WorkMode_None,
		WorkMode_Tracking,
		WorkMode_Withdraw,
		WorkMode_FastWithdraw,
	};

	struct Height
	{
		Height()
		{
			Zero();
		}
		void Zero()
		{
			_v=0.0f;
			_spd=0.0f;
			_acc=0.0f;

			_t=0.0f;
			_tUpdateTarget=0.0f;

			_target=-1.0f;
		}

		void Init(float v);

		void Update(float distEnemy,float dt);

		float GetCur()
		{
			return _v;
		}


	public:
		static const float _min;
		static const float _max;
		static const float _spdMax;
		static const float _dampSpdUp;
		static const float _dampSpdDown;
		static const float _stiffness;

		static const float _cycleUpdateTarget;
		static const float _cycleUpdateTargetVary;
		static const float _targetMin;
		static const float _targetMax;
		static const float _liftMax;
		static const float _distEnemyMax;

		static const BOOL _bEasyMode;

		float _t;

		float _tUpdateTarget;
		float _target;

		float _v;
		float _spd;
		float _acc;
	};

	Buff_TongueFly()
	{
		_mode=WorkMode_Tracking;
		_bPassive=TRUE;
		_tCur=FALSE;
		_idKnot=LevelObjID_Invalid;
		_gesKnot=NULL;
		_bFinished=FALSE;
		_bLiberated=FALSE;
		_hLichen=EoEnvLichenHandle_Invalid;
		_ht=0.0f;
		_htRoot=-1.0f;
		_htSkillPathStart=0.0f;
		_tSkillPathStart=0;
		_modeSkillPathStart=WorkMode_None;
		_bFlying=TRUE;

	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}
	virtual BOOL NeedSyncBigData()	{		return TRUE;	}

	BuffFlag GetFlags();

	//SkillPath
	virtual void StartSkillPath(LevelPos &pos,float ht) override;
	virtual void StopSkillPath() override;
	virtual BOOL CalcSkillPathXfm(AnimTick t,LevelPos &pos,float &ht,LevelFace &face) override;


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt) override;

	virtual void _WriteData(CBitPacket *dp) override;
	virtual void _WriteBigData(CDataPacket *dp)override;

	virtual void HandleEvent(LevelEvent &e) override;

	BOOL IsNearlyWithdrawn();


	BOOL GetKnotLoc(LevelPos &pos,LevelFace &face)
	{
		pos=_posKnot;
		face=_faceRoot;
		return TRUE;
	}

	void SetWorkMode_Withdraw()
	{
		if (_mode<WorkMode_Withdraw)
		{
			_mode=WorkMode_Withdraw;
			_tongue.SetSpeedScale(1.0f);
		}
	}
	void SetWorkMode_FastWithdraw()
	{
		if (_mode<WorkMode_FastWithdraw)
		{
			_mode=WorkMode_FastWithdraw;
			_tongue.SetSpeedScale(3.0f);
		}
	}

	BOOL IsTongueBrokenForAWhile(AnimTick dur);
	LevelObjID GetKnotID()	{		return _idKnot;	}

	BOOL CheckKnotKilled();

	DWORD GetTongueNodesPos(LevelPos *buf,DWORD szBuf);
	float GetTongueNodeRadius();

protected:

	void _OnUpdate_Core(AnimTick t);

	void _OnUpdate_CreateKnot();

	void _OnUpdate_Finish();
	void _OnUpdate_Liberated();

	DWORD _GetKnotNodeIndex()
	{
		return 7;//这个值与Knot在骨架里绑定的Bone的序号相同(目前为Bone07)
	}

	BOOL _IsWithdrawing()	{		return _mode==WorkMode_Withdraw||_mode==WorkMode_FastWithdraw;	}

	void _SwitchMoveMethod(BOOL bFlying);

	WorkMode _mode;
	BOOL _bFinished;
	BOOL _bLiberated;

	AnimTick _tCur;

	PBD::Simulation _sim;
	PBD::SimulationModel _model;

	CTongueBranchPhys _tongue;
	CTonguePhysEnv _env;

	BOOL _bPassive;

	AnimTick _tSkillPathStart;
	float _htSkillPathStart;
	WorkMode _modeSkillPathStart;
	Height _htSkillPath;

	LevelPos _posRoot;
	LevelFace _faceRoot;
	float _htRoot;

	LevelPos _posKnot;
	LevelObjID _idKnot;

	LevelPos _pos;
	LevelFace _face;
	float _ht;

	BOOL _bFlying;

	LevelPos _posTarget;
	LevelPos _velTarget;

	CLevelGesture_TongueKnot_Move *_gesKnot;

	void _StopLichen();
	EoEnvLichenHandle _hLichen;

};

