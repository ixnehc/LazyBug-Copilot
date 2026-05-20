#pragma once
#include "ColorSet/ColorSet.h"


class CRichGrid_ColorAlphaItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_ColorAlphaItem(CString strCaption);

	void Zero()
	{
		_col=NULL;
		_dwcol=NULL;
	}

	void Bind(float *color);
	void Bind(DWORD *color);
	virtual void OnValueChanged(CString v);

	virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);
	virtual CRect GetValueRect();

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	float *_col;
	DWORD *_dwcol;

};
