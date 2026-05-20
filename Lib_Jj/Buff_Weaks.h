#pragma once

#include "LevelBuff.h"
#include "LevelAttrs_DamageAttr.h"

struct BuffParam_Weaks:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Weaks);

	BEGIN_GOBJ_PURE(BuffParam_Weaks,1);

		GELEM_VAR_INIT(LevelWeakCategoryMask,categories,0)
			GELEM_EDITVAR("弱点类型",GVT_U,GSem(GSem_Flags,
			"硬直(前方)|硬直(前方):1,"
			"硬直(两侧)|硬直(两侧):8,"
			"硬直(背面)|硬直(背面):2,"
			"击退|击退:4"
			"快闪|快闪:16"
			),"弱点类型");
		//XXXXX: more LevelWeakCategory

		GELEM_VAR_INIT(DamageAttrMask,stunFwd,0);GELEM_UID(1)
			GELEM_EDITVAR("硬直(前方)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(全身)");
		GELEM_VAR_INIT(DamageAttrMask,stunSide,0);GELEM_UID(6)
			GELEM_EDITVAR("硬直(两侧)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(两侧)");
		GELEM_VAR_INIT(DamageAttrMask,stunBack,0);GELEM_UID(2)
			GELEM_EDITVAR("硬直(背面)",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"硬直弱点(背面)");
		GELEM_VAR_INIT(DamageAttrMask,kb,0);GELEM_UID(3)
			GELEM_EDITVAR("击退",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"击退");
		GELEM_VAR_INIT(DamageAttrMask,jink,0);GELEM_UID(7)
			GELEM_EDITVAR("快闪",GVT_U,GSem(GSem_Flags,DamageAttrMask_SemConstraint_Weaks),"快闪");

		GELEM_VAR_INIT(DWORD,nPsvtReq,0);GELEM_UID(4)
			GELEM_EDITVAR("弱点被击破几次后开始钝化",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"弱点被击破几次后开始钝化,0表示永不钝化");
		GELEM_VAR_INIT(AnimTick,durPsvtCD,ANIMTICK_FROM_SECOND(1.0f));GELEM_UID(5)
			GELEM_EDITVAR("钝化CD时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"钝化后多久恢复,0表示永不恢复");
	END_GOBJ();

	LevelWeakCategoryMask categories;

	DamageAttrMask stunFwd;
	DamageAttrMask stunSide;
	DamageAttrMask stunBack;
	DamageAttrMask kb;
	DamageAttrMask jink;
	//XXXXX: more LevelWeakCategory

	DWORD nPsvtReq;
	AnimTick durPsvtCD;
};


struct BuffArg_Weaks:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Weaks)
};


class Buff_Weaks:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Weaks,35)

	Buff_Weaks()
	{
	}


	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;

	virtual void HandleEvent(LevelEvent &e) override;

protected:

	struct WeakStatus
	{
		WeakStatus()
		{
			category=LevelWeakCategory_None;
			weak=0;
			nBroken=0;
			tRecentBroken=0;
		}

		LevelWeakCategory category;
		DamageAttrMask weak;
		
		DWORD nBroken;
		AnimTick tRecentBroken;
	};

	std::deque<WeakStatus> _statusWeaks;


};

