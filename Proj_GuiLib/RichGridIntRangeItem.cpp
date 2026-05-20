#include "stdh.h"

#include "RichGridSizeItem.h"
#include "RichGridIntRangeItem.h"
#include "RichGrid.h"

#include "stringparser/stringparser.h"


//////////////////////////////////////////////////////////////////////////
//CRichGridIntRangeItem

CRichGridIntRangeItem::~CRichGridIntRangeItem(void)
{
}
CRichGridIntRangeItem::CRichGridIntRangeItem(CString strValue)
:CXTPPropertyGridItem(strValue)
{
	_low= NULL;
	_hi = NULL;
	m_nFlags&=~xtpGridItemHasEdit;
}

void  CRichGridIntRangeItem::Bind(i_math::rangei*range)
{
	if(!range)
		return;
	_range=range;
	_low->Bind(&(_range->low));
	_hi->Bind(&(_range->hi));

	char buf[255] = {0};
	sprintf(buf,"From: %d ,To: %d",_range->low,_range->hi);
	SetValue(fromMBCS(buf));

}


void CRichGridIntRangeItem::OnAddChildItem()
{
	_low	  = (CRichGridIntPad *)AddChildItem(new CRichGridIntPad(_T("From"),&(_range->low)));
	_hi	  = (CRichGridIntPad *)AddChildItem(new CRichGridIntPad(_T("To"),&(_range->hi)));

	_low->SetDescription(_T("From"));
	_hi->SetDescription(_T("To"));
}

void CRichGridIntRangeItem::OnValueChanged(CString strValue)
{
	if(!_range)
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		sprintf(buf,"From: %d ,To: %d",_range->low,_range->hi);
		SetValue(fromMBCS(buf));
	}
}
