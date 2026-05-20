/********************************************************************
	created:	2007/8/29   16:29
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridResItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Resource Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridStrIDItem.h"

#include "stringparser/stringparser.h"


#include "StrLibDlg.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_StrIDItem::CRichGrid_StrIDItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_id=StringID_Invalid;
	_iCategory=STRLIB_CATEGORY_DEFAULT;
}

void CRichGrid_StrIDItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (!_id)
		return;

	StringID id=*_id;
	extern BOOL StrLibDlg_Browse(DWORD iCategory,StringID &id,const char *grp);
	if (StrLibDlg_Browse(_iCategory,id,_grp.c_str()))
	{
		const char *str=MakeShortStr(StrLib_GetStr(*_id),32);
		CXTPPropertyGridItem::OnValueChanged(CString(str));
		GetRichGrid(this)->OnBeginItemChange(this);
		(*_id)=id;
		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);
	}
	else
	{
		const char *str=MakeShortStr(StrLib_GetStr(*_id),32);
		CXTPPropertyGridItem::OnValueChanged(CString(str));
	}

}

void CRichGrid_StrIDItem::OnValueChanged(CString v)
{
//	CXTPPropertyGridItem::OnValueChanged(v);
}

void CRichGrid_StrIDItem::Bind(StringID*id,const char *grp,DWORD iCategory)
{
	_id=id;
	_grp=grp;
	_iCategory=iCategory;

	const char *str=MakeShortStr(StrLib_GetStr(*id),32);

	SetValue(CString(str));
}
