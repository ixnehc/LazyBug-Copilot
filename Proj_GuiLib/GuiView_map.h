#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

class  GuiLib_Api CGuiView_map :public CGuiView
{
public:
	CGuiView_map()
	{
		_bYInverse=TRUE;
	}
	virtual const char*GetName();
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;}
};
