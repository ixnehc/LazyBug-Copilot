#pragma once

#include "GuiLib.h"

#include "commondefines/general_stl.h"

#include "editor/editor.h"

#include "WorldSystem/IWorldSystemInterfaces.h"
#include "WorldSystem/IEntitySystemDefines.h"


struct GuiLib_Api GuiData_EntityMap:public GeData
{
	virtual const char *GetName()	{		return "entitymap";	}

	void Clear()
	{
		mf=NULL;
		mp=NULL;
		pES=NULL;
		pAS=NULL;
		selections.clear();
	}

	void AddSelection(EntityAddress addr)
	{
		if (addr!=EntityAddress_Null)
			selections.push_back(addr);
	}
	void SwitchSelection(EntityAddress addr)
	{
		if (addr==EntityAddress_Null)
			return;
		int idx;
		VEC_FIND(selections,addr,idx);
		if (idx==-1)
			selections.push_back(addr);
		else
			selections.erase(selections.begin()+idx);
	}

	void ClearSelection()
	{
		selections.clear(); 
	}

	IMapFile *mf;
	IEntitySystem *pES;
	IAssetSystem *pAS;
	IEntityMap *mp;

	std::vector<EntityAddress> selections;//all the selected entities

};
