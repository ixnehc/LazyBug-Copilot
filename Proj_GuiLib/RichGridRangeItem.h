#pragma once

#include "math/imath_all.h"

#include "math/range.h"

#include "RichGridVec3Item.h"




class CRichGridRangeItem :public CXTPPropertyGridItem
{
public:
	CRichGridRangeItem(CString strValue);
	~CRichGridRangeItem(void);

	virtual void  Bind(i_math::rangef*range);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:
	CRichGridFloatPad * _low;
	CRichGridFloatPad * _hi;

	i_math::rangef *_range;
};


