#pragma once

#include "LevelDeal.h"

#include "Level.h"
#include "LevelRecords.h"

#include "LevelRecordEO.h"

struct LevelRecordEo;
class Deal_CreateSpore:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_CreateSpore);

	BEGIN_GOBJ_PURE(Deal_CreateSpore,1);

		GELEM_VAR_INIT(float,_radiusDetonate,1.0f);
			GELEM_EDITVAR("引爆范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"引爆范围");
	END_GOBJ();

	void Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)override;

	float _radiusDetonate;

};
