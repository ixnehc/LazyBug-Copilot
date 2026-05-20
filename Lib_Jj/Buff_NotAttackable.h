#pragma once

#include "LevelBuff.h"


struct BuffParam_NotAttackable:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_NotAttackable);

	BEGIN_GOBJ_PURE(BuffParam_NotAttackable,1);

	END_GOBJ();

};

struct BuffArg_NotAttackable:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_NotAttackable);
};

struct BuffData_NotAttackable
{
	BuffData_NotAttackable()
	{
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
};



class Buff_NotAttackable:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_NotAttackable,40)

	Buff_NotAttackable()
	{
	}

	~Buff_NotAttackable()
	{
	}


	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Invisible的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();
	virtual LevelBuffMask GetForbiddingBuffs();

	virtual void _OnCreate(LevelBuffArg *param);


	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_NotAttackable _data;
};

