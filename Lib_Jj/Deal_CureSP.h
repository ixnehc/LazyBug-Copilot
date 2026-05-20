#pragma once

#include "LevelDeal.h"

class Deal_CureSP:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_CureSP);


	BEGIN_GOBJ_PURE(Deal_CureSP,1);


		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"按照数值回复:0"		"|回复比率,"
				"按照比率回复:1"	"|回复点数,"
				"回复自己的生命数值:2"	"|回复点数&回复比率,"
				),"奖励类型");
		GELEM_VAR_INIT(int,_nCure,100);
			GELEM_EDITVAR("回复点数",GVT_S,GSem_Interger,"回复点数");

		GELEM_VAR_INIT(float,_rateCure,0.0f);
			GELEM_EDITVAR("回复比率",GVT_F,GSem(GSem_Float,"0,1,0.01"),"回复最大SP的比率");
	END_GOBJ();

	int _mode;
	BOOL _bMaxHP;
	int _nCure;//回复值
	float _rateCure;//回复比率


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};
