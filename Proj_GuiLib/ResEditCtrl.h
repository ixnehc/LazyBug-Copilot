
#pragma once
#include "GuiLib.h"

//#include "resource.h"

class CResEditPanel;
struct ResEditPanelState;
struct ResData;
class GuiLib_Api CResEditCtrl
{
public:
	CResEditCtrl()
	{
		_panel=NULL;
		_state=NULL;
	}
	void SetPanel(CResEditPanel *panel)	{		_panel=panel;	}
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	virtual void EnableCtrl(BOOL bActive=TRUE)=0;

	void RefreshMod(BOOL bLockMe=TRUE);


protected:
	ResData*_GetResData();
	CResEditPanel *_panel;
	ResEditPanelState *_state;
};
