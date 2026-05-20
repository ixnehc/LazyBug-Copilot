#pragma once

#include "LevelBuff.h"


struct BuffParam_Immune:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Immune);

	BEGIN_GOBJ_PURE(BuffParam_Immune,1);

	END_GOBJ();

};

struct BuffArg_Immune:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Immune);
};

struct BuffData_Immune
{
	BuffData_Immune()
	{
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
};



class Buff_Immune:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Immune,24)

	Buff_Immune()
	{
	}

	~Buff_Immune()
	{
	}


	virtual int AddRef()
	{
		return __super::AddRef();
	}

	virtual int Release()
	{
		return __super::Release();
	}



	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Invisible的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_DamageImmune;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_Immune _data;
};

