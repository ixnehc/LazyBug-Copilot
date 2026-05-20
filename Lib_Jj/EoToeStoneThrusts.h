#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "Protocal.h"


#include "LoEffectObj.h"

#define CLASSUID_ToeStoneThrusts 43

struct EoParamToeStoneThrusts:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamToeStoneThrusts);

	BEGIN_GOBJ_PURE(EoParamToeStoneThrusts,1);

		GELEM_VAR_INIT(float,nThrustsPerSec,8.0f);
			GELEM_EDITVAR("每秒几次爆炸",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"每秒几次爆炸");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal, dealThrust,Deal_CreateEo, "结算(obsolete)", "选择不同的技能命中效果" );
            GELEM_DYNOBJPTR_CLASS_DEAL("04.创建Eo", Deal_CreateEo);

	END_GOBJ();

    float radiusGround;
    float nThrustsPerSec;

    CLevelDeal *dealThrust;
};



class EoToeStoneThrusts:public CLoEffectObj
{
public:
	EoToeStoneThrusts()
	{
		_nCommits=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoToeStoneThrusts,CLASSUID_ToeStoneThrusts);

	virtual const char *GetShowName()	{		return "有毒区域";	}

    void SetSites(i_math::pos2d_sh &ptBase, i_math::pos2db *sites, DWORD nSites);

protected:
	virtual void _OnPostCreate() override{}

	virtual void _OnUpdate() override;

    i_math::vector2df _sites[MAX_TOESTONE_THRUST_SITES];
    DWORD _nSites;
    std::vector<char> _indices;

    DWORD _nCommits;


};
