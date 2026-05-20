#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_BulletBomb 32

struct EoParamBulletBomb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamBulletBomb);

	BEGIN_GOBJ_PURE(EoParamBulletBomb,1);

		GELEM_VAR_INIT(float,radius,0.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(AnimTick,delay,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("延迟爆炸时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"延迟多久爆炸");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("爆炸持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"爆炸持续多长时间,0表示瞬间爆炸");

		GELEM_VAR_INIT(DWORD,nBullet,3);
			GELEM_EDITVAR("子弹个数",GVT_U,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10,11:11,12:12,13:13,14:14,15:15"),"有几个子弹");
	END_GOBJ();

	float radius;
	AnimTick delay;
	AnimTick dur;
	int nBullet;
};



class EoBulletBomb:public CLoEffectObj
{
public:
	EoBulletBomb()
	{
		_bBurst=FALSE;
		_nBurst=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoBulletBomb,CLASSUID_BulletBomb);

	virtual const char *GetShowName()	{		return "子弹爆炸";	}

protected:

	void _OnUpdate();

	BOOL _bBurst;
	int _nBurst;

};
