#pragma once

#include "LevelBuff.h"


struct BuffArg_MindCtrl:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_MindCtrl);
	LevelPlayerID idPlayer;
};

struct BuffData_MindCtrl
{
	BuffData_MindCtrl()
	{
		memset(this,0,sizeof(*this));
	}
	LevelPlayerID idPlayer;
};

class Buff_MindCtrl:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_MindCtrl,12)

	Buff_MindCtrl()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Retinue的Buff并存的情况


	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);

	BOOL CanTeleport()	{		return TRUE;	}
	void LoadTeleport(CLevelBuff *buffOrg)
	{
		_data=((Buff_MindCtrl*)buffOrg)->_data;
	}


	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	BuffData_MindCtrl _data;


};

