#pragma once

#include "GuiLib.h"
#include "WorldSystem/stubparams/param_sys.h"

#include "ref/ref.h"


class CRichGrid_ValueSetItem : public CXTPPropertyGridItemBool
{
public:
	DEFINE_REF()
	CRichGrid_ValueSetItem( CString strCaption );
	virtual ~CRichGrid_ValueSetItem()
	{
		BreakRef();
	}
	
	void Bind( ValueSet *vs,i_math::rectf &rcLimit);
	ValueSet *GetBind()	{		return _vs;	};
	DWORD GetLineCol();
	BOOL NeedFit()	{		return _bNeedFit;	}
	void ClearNeedFit()	{		_bNeedFit=FALSE;	}
	i_math::rectf GetRangeRect();
	i_math::rectf GetLimitRect();
protected:
	virtual BOOL OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);

	virtual void OnValueChanged(CString strValue);

	virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);

	ValueSet *_vs;
	DWORD _linecol;
	i_math::rectf _rcLimit;
	BOOL _bNeedFit;
};



