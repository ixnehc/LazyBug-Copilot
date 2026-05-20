/********************************************************************
	created:	2007/10/10   11:03
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridNameItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Simple String Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridNameItem.h"

#include "stringparser/stringparser.h"

#include "Log/LogFile.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_NameItem::CRichGrid_NameItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags |=xtpGridItemHasEdit;

	_s=NULL;
}

void CRichGrid_NameItem::OnValueChanged(CString v)
{
	CXTPPropertyGridItem::OnValueChanged(v);
	m_pGrid->SetFocus();

	if (!_s)
		return;

	GetRichGrid(this)->OnBeginItemChange(this);

	*_s = toMBCS((LPCTSTR)v);

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

void CRichGrid_NameItem::Bind(std::string *s)
{
	_s=s;
	
	SetValue(CString(s->c_str()));
}


