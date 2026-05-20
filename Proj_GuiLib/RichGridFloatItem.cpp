/********************************************************************
	created:	2006/10/31   15:47
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridFloatItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Float Item
*********************************************************************/

#include "stdh.h"
#include "RichGrid.h"

#include "RichGridFloatItem.h"

//////////////////////////////////////////////////////////////////////////
//CRichGrid_FloatItem

void CRichGrid_FloatItem::Bind(float *f,float min,float max)
{
	EnableFloatMode(TRUE);
	_f=f;
	if (_f)
	{
		SetRange((SlideSpinValue)min,(SlideSpinValue)max);
		SetValue((SlideSpinValue)*_f);
	}
}


void CRichGrid_FloatItem::OnBeginValueChange()
{
	//trigger the change
	GetRichGrid(this)->OnBeginItemChange(this);

	//the outside may lock the paint,this will make the item not updating when user drag the spin
	//so we unlock it temperorily
	GetRichGrid(this)->GetGridView().UnLockPaint();

	
}
void CRichGrid_FloatItem::OnValueChanged(SlideSpinValue v)
{
	if (_f)
		*_f=(float)v;

	GetRichGrid(this)->OnItemChange(this);
}

void CRichGrid_FloatItem::OnEndValueChange()
{
	//Lock back
	GetRichGrid(this)->GetGridView().LockPaint();

	GetRichGrid(this)->OnEndItemChange(this);
}


//////////////////////////////////////////////////////////////////////////
//CRichGrid_AnimTickItem
void CRichGrid_AnimTickItem::Bind(AnimTick *v,float min,float max)
{
	_v=v;
	_fv=ANIMTICK_TO_SECOND(*v);
	CRichGrid_FloatItem::Bind(&_fv,min,max);
}

void CRichGrid_AnimTickItem::OnValueChanged(SlideSpinValue v)
{
	if (_f)
		*_f=(float)v;

	if (_v)
		(*_v)=ANIMTICK_FROM_SECOND(v);

	GetRichGrid(this)->OnItemChange(this);
}
