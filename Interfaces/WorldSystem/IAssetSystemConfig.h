/********************************************************************
	created:	18:3:2009   14:38
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IAssetSystemConfig.h
	author:		chenxi
	
	purpose:	asset system config
*********************************************************************/
#pragma once

#include "gds/GObj.h"

struct RenderCfg
{
	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(RenderCfg,1);
		GELEM_VAR_INIT(BOOL ,bNormalSpec,1);
	END_GOBJ();

	BOOL bNormalSpec;//是否支持法线贴图及高光效果
};


struct AssetSystemConfig
{
	float wRenderer;//the renderer map's extent,in meter
	float wBodyMap;//the body map's extent,in meter
	float wEnvMap;//the environment map's extent,in meter

	RenderCfg cfgRender;


	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(AssetSystemConfig,1);

		GELEM_VAR_INIT(float,wRenderer,800);
		GELEM_VAR_INIT(float,wBodyMap,800);
		GELEM_VAR_INIT(float,wEnvMap,400);

		GELEM_OBJ(RenderCfg,cfgRender);

	END_GOBJ();

};


