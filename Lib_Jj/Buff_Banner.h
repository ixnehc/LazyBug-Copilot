#pragma once

#include "LevelBuff.h"

struct BuffParam_Banner:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Banner);

	BEGIN_GOBJ_PURE(BuffParam_Banner,1);

		GELEM_VAR_INIT(unsigned __int64,idBannerAtAgent,0);
			GELEM_EDITVAR("Agent上的旗帜Proto",GVT_Bx8,GSem_ProtoPath,"Agent上的旗帜Proto");
		GELEM_VAR_INIT(unsigned __int64,idBannerInHand,0);
			GELEM_EDITVAR("手中的旗帜Proto",GVT_Bx8,GSem_ProtoPath,"手中的旗帜Proto");
		GELEM_VAR_INIT(unsigned __int64,idBannerOnGround,0);
			GELEM_EDITVAR("地上的旗帜Proto",GVT_Bx8,GSem_ProtoPath,"地上的旗帜Proto");
		GELEM_VARVECTOR_INIT(StringID,actsInHand,StringID_Invalid);
			GELEM_EDITVAR( "可以拿着旗帜的Act类型", GVT_U, GSem(GSem_StringID,"ActType"), "可以拿着旗帜的Act类型" );

	END_GOBJ();

	unsigned __int64 idBannerAtAgent;
	unsigned __int64 idBannerInHand;
	unsigned __int64 idBannerOnGround;
	std::vector<StringID> actsInHand;

};


struct BuffArg_Banner
{
	DEFINE_CLASS(BuffArg_Banner)
};


//OverAll Speed
class Buff_Banner:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Banner,49)

	Buff_Banner()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//时间到结束时是否需要同步给客户端


	virtual void _OnCreate(LevelBuffArg *param)
	{
	}

	virtual void _OnUpdate(AnimTick dt);



protected:


};

