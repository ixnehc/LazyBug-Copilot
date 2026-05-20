#pragma once

#include "LevelBuff.h"

struct BuffParam_Cold:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Cold);

	BEGIN_GOBJ_PURE(BuffParam_Cold,1);
		GELEM_VAR_INIT(float,str,0.8f);
			GELEM_EDITVAR("冷冻强度",GVT_F,GSem(GSem_Float,"0.1,1.0,0.05"),"冷冻强度");
	END_GOBJ();

	float str;
};


struct BuffArg_Cold
{
	DEFINE_CLASS(BuffArg_Cold)
};


//OverAll Speed
class Buff_Cold:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Cold,2)

	Buff_Cold()
	{
		_str=0.0f;
	}

	virtual BOOL NeedSync()override	{		return TRUE;	} //是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Buff并存的情况


	virtual void _OnCreate(LevelBuffArg *param)override;

	virtual BOOL Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)override;


	virtual float GetSlow()override	{		return _str;	}

protected:

	virtual void _WriteData(CBitPacket *dp)override;

	float _str;

};

