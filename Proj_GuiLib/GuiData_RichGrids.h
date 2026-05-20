#pragma once

#include "GuiLib.h"

#include "editor/editor.h"

#include "WorldSystem/IEntitySystem.h"
#include "RenderSystem/IRenderSystem.h"

#include <unordered_map>


class CRichGrid;
struct GuiData_RichGrids:public GeData
{
	virtual const char *GetName()	{		return "richgrids";	}
	GuiData_RichGrids()
	{
	}

	void RegisterRichGrid(const char *name,CRichGrid *grid)
	{
		_grids[std::string(name)]=grid;
	}

	CRichGrid *FindGrid(const char *name)
	{
		std::unordered_map<std::string,CRichGrid*>::iterator it=_grids.find(std::string(name));
		if (it==_grids.end())
			return NULL;
		return (*it).second;
	}

	const char *GetCurGrid();


	std::unordered_map<std::string,CRichGrid*> _grids;

};

