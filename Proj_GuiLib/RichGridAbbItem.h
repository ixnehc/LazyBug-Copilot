#pragma once

#include "RichGridFloatItem.h"
#include "RichGridVec3Item.h"
#include "math/imath_all.h"

class CRichGridAbbItem : public CXTPPropertyGridItem
{
	class  CRichGridAbbItemFloat;
	friend class CRichGridAbbItemFloat;
public:
	CRichGridAbbItem(CString strValue)
		:CXTPPropertyGridItem(strValue)
	{
		_widthItem = NULL;
		_heightItem = NULL;
		_thickItem = NULL;
		_padMin = NULL;
		_padMax = NULL;
		_aabb = NULL;
	}

	~CRichGridAbbItem(void);

	virtual void Bind(i_math::aabbox3df * aabb);
protected:
	virtual void OnValueChanged(CString strValue);
	virtual void OnAddChildItem();
	virtual void UpdateSize();

private:
	CRichGridAbbItemFloat	* _widthItem;
	CRichGridAbbItemFloat	* _heightItem;
	CRichGridAbbItemFloat	* _thickItem;
	CRichGridVec3Item   * _padMin;
	CRichGridVec3Item   * _padMax;
	
	i_math::aabbox3df *_aabb;
	float _w;
	float _h;
	float _t;
};
