/********************************************************************
	created:	2006/9/7   17:35
	filename: 	e:\IxEngine\Proj_GuiLib\AnimEventList.cpp
	author:		cxi
	
	purpose:	an spin item used in grid
*********************************************************************/

#include "stdh.h"
#include "GridItemSpin.h"
#include "WMGuiLib.h"

#include "stringparser/stringparser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////////
//CGridItemSpinInplaceButton 
void CGridItemSpinInplaceButton::OnBeginValueChange()
{
	m_pItem->OnBeginValueChange();
}

void CGridItemSpinInplaceButton::OnEndValueChange()
{
	m_pItem->OnEndValueChange();
}

void CGridItemSpinInplaceButton::OnValueChange(SlideSpinValue vNew)
{
	CString str;
	m_pItem->ValueToString(vNew,str);
	m_pItem->CXTPPropertyGridItem::SetValue(str);//update the showing content 

	m_pItem->OnValueChanged(vNew);//let the item know
}
 

//////////////////////////////////////////////////////////////////////////
//CGridItemSpin

CGridItemSpin::CGridItemSpin(CString strCaption)
: CXTPPropertyGridItemNumber(strCaption)
{
	m_wndSpin.m_pItem = this;
	m_bFloatMode=FALSE;
}
void CGridItemSpin::OnDeselect()
{
	CXTPPropertyGridItemNumber::OnDeselect();

	if (m_wndSpin.m_hWnd) m_wndSpin.ShowWindow(SW_HIDE);
}

void CGridItemSpin::OnSelect()
{
	CXTPPropertyGridItemNumber::OnSelect();

	if (!m_bReadOnly)
	{
		CRect rc = GetItemRect();
		rc.left = rc.right - 15;
		GetSpin()->MoveWindow(rc);
		GetSpin()->ShowWindow(SW_SHOW);
	}
}

//this will be only called when the user changed something in the editor
void CGridItemSpin::OnValueChanged(CString strValue)
{
	SlideSpinValue v,vOrg;
	if (!m_bFloatMode)
		v = IntFromString(toMBCS((LPCTSTR)strValue));
	else
		v=DoubleFromString(toMBCS((LPCTSTR)strValue));

	vOrg=m_wndSpin.GetValue();
	m_wndSpin.SetValue(v);
	v=m_wndSpin.GetValue();

	if (v==vOrg)
		return;

	ValueToString(v,strValue);

	CXTPPropertyGridItemNumber::OnValueChanged(strValue);

	OnBeginValueChange();
	OnValueChanged(v);
	OnEndValueChange();
}

CRect CGridItemSpin::GetValueRect()
{
	CRect rcValue(CXTPPropertyGridItemNumber::GetValueRect());
	rcValue.right -= 17;
	return rcValue;
}

CGridItemSpinInplaceButton *CGridItemSpin::GetSpin()
{
	if (m_wndSpin.m_hWnd)
		return &m_wndSpin;
	CRect rc = GetItemRect();
	rc.left = rc.right - 15;
	m_wndSpin.Create(UDS_ARROWKEYS|WS_CHILD,rc, (CWnd*)m_pGrid, 110);
	m_wndSpin.SetRange(0, 100);
	DWORD style=::GetClassLong(m_wndSpin.m_hWnd,GCL_STYLE);
	style|=CS_DBLCLKS;
	::SetClassLong(m_wndSpin.m_hWnd,GCL_STYLE,style);

//	UDACCEL a;
//	a.nInc=0;
//	a.nSec=10000;
//	m_wndSpin.SetAccel(1,&a);

	return &m_wndSpin;
}

void CGridItemSpin::SetRange(SlideSpinValue start,SlideSpinValue end)
{
	GetSpin()->SetRange(start,end);
	SetValue(GetSpin()->GetValue());
}

void CGridItemSpin::SetValue(SlideSpinValue v)
{
	GetSpin()->SetValue(v);

	v=GetSpin()->GetValue();

	CString str;
	ValueToString(v,str);
	CXTPPropertyGridItem::SetValue(str);
}


void CGridItemSpin::ValueToString(SlideSpinValue v,CString &str)
{
	if (!m_bFloatMode)
		str.Format(_T("%d"),(int)v);
	else
		str.Format(_T("%f"),(float)v);
}

