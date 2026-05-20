/********************************************************************
	created:	2006/10/31   14:54
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridItems.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Bool Item
*********************************************************************/


#include "stdh.h"
#include "resource.h"
#include "RichGrid.h"
#include "RichGridButtonItem.h"
#include ".\richgridbuttonitem.h"

#include <assert.h>


RGIB_Info g_RGIBInfo[]=
{
	{ID_RGIB_MoveUp,			0,						FALSE},
	{ID_RGIB_MoveDown,		1,						FALSE},
	{ID_RGIB_Remove,				2,						FALSE},
	{ID_RGIB_Clone,				3,						FALSE},
	{ID_RGIB_Clear,				4,						FALSE},
	{ID_RGIB_Browse,				5,						FALSE},
	{ID_RGIB_New,					6,						FALSE},
	{ID_RGIB_Menu,				7,						FALSE},
	{ID_RGIB_Reserve1,			8,						FALSE},
	{ID_RGIB_Reserve2,			9,						FALSE},
	{ID_RGIB_Reserve3,			10,					FALSE},

	{ID_RGIB_Visible,				11,					TRUE},
	{ID_RGIB_Reserve4,			12,					TRUE},
	{ID_RGIB_Reserve5,			13,					TRUE},
	{ID_RGIB_Reserve6,			14,					TRUE},
};

int RGIBInfo_IndexFromID(DWORD id)
{
	for (int i=0;i<ARRAY_SIZE(g_RGIBInfo);i++)
		if (g_RGIBInfo[i].id==id)
			return i;
	return -1;
}

#define BUTTON_WIDTH 15
#define BUTTON_GAP 0
#define BUTTON_STEP (BUTTON_WIDTH+BUTTON_GAP*2)

//////////////////////////////////////////////////////////////////////////
//CRichGridItemButton
CImageList CRichGridItemButton::_imagelist;

BEGIN_MESSAGE_MAP(CRichGridItemButton, CXTButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnBnClicked)
END_MESSAGE_MAP()

CImageList *CRichGridItemButton::GetImageList()
{
	if (_imagelist.GetSafeHandle())
		return &_imagelist;

	extern BOOL CreateImageList(CImageList& il, UINT nID,int w,int h);
	CreateImageList(_imagelist,IDB_RGBUTTONS,12,12);

	return &_imagelist;
}

void CRichGridItemButton::OnBnClicked()
{
	if (_pItem)
		_pItem->OnButtonClick(GetDlgCtrlID());

}

BOOL CRichGridItemButton::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	if (_pItem)
		_pItem->OnButtonMenuCmd(LOWORD(wParam));

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CRichGrid_ButtonItem

CRichGrid_ButtonItem::CRichGrid_ButtonItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = 0;
	_maskBtn=0;
	for (int i=0;i<RDIB_COUNT;i++)
	{
		_buttons[i]._pItem=this;
		_binds[i]=NULL;
	}
}

void CRichGrid_ButtonItem::ClearButtonMask()
{
	_maskBtn=0;
}

void CRichGrid_ButtonItem::AddButtonMask(DWORD maskBtns)
{
	_maskBtn|=maskBtns;
}

BOOL CRichGrid_ButtonItem::Bind(DWORD idButton,BOOL *b)
{
	int idx=RGIBInfo_IndexFromID(idButton);
	if (idx==-1)
		return FALSE;

	_binds[idx]=b;

	if(_buttons[idx].m_hWnd)
	{
		if (*b)
			_buttons[idx].SetCheck(1);
		else
			_buttons[idx].SetCheck(0);
	}

	return TRUE;

}


void CRichGrid_ButtonItem::OnDeselect()
{
	CXTPPropertyGridItem::OnDeselect();

	for (int i=0;i<RDIB_COUNT;i++)
	{
		if (_buttons[i].m_hWnd)
			_buttons[i].DestroyWindow();
	}
}

//create the buttons corresponding to the bit set in _maskBtn
void CRichGrid_ButtonItem::_CreateButtons()
{
	assert(ARRAY_SIZE(g_RGIBInfo)==RDIB_COUNT);
	CRect rc;
	rc.SetRect(0,0,1,1);
	for (int i=0;i<ARRAY_SIZE(g_RGIBInfo);i++)
	{
		if (g_RGIBInfo[i].id&_maskBtn)
		{
			if (!_buttons[i].m_hWnd)
			{
				if (!g_RGIBInfo[i].bCheckBox)
					_buttons[i].Create(NULL, WS_CHILD|BS_PUSHBUTTON, rc, (CWnd*)m_pGrid, g_RGIBInfo[i].id);
				else
					_buttons[i].Create(NULL, WS_CHILD|BS_CHECKBOX|BS_PUSHBUTTON, rc, (CWnd*)m_pGrid, g_RGIBInfo[i].id);

					_buttons[i].SetColorFace(RGB(255,184,101));
					_buttons[i].SetBorderGap(2);
					_buttons[i].SetIcon(CSize(12,12), _buttons[i].GetImageList()->ExtractIcon(g_RGIBInfo[i].idxImage));
			}
		}
	}
}

void CRichGrid_ButtonItem::_GetButtonCount(DWORD &cLeft,DWORD &cRight)
{
	cLeft=0;cRight=0;
	CRect rc = CXTPPropertyGridItem::GetValueRect();
	for (int i=0;i<ARRAY_SIZE(g_RGIBInfo);i++)
	{
		if (g_RGIBInfo[i].id&_maskBtn)
		{
			if (g_RGIBInfo[i].bCheckBox)
			{
				if ((cLeft+cRight+1)*BUTTON_STEP>=rc.Width())
					break;
				cLeft++;
			}
		}
	}
	for (int i=0;i<ARRAY_SIZE(g_RGIBInfo);i++)
	{
		if (g_RGIBInfo[i].id&_maskBtn)
		{
			if (!g_RGIBInfo[i].bCheckBox)
			{
				if ((cLeft+cRight+1)*BUTTON_STEP>=rc.Width())
					break;
				cRight++;
			}
		}
	}
}


void CRichGrid_ButtonItem::_ArrangeButtons()
{
	CRect rc = CXTPPropertyGridItem::GetValueRect();
	DWORD c1,c2;
	_GetButtonCount(c1,c2);
	CRect rc1,rc2;
	rc1.SetRect(0,0,BUTTON_WIDTH,rc.Height());
	rc1+=rc.TopLeft();
	rc2=rc1;
	rc1+=CPoint(BUTTON_GAP,0);
	rc2+=CPoint(rc.Width()-c2*BUTTON_STEP,0);
	rc2.bottom--;
	rc1.bottom--;

	for (int i=0;i<ARRAY_SIZE(g_RGIBInfo);i++)
	{
		if (!(g_RGIBInfo[i].id&_maskBtn))
			continue;
		assert(_buttons[i].m_hWnd);
		_buttons[i].ShowWindow(SW_HIDE);
		if (g_RGIBInfo[i].bCheckBox)
		{
			if (c1>0)
			{
				_buttons[i].ShowWindow(SW_SHOW);
				_buttons[i].MoveWindow(rc1);
				rc1+=CPoint(BUTTON_STEP,0);
				c1--;
			}
		}
		else
		{
			if (c2>0)
			{
				_buttons[i].ShowWindow(SW_SHOW);
				_buttons[i].MoveWindow(rc2);
				rc2+=CPoint(BUTTON_STEP,0);
				c2--;
			}
		}
	}
}

void CRichGrid_ButtonItem::OnSelect()
{
	CXTPPropertyGridItem::OnSelect();

	if (CXTPPropertyGridItem::GetReadOnly())
		return;

	_CreateButtons();
	_ArrangeButtons();
}

CRect CRichGrid_ButtonItem::GetValueRect()
{
	CRect rc(CXTPPropertyGridItem::GetValueRect());
	DWORD c1,c2;

	_GetButtonCount(c1,c2);

	rc.left+=c1*BUTTON_STEP;
	rc.right-=c2*BUTTON_STEP;

	return rc;
}

CRichGridItemButton *CRichGrid_ButtonItem::_GetButtonFromID(DWORD idButton)
{
	return (CRichGridItemButton *)(m_pGrid->GetDlgItem(idButton));
}

void CRichGrid_ButtonItem::OnButtonMenuCmd(DWORD idCmd)
{

}

void CRichGrid_ButtonItem::OnButtonClick(DWORD idButton)
{
	GetRichGrid(this)->OnBeginItemChange(this);

	//check the binding
	if (TRUE)
	{
		int idx=RGIBInfo_IndexFromID(idButton);
		if (idx!=-1)
		{
			if ((_binds[idx])&&(_buttons[idx].m_hWnd)&&(g_RGIBInfo[idx].bCheckBox))
			{
				(*_binds[idx])=_buttons[idx].GetCheck();
				GetRichGrid(this)->OnItemChange(this);
			}
		}
	}

	//now ,let the parent do the command
	GetRichGrid(this)->OnItemCommand(this,idButton);

	GetRichGrid(this)->OnEndItemChange(this);

}

