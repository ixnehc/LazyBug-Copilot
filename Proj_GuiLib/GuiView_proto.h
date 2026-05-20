#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

class GuiLib_Api CGuiView_Proto :public CGuiView
{
public:
	virtual const char*GetName()	{		return "proto";	}
	virtual void _OnDraw(IRenderPort *rp);

	virtual BOOL Respond(CtrlOp &co);

protected:


};




