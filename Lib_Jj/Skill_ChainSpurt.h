#pragma once

#include "LevelSkill.h"

struct SkillGradeInfo_ChainSpurt
{
	DWORD count;//几次攻击
	AnimTick dur;//在多长时间内
	float rateDmg;
	float range;//延伸距离
	float swing;
	float shift;
	float radius;

	BEGIN_GOBJ_PURE(SkillGradeInfo_ChainSpurt,1);
		GELEM_VAR_INIT(DWORD,count,3);
			GELEM_EDITVAR("喷射次数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"连续攻击的次数");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(1.2f));
			GELEM_EDITVAR("持续喷射时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续喷射时间");
		GELEM_VAR_INIT(float,rateDmg,1.0f);
			GELEM_EDITVAR("伤害比率",GVT_F,GSem(GSem_Float,"0,100,0.1"),"伤害的比例");
		GELEM_VAR_INIT(float,range,5.0f);
			GELEM_EDITVAR("延伸距离",GVT_F,GSem(GSem_Float,"0,100,0.01"),"向前多少距离内喷射");
		GELEM_VAR_INIT(float,swing,1.0f);
			GELEM_EDITVAR("横向偏移范围",GVT_F,GSem(GSem_Float,"0,100,0.01"),"喷射点在和延伸方向垂直的方向上的偏移幅度");
		GELEM_VAR_INIT(float,shift,1.0f);
			GELEM_EDITVAR("纵向偏移范围",GVT_F,GSem(GSem_Float,"0,100,0.01"),"喷射点在延伸方向上的偏移幅度");
		GELEM_VAR_INIT(float,radius,1.0f);
			GELEM_EDITVAR("单个喷射伤害范围",GVT_F,GSem(GSem_Float,"0,100,0.01"),"每一次喷射的影响范围");
	END_GOBJ();

};

struct SkillParam_ChainSpurt:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_ChainSpurt);

	BEGIN_GOBJ_PURE(SkillParam_ChainSpurt,1);

		GELEM_OBJVECTOR(SkillGradeInfo_ChainSpurt,grdinfos)
			GELEM_EDITOBJ("等级参数","等级参数");
		GELEM_VAR_INIT(unsigned __int64,idSpurt,0);
			GELEM_EDITVAR("喷发效果",GVT_Bx8,GSem_ProtoPath,"喷发效果");

	END_GOBJ();
 
	SkillGradeInfo_ChainSpurt *GetGrdInfo(LevelSkillGrade grd)
	{
		SkillGradeInfo_ChainSpurt *info=NULL;
		if (grd!=LevelSkillGrade_Invalid)
		{
			int idx=grd-1;
			if (grdinfos.size()>0)
			{
				if (idx>=grdinfos.size())
					idx=grdinfos.size()-1;
				info=&grdinfos[idx];
			}
		}
		if (!info)
		{
			static SkillGradeInfo_ChainSpurt t;
			info=&t;
		}
		return info;
	}

	std::vector<SkillGradeInfo_ChainSpurt> grdinfos;
	unsigned __int64 idSpurt;

};


class Skill_ChainSpurt:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_ChainSpurt,22);

	Skill_ChainSpurt()
	{
		_nDamages=0;
		_nToDamages=0;

		_tLastHit=0;

		_dur=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_Aim|1<<LevelSkillTarget::Target_DefObj;
	}

protected:
	virtual void _OnStart();
	virtual void _OnBreak()	{		if(_state==SkillState_Casting) _SetState(SkillState_Finished);	}
	virtual void _OnUpdate(AnimTick dt);

	void _UpdateDamage(AnimTick dt);

	CLevelSkillCasting _casting;


	AnimTick _dur;
	BYTE _nDamages;
	BYTE _nToDamages;

	LevelPos _dir;

	AnimTick _tLastHit;

};

