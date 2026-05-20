#pragma once

#include "LevelBuff.h"


struct BuffArg_Retinue:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Retinue);
	LevelPlayerID idPlayer;
};

struct BuffData_Retinue
{
	BuffData_Retinue()
	{
		memset(this,0,sizeof(*this));
	}
	LevelPlayerID idPlayer;
};

class Buff_Retinue:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Retinue,12)

	Buff_Retinue()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Retinue的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	BOOL CanTeleport()	{		return TRUE;	}
	void LoadTeleport(CLevelBuff *buffOrg)
	{
		_data=((Buff_Retinue*)buffOrg)->_data;
	}


	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;	}

protected:

	virtual void _WriteData(CDataPacket *dp);

	BuffData_Retinue _data;


};

