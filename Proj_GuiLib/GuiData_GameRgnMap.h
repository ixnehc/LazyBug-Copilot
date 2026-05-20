#pragma once

#include "GuiLib.h"

#include "WorldSystem/IGameRgnMap.h"

#include "editor/editor.h"

#define GAMERGNGRID_UNIQUEFILENAME "GameRgnGrid"

#define GAMERGNGRID_GRIDLEN 4

struct GuiLib_Api GuiData_GameRgnMap:public GeData
{

	virtual const char *GetName()	{		return "gamergnmap";	}

	GuiData_GameRgnMap()
	{
		Zero();
	}
	~GuiData_GameRgnMap()
	{
	}
	void Zero()
	{
		pES = NULL;
		pRS = NULL;
		pWS = NULL;
		radius = 4;
		regionID = 0;
	}
	void Clear()
	{
		Zero();
	}
   
	CGameRgnGrids *ObtainGrids();
	const char *GetFullPath();
	const char *GetUniqueFileName()	{		return GAMERGNGRID_UNIQUEFILENAME;	}

	CGameRgnGrids _grids;
	
	IEntitySystem	*pES;
	IRenderSystem	*pRS;
	IWorldSystem	*pWS; 
	int radius;
	DWORD regionID;
};





