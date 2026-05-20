#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/ISpg.h"

#include "WorldSystem/IEntitySystem.h"

#include "WorldSystem/ITrrn.h"

#include "SscBase.h"

struct GuiLib_Api GuiData_Vegetable :public GeData
{
	GuiData_Vegetable(){op = 0;radius = 4.0f; density = 0.2f;scaleMin = 0.5f;scaleMax = 2.0f;}
	virtual const char *GetName() {return "vegetable";}
	
	enum
	{
		Op_Idle = 0,
		Op_Paint = 1,
		Op_Remove = 2,
	};

	ISpgEditor * GetEditor()
	{	
		ISpgEditor * editor = NULL;
		if(ens){
			editor = (ISpgEditor *)ens->FindObjMapEditor(OBJMAP_TYPE_VEGETABLE);
		}
		return editor; 
	}
	
	ITrrnMapEditor * GetTrrnEditor()
	{	
		ITrrnMapEditor * editor = NULL;
		if(ens)
			editor = ens->FindTrrn()->GetEditor();
		return editor; 
	}

	IWorldSystem * pWS;
	CSscSystemWrapper * ssc;
	IEntitySystem *ens;	
	float radius;
	float density;
	float scaleMin,scaleMax;
	DWORD op; // 1 : paint  2:remove
};



