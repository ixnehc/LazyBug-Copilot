/********************************************************************
	created:	2007/8/29   16:29
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridResItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Resource Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridRandomItem.h"

#include "timer/timer.h"

#include <tchar.h>



///////////////////////////////////////////////////////////////////////////////

CRichGrid_RandomItem::CRichGrid_RandomItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_bind=NULL;
}

void CRichGrid_RandomItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (!_bind)
		return;

	DWORD v=(DWORD)GetTSC();
	v*=(DWORD)GetTSC();
	v*=(DWORD)GetTSC();
	v*=(DWORD)GetTSC();

	CString s;
	s.Format(_T("%u"),v);

	CXTPPropertyGridItem::OnValueChanged(s);
	GetRichGrid(this)->OnBeginItemChange(this);
	(*_bind)=v;
	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);

}

void CRichGrid_RandomItem::OnValueChanged(CString v)
{
//	CXTPPropertyGridItem::OnValueChanged(v);
}

void CRichGrid_RandomItem::Bind(DWORD*v)
{
	_bind=v;
	CString s;
	s.Format(_T("%u"),*v);

	SetValue(s);
}
