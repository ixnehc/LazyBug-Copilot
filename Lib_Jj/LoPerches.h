#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"

#include "LevelChancer.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"
#include "LevelObjResidable.h"

#include "LevelAttrs.h"

#include "LevelBuff.h"

#define CLASSUID_Perches 10


struct LopPerches:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopPerches,CLASSUID_Perches);

	BEGIN_GOBJ_PURE(LopPerches,1);

		GELEM_VARVECTOR(i_math::matrix43f,sites)
			GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点");
	END_GOBJ();


	std::vector<i_math::matrix43f> sites;//世界空间的位点

};

struct LosPerches:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosPerches,CLASSUID_Perches);

	BEGIN_GOBJ_PURE(LosPerches,1);

		GELEM_VARVECTOR(i_math::matrix43f,sites)
			GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点");

	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

	std::vector<i_math::matrix43f> sites;//局部空间的位点


};


class CLoPerches:public CLoAgent
{
public:
	CLoPerches()
	{
	}
	DEFINE_LEVELOBJ_CLASS(CLoPerches,CLASSUID_Perches);

	virtual const char *GetShowName()	{		return "栖息点";	}

	virtual void PostCreate();
	virtual void OnDestroy();

	virtual BOOL IsServerOnly()	{		return TRUE;	}


protected:


};
