#pragma once

#include "RichGridIntItem.h"

#include "math/imath_all.h"

class CRichGridRectItemPad:public CRichGrid_IntItem<int>
{
public:
	CRichGridRectItemPad(CString strValue):CRichGrid_IntItem<int>(strValue)
	{
	}
	virtual void OnBeginValueChange();
	virtual void OnValueChanged(SlideSpinValue v);
	virtual void CRichGridRectItemPad::OnEndValueChange();

};


class CRichGridRectItem : public CXTPPropertyGridItem
{
public:
	CRichGridRectItem(CString strValue)
		:CXTPPropertyGridItem(strValue)
	{
		m_nFlags = 0;
		_left=NULL;
		_top=NULL;
		_right=NULL;
		_bottom=NULL;
		_width=NULL;
		_height=NULL;
		_rc=NULL;
		_bNoRepair=FALSE;
		_gran=1;
	}

	~CRichGridRectItem(void);

	virtual void Bind(i_math::recti *rc);
	void SetNoRepair()	{		_bNoRepair=TRUE;	}
	void SetGran(int gran)	{		_gran=gran;	}
	virtual void UpdateSize();
	void UpdateGran();
protected:
	virtual void OnAddChildItem();

	virtual BOOL IsShowSize()
	{
		return FALSE;
	}

private:

	void _SetValue();


	CRichGridRectItemPad* _left;
	CRichGridRectItemPad* _top;
	CRichGridRectItemPad* _right;
	CRichGridRectItemPad* _bottom;
	CRichGridRectItemPad* _width;
	CRichGridRectItemPad* _height;

	int _w,_h;
	int _l,_t,_r,_b;

	BOOL _bNoRepair;
	int _gran;

	
	i_math::recti*_rc;
};

class CRichGridRectItem_ShowSize:public CRichGridRectItem
{
public:
	CRichGridRectItem_ShowSize(CString strValue)
		:CRichGridRectItem(strValue)
	{
	}

	virtual BOOL IsShowSize()
	{
		return TRUE;
	}

};