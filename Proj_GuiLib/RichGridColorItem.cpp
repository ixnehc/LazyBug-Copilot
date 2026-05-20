/********************************************************************
	created:	2006/10/31   15:14
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridColorItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Color Item
*********************************************************************/

#include "stdh.h"
#include "RichGrid.h"

#include "RichGridColorItem.h"

#include "ColorAlphaDialog.h"

//////////////////////////////////////////////////////////////////////////
//CCustomItemColorPopup

class CCustomItemColorPopup: public CXTColorPopup
{
	friend class CCustomItemColor;
public:
	CCustomItemColorPopup() : CXTColorPopup(TRUE) {}
private:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSelEndOK(WPARAM wParam, LPARAM lParam);

	CCustomItemColor* m_pItem;
};

BEGIN_MESSAGE_MAP(CCustomItemColorPopup, CXTColorPopup)
	ON_MESSAGE(CPN_XT_SELENDOK, OnSelEndOK)
END_MESSAGE_MAP()


LRESULT CCustomItemColorPopup::OnSelEndOK(WPARAM wParam, LPARAM /*lParam*/)
{
	m_pItem->OnValueChanged(m_pItem->RGBToString((COLORREF)wParam));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//CCustomItemColor

CCustomItemColor::CCustomItemColor(CString strCaption, COLORREF clr)
	: CXTPPropertyGridItemColor(strCaption, clr)
{
	m_nFlags = xtpGridItemHasComboButton|xtpGridItemHasEdit;
	SetColor(clr);
}

void CCustomItemColor::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (TRUE)
	{
		CColorAlphaDialog dlg;
		dlg.DoModal();
	}

	CCustomItemColorPopup *pColorPopup = new CCustomItemColorPopup();

	CRect rcItem= GetItemRect();
	m_pGrid->ClientToScreen(&rcItem);
	rcItem.left = rcItem.right - 158; // small hack. need to add CPS_XT_LEFTALLIGN

	pColorPopup->Create(rcItem, m_pGrid, CPS_XT_EXTENDED|CPS_XT_MORECOLORS|CPS_XT_SHOW3DSELECTION|CPS_XT_SHOWHEXVALUE, GetColor(), GetColor());
	pColorPopup->SetOwner(m_pGrid);
	pColorPopup->SetFocus();
	pColorPopup->AddListener(pColorPopup->GetSafeHwnd());
	pColorPopup->m_pItem = this;

}

//////////////////////////////////////////////////////////////////////////
//CRichGrid_ColorItem
void CRichGrid_ColorItem::Bind(float *color)
{
	_dwCol=NULL;
	_col=color;
	if (_col)
	{
		unsigned int dw;
		((color3df*)_col)->toDwordColor(dw);
		SetColor(dw);
	}
}

void CRichGrid_ColorItem::Bind(DWORD *color)
{
	_col=NULL;
	_dwCol=color;
	if (_dwCol)
		SetColor(*_dwCol);
}


void CRichGrid_ColorItem::OnValueChanged(CString strValue)
{
	CCustomItemColor::OnValueChanged(strValue);
	if ((!_col)&&(!_dwCol))
		return;

	GetRichGrid(this)->OnBeginItemChange(this);

	DWORD dw;
	dw=StringToRGB(strValue);
	dw=RGB(GetBValue(dw),GetGValue(dw),GetRValue(dw));
	dw|=0xff000000;
	SetColor(dw);

	if (_col)
		((color3df*)_col)->fromDwordColor(dw);
	if (_dwCol)
		*_dwCol=dw;

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

//////////////////////////////////////////////////////////////////////////
//CRichGrid_ColorSetItem
CRichGrid_ColorSetItem::CRichGrid_ColorSetItem( CString strCaption )
	:CXTPPropertyGridItem( strCaption )
{
	m_nFlags = xtpGridItemHasComboButton|xtpGridItemHasEdit;
}

void CRichGrid_ColorSetItem::Bind(  ColorSet* colSet )
{
//	_dlg.BindProperty( colSet );
}

void CRichGrid_ColorSetItem::OnInplaceButtonDown( CXTPPropertyGridInplaceButton* pButton )
{

//	if ( _dlg.DoModal() == IDOK )
//	{
//		bool bBreak = true;
//	}
}