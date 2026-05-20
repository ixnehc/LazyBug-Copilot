#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Area 24

struct EoParamArea:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamArea);

	BEGIN_GOBJ_PURE(EoParamArea,1);

		GELEM_VAR_INIT(BOOL,bBindSkill,FALSE);
			GELEM_EDITVAR("绑定施放技能",GVT_S,GSem_Boolean,"绑定创建出这个Eo的技能");

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"持续时间,如果为0,则只在一开始产生伤害");


		GELEM_VAR_INIT(int,cycle,1);
			GELEM_EDITVAR("作用周期",GVT_S,GSem(GSem_Interger,"x1:1,x2:2,x3:3,x4:4,x5:5,x6:6,x7:7,x8:8"),"每次作用的周期倍率(基本值为0.2秒一次作用)");

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");

		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("作用对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"作用对象的特定需求");

	END_GOBJ();

	BOOL bBindSkill;

	float radius;
	AnimTick dur;

	int cycle;

	std::vector<LevelDetectTargetFlag> flagsDetect;
	std::vector<LevelObjRequire> requires;
};



class EoArea:public CLoEffectObj
{
public:
	EoArea()
	{
		_iCycle=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoArea,CLASSUID_Area);

	virtual const char *GetShowName()	{		return "区域影响";	}

protected:
	void _OnUpdate();
	DWORD _iCycle;
};
