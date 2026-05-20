
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include <vector>
#include "resdata/SptData.h"

#include "ResEditCtrl.h"

#include "EditListBoxEx.h"

class CResEditPanel;
struct  ResEditPanelState;
struct  SpeedTreePanelSate;
class GuiLib_Api CSpeedWindList: public CEditListBoxEx,public CResEditCtrl
{
public:
	CSpeedWindList()
	{
		_SetNameLimit(MAX_NUM_WINDS-1);
	}

public:
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
protected:
	virtual void EnableCtrl(BOOL bActive=TRUE);
	virtual void _OnNewItem(DWORD idx,const char *name);
	virtual void _OnChangeItem(DWORD idx,const char *name)	;
	virtual void _OnDeleteItem(DWORD idx);
	virtual void _OnSwapItem(DWORD idx1,DWORD idx2);
	virtual void _OnSelChange(DWORD iSel);
 
	SptData * GetResData(ResEditPanelState *state)
	{
		return static_cast<SptData *>(CResEditCtrl::_GetResData());
	}
	SpeedTreePanelSate * GetState(ResEditPanelState *state) {return (SpeedTreePanelSate *)(state);}
};








