#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/IWorldSystemInterfaces.h"
#include "WorldSystem/IAssetSystemDefines.h"

#include "WorldSystem/IWater.h"

enum CameraCategory
{
	Camera_Perspective,
	Camera_Left,
	Camera_Right,
	Camera_Top,
	Camera_Bottom,
	Camera_Front,
	Camera_Back,

	Camera_Max,
};


struct GuiLib_Api GuiData_System:public GeData
{
	virtual const char *GetName()	{		return "system";	}

	GuiData_System()
	{
		Zero();
	}
	void Zero()
	{
		pWS=NULL;
		pRS=NULL;
		pAS=NULL;
		pES=NULL;
		mf=NULL;
		sevent=NULL;
		ssc = NULL;
	}
	void Clear()
	{
		Zero();
	}

	IWorldSystem *pWS;
	IRenderSystem *pRS;
	IAssetSystem *pAS;
	IEntitySystem *pES;
	IMapFile * mf;
	IAssetEventer * sevent;
	CSscSystemWrapper * ssc;

};

class ILinkIn;
class IRenderPort;
struct GuiLib_Api GuiData_Trrn:public GeData
{
	virtual const char *GetName()	{		return "terrain";	}

	void Clear()
	{
		pES=NULL;
		bPaint = FALSE;
	}
	ITrrnMap *GetTrrnMap();
	ITrrnMapEditor *GetTrrnMapEditor();
	ITrrnBrushLib *GetBrushLib();
	BOOL GetHitPos(int x,int y,IRenderPort *rp,i_math::vector3df &pos);
	BOOL bPaint;
	IEntitySystem *pES;
};

class ICamera;
struct GuiLib_Api GuiData_Camera:public GeData
{
	virtual const char *GetName()	{		return "cameras";	}
	GuiData_Camera()
	{
		Zero();
	}
	void Zero()
	{
		memset(cams,0,sizeof(cams));
		scaleMove=0;
	}
	void Clear();

	ICamera *cams[Camera_Max];
	int scaleMove;//camera移动的缩放值
};

class CSscSystemWrapper;
struct GuiLib_Api GuiData_Prl:public GeData
{
	GuiData_Prl()
	{
		Clear();
	}
	virtual const char *GetName()	{		return "protolib";	}

	void Clear()
	{
		pES=NULL;
		lib=NULL;
		ssc=NULL;
	}

	IEntitySystem *pES;
	IProtoLib *lib;

	std::string pathStartupProto;//调试时的起始proto

	CSscSystemWrapper *ssc;
};



struct GuiLib_Api GuiData_Shell :public GeData
{
	virtual const char *GetName()	{		return "shell";	}

	void Clear()
	{
		pAS=NULL;
	}
	
	IAssetSystem *pAS;
};

struct GuiLib_Api GuiData_ViewSwitch:public GeData
{
	virtual const char *GetName()	{		return "viewswitch";	};

	GuiData_ViewSwitch()
	{
		mgr=NULL;
		bShowProfiler=FALSE;
	};

	ProfilerMgr *mgr;
	BOOL bShowProfiler;
};


