#include "stdh.h"
#include "RichGridRectItem.h"
#include "RichGrid.h"



//////////////////////////////////////////////////////////////////////////
//CRichGridRectItemPad

void CRichGridRectItemPad::OnBeginValueChange()
{
	GetRichGrid(GetParentItem())->OnBeginItemChange(GetParentItem());
	//the outside may lock the paint,this will make the item not updating when user drag the spin
	//so we unlock it temperorily
	GetRichGrid(GetParentItem())->GetGridView().UnLockPaint();
}
void CRichGridRectItemPad::OnEndValueChange()
{
	CRichGridRectItem * parent = (CRichGridRectItem *) GetParentItem();
	parent->UpdateGran();
	//Lock back
	GetRichGrid(GetParentItem())->GetGridView().LockPaint();

	GetRichGrid(GetParentItem())->OnEndItemChange(GetParentItem());
}

void CRichGridRectItemPad::OnValueChanged(SlideSpinValue v)
{
	if (_v)
		*_v=(int)v;
	CRichGridRectItem * parent = (CRichGridRectItem *) GetParentItem();
	parent->UpdateSize();
}

//////////////////////////////////////////////////////////////////////////
//CRichGridRectItem

CRichGridRectItem::~CRichGridRectItem(void)
{

}


void CRichGridRectItem::Bind(i_math::recti *rc)
{	
	_rc= rc;

	if (rc)
	{
		_w=rc->getWidth();
		_h=rc->getHeight();
		_l=rc->Left();
		_t=rc->Top();
		_r=rc->Right();
		_b=rc->Bottom();
	}

	_top->Bind(&_t,-10000,10000);
	_left->Bind(&_l,-10000,10000);
	if (IsShowSize())
	{
		_width->Bind(&_w,0,10000);
		_height->Bind(&_h,0,10000);
	}
	else
	{
		_right->Bind(&_r,-10000,10000);
		_bottom->Bind(&_b,-10000,10000);
	}

	_SetValue();
}

void CRichGridRectItem::_SetValue()
{
	if(!_rc)
		SetValue(_T("<UnBound>"));
	else
	{
		char buf[255];
		sprintf(buf,"(%d,%d)~(%d,%d)",_rc->Left(),_rc->Top(),_rc->Right(),_rc->Bottom());
		SetValue(fromMBCS(buf));
	}
}


void CRichGridRectItem::UpdateSize()
{	
	_rc->Left()=_l;
	_rc->Top()=_t;
	if (IsShowSize())
	{
		_rc->Right()=_l+_w;
		_rc->Bottom()=_t+_h;
	}
	else
	{
		_rc->Right()=_r;
		_rc->Bottom()=_b;
	}
	if (!_bNoRepair)
		_rc->repair();


	_SetValue();
	GetRichGrid(this)->OnItemChange(this);

}

void CRichGridRectItem::UpdateGran()
{
	if (_gran>1)
	{
		_l=i_math::idiv_signed(_l,_gran)*_gran;
		_t=i_math::idiv_signed(_t,_gran)*_gran;
		if (IsShowSize())
		{
			_w=i_math::idiv_signed(_w,_gran)*_gran;
			_h=i_math::idiv_signed(_h,_gran)*_gran;
		}
		else
		{
			_r=i_math::idiv_signed(_r,_gran)*_gran;
			_b=i_math::idiv_signed(_b,_gran)*_gran;
		}
		UpdateSize();
	}
}


void CRichGridRectItem::OnAddChildItem()
{	
	_left= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Left")));
	_top= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Top")));
	if (IsShowSize())
	{
		_width= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Width")));
		_height= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Height")));
	}
	else
	{
		_right= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Right")));
		_bottom= (CRichGridRectItemPad*) AddChildItem(new CRichGridRectItemPad(_T("Bottom")));
	}

}


