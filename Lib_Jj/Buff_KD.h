#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"
#include "LevelObjPauser.h"

struct BuffParam_KD:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_KD);

	BEGIN_GOBJ_PURE(BuffParam_KD,1);

		GELEM_VAR_INIT(unsigned __int64,knockdown,0);
			GELEM_EDITVAR("倒下效果",GVT_Bx8,GSem_ProtoPath,"倒下效果");
		GELEM_VAR_INIT(AnimTick,durKnockDown,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("倒下时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"倒下过程有多长时间,单位为秒");
		GELEM_VAR_INIT(unsigned __int64,raiseup,0);
			GELEM_EDITVAR("爬起效果",GVT_Bx8,GSem_ProtoPath,"爬起效果");
		GELEM_VAR_INIT(AnimTick,durRaiseUp,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("爬起时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"爬起过程有多长时间,单位为秒");
		GELEM_VAR_INIT(float,dist,1.0f);
			GELEM_EDITVAR("退后距离",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"退后距离");
	END_GOBJ();

	unsigned __int64 knockdown;
	AnimTick durKnockDown;
	unsigned __int64 raiseup;
	AnimTick durRaiseUp;
	float dist;


};


struct BuffArg_KD:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_KD);
	i_math::vector2df dir;
};

struct BuffData_KD
{
	BuffData_KD()
	{
		memset(this,0,sizeof(*this));
	}
	LevelPos target;//击倒到的位置
	float face;
	LevelSkillID idBroken;
	LevelTeleportID idTeleport;
	BYTE bKB;
};
 
class Buff_KD:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_KD,5)

	Buff_KD()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个KD的Buff并存的情况

	virtual LevelBuffMask GetForbiddingBuffs();
	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_LayDown|BuffFlag_Pausing;	}

	void RaiseUp();

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_KD _data;

};

