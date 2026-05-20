#pragma once

#include "LevelBuff.h"

struct DormantForm
{
	BEGIN_GOBJ_PURE(DormantForm,1);

		GELEM_VAR_INIT(unsigned __int64,base,0);
			GELEM_EDITVAR("休眠效果",GVT_Bx8,GSem_ProtoPath,"休眠时的效果Proto");
		GELEM_VAR_INIT(unsigned __int64,revive,0);
			GELEM_EDITVAR("苏醒过程效果",GVT_Bx8,GSem_ProtoPath,"苏醒过程的效果Proto");

		GELEM_VAR_INIT(AnimTick,durRevive,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("苏醒	时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.02"),"苏醒过程的时间,单位为秒");

	END_GOBJ();

	unsigned __int64 base;
	unsigned __int64 revive;
	AnimTick durRevive;
};

struct BuffParam_Dormant:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Dormant);

	BEGIN_GOBJ_PURE(BuffParam_Dormant,1);

		GELEM_OBJVECTOR(DormantForm,forms)
			GELEM_EDITOBJ("各种休眠形态","各种休眠形态");

	END_GOBJ();

	std::vector<DormantForm> forms;

};



struct BuffArg_Dormant:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Dormant);
	BuffArg_Dormant()
	{
		sitesRevive=NULL;
		nReviveSites=0;
	}
	DWORD nReviveSites;
	i_math::matrix43f *sitesRevive;
};

class Buff_Dormant:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Dormant,14)

	Buff_Dormant()
	{
		_bNeedRevive=0;
		_bRevived=0;
		_nReviveSites=0;
		_sitesRevive=NULL;
		_idReviveTarget=LevelObjID_Invalid;
		_iForm=-1;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Birth的Buff并存的情况

	virtual LevelBuffMask GetReplaceBuffs();

	void Update(AnimTick dt);

	void Revive(LevelObjID idReviveTarget)	
	{		
		_idReviveTarget=idReviveTarget;
		_bNeedRevive=TRUE;	
	}

	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();
	virtual void _OnUpdate(AnimTick t);

	//Factor Overriding
	BuffFlag GetFlags()	
	{		
		if (!_bRevived)
			return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_Birth|BuffFlag_Pausing|BuffFlag_Dormant;	
		return BuffFlag_NotAttackable|BuffFlag_Birth|BuffFlag_Pausing;	

	}

	virtual void _WriteData(CBitPacket *dp);

protected:

	i_math::matrix43f *_sitesRevive;
	WORD _nReviveSites;

	LevelObjID _idReviveTarget;//Revive的目标
	BYTE _bNeedRevive;
	BYTE _bRevived;
	char _iForm;//哪个形态
};



