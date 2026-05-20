/********************************************************************
	created:	14:1:2009   11:00
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IMapFileArgs.h
	author:		chenxi
	
	purpose:	args stored in map file
*********************************************************************/
#pragma once

#include "gds/GObj.h"

#define ENTITYMAP_MINEXT 16//以米为单位

struct MfArg_Map
{
	BOOL bEnable;
	int ext;//以米为单位
	std::string pathLib;
};

struct MfArg_EntityMap:public MfArg_Map
{
	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(MfArg_EntityMap,1);
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);
			GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"entity map是否有效");
		GELEM_VAR_INIT(int,ext,2000);
			GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"entity map的边长,以米为单位,必须为8的倍数");
	END_GOBJ();    
};

#define TRRNMAP_MINEXT 16

struct MfArg_TrrnMap:public MfArg_Map
{
	BEGIN_GOBJ_PURE(MfArg_TrrnMap,1);
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);
			GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"terrain map是否有效");
		GELEM_VAR_INIT(int,ext,2000);
			GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"terrain map的边长,以米为单位");
		GELEM_STRING_INIT(pathLib,"test.tblib");
			GELEM_EDITVAR("Brush Lib Path",GVT_String,GSem_TrrnBrushLibPath,"地表使用的笔刷库文件");
	END_GOBJ();    
};

#define FORESTMAP_DEFAULTEXT 2000
struct MfArg_ForestMap:public MfArg_Map
{
	std::string wndlibPath;

	BEGIN_GOBJ_PURE(MfArg_ForestMap,1)
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);
			GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"forest 是否存在");
		GELEM_VAR_INIT(int,ext,FORESTMAP_DEFAULTEXT);
			GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"forest map的边长,以米为单位");
		GELEM_STRING_INIT(pathLib,"tree.brlib");
			GELEM_EDITVAR("Tree library path",GVT_String,GSem(GSem_BrushLibPath,"Tree"),"森林的植被库文件");
		GELEM_STRING_INIT(wndlibPath,"wind.brlib");
			GELEM_EDITVAR("Wind library path",GVT_String,GSem(GSem_BrushLibPath,"TreeWind"),"森林风的库文件");
		END_GOBJ();
};

#define WATERMAP_DEFAULTEXT 2000
struct MfArg_WaterMap:public MfArg_Map
{
	BEGIN_GOBJ_PURE(MfArg_WaterMap,1)
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);
			GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"water 是否存在");
		GELEM_STRING_INIT(pathLib,"water.brlib");
			GELEM_EDITVAR("Water Brush library path",GVT_String,GSem(GSem_BrushLibPath,"WaterWave"),"水的笔刷库的文件名");
		GELEM_VAR_INIT(int,ext,WATERMAP_DEFAULTEXT);
			GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"water map的边长,以米为单位");
	END_GOBJ();
};

#define VEGETABLEMAP_DEFAULTEXT 1000
struct MfArg_VegetableMap:public MfArg_Map
{
	BEGIN_GOBJ_PURE(MfArg_VegetableMap,1)
		GELEM_VAR_INIT(BOOL,bEnable,TRUE);
			GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"vegetable 是否存在");
		GELEM_VAR_INIT(int,ext,VEGETABLEMAP_DEFAULTEXT);
			GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"vegetable map的边长,以米为单位");
		GELEM_STRING_INIT(pathLib,"grass.brlib");
			GELEM_EDITVAR("SpeedGrass library path",GVT_String,GSem(GSem_BrushLibPath,"Grass"),"森林的植被库文件");
	END_GOBJ();
};

#define SHOREMAP_DEFAULTEXT 400
struct MfArg_ShoreMap:public MfArg_Map
{
	BEGIN_GOBJ_PURE(MfArg_ShoreMap,1);
	GELEM_VAR_INIT(BOOL,bEnable,TRUE);
		GELEM_EDITVAR("Enable",GVT_S,GSem_Boolean,"shore map是否有效");
	GELEM_STRING_INIT(pathLib,"shore.brlib");
		GELEM_EDITVAR("ShoreWave library path",GVT_String,GSem(GSem_BrushLibPath,"ShoreWave"),"波浪的库文件");
	GELEM_VAR_INIT(int,ext,800);
		GELEM_EDITVAR("Extent",GVT_S,GSem_Interger,"shore map的边长,以米为单位,必须为8的倍数");
	END_GOBJ();    
};

struct MfArg_Sky
{
	BOOL bVisible;
	i_math::vector3df dir;
	float softness;//反映散射光与直射光的比例,取值范围为0..1
	i_math::vector4db col;

	BEGIN_GOBJ_PURE(MfArg_Sky,1);
		GELEM_VAR_INIT(BOOL,bVisible,TRUE);
			GELEM_EDITVAR("Visible",GVT_S,GSem_Boolean,"是否要绘制天空体");
		GELEM_VAR_INIT(i_math::vector3df,dir,i_math::vector3df(1.0f,-1.0f,0.0f));
			GELEM_EDITVAR("直射光方向",GVT_Fx3,GSem_Normal,"直射光方向");
		GELEM_VAR_INIT(i_math::vector4db,col,i_math::vector4db(0x9f,0x9f,0x9f,0xff));
			GELEM_EDITVAR("颜色",GVT_Bx4,GSem_ColorAlphaU,"天空光照的颜色");
		GELEM_VAR_INIT(float,softness,0.66f);
			GELEM_EDITVAR("柔和度",GVT_F,GSem(GSem_Float,"0.0f,1.0f,0.02f"),"光照的柔和程度(越柔和则散射光越强)");
	END_GOBJ(); 
};


//XXXXX:more map file args

#define MfArg_Name "mfarg"

//用来新建一个map的参数
struct MapFileArgs
{
	i_math::recti rc;//以米为单位
	MfArg_EntityMap mfaEntityMap;
	MfArg_TrrnMap mfaTrrnMap;
	MfArg_ForestMap mfaForestMap;
	MfArg_WaterMap mfaWaterMap;
	MfArg_Sky mfaSky;
	MfArg_VegetableMap mfaVegetableMap;
	MfArg_ShoreMap mfaShoreMap;

	//XXXXX:more map file args

	BEGIN_GOBJ_PURE(MapFileArgs,1)
		GELEM_VAR_INIT(i_math::recti,rc,i_math::recti(-1024,-1024,1024,1024));
			GELEM_EDITVAR("Map Rect",GVT_Sx4,GSem(GSem_Rect,"Gran:64"),"地图的范围,以米为单位");
		GELEM_OBJ(MfArg_EntityMap,mfaEntityMap);
			GELEM_EDITOBJ("Entity Map","Entity Map参数");
		GELEM_OBJ(MfArg_TrrnMap,mfaTrrnMap);
			GELEM_EDITOBJ("Terrain","Terrain参数");
		GELEM_OBJ(MfArg_ForestMap,mfaForestMap)
			GELEM_EDITOBJ("Tree","Tree Map参数");
		GELEM_OBJ(MfArg_WaterMap,mfaWaterMap)
			GELEM_EDITOBJ("Water","Water Map参数");
		GELEM_OBJ(MfArg_Sky,mfaSky);
			GELEM_EDITOBJ("Sky","Sky参数");
		GELEM_OBJ(MfArg_VegetableMap,mfaVegetableMap);
			GELEM_EDITOBJ("Grass","Grass Map参数");
		GELEM_OBJ(MfArg_ShoreMap,mfaShoreMap);
			GELEM_EDITOBJ("Shore Wave","Shore Wave Map参数");
	END_GOBJ();

};

