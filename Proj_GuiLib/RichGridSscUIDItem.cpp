/********************************************************************
	created:	2007/8/29   16:29
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridResItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Resource Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridSscUIDItem.h"

#include "stringparser/stringparser.h"

#include "SscUID.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_SscUIDItem::CRichGrid_SscUIDItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_id=NULL;
}


void CRichGrid_SscUIDItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (!_id)
		return;

	extern SscUID SscUID_SafeGen();
	SscUID uid=SscUID_SafeGen();
	if (uid==SscUID_Invalid)
		return;

	CString s;
	if (uid!=SscUID_Invalid)
		s.Format(_T("%u"),uid);
	else
		s=_T("n/a");

	OnValueChanged(s);

// 	GetRichGrid(this)->OnBeginItemChange(this);
// 	*_id=uid;
// 	GetRichGrid(this)->OnItemChange(this);
// 	GetRichGrid(this)->OnEndItemChange(this);

}

void CRichGrid_SscUIDItem::OnValueChanged(CString v)
{
	CXTPPropertyGridItem::OnValueChanged(v);
	m_pGrid->SetFocus();

	if (!_id)
		return;

	DWORD id=SscUID_Invalid;
	if ((v!="n/a")&&(v!=""))
		id = IntFromString(toMBCS((LPCTSTR)v));

	GetRichGrid(this)->OnBeginItemChange(this);

	if (_id)
		(*_id)=id;

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

void CRichGrid_SscUIDItem::Bind(DWORD*id)
{
	_id=id;
	if (*_id!=SscUID_Invalid)
	{
		CString s;
		s.Format(fromMBCS("%u"), *_id);
		SetValue(s);
	}
	else
		SetValue(CString(_T("n/a")));
}

