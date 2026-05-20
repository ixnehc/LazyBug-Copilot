#pragma once

#include "LevelBuff.h"


struct BuffParam_InSlates:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_InSlates);

	BEGIN_GOBJ_PURE(BuffParam_InSlates,1);

	END_GOBJ();

};

struct BuffArg_InSlates:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_InSlates);
	BuffArg_InSlates()
	{
		idSlates=LevelObjID_Invalid;
	}
	LevelObjID idSlates;
};

struct BuffData_InSlates
{
	BuffData_InSlates()
	{
		idSlates=LevelObjID_Invalid;
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
	LevelObjID idSlates;

};



class Buff_InSlates:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_InSlates,31)

	Buff_InSlates()
	{
	}

	~Buff_InSlates()
	{
	}


	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_GhostCollide;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_InSlates _data;
};

