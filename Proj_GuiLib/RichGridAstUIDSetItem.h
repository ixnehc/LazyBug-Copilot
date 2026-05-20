#pragma once

#include "GuiLib.h"
#include "WorldSystem/stubparams/param_sys.h"

#include "ref/ref.h"


class CRichGrid_AstUIDSetItem : public CXTPPropertyGridItem
{
public:
	CRichGrid_AstUIDSetItem( CString strCaption );
	virtual ~CRichGrid_AstUIDSetItem()
	{
	}
	
	void Bind(std::vector<DWORD> *uids);
	std::vector<DWORD> *GetBindUIDs()	{		return _uids;	};

	void UpdateValue();
protected:
	virtual BOOL OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);

	virtual void OnValueChanged(CString strValue);

	virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);

	std::vector<DWORD> *_uids;

	DECLARE_DYNAMIC(CRichGrid_AstUIDSetItem)

};



