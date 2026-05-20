#pragma once

#include "LevelDeal.h"

class Deal_Suck:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_Suck);


	BEGIN_GOBJ_PURE(Deal_Suck,1);

		GELEM_VAR_INIT(int,_mode,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"按照数值吸取HP:0"		"|吸取HP比率,"
				"按照比率吸取HP(未实现):1"	"|吸取HP点数"
				),"模式");
		GELEM_VAR_INIT(int,_nSuck,100);
			GELEM_EDITVAR("吸取HP点数",GVT_S,GSem_Interger,"回复点数");

		GELEM_VAR_INIT(float,_rateSuck,0.0f);
			GELEM_EDITVAR("吸取HP比率",GVT_F,GSem(GSem_Float,"0,1,0.01"),"回复最大HP的比率");
	END_GOBJ();

	int _mode;
	int _nSuck;//回复值
	float _rateSuck;


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

};
