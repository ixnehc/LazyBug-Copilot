#pragma once

#include "math/imath_all.h"

#include "math/range.h"

#include "RichGridVec3Item.h"


class CRichGridIntPad;

class CRichGridIntRangeItem :public CXTPPropertyGridItem
{
public:
	CRichGridIntRangeItem(CString strValue);
	~CRichGridIntRangeItem(void);

	virtual void  Bind(i_math::rangei*range);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:
	CRichGridIntPad * _low;
	CRichGridIntPad * _hi;

	i_math::rangei *_range;
};


