/********************************************************************
	created:	2006/11/1   14:22
	filename: 	e:\IxEngine\Proj_GuiLib\ResEditCtrl.cpp
	author:		CXI
	
	purpose:	base class for controls used in CResEditPanel
*********************************************************************/
#include "stdh.h"
#include ".\ResEditCtrl.h"

#include "ResEditPanel.h"

#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



void CResEditCtrl::Bind(ResEditPanelState *state,BOOL bUpdateCtrl)
{
	_state=state;
}
void CResEditCtrl::RefreshMod(BOOL bLockMe)
{
	if (_panel)
	{
		if (bLockMe)
			_panel->LockControl(this);
		_panel->RefreshStateMod();//
		if (bLockMe)
			_panel->UnlockControl(this);
	}
}

ResData*CResEditCtrl::_GetResData()
{
	return _state->resdata;
}
