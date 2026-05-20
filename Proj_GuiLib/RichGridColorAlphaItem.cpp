/********************************************************************
	created:	2006/10/31   15:14
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridColorItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Color Item
*********************************************************************/

#include "stdh.h"
#include "RichGrid.h"

#include "RichGridColorAlphaItem.h"

#include "ColorAlphaDialog.h"

#include "graphicsgraph.h"

#include "anim/KeySet.h"

CRichGrid_ColorAlphaItem::CRichGrid_ColorAlphaItem(CString strCaption)
: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton;

	Zero();
}


void CRichGrid_ColorAlphaItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if ((!_col)&&(!_dwcol))
		return;

	CColorAlphaDialog dlg;

	DWORD color;


	if (_col||_dwcol)
	{
		if (_col)
			((i_math::vector4df*)_col)->toDwordColor((i_math::u32&)color);
		if (_dwcol)
			color=*_dwcol;
		dlg.Bind(&color);
	}


	if (IDOK!=dlg.DoModal())
		return;

	OnValueChanged(CString());//Пе
	m_pGrid->SetFocus();

	GetRichGrid(this)->OnBeginItemChange(this);

	if (_col)
		((i_math::vector4df*)_col)->fromDwordColor(color);

	if (_dwcol)
		(*_dwcol)=color;

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);

}

void CRichGrid_ColorAlphaItem::OnValueChanged(CString v)
{
	CXTPPropertyGridItem::OnValueChanged(v);
}

void CRichGrid_ColorAlphaItem::Bind(float *color)
{
	Zero();
	_col=color;
}
void CRichGrid_ColorAlphaItem::Bind(DWORD *color)
{
	Zero();
	_dwcol=color;
}



BOOL CRichGrid_ColorAlphaItem::OnDrawItemValue(CDC& dc, CRect rcValue)
{
	if ((!_col)&&(!_dwcol))
		return TRUE;
	Gdiplus::Graphics graph(dc.m_hDC);

	GraphicsGraph gg;
	gg.Create(rcValue.Width(),rcValue.Height());


	i_math::recti rc(rcValue.left,rcValue.top,rcValue.right,rcValue.bottom);

	rc.zeroBase();

	gg.FillHatchRect(rc,0xffffffff,0,22);

	if (TRUE)
	{
		DWORD col;
		if (_dwcol)
			col=*_dwcol;
		else
			((i_math::vector4df*)_col)->toDwordColor((i_math::u32&)col);

		DWORD alpha=ColorAlpha_Alpha(col);
		col=COLOR_SWAP_RB(col);
		gg.FillSolidRect(rc,col,alpha);
	}
// 	else
// 	{
// 		KeySet *ks;
// 		KeySet ksT;
// 		if (_ks)
// 			ks=_ks;
// 		if (_cs)
// 		{
// 			ksT.CopyFrom(*_cs);
// 			ks=&ksT;
// 		}
// 
// 		if (ks->GetKeyCount()>0)
// 		{
// 			i_math::recti rcSeg;
// 			Key_col*k1,*k2;
// 			DWORD a1,a2;
// 			DWORD c1,c2;
// 			int x1,x2;
// 			int i;
// 			for (i=0;i<ks->GetKeyCount()-1;i++)
// 			{
// 				k1=(Key_col*)ks->GetKey(i);
// 				k2=(Key_col*)ks->GetKey(i+1);
// 
// 				x1=(int)(20.0f*ANIMTICK_TO_SECOND(k1->t));
// 				x2=(int)(20.0f*ANIMTICK_TO_SECOND(k2->t));
// 
// 				rcSeg=rc;
// 
// 				rcSeg.Left()=x1;
// 				rcSeg.Right()=x2;
// 
// 				a1=ColorAlpha_Alpha(k1->color);
// 				a2=ColorAlpha_Alpha(k2->color);
// 
// 				c1=COLOR_SWAP_RB(k1->color);
// 				c2=COLOR_SWAP_RB(k2->color);
// 
// 				gg.GradientH(rcSeg,c1,c2,a1,a2);
// 			}
// 
// 			k1=(Key_col*)ks->GetKey(i);
// 			x1=(int)(20.0f*ANIMTICK_TO_SECOND(k1->t));
// 			a1=ColorAlpha_Alpha(k1->color);
// 			c1=COLOR_SWAP_RB(k1->color);
// 			rcSeg=rc;
// 			rcSeg.Left()=x1;
// 			if (rcSeg.Right()<x1)
// 				rcSeg.Right()=x1;
// 			gg.FillSolidRect(rcSeg,c1,a1);
// 		}
// 	}

	gg.DrawFrameRect(rc,0,1);
	graph.DrawImage(gg.GetBg(), Gdiplus::Point(rcValue.left, rcValue.top));

	return TRUE;
}

CRect CRichGrid_ColorAlphaItem::GetValueRect()
{
	CRect rcValue(CXTPPropertyGridItem::GetValueRect());
	rcValue.left += 25;
	return rcValue;
}
