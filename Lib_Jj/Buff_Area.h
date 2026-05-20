#pragma once

#include "LevelBuff.h"

struct BuffParam_Area:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Area);

	BEGIN_GOBJ_PURE(BuffParam_Area,1);

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("影响范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"影响范围");
		GELEM_VAR_INIT(float,speed,1.0f);
			GELEM_EDITVAR("每秒作用几次",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"每秒作用几次");

	END_GOBJ();

	float radius;
	float speed;

};


struct BuffArg_Area
{
	DEFINE_CLASS(BuffArg_Area)
};


//OverAll Speed
class Buff_Area:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Area,52)

	Buff_Area()
	{
		_nDealed=0;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}

	virtual void _OnUpdate(AnimTick dt);



protected:

	int _nDealed;


};

