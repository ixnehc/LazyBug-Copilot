#pragma once

#include "GuiLib.h"

#include "commondefines/general_stl.h"

#include "editor/editor.h"

#include "WorldSystem/IEnvLight.h"

#include "WorldSystem/IWorldSystemInterfaces.h"


#define DEFINE_GUIDATA_EL(v)															\
	GuiData_El*v=(GuiData_El*)FindData("envlight");


#define LIMIT_LEN_MAX	50.0f
#define LIMIT_LEN_MIN	0.5f
class IEnvLight;
struct GuiLib_Api GuiData_El:public GeData
{
	virtual const char *GetName()	{		return "envlight";	}

	GuiData_El()
	{
		Zero();
	}

	void Zero()
	{
		pES=NULL;
		ver = 0;
		bShowGrid = TRUE;
		bShowSample = FALSE;
		bOnAdd = FALSE;
	}

	void Clear()
	{
		Zero();
	}

	IEnvLight *GetIEnvLight();
	IProbeCubeMapEditor * GetEditor();

	IEntitySystem *pES;
	std::vector<HMapObj> hObjSels;
	BOOL bShowGrid;
	BOOL bShowSample;
	BOOL bOnAdd;
	DWORD ver;
};
