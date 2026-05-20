

#pragma once

#include "math/vector3d.h"

#include "gds/GObj.h"

#include "WorldSystem/IBrushLib.h"

#include "WorldSystem/IObjMap.h"

#define WATERBLOCK_LEN 64

class IWaterEditor;
//struct WaterDrawArg
//{
//	WaterDrawArg(){
//		lvl = 0;
//		bUnderWater = FALSE;
//		mesh = NULL;
//		editor = NULL;
//		szBlock = 0;
//		density = 1;
//	}
//	int	szBlock;
//	int density;
//	int  lvl;
//	BOOL bUnderWater;
//	WaterMesh * mesh;
//	IWaterEditor * editor;
//};

enum WPaintOP
{
	WPaintOP_Clear,
	WPaintOP_Add,
	WPaintOp_Complete,
	WPaintOP_Idle,
};

enum Eye2WaterState
{
	EWState_No,
	EWState_Below,
	EWState_Above,
	EWState_Intersec,
};

struct WaterInfo
{
	float height;
	BRUID idBr;
};

struct WBrushInfo
{
	float height;
};

class ICamera;
class IWaterEditor :public IObjMapEditor
{
public:
	virtual IBrushLib * GetBrushLib() = 0;

	virtual void Paint(const BRUID &brID,i_math::pos2di & s,i_math::pos2di &e,WPaintOP op) = 0; 
	
	virtual const WaterInfo * GetWaterInfo(const HMapObj &hObj) = 0;

	virtual Eye2WaterState GetEyeState(ICamera * cam,HMapObj &hObj) = 0; 

	virtual BOOL CheckVisible(const HMapObj &hObj) = 0;	//清除 对象不可见的部分
};





