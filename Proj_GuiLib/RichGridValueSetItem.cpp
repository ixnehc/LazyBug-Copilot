
#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridValueSetItem.h"

#include "graphicsgraph.h"

#include "stringparser/stringparser.h"


CRichGrid_ValueSetItem::CRichGrid_ValueSetItem( CString strCaption )
		: CXTPPropertyGridItemBool( strCaption )
{
	m_nFlags = xtpGridItemHasExpandButton;
	SetCheckBoxStyle(TRUE);

	_linecol=0;

	_bNeedFit=FALSE;

	_vs=NULL;
}

void CalcValueSetDesc(ValueSet *vs,std::string &str)
{
	str="";
	char buf[32];
	for ( int i = 0; i < vs->GetKeyCount(); ++i )
	{
		sprintf( &buf[0], "(%0.2f)->", ((Key_f*)vs->GetKey(i))->v);
		str+= buf;
	}
}

DWORD CalcLineColor(const char *str)
{
	DWORD idx=CalcHashCode(str);

	DWORD lo,hi,step;
	lo=144;
	hi=240;
	step=16;
	DWORD count=(hi-lo)/step+1;//每个channel的个数

	int total=count*count*count;

	idx%=total;

	DWORD r=idx/(count*count);
	idx=idx%(count*count);
	DWORD g=idx/count;
	DWORD b=idx%count;

	r=lo+r*step;
	g=lo+g*step;
	b=lo+b*step;

	return RGB(r,g,b);
}

void CRichGrid_ValueSetItem::Bind(ValueSet *vs,i_math::rectf &rcLimit)
{
	_vs=vs;
	_rcLimit=rcLimit;
	if (_vs)
		SetBool(_vs->_bVisible);
}

BOOL CRichGrid_ValueSetItem::OnLButtonDown(UINT nFlags, CPoint point)
{

	if (PtInCheckBoxRect(point) && !GetReadOnly())
	{
		SelectNextConstraint();
		return TRUE;
	}

	return CXTPPropertyGridItem::OnLButtonDown(nFlags, point);
}

void CRichGrid_ValueSetItem::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	_bNeedFit=TRUE;
	return;
}



void CRichGrid_ValueSetItem::OnValueChanged(CString strValue)
{
	CXTPPropertyGridItemBool::OnValueChanged(strValue);
	if (!_vs)
		return;
	GetRichGrid(this)->OnBeginItemChange(this);
	if (_vs)
		_vs->_bVisible=m_bValue;
	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}


DWORD CRichGrid_ValueSetItem::GetLineCol()
{
	if (_linecol==0)
	{
		_linecol=CalcLineColor(GetRichGrid(this)->PathFromItem(this));
		_linecol=ColorAlpha(_linecol,0xff);
	}
	return _linecol;
}

BOOL CRichGrid_ValueSetItem::OnDrawItemValue(CDC& dc, CRect rcValue0)
{
	Gdiplus::Graphics graph(dc.m_hDC);

	GraphicsGraph gg;
	gg.Create(rcValue0.Width(),rcValue0.Height());

	i_math::recti rcValue;
	memcpy(&rcValue,&rcValue0,sizeof(rcValue));
	rcValue.zeroBase();

	gg.FillSolidRect(rcValue,0xffffffff);

	if (_vs)
	{
		if (_vs->GetKeyType()==KT_Float)
		{
			DWORD col=GetLineCol();
			col&=0xffffff;
			col=COLOR_SWAP_RB(col);
			i_math::recti rc=rcValue;
			rc.Top()=rc.Top()+4;
			rc.Bottom()=rc.Bottom()-4;
			gg.FillSolidRect(rc,col);
		}
		if (_vs->GetKeyType()==KT_Color)
		{
			extern void DrawColorBand(std::vector<int>&keys,std::vector<DWORD>cols,i_math::recti &rc0,GraphicsGraph *gg);
			std::vector<int>keys;
			std::vector<DWORD>cols;

			AnimTick tRange=_vs->GetRange();
			keys.resize(_vs->GetKeyCount());
			cols.resize(_vs->GetKeyCount());

			for (int i=0;i<_vs->GetKeyCount();i++)
			{
				Key_col *k=(Key_col *)_vs->GetKey(i);
				if (tRange>0)
					keys[i]=k->t*rcValue.getWidth()/tRange;
				else
					keys[i]=0;
				cols[i]=k->color;
			}

			DrawColorBand(keys,cols,rcValue,&gg);
		}

	}

//	gg.DrawFrameRect(rcValue,0,1);
	graph.DrawImage(gg.GetBg(), Gdiplus::Point(rcValue0.left,rcValue0.top));


	return CXTPPropertyGridItemBool::OnDrawItemValue(dc,rcValue0);
}

i_math::rectf CRichGrid_ValueSetItem::GetRangeRect()
{
	i_math::rectf rc=_rcLimit;
	if (rc.Right()<=rc.Left())
		rc.Right()=rc.Left()+10.0f;//无限大的话,我们给一个合理的值
	if (rc.Bottom()<=rc.Top())
		rc.Bottom()=rc.Top()+10.0f;//无限大的话,我们给一个合理的值
	return rc;
}

i_math::rectf CRichGrid_ValueSetItem::GetLimitRect()
{
	i_math::rectf rc=_rcLimit;
	if (rc.Right()<=rc.Left())
		rc.Right()=10000000000.0f;//无限大的话,我们给一个很大的值
	if (rc.Bottom()<=rc.Top())
		rc.Bottom()=10000000000.0f;//无限大的话,我们给一个很大的值
	return rc;
}
