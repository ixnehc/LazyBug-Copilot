#pragma once

#include "math/imath_all.h"


class CRichGridIntPad:public CXTPPropertyGridItem
{
public:

	CRichGridIntPad(CString strValue,int * value)
		:CXTPPropertyGridItem(strValue)
	{
		_valueBind = value;
		_valueBindB=NULL;
	}
	CRichGridIntPad(CString strValue,BYTE* value)
		:CXTPPropertyGridItem(strValue)
	{
		_valueBind = NULL;
		_valueBindB=value;
	}

	virtual void OnValueChanged(CString strValue);
	void Bind(int * value);
	void Bind(BYTE* value);
private:
	int * _valueBind;
	BYTE *_valueBindB;
};


class CRichGridSizeItem :public CXTPPropertyGridItem
{
public:
	CRichGridSizeItem(CString strValue);
	~CRichGridSizeItem(void);

	virtual void  Bind(i_math::vector2di *vec_);
	virtual void  Bind(i_math::vector2db *vec_);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:

	virtual const char *GetHorDesc()	{		return "Width";	}
	virtual const char *GetVerDesc()	{		return "Height";	}
	CRichGridIntPad * m_itemW;
	CRichGridIntPad * m_itemH;

	i_math::vector2di *m_vec;
	i_math::vector2db *m_vecB;
};


class CRichGridPointItem :public CRichGridSizeItem
{
public:
	CRichGridPointItem(CString strValue):CRichGridSizeItem(strValue)
	{

	}
	virtual const char *GetHorDesc()	{		return "x";	}
	virtual const char *GetVerDesc()	{		return "y";	}
};
