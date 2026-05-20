
#pragma once
#include "GuiLib.h"


#include <vector>
#include "WndBase.h"

class CEditListBoxEx:public CXTEditListBox
{
public:
	CEditListBoxEx()
	{
		_bAcceptSelChange=TRUE;
		_limit=0;
		_bAllowDupe=FALSE;
		_bAllowBlank=FALSE;
	}
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEndLabelEdit();
	afx_msg void OnCancelLabelEdit();
	afx_msg void OnDeleteItem();
	afx_msg void OnMoveItemUp();
	afx_msg void OnMoveItemDown();
	afx_msg void OnSelChange();

protected:
	//overridable
	virtual void _OnNewItem(DWORD idx,const char *name)	{	}
	virtual void _OnChangeItem(DWORD idx,const char *name)	{	}
	virtual void _OnPreDeleteItem(DWORD idx)	{	}
	virtual void _OnDeleteItem(DWORD idx)	{	}
	virtual void _OnSwapItem(DWORD idx1,DWORD idx2)	{	}
	virtual void _OnSelChange(DWORD iSel)	{	}
protected:
	void _SetNameLimit(DWORD c)	{		_limit=c;	}
	void _AllowDupe(BOOL bAllow)	{		_bAllowDupe=bAllow;	}
	void _AllowBlank(BOOL bAllow)	{		_bAllowBlank=bAllow;	}
	DWORD _limit;//item name limit,NOT including the terminator '\0'. If 0, no limit
	BOOL _bAllowDupe;//whether an item name could be the same with the other
	BOOL _bAllowBlank;//whether an item name could be blank

	BOOL _bAcceptSelChange;
private:
	void _EndLabelEdit(BOOL bCancel);

};


class CXTEditListBoxEx: public CXTEditListBox
{
public:
	CXTEditListBoxEx()
	{
		_bAcceptSelChange=TRUE;
	}
public:

	//if iItem is -1,this name is for new item
	virtual void OnNewItem(int iItem,const char *name);
	virtual void OnMoveItemUp();
	virtual void OnMoveItemDown();
	virtual void OnSelChange();
	virtual BOOL IsCaseSensitive()	{		return TRUE;	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEndLabelEdit();
	afx_msg void OnCancelLabelEdit();
	virtual void OnDeleteItem();
	afx_msg void OnSelChange0();
protected:
	void _EndLabelEdit(BOOL bCancel);

	//if iItem is -1,this name is for new item
	virtual BOOL _CheckItemName(int iItem,const char *name)	{		return TRUE;	}

	BOOL _bAcceptSelChange;
	
};
