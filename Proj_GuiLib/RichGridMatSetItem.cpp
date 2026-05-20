
#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridMatSetItem.h"

#include "stringparser/stringparser.h"

IMPLEMENT_DYNAMIC(CRichGrid_MatSetItem,CXTPPropertyGridItem)


CRichGrid_MatSetItem::CRichGrid_MatSetItem( CString strCaption )
		: CXTPPropertyGridItem( strCaption )
{
	_bLS=FALSE;

	_mats=NULL;
	_vecs=NULL;
	_sphs=NULL;
}

void CRichGrid_MatSetItem::Bind(std::vector<i_math::matrix43f> *mats)
{
	_mats=mats;
	UpdateValue();
}

void CRichGrid_MatSetItem::Bind(std::vector<i_math::vector3df> *vecs)
{
	_vecs=vecs;
	UpdateValue();

}

void CRichGrid_MatSetItem::Bind(std::vector<i_math::spheref> *sphs)
{
	_sphs=sphs;
	UpdateValue();
}


BOOL CRichGrid_MatSetItem::OnLButtonDown(UINT nFlags, CPoint point)
{

	return CXTPPropertyGridItem::OnLButtonDown(nFlags, point);
}

void CRichGrid_MatSetItem::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	return;
}



void CRichGrid_MatSetItem::OnValueChanged(CString strValue)
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

BOOL CRichGrid_MatSetItem::OnDrawItemValue(CDC& dc, CRect rcValue0)
{
	return CXTPPropertyGridItem::OnDrawItemValue(dc,rcValue0);
}

void CRichGrid_MatSetItem::UpdateValue()
{
	DWORD c=0;
	if (_mats)
		c=_mats->size();
	if (_vecs)
		c=_vecs->size();
	if (_sphs)
		c=_sphs->size();
	CString s;
	s.Format(_T("%d个位点"),c);
	OnValueChanged(s);
}
