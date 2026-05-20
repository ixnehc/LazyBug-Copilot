
#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridAstUIDSetItem.h"

#include "stringparser/stringparser.h"

IMPLEMENT_DYNAMIC(CRichGrid_AstUIDSetItem,CXTPPropertyGridItem)


CRichGrid_AstUIDSetItem::CRichGrid_AstUIDSetItem( CString strCaption )
		: CXTPPropertyGridItem( strCaption )
{
	_uids=NULL;
}

void CRichGrid_AstUIDSetItem::Bind(std::vector<DWORD> *uids)
{
	_uids=uids;
	UpdateValue();
}


BOOL CRichGrid_AstUIDSetItem::OnLButtonDown(UINT nFlags, CPoint point)
{

	return CXTPPropertyGridItem::OnLButtonDown(nFlags, point);
}

void CRichGrid_AstUIDSetItem::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	return;
}



void CRichGrid_AstUIDSetItem::OnValueChanged(CString strValue)
{
	CXTPPropertyGridItem::OnValueChanged(strValue);
// 	if (!_vs)
// 		return;
// 	GetRichGrid(this)->OnBeginItemChange(this);
// 	if (_vs)
// 		_vs->_bVisible=m_bValue;
// 	GetRichGrid(this)->OnItemChange(this);
// 	GetRichGrid(this)->OnEndItemChange(this);
}

BOOL CRichGrid_AstUIDSetItem::OnDrawItemValue(CDC& dc, CRect rcValue0)
{
	return CXTPPropertyGridItem::OnDrawItemValue(dc,rcValue0);
}

void CRichGrid_AstUIDSetItem::UpdateValue()
{
	DWORD c=0;
	if (_uids)
		c=_uids->size();
	CString s;
	s.Format(_T("%d¸ö"),c);
	OnValueChanged(s);
}
