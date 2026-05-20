#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

class GuiLib_Api CGuiView_Acl:public CGuiView
{
public:
	virtual const char*GetName()	{		return "assetclasslib";	}
	virtual void _OnDraw(IRenderPort *rp);

};




