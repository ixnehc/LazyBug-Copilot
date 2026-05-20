#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "records/records.h"

#include "LevelDefines.h"


struct LevelRecordMap:public CRecord
{
	DEFINE_CLASS(LevelRecordMap);

	std::string Name; 

	std::string path;

	std::string pathTrimmedTex;
	i_math::recti rcTexTrim;

	BOOL bSight;

	float angleCam;//相机的方向

	LevelMapLayor layor;

	i_math::pos2di ptLoc;

	std::vector<StringID> ais;

	BEGIN_GOBJ_PURE(LevelRecordMap,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"关卡的名称");
		GELEM_STRING_INIT(path,"");
			GELEM_EDITVAR("地图路径",GVT_String,GSem_MapFilePath,"map file的路径");

		GELEM_STRING_INIT(pathTrimmedTex,"");
			GELEM_EDITVAR("裁剪过的小地图贴图",GVT_String,GSem_TexturePath,"贴图");

		GELEM_VAR(i_math::recti,rcTexTrim);
			GELEM_EDITVAR("裁剪区域",GVT_Sx4,GSem(GSem_Rect,"ShowSize"),"裁剪区域");

		GELEM_VAR(i_math::pos2di,ptLoc);
			GELEM_EDITVAR("位置",GVT_Sx2,GSem_Point,"地图左上角在全地图中的位置(以像素为单位)");

		GELEM_VAR_INIT(LevelMapLayor,layor,LevelMapLayor_Ground);
			GELEM_EDITVAR("层级",GVT_S,GSem(GSem_Interger,"地表:1,地下:2"),"地图层级");
		GELEM_VAR_INIT(BOOL,bSight,0);
			GELEM_EDITVAR("是否需要视野",GVT_S,GSem_Boolean,"在黑暗的地图里需要视野");

		GELEM_VAR_INIT(float,angleCam,180.0f);
			GELEM_EDITVAR("相机角度",GVT_F,GSem(GSem_Float,"0,360,1.0f"),"相机的角度");

		GELEM_VARVECTOR_INIT(StringID,ais,StringID_Invalid);
			GELEM_EDITVAR("AI",GVT_U,GSem(GSem_StringID,"行为图名称"),"名称");
	END_GOBJ();


};