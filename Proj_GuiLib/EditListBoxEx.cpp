/********************************************************************
	created:	2006/9/6   10:48
	filename: 	e:\IxEngine\Proj_GuiLib\AnimPieceList.cpp
	author:		cxi
	
	purpose:	a list that could edit anim pieces for AnimData
*********************************************************************/

#include "stdh.h"
#include "resource.h"

#include "CommonCtrlBase.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include <assert.h>

#include "EditListBoxEx.h"

#include "setter/setter.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////////
//CEditListBoxEx
#define XT_IDC_GROUP_EDIT 200			//copied from XTEditListBox.cpp

BEGIN_MESSAGE_MAP(CEditListBoxEx, CXTEditListBox)
	ON_LBN_XT_LABELEDITEND(XT_IDC_LBOX_EDIT, OnEndLabelEdit)
	ON_LBN_XT_LABELEDITCANCEL(XT_IDC_LBOX_EDIT, OnCancelLabelEdit)
	ON_LBN_XT_DELETEITEM(XT_IDC_GROUP_EDIT, OnDeleteItem)
	ON_LBN_XT_MOVEITEMUP(XT_IDC_GROUP_EDIT, OnMoveItemUp)
	ON_LBN_XT_MOVEITEMDOWN(XT_IDC_GROUP_EDIT, OnMoveItemDown)
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelChange)

END_MESSAGE_MAP()


void CEditListBoxEx::OnEndLabelEdit()
{
	BoolSetter setter(_bAcceptSelChange,FALSE);
	_EndLabelEdit(FALSE);
}

void CEditListBoxEx::OnCancelLabelEdit()
{
	BoolSetter setter(_bAcceptSelChange,FALSE);
	_EndLabelEdit(TRUE);
}


void CEditListBoxEx::_EndLabelEdit(BOOL bCancel)
{
	BOOL bInvalid=FALSE;
	while (1)
	{
		std::string sItem = toMBCS((LPCTSTR)m_strItemText);
		int idx;
		idx=ListBox_Find(this,sItem.c_str(),FALSE);
		if ((_limit>0)&&(sItem.length()>_limit))
		{
			LogFile::Prompt("Too long name found! Name length should not exceed %d",_limit);
			bInvalid=TRUE;
			break;
		}
		if ((!_bAllowDupe)&&(idx!=-1)&&(idx!=GetCurrentIndex()))
		{
			LogFile::Prompt("Duplicated name found!");
			bInvalid=TRUE;
			break;
		}
		if (!_bAllowBlank)
			if ((m_bNewItem&&(sItem!=""))||(!m_bNewItem))
			{
				RemoveTailBlank(sItem);
				if (sItem=="")
				{
					LogFile::Prompt("Blank name found!");
					bInvalid=TRUE;
				}
				break;
			}

			break;
	}

	if (bInvalid)
	{
		BOOL b=m_bNewItem;
		EditItem(GetCurrentIndex());
		m_bNewItem=b;
		return;
	}
	SetCurSel(GetCurrentIndex());
	CXTEditListBox::OnEndLabelEdit();

	if (bCancel)
		return;

	if (m_strItemText=="")
		return;

	int idx;
	idx=GetCurrentIndex();

	if (m_bNewItem)
	{
		_OnNewItem(idx, toMBCS((LPCTSTR)m_strItemText));
	}
	else
	{
		_OnChangeItem(idx, toMBCS((LPCTSTR)m_strItemText));
	}

}


void CEditListBoxEx::OnDeleteItem()
{
	BoolSetter setter(_bAcceptSelChange,FALSE);
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;

	_OnPreDeleteItem(idx);
	
	CXTEditListBox::OnDeleteItem();

	_OnDeleteItem(idx);
}

void CEditListBoxEx::OnMoveItemUp()
{
	BoolSetter setter(_bAcceptSelChange,FALSE);
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;

	CXTEditListBox::OnMoveItemUp();
	if (idx<=0)
		return;
	_OnSwapItem(idx-1,idx);
}
void CEditListBoxEx::OnMoveItemDown()
{
	BoolSetter setter(_bAcceptSelChange,FALSE);
	int idx;
	idx=GetCurSel();
	if (idx==-1)
		return;

	CXTEditListBox::OnMoveItemDown();
	if (idx>=GetCount()-1)
		return;
	_OnSwapItem(idx,idx+1);
}

void CEditListBoxEx::OnSelChange()
{
	if (!_bAcceptSelChange)
		return;
	
	int idx = GetCurSel();
	_OnSelChange(idx);
}

//////////////////////////////////////////////////////////////////////////
//CXTEditListBoxEx

BEGIN_MESSAGE_MAP(CXTEditListBoxEx, CXTEditListBox)
	ON_LBN_XT_LABELEDITEND(XT_IDC_LBOX_EDIT, OnEndLabelEdit)
	ON_LBN_XT_LABELEDITCANCEL(XT_IDC_LBOX_EDIT, OnCancelLabelEdit)
	ON_LBN_XT_DELETEITEM(XT_IDC_GROUP_EDIT, OnDeleteItem)
	ON_LBN_XT_MOVEITEMUP(XT_IDC_GROUP_EDIT, OnMoveItemUp)
	ON_LBN_XT_MOVEITEMDOWN(XT_IDC_GROUP_EDIT, OnMoveItemDown)
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelChange0)

END_MESSAGE_MAP()


void CXTEditListBoxEx::OnEndLabelEdit()
{
	_bAcceptSelChange=FALSE;
	_EndLabelEdit(FALSE);
	_bAcceptSelChange=TRUE;
	
	int idx = GetCurSel();
//	NotifyEvent(Event_Rename,idx,m_strItemText.GetBuffer());

	OnSelChange();
}

void CXTEditListBoxEx::OnCancelLabelEdit()
{
	_bAcceptSelChange=FALSE;
	_EndLabelEdit(TRUE);
	_bAcceptSelChange=TRUE;
	OnSelChange();
}


void CXTEditListBoxEx::_EndLabelEdit(BOOL bCancel)
{
	BOOL bInvalid=FALSE;
	int idx=-1;
	if (!bCancel)
	{
		if (!m_bNewItem)
			idx=GetCurrentIndex();
		bInvalid=!_CheckItemName(idx, toMBCS((LPCTSTR)m_strItemText));
	}
	if (bInvalid)
	{
		BOOL b=m_bNewItem;
		EditItem(GetCurrentIndex());
		m_bNewItem=b;
		return;
	}
	SetCurSel(GetCurrentIndex());
	CXTEditListBox::OnEndLabelEdit();

	if (bCancel)
		return;

	OnNewItem(idx, toMBCS((LPCTSTR)m_strItemText));
}

void CXTEditListBoxEx::OnSelChange0()
{
	if(!_bAcceptSelChange)
		return;

	OnSelChange();
}

void CXTEditListBoxEx::OnDeleteItem()
{
	int idx = GetCurSel();
	
	if(idx>=0)
	{
		CString str;
		GetText(idx,str);
//		NotifyEvent(Event_DeleteItem,idx,str.GetBuffer());
	}
	
	CXTEditListBox::OnDeleteItem();
	OnSelChange0();
}

void CXTEditListBoxEx::OnMoveItemUp()
{
	CXTEditListBox::OnMoveItemUp();
}
void CXTEditListBoxEx::OnMoveItemDown()
{
	CXTEditListBox::OnMoveItemDown();
}
void CXTEditListBoxEx::OnSelChange()
{
	int idx = GetCurSel();
//	NotifyEvent(Event_SelChange,idx,m_strItemText.GetBuffer());
}
void CXTEditListBoxEx::OnNewItem(int iItem,const char *name)
{
//	if(iItem<0)
//		NotifyEvent(Event_NewItem,iItem,m_strItemText.GetBuffer());
}

