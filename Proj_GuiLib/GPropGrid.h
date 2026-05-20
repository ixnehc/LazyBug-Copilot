
#pragma once
#include "GuiLib.h"

#include <vector>


#include "RichGrid.h"

#include "gds/GDefines.h"

#include "GObjGrid.h"


struct GProperty;
class GuiLib_Api CGPropGrid:public CGObjGrid
{
public:
	CGPropGrid()
	{
	}

	CXTPPropertyGridItem *InsertProp(GProperty *prop,const char *name,GSem &sem,const char *desc);



};
