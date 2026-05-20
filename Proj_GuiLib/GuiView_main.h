#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

class GuiLib_Api CGuiView_Main:public CGuiView
{
public:
	virtual const char*GetName()	{		return "perspective";	}
protected:
	virtual void _OnPreDraw(IRenderPort *rp);
	virtual void _OnDraw(IRenderPort *rp);
	virtual void _OnPostDraw(IRenderPort *rp);

	void _DrawPathes(IRenderPort *rp);

	std::vector<i_math::vector3df>_lines;//用于_DrawPathes()的临时buffer


};




