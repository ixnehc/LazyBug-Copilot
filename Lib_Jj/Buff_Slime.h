#pragma once

#include "LevelBuff.h"

struct BuffParam_Slime:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Slime);

	BEGIN_GOBJ_PURE(BuffParam_Slime,1);

		GELEM_VAR_INIT(float,durDead,10.0f);
			GELEM_EDITVAR("死亡持续时间",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"死亡持续时间");
		GELEM_VAR_INIT(float,durTrampleDmg,8.0f);
			GELEM_EDITVAR("践踏杀死时间",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"践踏杀死时间");
		GELEM_VAR_INIT(float,durRecover,12.0f);
			GELEM_EDITVAR("完全恢复时间",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"完全恢复时间");
		GELEM_VAR_INIT(RecordID,idBuff_NotAttackable,RecordID_Invalid);
			GELEM_EDITVAR("不可攻击的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
	END_GOBJ();

	float durDead;
	float durTrampleDmg;
	float durRecover;
	RecordID idBuff_NotAttackable;

};

struct BuffArg_Slime:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Slime);
};



class Buff_Slime:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Slime,59)

	enum State
	{
		State_None,
		State_Flying,
		State_Ready,
		State_Exhausted,
		State_Trampled,
		State_Dead,
	};

	Buff_Slime()
	{
		_state=State_Exhausted;
		_bDeadPermanently=FALSE;
		_hp=0.0f;
		_tStateStart=0;
		_hpSent=-1;
	}


	virtual BOOL NeedSync()  override	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()  override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Slime的Buff并存的情况

	virtual LevelBuffMask GetReplaceBuffs()  override;

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnUpdate(AnimTick dt)  override;
	virtual void _OnDestroy() override;

	virtual void _WriteData(CBitPacket *dp) override;

	BOOL IsDead()	{		return _state==State_Dead&&_bDeadPermanently;	}

	BuffFlag GetFlags() override
	{		
		if (_state==State_Ready)
			return BuffFlag_NotAttackable;
		if (_state==State_Dead)
			return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_Dead|BuffFlag_LayDown|BuffFlag_Pausing;
		return BuffFlag_GhostCollide|BuffFlag_TrampleTarget;	
	}

protected:
	State _state;
	AnimTick _tStateStart;
	AnimTick _durDead;

	BOOL _bDeadPermanently;

	LevelPos _pos;

	float _hp;
	float _hpMax;

	int _hpSent;

	friend class CBgn_SlimeOp;

};

