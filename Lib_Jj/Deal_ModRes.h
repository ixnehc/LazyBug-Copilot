#pragma once

#include "LevelDeal.h"

class Deal_ModRes:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_ModRes);


	BEGIN_GOBJ_PURE(Deal_ModRes,1);

		GELEM_VAR_INIT(int,_nMod,1);
			GELEM_EDITVAR("修改值",GVT_S,GSem_Interger,"修改的资源数量");

		GELEM_VAR_INIT(LevelResourceType,_tp,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");

	END_GOBJ();

	LevelResourceType _tp;
	int _nMod;


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};
