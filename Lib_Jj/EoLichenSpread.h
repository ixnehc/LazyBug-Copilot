#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "EoEnv.h"

#define CLASSUID_LichenSpread 56

class CCubicSpline;
struct EoParamLichenSpread:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamLichenSpread);

	BEGIN_GOBJ_PURE(EoParamLichenSpread,1);

		GELEM_VAR_INIT(float,fov,120.0f);
			GELEM_EDITVAR("FOV",GVT_F,GSem(GSem_Float,"0.1,180.0,0.05"),"FOV");
		GELEM_VAR_INIT(DWORD,nBranches,6);
			GELEM_EDITVAR("分支个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"分支个数");
		GELEM_VAR_INIT(DWORD,nSegsPerBranch,6);
			GELEM_EDITVAR("每分支段数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"每分支段数");
		GELEM_VAR_INIT(float,lengthBranch,20.0f);
			GELEM_EDITVAR("分支长度",GVT_F,GSem(GSem_Float,"0.1,30.0,0.05"),"分支长度");

		GELEM_VAR_INIT(float,radiusMin,1.0f);
			GELEM_EDITVAR("最小地丝半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.01"),"最小地丝半径");
		GELEM_VAR_INIT(float,radiusMax,3.0f);
			GELEM_EDITVAR("最大地丝半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.01"),"最大地丝半径");

		GELEM_VAR_INIT(AnimTick,durStart,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("发射阶段持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"发射阶段持续时间");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("发射后持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"发射后持续时间");
		GELEM_VAR_INIT(AnimTick,durEnd,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("收回阶段持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"收回阶段持续时间");
		GELEM_VAR_INIT(AnimTick,durFI,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("淡入时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"淡入时间");
		GELEM_VAR_INIT(AnimTick,durFO,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("淡出时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"淡出时间");

	END_GOBJ();

	float radiusMax;
	float radiusMin;
	float fov;
	DWORD nBranches;
	DWORD nSegsPerBranch;
	float lengthBranch;
	AnimTick durStart;
	AnimTick dur;
	AnimTick durEnd;
	AnimTick durFI;
	AnimTick durFO;

};


class EoLichenSpread:public CLoEffectObj
{
public:
	EoLichenSpread()
	{
		_nBranches=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoLichenSpread,CLASSUID_LichenSpread);

	virtual const char *GetShowName()	{		return "地丝";	}

protected:

	struct Sample
	{
		LevelPos pos;
		EoEnvLichenHandle h;
		float radius;
		AnimTick tStart;
		AnimTick tEnd;
	};

	struct Branch
	{
		std::deque<Sample> samples;
	};

	virtual void _OnPostCreate() override;
	virtual void _OnDetroy() override;

	virtual void _OnUpdate() override;

	void _BuildBranch(LevelFace face,Branch &branch);
	void _BuildBranches();

	CCubicSpline &GetWorkingSpline();

	Branch _branches[20];//Big enough
	DWORD _nBranches;


};
