#pragma once

#include "ActBase.h"


struct ActParam_AttackOnce3D:public ActParam
{
	DEFINE_CLASS(ActParam_AttackOnce3D);
	RecordID idSkill;
	float rangeMin;
	float rangeMax;
	float rangeVerMin;
	float rangeVerMax;

	BEGIN_GOBJ_PURE(ActParam_AttackOnce3D,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"施放的技能");
		GELEM_VAR_INIT(float,rangeMin,1.0f);
			GELEM_EDITVAR("最小范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个范围以外");
		GELEM_VAR_INIT(float,rangeMax,5.0f);
			GELEM_EDITVAR("最大范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个范围以内");
		GELEM_VAR_INIT(float,rangeVerMin,1.0f);
			GELEM_EDITVAR("最低范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个(相对)高度以上");
		GELEM_VAR_INIT(float,rangeVerMax,5.0f);
			GELEM_EDITVAR("最高范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"至少要在这个(相对)高度以下");
	END_GOBJ();
};


//逃跑
class Act_AttackOnce3D:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_AttackOnce3D);

	Act_AttackOnce3D()
	{
		_tStart=0;
		_result=A_Pending;
		_bAttacking=FALSE;
	}

	void Start(AnimTick t,LevelObjID idTarget);
	virtual void Finish()		{		}
	void Update(AnimTick t);

	AResult GetResult()	{		return _result;	}
protected: 
	void _Update(AnimTick t);
	LevelObjID _idTarget;
	AResult _result;
	AnimTick _tStart;
	BOOL _bAttacking;

};

