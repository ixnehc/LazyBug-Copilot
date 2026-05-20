#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"
#include "LevelSlateDefines.h"

#include "LevelChancer.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#include "LevelAttrs.h"

#include "LevelBuff.h"

#define CLASSUID_Slate 41


struct LopSlate:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopSlate,CLASSUID_Slate);

	BEGIN_GOBJ_PURE(LopSlate,1);

// 		GELEM_VAR_INIT(LevelSlateType,tp,LevelSlateTypeA_Blank);
// 			GELEM_EDITVAR("类型",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateTypeA),"类型");
	END_GOBJ();


//	LevelSlateType tp;


};

struct LosSlate:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosSlate,CLASSUID_Slate);

	BEGIN_GOBJ_PURE(LosSlate,1);

		GELEM_ALLOWDISABLE();
		GELEM_AGENTRECORD();

		GELEM_VARVECTOR(i_math::matrix43f,links)
			GELEM_EDITVAR("链接位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"链接位点");

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	std::vector<i_math::matrix43f> links;//链接位点


};


