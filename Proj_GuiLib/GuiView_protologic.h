#pragma once

#include "GuiEditor.h"
#include "GuiLib.h"

#include "class/class.h"



#include <map>

class GraphicsGraph;
class GuiLib_Api CGuiView_ProtoLogic:public CGuiView
{
public:
	virtual const char*GetName()	{		return "proto_logic";	}
	virtual void _OnDraw(GraphicsGraph *gg);
	virtual BOOL Respond(CtrlOp &co);

	virtual BOOL RespondMsg(CPoint &ptCursor,UINT msg,WPARAM wParam,LPARAM lParam,LRESULT &ret);


protected:

	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;	}


	void _DrawStuff(GraphicsGraph *gg);

};


