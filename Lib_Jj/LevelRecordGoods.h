#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"



struct LevelRecordGoods:public CRecord
{
	DEFINE_CLASS(LevelRecordGoods);

	std::string Name;
	int hnrMin;
	float wt;

	LevelAward::Type tp;
	RecordID idItem;
	LevelResourceType tpRes;
	int nMin;
	int nMax;
	int price;

    BEGIN_GOBJ_PURE(LevelRecordGoods,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"Goods的名称");

		GELEM_VAR_INIT( int,price,10);	
			GELEM_EDITVAR( "价格", GVT_S, GSem_Interger, "价格" );

		GELEM_VAR_INIT(int,hnrMin,1);
			GELEM_EDITVAR("最小Honor需求",GVT_S,GSem_Interger,"最小Honor需求");

		GELEM_VAR_INIT(float,wt,1.0f);
			GELEM_EDITVAR("权重",GVT_F,GSem(GSem_Float,"0.01,1000000,0.01"),"权重");

		GELEM_VAR_INIT(LevelAward::Type,tp,LevelAward::Item);
		GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,
			"道具:0"		"|资源类型,"
			"资源:2"		"|道具"
			),"奖励类型");

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"哪个道具");

		GELEM_VAR_INIT(LevelResourceType,tpRes,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");

		GELEM_VAR_INIT( int,nMin,1);	
			GELEM_EDITVAR( "最小值", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,5);	
			GELEM_EDITVAR( "最大值", GVT_S, GSem_Interger, "最大值" );

    END_GOBJ();    
};
