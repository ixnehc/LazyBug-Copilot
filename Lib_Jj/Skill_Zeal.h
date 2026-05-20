#pragma once

#include "LevelSkill.h"

struct SkillGradeInfo_Zeal
{
	DWORD count;//几次攻击
	AnimTick dur;//在多长时间内

	BEGIN_GOBJ_PURE(SkillGradeInfo_Zeal,1);
		GELEM_VAR_INIT(DWORD,count,3);
			GELEM_EDITVAR("攻击次数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"连续攻击的次数");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(1.2f));
			GELEM_EDITVAR("攻击时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"攻击持续时间");
	END_GOBJ();

};

struct SkillParam_Zeal:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Zeal);

	enum AimMode
	{
		Aim_None,
		Aim_Directional,
		Aim_ToAimPos,

		Aim_ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(SkillParam_Zeal,1);

		GELEM_VAR_INIT(BOOL,bMelee,TRUE);
//			GELEM_EDITVAR("近战模式",GVT_S,GSem_Boolean,"近战模式");
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"近战模式:1"		"|瞄准模式&发射高度&发射个数,"
				"远程模式:0"		""
				),"攻击模式");

		GELEM_VAR_INIT(AimMode,modeAim,Aim_Directional);
			GELEM_EDITVAR("瞄准模式",GVT_U,GSem(GSem_Interger,"与地面平行:1,对准目标单位瞄准点:2"),"瞄准模式");

		GELEM_VAR_INIT(float,htCast,0.5f);
			GELEM_EDITVAR("发射高度",GVT_F,GSem(GSem_Float,"0.0,10.0,0.01"),"发射高度");

		GELEM_VAR_INIT(int,countCast,1);
			GELEM_EDITVAR("发射个数",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10"),"发射个数");

		GELEM_VAR_INIT(BOOL,bAutoTargetObj,TRUE);
			GELEM_EDITVAR("自动选择目标",GVT_S,GSem_Boolean,"自动对准目标");

		GELEM_OBJVECTOR(SkillGradeInfo_Zeal,grdinfos)
			GELEM_EDITOBJ("等级参数","等级参数");

		GELEM_VAR_INIT(AnimTick,durCast,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("释放效果时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"单次攻击效果的时间长度,单位为秒(所有的释放效果长度必须一致)");

		GELEM_VARARRAY_INIT(unsigned __int64,casts,0);
			GELEM_VERSION(2);
			GELEM_EDITVAR("释放效果",GVT_Bx8,GSem(GSem_ProtoPath,
				"$Lable{//n/a,徒手,//单手盾,单手短兵器+盾,单手短兵器,单手长兵器+盾,单手长兵器,双手剑,弓,弩,双手矛,双手斧,双手杖,双手拖刀,双持剑,单手魔法物件+盾,单手魔法物件}"),
				"各种姿势的释放效果,注意所有的释放效果的时间长度必须一致");
		//XXXXX:more LevelPostureType



	END_GOBJ();
 
	SkillGradeInfo_Zeal *GetGrdInfo(LevelSkillGrade grd)
	{
		SkillGradeInfo_Zeal *info=NULL;
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
			static SkillGradeInfo_Zeal t;
			info=&t;
		}
		return info;
	}

	BOOL bAutoTargetObj;

	AimMode modeAim;
	float htCast;
	int countCast;

	std::vector<SkillGradeInfo_Zeal> grdinfos;

	unsigned __int64 casts[LevelPosture_Max];

	AnimTick durCast;

	BOOL bMelee;

};


class Skill_Zeal:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Zeal,14);

	Skill_Zeal()
	{
		_nDamages=0;
		_nToDamages=0;

		_tCasting=0;
		_tLastHit=0;

		_dur=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_DefObj)|(1<<LevelSkillTarget::Target_Aim);
	}

protected:
	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}
	virtual void _OnUpdate(AnimTick dt);

	void _UpdateDamage(AnimTick dt);

	AnimTick _tCasting;

	AnimTick _dur;
	BYTE _nDamages;
	BYTE _nToDamages;

	AnimTick _tLastHit;

	LevelPos3D _GetTarget(CLevelObj *&lo,int iCast);
	std::unordered_map<LevelObjID,LevelPos3D> _targetsitesCache;


};

