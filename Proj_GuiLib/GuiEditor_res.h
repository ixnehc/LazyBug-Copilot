#pragma once
#include "GuiLib.h"

#include "RenderSystem/ITools.h"


#include "GuiEditor.h"
#include "../Interfaces/RenderSystem/IRenderSystem.h"

struct ResEditPanelState;
class CResEditPanel;
class GuiLib_Api CGuiData_Res: public GeData
{
public:
	CGuiData_Res()
	{
		state=NULL;
		cam=NULL;
		panel=NULL;
	}
	void Clear()
	{
		SAFE_RELEASE(cam);
	}

	virtual const char*GetName()	{		return "resource";	}


	ResEditPanelState *GetState()	{		return state;	}
	ResEditPanelState *state;
	CResEditPanel *panel;
	ICamera *cam;
};

class GuiLib_Api CGuiView_Res :public CGuiView
{
public:
	virtual const char*GetName()	{		return "resource";	}

	virtual void _OnPreDraw(IRenderPort *rp);
	virtual void _OnDraw(IRenderPort *rp);
	virtual void _OnPostDraw(IRenderPort *rp);

};

class CResEditPanel;
class GuiLib_Api CGuiView_Res2 :public CGuiView
{
public:
	virtual const char*GetName()	{		return "resource2";	}

protected:
	virtual DrawMechanism _GetDrawMechanism()	{		return UsingGG;	}

	void _OnDraw(GraphicsGraph *gg);
};


class GuiLib_Api CGuiActor_Res: public CGuiActor
{
public:
	virtual const char*GetName()	{		return "resource";	}
};




