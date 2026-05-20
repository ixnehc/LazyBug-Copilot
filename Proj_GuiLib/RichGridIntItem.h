
#pragma once

#include "GridItemSpin.h"


//T could be any interger value,like int,short,char
template<class T>
class CRichGrid_IntItem:public CGridItemSpin
{
public:
	CRichGrid_IntItem(CString strCaption):CGridItemSpin(strCaption)
	{
		_v=NULL;
	}
	void Bind(T *v,T min,T max)
	{
		EnableFloatMode(FALSE);
		_v=v;
		if (_v)
		{
			SetRange((SlideSpinValue)min,(SlideSpinValue)max);
			SetValue((SlideSpinValue)*_v);
		}
	}

	virtual void OnBeginValueChange()
	{
		//trigger the change
		GetRichGrid(this)->OnBeginItemChange(this);

		//the outside may lock the paint,this will make the item not updating when user drag the spin
		//so we unlock it temperorily
		GetRichGrid(this)->GetGridView().UnLockPaint();
	}
	virtual void OnValueChanged(SlideSpinValue v)
	{
		if (_v)
			*_v=(T)v;

		GetRichGrid(this)->OnItemChange(this);
	}
	virtual void OnEndValueChange()
	{
		//Lock back
		GetRichGrid(this)->GetGridView().LockPaint();

		GetRichGrid(this)->OnEndItemChange(this);
	}
protected:
	 T*_v;
};

