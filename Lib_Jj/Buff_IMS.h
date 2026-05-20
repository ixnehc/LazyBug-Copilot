#pragma once

#include "LevelBuff.h"

struct BuffParam_IMS:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_IMS);

	BEGIN_GOBJ_PURE(BuffParam_IMS,1);

		GELEM_VAR_INIT(float,ims,0.0f);
			GELEM_EDITVAR("速度增加多少",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"速度增加倍率");

	END_GOBJ();
	float ims;
};


struct BuffArg_IMS
{
	DEFINE_CLASS(BuffArg_IMS)
};


class Buff_IMS:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_IMS,28)

	Buff_IMS()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}


	virtual float GetIMS();

protected:


};

