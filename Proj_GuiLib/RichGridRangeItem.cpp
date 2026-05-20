#include "stdh.h"
#include "RichGridRangeItem.h"
#include "RichGrid.h"


//////////////////////////////////////////////////////////////////////////
//CRichGridRangeItem

CRichGridRangeItem::~CRichGridRangeItem(void)
{
}
CRichGridRangeItem::CRichGridRangeItem(CString strValue)
:CXTPPropertyGridItem(strValue)
{
	_low= NULL;
	_hi = NULL;
	m_nFlags&=~xtpGridItemHasEdit;
}

void  CRichGridRangeItem::Bind(i_math::rangef*range)
{
	if(!range)
		return;
	_range=range;
	_low->Bind(&(_range->low));
	_hi->Bind(&(_range->hi));

	char buf[255] = {0};
	sprintf(buf,"From: %.3f ,To: %.3f",_range->low,_range->hi);
	SetValue(fromMBCS(buf));

}


void CRichGridRangeItem::OnAddChildItem()
{
	_low	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("From"),&(_range->low)));
	_hi	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("To"),&(_range->hi)));

	_low->SetDescription(_T("From"));
	_hi->SetDescription(_T("To"));
}

void CRichGridRangeItem::OnValueChanged(CString strValue)
{
	if(!_range)
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		sprintf(buf,"From: %.3f ,To: %.3f",_range->low,_range->hi);
		SetValue(fromMBCS(buf));
	}
}
