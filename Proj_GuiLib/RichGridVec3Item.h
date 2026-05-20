#pragma once

#include "math/imath_all.h"

class CRichGridFloatPad:public CXTPPropertyGridItem
{
public:

	CRichGridFloatPad(CString strValue,float * value)
		:CXTPPropertyGridItem(strValue)
	{
		_valueBind = value;
	}
	virtual void OnValueChanged(CString strValue);
	void Bind(float * value);
private:
	float * _valueBind;
};

class CRichGridVec4Item :public CXTPPropertyGridItem
{
public:
	CRichGridVec4Item(CString strValue);
	~CRichGridVec4Item(void);

	virtual void  Bind(i_math::vector4df *vec_);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:
	CRichGridFloatPad * m_itemX;
	CRichGridFloatPad * m_itemY;
	CRichGridFloatPad * m_itemZ;
	CRichGridFloatPad * m_itemW;

	i_math::vector4df  *vec;
};


class CRichGridVec3Item :public CXTPPropertyGridItem
{
public:
	CRichGridVec3Item(CString strValue);
	~CRichGridVec3Item(void);

	virtual void  Bind(i_math::vector3df *vec_);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:
	CRichGridFloatPad * m_itemX;
	CRichGridFloatPad * m_itemY;
	CRichGridFloatPad * m_itemZ;
	
	i_math::vector3df  *vec;
};


class CRichGridVec2Item :public CXTPPropertyGridItem
{
public:
	CRichGridVec2Item(CString strValue);
	~CRichGridVec2Item(void);

	virtual void  Bind(i_math::vector2df *vec_);
	virtual void OnAddChildItem();
	virtual void OnValueChanged(CString strValue);

private:
	CRichGridFloatPad * m_itemX;
	CRichGridFloatPad * m_itemY;

	i_math::vector2df  *vec;
};
