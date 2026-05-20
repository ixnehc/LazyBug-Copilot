#pragma once

#include "LevelSkill.h"


struct SkillParam_UtumSummon:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_UtumSummon);

	BEGIN_GOBJ_PURE(SkillParam_UtumSummon,1);

		GELEM_VAR_INIT(DWORD,count,3);
			GELEM_EDITVAR("召唤数量",GVT_S,GSem_Interger,"召唤单位的数量");
		GELEM_VAR_INIT(DWORD,countMax,12);
			GELEM_EDITVAR("最多召唤数量",GVT_S,GSem_Interger,"一次最多召唤几个乌图姆");

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位ID",GVT_U,GSem(GSem_RecordID,"units"),"召唤什么单位");

		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("出生的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位召唤出来后,加什么出生Buff");

		GELEM_VAR_INIT( StringID,nmThrustEuler,StringID_Invalid);	
			GELEM_EDITVAR( "冲刺角度变量名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "冲刺角度变量名称" );
		GELEM_VAR_INIT( StringID,nmAttackTarget,StringID_Invalid);	
			GELEM_EDITVAR( "攻击对象变量名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "攻击对象变量名称" );

		GELEM_VAR_INIT(RecordID,idAttackSkill,RecordID_Invalid);
			GELEM_EDITVAR("攻击的Skill",GVT_U,GSem(GSem_RecordID,"skills"),"单位召唤出来后,用什么Skill进行攻击");

		GELEM_VAR_INIT(float,rangeAttack,10.0f);
			GELEM_EDITVAR("攻击范围",GVT_F,GSem(GSem_Float,"2,40,0.01"),"召唤出的单位对多大范围内的单位进行攻击");
		GELEM_VAR_INIT(float,distReturn,5.0f);
			GELEM_EDITVAR("返回范围",GVT_F,GSem(GSem_Float,"2,40,0.01"),"召唤出的单位在多大范围外开始返回");

		GELEM_VAR_INIT(float,fovMin,30.0f);
			GELEM_EDITVAR("最小侦测张角",GVT_F,GSem(GSem_Float,"0,360,0.1"),"最小的侦测张角");
		GELEM_VAR_INIT(float,fovMax,120.0f);
			GELEM_EDITVAR("最大侦测张角",GVT_F,GSem(GSem_Float,"0,360,0.1"),"最大的侦测张角");
		GELEM_VAR_INIT(float,descendMax,45.0f);
			GELEM_EDITVAR("最大下降角度",GVT_F,GSem(GSem_Float,"0,360,0.1"),"最大的侦测张角");

		GELEM_VAR_INIT(AnimTick,durThrust,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("冲刺时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"冲刺持续多久");
		GELEM_VAR_INIT(AnimTick,durPostAttack,ANIMTICK_FROM_SECOND(0.4f));
			GELEM_EDITVAR("攻击后滑行时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"攻击后滑行多长时间后返回");

		GELEM_VAR_INIT(RecordID,idReturnSkill,RecordID_Invalid);
			GELEM_EDITVAR("返回的Skill",GVT_U,GSem(GSem_RecordID,"skills"),"单位返回时,用什么Skill进入玩家身体");
		GELEM_VAR_INIT(AnimTick,durMinReturnFollow,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("返回跟踪时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"返回时允许最多跟踪多长时间(最小值)");
		GELEM_VAR_INIT(AnimTick,durMaxReturnFollow,ANIMTICK_FROM_SECOND(4.0f));
			GELEM_EDITVAR("返回跟踪时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"返回时允许最多跟踪多长时间(最大值)");
		GELEM_VAR_INIT(RecordID,idDirectlyReturnSkill,RecordID_Invalid);
			GELEM_EDITVAR("直接返回的Skill",GVT_U,GSem(GSem_RecordID,"skills"),"单位返回时,用什么Skill直接进入玩家身体");


	END_GOBJ();
 
	RecordID idBirthBuff;
	RecordID idUnit;
	DWORD count;
	DWORD countMax;
	StringID nmThrustEuler;//冲刺角度变量名称
	StringID nmAttackTarget;//冲刺角度变量名称

	//单位冲刺时用到的参数
	RecordID idAttackSkill;
	float rangeAttack;
	float fovMin;
	float fovMax;
	float descendMax;
	AnimTick durThrust;
	AnimTick durPostAttack;
	float distReturn;

	//单位返回时用到的参数
	AnimTick durMinReturnFollow;//返回时允许跟踪多长时间
	AnimTick durMaxReturnFollow;//返回时允许跟踪多长时间
	RecordID idReturnSkill;
	RecordID idDirectlyReturnSkill;


};

#define MAX_UTUM_SUMMON_OFFSET (10.0f)

struct UtumSummonArg
{
	LevelPos GetOff()
	{
		return LevelPos( ((float)xOff)/(float)30000.0f*MAX_UTUM_SUMMON_OFFSET,
									((float)yOff)/(float)30000.0f*MAX_UTUM_SUMMON_OFFSET );
	}
	void SetOff(float x,float y)
	{
		x=i_math::clamp_f(x,-MAX_UTUM_SUMMON_OFFSET,MAX_UTUM_SUMMON_OFFSET);
		y=i_math::clamp_f(y,-MAX_UTUM_SUMMON_OFFSET,MAX_UTUM_SUMMON_OFFSET);

		xOff=(short)(x/MAX_UTUM_SUMMON_OFFSET*30000.0f);
		yOff=(short)(y/MAX_UTUM_SUMMON_OFFSET*30000.0f);
	}
	short xOff;
	short yOff;
	LevelObjID idTarget;
};


class Skill_UtumSummon:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_UtumSummon,38);

	Skill_UtumSummon()
	{
		_bSummon=FALSE;
		_tCasting=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_Aim;
	}


protected:
	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}
	virtual void _OnUpdate(AnimTick dt);

	void _UpdateSummon(AnimTick dt);

	void _DoSummon();

	AnimTick _tCasting;

	BOOL _bSummon;
};

