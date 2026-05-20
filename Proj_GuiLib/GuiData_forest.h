#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/ISpt.h"
#include "WorldSystem/IEntitySystem.h"

class CSscSystemWrapper;
struct TreeDrawCfg;
struct GuiLib_Api GuiData_Forest :public GeData
{
	GuiData_Forest()
	{
		pWS = NULL;
		bOnAdd = FALSE;
		colObjVisible = FALSE;
		ver = 0;
		scaleMin = 0.5;
		scaleMax = 1.5f;	
		Reseed();
	}

	virtual const char *GetName() {return "forest";}
	IForestEditor * GetEditor()
	{	
		IForestEditor * editor = NULL;
		if(ens){
			IObjMapEditor *objEditor =  ens->FindObjMapEditor(OBJMAP_TYPE_TREE);
			if(objEditor)
				editor = (IForestEditor*)(objEditor);
		}
		return editor; 
	}

	IBrushLib * GetLib()
	{
		IForestEditor * editor = GetEditor();
		if(editor)
			return editor->GetSptLib();
		return NULL;
	}
	
	void Reseed()
	{
		float scale = 1.0f;
		int seed = rand();
		assert(scaleMax>scaleMin);

		scale = (scaleMax-scaleMin)*(float(seed)/RAND_MAX)+scaleMin;
		info.scale = scale;

		const float pi = 3.14159265358979323846f;
		seed = rand();
		float rotY = 2*pi*(float(seed)/(RAND_MAX)) - pi;

		info.rotY = rotY;
	}

	std::vector<HMapObj> hTreeSels;
	i_math::vector3df location;

	IWorldSystem * pWS;
	CSscSystemWrapper * ssc;
	IAssetSystem * ass;
	IEntitySystem *ens;

	TreeInfo info;
	DWORD  ver;
	BOOL colObjVisible;
	BOOL bOnAdd;
	float scaleMin,scaleMax;
};



