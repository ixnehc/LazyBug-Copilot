#pragma once

#include "LevelSkill.h"

struct SkillGradeInfo_RavenSummon
{
	DWORD count;//召唤几只
	AnimTick dur;//持续时间

	BEGIN_GOBJ_PURE(SkillGradeInfo_RavenSummon,1);
		GELEM_VAR_INIT(DWORD,count,3);
			GELEM_EDITVAR("召唤数量",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"召唤火鸦的数量");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(1.2f));
			GELEM_EDITVAR("火鸦寿命",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"召唤出的火鸦的寿命");
	END_GOBJ();

};

struct SkillParam_RavenSummon:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_RavenSummon);

	BEGIN_GOBJ_PURE(SkillParam_RavenSummon,1);

		GELEM_OBJVECTOR(SkillGradeInfo_RavenSummon,grdinfos)
			GELEM_EDITOBJ("等级参数","等级参数");

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位ID",GVT_U,GSem(GSem_RecordID,"units"),"召唤什么单位");

		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("出生的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位召唤出来后,加什么出生Buff");

		GELEM_VAR_INIT(RecordID,idVanishBuff,RecordID_Invalid);
			GELEM_EDITVAR("消失的死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位召唤出新的单位后,会杀死一个旧的单位,使用哪个Buff");

	END_GOBJ();
 
	SkillGradeInfo_RavenSummon *GetGrdInfo(LevelSkillGrade grd)
	{
		SkillGradeInfo_RavenSummon *info=NULL;
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
			static SkillGradeInfo_RavenSummon t;
			info=&t;
		}
		return info;
	}

	std::vector<SkillGradeInfo_RavenSummon> grdinfos;

	RecordID idBirthBuff;
	RecordID idVanishBuff;
	RecordID idUnit;

};


class Skill_RavenSummon:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_RavenSummon,15);

	Skill_RavenSummon()
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

	void _UpdateRetinueCount(DWORD nMax,RecordID idUnit,RecordID idVanishBuff,CLoUnit *loIgnore);

	AnimTick _tCasting;

	BOOL _bSummon;
};

