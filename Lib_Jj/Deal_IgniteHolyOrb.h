#pragma once

#include "LevelDeal.h"

class CLoAgent;
class Deal_IgniteHolyOrb:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_IgniteHolyOrb);


	BEGIN_GOBJ_PURE(Deal_IgniteHolyOrb,1);

		GELEM_VAR_INIT(RecordID,_idAgent,RecordID_Invalid);
			GELEM_EDITVAR("圣光石Agent",GVT_U,GSem(GSem_RecordID,"agents"),"Agent");

		GELEM_VAR_INIT(RecordID,_idEo,RecordID_Invalid);
			GELEM_EDITVAR("HolyOrb Eo",GVT_U,GSem(GSem_RecordID,"eos"),"EO");

		GELEM_VAR_INIT(float,_radius,1.0f);
			GELEM_EDITVAR("引爆范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"引爆范围");
 		GELEM_BEHAVIORMEM_RESOURCERECORD(_nmVarPathRes,"路径资源变量","路径资源变量");
		GELEM_BEHAVIORMEM_INTERGER(_nmVarIsActive,"是否激活变量","是否激活变量");
	END_GOBJ();


	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;
	void Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)override;

public:
	RecordID _idAgent;
	RecordID _idEo;
	float _radius;
	StringID _nmVarPathRes;//从Agent的哪个变量里取得路径资源
	StringID _nmVarIsActive;//从Agent的哪个变量里取得圣光石是否激活的标志

	BOOL _DetectEnumCallBack(CLevelObj *lo,float dist2);
	LevelPos3D _posSrc;

	void _DoIgnite(LevelOSB &osbSrc,CLoAgent *loAgent,DealArg&arg);


};
