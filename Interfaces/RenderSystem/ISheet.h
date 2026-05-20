
#pragma once
#include "IResource.h"

#include "sheet/sheet.h"



class ISheet:public IResource
{
public:
	virtual CSheet *GetCore()=0;
};

class ISheetMgr:public IResourceMgr
{

};

