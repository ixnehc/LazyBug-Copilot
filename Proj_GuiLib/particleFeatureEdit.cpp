
#include "stdh.h"
#include "particleFeatureEdit.h"


CParticleFeatureEdit::CParticleFeatureEdit()
{
	_fScale = ScaleDefault;
	_bChoose = false;
	_bChangeScale = false;
	_op = OP_NONE;
	LOGBRUSH br;
	br.lbStyle = BS_HOLLOW;
	br.lbColor = 0;
	br.lbHatch = 0;
	_brBackGround.CreateBrushIndirect( &br );
	_penGrid.CreatePen( PS_SOLID, 1, RGB( 240, 0, 0 ) );
	_penBrokenLine.CreatePen( PS_SOLID, 2, RGB( 0, 0, 0 ) );

	_ptView.x = 0;
	_ptView.y = 0;
}

CParticleFeatureEdit::~CParticleFeatureEdit()
{

}

BEGIN_MESSAGE_MAP( CParticleFeatureEdit, CWnd )
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CParticleFeatureEdit::Draw( CDC* pDC )
{	
	_DrawEditBackground( pDC, _bChangeScale );
	_Zoom( pDC, _fScale );
}

void CParticleFeatureEdit::OnRButtonDown( UINT nFlags, CPoint point )
{
	GetCursorPos(&_ptRight);
	if ( _fScale - ScaleDefault > FLT_EPSILON )
	{
		_op = OP_BACKGROUND;
	}
}

void CParticleFeatureEdit::OnRButtonUp( UINT nFlags, CPoint point )
{
	if ( OP_BACKGROUND == _op )
	{
		_op = OP_NONE;
		_ptView.x = _ptOffset.x;
		_ptView.y = _ptOffset.y;
	}
	
}

void CParticleFeatureEdit::OnLButtonDown( UINT nFlags, CPoint point )
{

}

void CParticleFeatureEdit::OnLButtonUp( UINT nFlags, CPoint point )
{

}

// mousemove 只能计算出偏移的位置 所以绘制那里应该使用一个mousemove这里计算的位置
void CParticleFeatureEdit::OnMouseMove( UINT nFlags, CPoint point )
{
	bool bBreak = true;
	RECT rc;
	GetClientRect( &rc );
	if ( OP_BACKGROUND == _op )
	{
		POINT ptCurrent;
		GetCursorPos( &ptCurrent );

		_ptOffset.x = ptCurrent.x - _ptRight.x;
		_ptOffset.y = ptCurrent.y - _ptRight.y;
		InvalidateRect( NULL, false );
	}
}

void CParticleFeatureEdit::_DrawEditBackground( CDC* pDC, bool changeScale )
{	
	CRect rc;
	GetClientRect(&rc);
	
	
	int nScaleW = (int)( _fScale * rc.Width() );
	int nScaleH = (int)( _fScale * rc.Height() );

	if ( _dc.GetSafeHdc() == NULL )
	{
		_dc.CreateCompatibleDC( pDC );
		_bmp.CreateCompatibleBitmap( pDC, nScaleW, nScaleH );
		_dc.SelectObject( &_bmp );
	}
	if ( changeScale )
	{
		_bmp.DeleteObject();
		_bmp.CreateCompatibleBitmap( pDC, nScaleW, nScaleH );
		_dc.SelectObject( &_bmp );
		_bChangeScale = false;
	}

	_dc.FillSolidRect( rc.left, rc.top, nScaleW, nScaleH, RGB( 255, 255, 255 ) );

	CBrush *OleBrush = _dc.SelectObject( &_brBackGround );
	CPen *OldPen = _dc.SelectObject( &_penGrid );

	POINT pStart;
	POINT pEnd;
	for ( int i = 0; i < GRID_NUM ; ++i )
	{
		pStart.x = i * nScaleW / GRID_NUM;
		pStart.y = 0;
		_dc.MoveTo( pStart );
		pEnd.x = pStart.x;
		pEnd.y = nScaleH;
		_dc.LineTo( pEnd );
	}
	
	for ( int i = 0; i < GRID_NUM; ++i )
	{
		pStart.x = 0;
		pStart.y = i * nScaleH / GRID_NUM;
		_dc.MoveTo( pStart );
		pEnd.x = nScaleW;
		pEnd.y = pStart.y;
		_dc.LineTo( pEnd );
	}

	_dc.SelectObject( OleBrush );
	_dc.SelectObject( OldPen );
}


void CParticleFeatureEdit::_Zoom( CDC *pDC,float scale )
{	
	CRect rc;
	GetClientRect(&rc);
	
	int width = rc.Width();
	int height = rc.Height();
	int ScaleWidth = (int)( width * _fScale );
	int ScaleHeight = (int)( height * _fScale );

	int stretchBliOld = SetStretchBltMode( pDC->GetSafeHdc(), HALFTONE );
	_ptView.x = _ptOffset.x;
	_ptView.y = _ptOffset.y;
	if ( _ptView.x < 0)
	{
		_ptView.x = 0;
	}
	if ( _ptView.x > ScaleWidth - width )
	{
		_ptView.x = ScaleWidth - width;
	}
	if ( _ptView.y < 0 )
	{
		_ptView.y = 0;
	}
	if ( _ptView.y > ScaleHeight - height )
	{
		_ptView.y = ScaleHeight - height;
	}
	
	StretchBlt( pDC->GetSafeHdc(), 0, 0, width, height, 
		_dc, _ptView.x, _ptView.y, width, height, SRCCOPY );

	SetStretchBltMode( pDC->GetSafeHdc(), stretchBliOld );
}