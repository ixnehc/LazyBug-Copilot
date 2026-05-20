#pragma once

#include "LevelDeal.h"

struct SummonUnitCategory
{
	enum FacingMode
	{
		Facing_Random,
		Facing_FaceToOrg,
		Facing_BackToOrg,

		Facing_ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(SummonUnitCategory,1);
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("召唤的单位",GVT_U,GSem(GSem_RecordID,"units"),"召唤的单位");
		GELEM_VAR_INIT(RecordID,idBirth,RecordID_Invalid);
			GELEM_EDITVAR("出生的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"召唤后的单位的出生Buff");
		GELEM_VAR_INIT(RecordID,idBirthSkill,RecordID_Invalid);
			GELEM_EDITVAR("出生的Skill",GVT_U,GSem(GSem_RecordID,"skills"),"召唤后的单位的出生Skill");
		GELEM_VAR_INIT(int,tpBirthSkillTarget,0);
			GELEM_EDITVAR("出生Skill的Target类型",GVT_U,GSem(GSem_Interger,"没有Target,使用Owner当前技能的Target"),"出生Skill的Target类型");
		GELEM_VAR_INIT(float,radius,0.2f);
			GELEM_EDITVAR("释放范围",GVT_F,GSem(GSem_Float,"0.0,10,0.01"),"释放的范围,单位为米");
		GELEM_VAR_INIT(float,fov,360.0f);
			GELEM_EDITVAR("施放Fov",GVT_F,GSem(GSem_Float,"0.0,360,0.01"),"释放的角度范围");

		GELEM_VAR_INIT(float,ht,0.0f);
			GELEM_EDITVAR("离地高度",GVT_F,GSem(GSem_Float,"0.00,10,0.01"),"离地高度,单位为米");
		GELEM_VAR_INIT(FacingMode,modeFacing,Facing_Random);
			GELEM_EDITVAR("朝向模式",GVT_U,GSem(GSem_Interger,"随机,朝向召唤原点,背对召唤原点"),"朝向模式");
		GELEM_VAR_INIT(int,count,1);
			GELEM_EDITVAR("个数",GVT_S,GSem(GSem_Interger,"缺省,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"个数");

		GELEM_VAR_INIT(BOOL,bRetinue,TRUE);
			GELEM_EDITVAR("是否是随从",GVT_S,GSem_Boolean,"创建的单位是否要成为随从");
	END_GOBJ();

	RecordID idUnit;
	RecordID idBirth;//出生Buff
	RecordID idBirthSkill;//出生Skill
	int tpBirthSkillTarget;//出生Skill的目标
	float fov;
	float radius;
	float ht;
	FacingMode modeFacing;
	int count;
	BOOL bRetinue;

};


class Deal_SummonUnit:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_SummonUnit);


	BEGIN_GOBJ_PURE(Deal_SummonUnit,1);

		GELEM_OBJVECTOR(SummonUnitCategory,_cats);
			GELEM_EDITOBJ("召唤的单位","召唤的各种单位");
		GELEM_VAR_INIT(Mode,_mode,UsingOrgPos);
			GELEM_EDITVAR("召唤位置产生模式",GVT_S,GSem(GSem_Interger,"使用召唤者脚下位置(2D+离地高度),使用目标点(2D+离地高度),使用目标点(3D,飞行模式)"),"用什么方式产生召唤位置");

	END_GOBJ();

	enum Mode
	{
		UsingOrgPos=0,
		UsingTargetPos,
		UsingTargetPos3D,

		ForceDword=0xffffffff,
	};


	std::vector<SummonUnitCategory>_cats;
	Mode _mode;

	void Make(LevelOSB &osbSrc,LevelPos3D &pos3DTarget,DealArg&arg,DealResult *result)override;
	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};
