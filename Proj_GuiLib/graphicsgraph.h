#pragma once

#include <GdiPlus.h>
// using namespace Gdiplus;

#include "GuiLib.h"

#include "class/class.h"

#include <map>

#define DRAWCONNECT_BIDIRECTIONAL 1
#define DRAWCONNECT_DASH 2
#define DRAWCONNECT_INV 4
#define DRAWCONNECT_NOCAP 8

#define ColorRGB(col) (Gdiplus::Color(GetRValue(col),GetGValue(col),GetBValue(col)))
#define ColorRGBA(col,alpha) (Gdiplus::Color((BYTE)(alpha),(BYTE)GetRValue(col),(BYTE)GetGValue(col),(BYTE)GetBValue(col)))


class Gdiplus::Image;
class GuiLib_Api GraphicsGraph
{
public:
	GraphicsGraph();
	~GraphicsGraph();

	void Zero();


	BOOL Create(DWORD w,DWORD h,int fontsize=11);
	void Destroy();

	Gdiplus::Image *GetBg()	{		return _bmp;	}
	DWORD GetWidth()	{		return _w;	}
	DWORD GetHeight()	{		return _h;	}

	void GradientV(i_math::recti &rc,DWORD col1,DWORD col2);
	void GradientH(i_math::recti &rc,DWORD col1,DWORD col2,DWORD alpha1,DWORD alpha2);
	void GradientArrow(i_math::pos2di &pt,DWORD col1,DWORD col2,BOOL bInv=FALSE);
	void GradientCheck(i_math::pos2di &pt,DWORD col1,DWORD col2);
	void GradientCross(i_math::pos2di &pt,DWORD col1,DWORD col2);
	void GradientPie(i_math::recti &rc,DWORD col1,DWORD col2,float start=0.0f,float sweep=360.0f);
	void GradientPyrimid(i_math::recti &rc,DWORD col1,DWORD col2);
	void GradientPyrimidInv(i_math::recti &rc,DWORD col1,DWORD col2);
	void FramePie(i_math::recti &rc,DWORD col,int width,DWORD alpha=255,BOOL bDash=FALSE);
	void GradientFlash(i_math::pos2di &pt,DWORD col1,DWORD col2);
	void DrawConnectH(i_math::pos2di &ptSrc,i_math::pos2di &ptDest,DWORD col,DWORD flag=0);
	void DrawConnectV(i_math::pos2di &ptSrc,i_math::pos2di &ptDest,DWORD col,DWORD flag=0);
	void DrawConnect(i_math::pos2di &ptSrc,i_math::pos2di &ptSrcCtrl,i_math::pos2di &ptDest,i_math::pos2di &ptDestCtrl,DWORD col,DWORD flag=0);

	void DrawRoundRect(i_math::recti &rc,DWORD col1,DWORD col2);
	void DrawRoundRectF(i_math::rectf &rc,DWORD col1,DWORD col2);

	void DrawRoundCornerRect(i_math::recti &rc,int radiusCorner,DWORD col1,DWORD col2,DWORD alpha=255);
	void FrameRoundCornerRect(i_math::recti &rc,int radiusCorner,DWORD col,int width,DWORD alpha=255,BOOL bDash=FALSE);

	void DrawMore(i_math::pos2di &pt);
	void DrawMore(i_math::pos2df &pt);
	void DrawShrink(i_math::pos2di &pt);
	
	// add by yuyang for gg
	void DrawLine( DWORD crColor, float fWidth, i_math::pos2di &ptFrom, i_math::pos2di &ptTo );
	void DrawLine( DWORD crColor, float fWidth, i_math::pos2df &ptFrom, i_math::pos2df &ptTo );
	void SetSmoothingMode( Gdiplus::SmoothingMode mode );
	void ResetTransform();
	void DrawConnectLine( i_math::pos2df &ptSrc, i_math::pos2df &ptDest, DWORD col, float width = 1.0f, DWORD flag = 0 );
	// end
	void DrawText(const char *str,i_math::rectf &rc,DWORD align=DT_LEFT,BOOL bYInverse = FALSE);
	void DrawText(const char *str,i_math::recti &rc,DWORD align=DT_LEFT,BOOL bYInverse = FALSE,DWORD col=0);
	void FillSolidRect(i_math::recti &rc,DWORD col,DWORD alpha=255);
	void FillSolidRectF( i_math::rectf &rc, DWORD col, DWORD alpha=255 );
	void FillHatchRect(i_math::recti &rc,DWORD colFg,DWORD colBg,int style=4,DWORD alpha=255);
	void DrawFrameRect(i_math::recti &rc,DWORD col,int width,DWORD alpha = 255,BOOL bDash=FALSE);
	void FillTri(i_math::pos2di * pos,DWORD col,DWORD alpha = 255);
	void FillTriF(i_math::pos2df * pos,DWORD col,DWORD alpha = 255);
	void FillCircle(i_math::pos2di &center,int r,DWORD col0,DWORD col1,DWORD alpha = 255);
	void FrameCircle(i_math::pos2di &center,int r,DWORD col,int width,DWORD alpha = 255,BOOL bDash=FALSE);
	void ClearBg(DWORD col);

	void DrawImageData(i_math::rectf &rc,DWORD *data,DWORD w,DWORD h);//data里面保存着RGBA(32BIT)的数据
	
	// Image
	Gdiplus::Graphics * GetGraphics(){return _grph;}
	Gdiplus::Bitmap * GetTarget(){return _bmp;}

	i_math::size2di MessureText(const char *str);
	i_math::size2di MessureText(const char *str,DWORD w);

	void Transform(i_math::pos2df &off,i_math::pos2df &scale,BOOL bYInverse);
	void Transform(i_math::pos2df &off,i_math::pos2df &scale);
	void GetTranform(i_math::pos2df &off,i_math::pos2df &scale);


protected:

	void _DrawConnect(i_math::pos2di *cps,DWORD col,DWORD flag);

	DWORD _w,_h;
	DWORD _fontsize;

	Gdiplus::Graphics *_grph;

	Gdiplus::Bitmap *_bmp;
	std::vector<BYTE>_buf;

	Gdiplus::Font *_font;

	i_math::pos2df _off;
	i_math::pos2df _scale;

};

