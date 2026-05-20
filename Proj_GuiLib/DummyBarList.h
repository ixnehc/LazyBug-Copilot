#pragma once
#include "EditListBoxEx.h"
#include "ResEditCtrl.h"
class GuiLib_Api CDummyBarList: public CEditListBoxEx ,public CResEditCtrl
{
public:
	CDummyBarList(void);
	~CDummyBarList(void);
public:
	//Override function deriver from  CResEditCtrl
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	virtual void EnableCtrl(BOOL bActive=TRUE);
public:
	DECLARE_MESSAGE_MAP()
	virtual void _OnNewItem(DWORD idx,const char *name);
	virtual void _OnChangeItem(DWORD idx,const char *name);
	virtual void _OnDeleteItem(DWORD idx);
	virtual void _OnSwapItem(DWORD idx1,DWORD idx2);
	virtual void _OnSelChange(DWORD iSel);
};
