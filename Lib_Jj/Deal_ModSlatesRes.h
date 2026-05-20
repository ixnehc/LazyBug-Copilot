#pragma once

#include "LevelDeal.h"

class Deal_ModSlatesRes:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_ModSlatesRes);


	BEGIN_GOBJ_PURE(Deal_ModSlatesRes,1);

		GELEM_VAR_INIT(int,_nMod,1);
			GELEM_EDITVAR("修改值",GVT_S,GSem_Interger,"修改的Slates资源数量");

	END_GOBJ();

	int _nMod;


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};
