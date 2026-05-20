#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

struct LevelRecordRes:public CRecord
{
	DEFINE_CLASS(LevelRecordRes);

	std::string Name;

	std::string path;

	float scalePathSpeed;

	float facePath;

	BEGIN_GOBJ_PURE(LevelRecordRes,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"资源的名称");
		GELEM_STRING_INIT(path,"");
			GELEM_EDITVAR("路径动画",GVT_String,GSem_XformAnimPath,"路径动画的资源");
		GELEM_VAR_INIT(float,scalePathSpeed,1.0f);
			GELEM_EDITVAR("路径动画速度缩放",GVT_F,GSem(GSem_Float,"0.001,20.0,0.05"),"路径动画时间缩放");
		GELEM_VAR_INIT(float,facePath,0.0f);
			GELEM_EDITVAR("路径方向",GVT_F,GSem(GSem_Float,"-180.0,180.0,0.1"),"动画内路径的方向(0表示正前方,向左为负,向右为正)");

	END_GOBJ();


};