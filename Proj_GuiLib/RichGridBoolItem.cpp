/********************************************************************
	created:	2006/10/31   14:54
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridItems.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Bool Item
*********************************************************************/


#include "stdh.h"
#include "RichGrid.h"
#include "RichGridBoolItem.h"



//////////////////////////////////////////////////////////////////////////
//CInplaceCheckBox 

BEGIN_MESSAGE_MAP(CInplaceCheckBox, CButton)
	ON_MESSAGE(BM_SETCHECK, OnCheck)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH CInplaceCheckBox::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	class CGridView : public CXTPPropertyGridView
	{
		friend class CInplaceCheckBox;
	};

	CGridView* pGrid = (CGridView*)m_pItem->m_pGrid;

	COLORREF clr = pGrid->GetPaintManager()->GetItemMetrics()->m_clrBack;

	if (clr != m_clrBack || !m_brBack.GetSafeHandle())
	{
		m_brBack.DeleteObject();
		m_brBack.CreateSolidBrush(clr);
		m_clrBack = clr;
	}

	pDC->SetBkColor(m_clrBack);
	return m_brBack;
}

LRESULT CInplaceCheckBox::OnCheck(WPARAM wParam, LPARAM lParam)
{
	m_pItem->m_bValue = (wParam == BST_CHECKED);
	m_pItem->OnValueChanged(m_pItem->GetValue());

	return CButton::DefWindowProc(BM_SETCHECK, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////
//CCustomItemCheckBox

CCustomItemCheckBox::CCustomItemCheckBox(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_wndCheckBox.m_pItem = this;
	m_nFlags = 0;
	m_bValue = FALSE;
}

void CCustomItemCheckBox::OnDeselect()
{
	CXTPPropertyGridItem::OnDeselect();

	if (m_wndCheckBox.m_hWnd) m_wndCheckBox.DestroyWindow();
}

void CCustomItemCheckBox::OnSelect()
{
	CXTPPropertyGridItem::OnSelect();

	if (!m_bReadOnly)
	{
		CRect rc = GetValueRect();
		rc.left -= 15;
		rc.right = rc.left + 15;

		if (!m_wndCheckBox.m_hWnd)
		{
			m_wndCheckBox.Create(NULL, WS_CHILD|BS_AUTOCHECKBOX|BS_FLAT, rc, (CWnd*)m_pGrid, 0);

		}
		if (m_wndCheckBox.GetCheck() != m_bValue) m_wndCheckBox.SetCheck(m_bValue);
		m_wndCheckBox.MoveWindow(rc);
		m_wndCheckBox.ShowWindow(SW_SHOW);
	}
}

CRect CCustomItemCheckBox::GetValueRect()
{
	CRect rcValue(CXTPPropertyGridItem::GetValueRect());
	rcValue.left += 17;
	return rcValue;
}

BOOL CCustomItemCheckBox::OnDrawItemValue(CDC& dc, CRect rcValue)
{
	CRect rcText(rcValue);

	if (m_wndCheckBox.GetSafeHwnd() == 0 && m_bValue)
	{
		CRect rcCheck(rcText.left , rcText.top, rcText.left + 13, rcText.bottom -1);
		dc.DrawFrameControl(rcCheck, DFC_MENU, DFCS_MENUCHECK);
	}

	rcText.left += 17;
	dc.DrawText( GetValue(), rcText,  DT_SINGLELINE|DT_VCENTER);
	return TRUE;
}


BOOL CCustomItemCheckBox::GetBool()
{
	return m_bValue;
}
void CCustomItemCheckBox::SetBool(BOOL bValue)
{
	m_bValue = bValue;

	if (m_wndCheckBox.GetSafeHwnd())
		m_wndCheckBox.SetCheck(bValue);

}


//////////////////////////////////////////////////////////////////////////
//CRichGrid_BoolItem

void CRichGrid_BoolItem::_ApplyCaptioonHides(BOOL b)
{
	if (!b)
	{
		for (int i=0;i<_hides.size();i++)
			GetRichGrid(this)->AddCaptionHide(_hides[i].c_str());
	}
}


void CRichGrid_BoolItem::_ParseConstraint(const char *contraint)
{
	extern void SplitStringBy(const char *sep,const std::string &total,std::vector<std::string>*pPieces);
	SplitStringBy(",",std::string(contraint),&_hides);
}


void CRichGrid_BoolItem::Bind(BOOL *b,const char *constraint)
{
	_ParseConstraint(constraint);
	_b=b;
	if (_b)
	{
		SetBool(*_b);
		_ApplyCaptioonHides(*_b);
	}
}


void CRichGrid_BoolItem::Bind(BYTE*b,const char *constraint)
{
	_ParseConstraint(constraint);
	_b2=b;
	if (_b2)
	{
		SetBool((BOOL)(*_b2));
		_ApplyCaptioonHides((BOOL)(*_b2));
	}
}

void CRichGrid_BoolItem::Bind(DWORD *flag,DWORD mask,const char *constraint)
{
	_ParseConstraint(constraint);
	_flag=flag;
	_mask=mask;
	if (_flag)
	{
		if ((*_flag)&_mask)
		{
			SetBool(TRUE);
			_ApplyCaptioonHides(TRUE);
		}
		else
		{
			SetBool(FALSE);
			_ApplyCaptioonHides(FALSE);
		}
	}
}


void CRichGrid_BoolItem::OnValueChanged(CString strValue)
{
	CXTPPropertyGridItemBool::OnValueChanged(strValue);
	if ((!_b)&&(!_b2)&&(!_flag))
		return;
	GetRichGrid(this)->OnBeginItemChange(this);
	if (_b)
		*_b=m_bValue;
	if (_b2)
		*_b2=m_bValue;
	if (_flag)
	{
		if (m_bValue)
			(*_flag)|=_mask;
		else
			(*_flag)&=(~_mask);
	}
	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

