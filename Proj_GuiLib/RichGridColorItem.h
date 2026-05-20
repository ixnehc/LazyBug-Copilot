#pragma once
#include "ColorSet/ColorSet.h"



class CCustomItemColor: public CXTPPropertyGridItemColor
{
public:
	CCustomItemColor(CString strCaption,  COLORREF clr = 0);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

public:
// 	COLORREF StringToRGB(CString str)
// 	{
// 		TCHAR *stopstring;
// 		int nValue = _tcstoul((LPCTSTR)str, &stopstring, 16);
// 		return RGB(GetBValue(nValue), GetGValue(nValue), GetRValue(nValue));
// 	}
// 
// 	CString RGBToString(COLORREF clr)
// 	{
// 		CString str;
// 		str.Format(_T("%2X%2X%2X"), GetRValue(clr), GetGValue(clr), GetBValue(clr));
// 		str.Replace(_T(" "), _T("0"));
// 		return str;
// 	}

	void SetValue(CString strValue)
	{
		SetColor(StringToRGB(strValue));
	}

	void SetColor(COLORREF clr)
	{
		clr=RGB(GetBValue(clr),GetGValue(clr),GetRValue(clr));
		m_clrValue = clr&0xffffff;
		CXTPPropertyGridItem::SetValue(RGBToString(clr));
	}
};


class CRichGrid_ColorItem:public CCustomItemColor
{
public:
	CRichGrid_ColorItem(CString strCaption):CCustomItemColor(strCaption)
	{
		_col=NULL;
		_dwCol=NULL;
	}
	void Bind(float *color);
	void Bind(DWORD *color);
	virtual void OnValueChanged(CString strValue);
protected:
	float *_col;
	DWORD *_dwCol;

};

//////////////////////////////////////////////////////////////////////////
//CRichGrid_ColorSetItem
class CRichGrid_ColorSetItem : public CXTPPropertyGridItem
{
public:
	CRichGrid_ColorSetItem( CString strCaption );

	void Bind( ColorSet* colSet );

protected:
	virtual void OnInplaceButtonDown( CXTPPropertyGridInplaceButton* pButton );
	
//	CColorSetDialog _dlg;
};

