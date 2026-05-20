/********************************************************************
	created:	2008/5/22   14:00
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	proto graphic elements
*********************************************************************/

#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>

#include "graphicsgraph.h"

//////////////////////////////////////////////////////////////////////////
//GraphicsGraph

GraphicsGraph::GraphicsGraph()
{
	Zero();
}

GraphicsGraph::~GraphicsGraph()
{
	Destroy();
}

void GraphicsGraph::Zero()
{
	_bmp=NULL;
	_grph=NULL;
	_font=NULL;

	_off.set(0,0);
	_scale.set(1,1);

	_w=_h=0;
	_fontsize=0;

}


void GraphicsGraph::Destroy()
{
	SAFE_DELETE(_font);
	SAFE_DELETE(_bmp);
	SAFE_DELETE(_grph);
	_buf.clear();

	Zero();
}


BOOL GraphicsGraph::Create(DWORD w,DWORD h,int fontsize)
{
	if ((w==_w)&&(h==_h)&&(fontsize==_fontsize))
		return TRUE;//没有变化

	Destroy();

	_buf.resize(w*h*4);
	_bmp=new Gdiplus::Bitmap(w,h,4*w,PixelFormat24bppRGB,_buf.data());

	_w=w;
	_h=h;

	_grph=new Gdiplus::Graphics(_bmp);


	_font=new Gdiplus::Font(L"微软雅黑",(float) fontsize);
	_fontsize=fontsize;

	return TRUE;
}

void GraphicsGraph::ClearBg(DWORD col)
{
	_grph->Clear(ColorRGB(col));
}
void GraphicsGraph::GradientV(i_math::recti &rc,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());
	Gdiplus::LinearGradientBrush br(rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeVertical);

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	_grph->FillRectangle(&br,rcBr);

// 	TRIVERTEX buf[2];
// 	TRIVERTEX_SET(buf[0],rc.Left(),rc.Top(),col1);
// 	TRIVERTEX_SET(buf[1],rc.Right(),rc.Bottom(),col2);
// 
// 	GRADIENT_RECT mesh;
// 	mesh.UpperLeft=0;
// 	mesh.LowerRight=1;
// 	GradientFill(buf,2,&mesh,1,GRADIENT_FILL_RECT_V);
}

void GraphicsGraph::GradientH(i_math::recti &rc,DWORD col1,DWORD col2,DWORD alpha1,DWORD alpha2)
{
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());
	Gdiplus::LinearGradientBrush br(rcBr,ColorRGBA(col1,alpha1),ColorRGBA(col2,alpha2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	_grph->FillRectangle(&br,rcBr);
}




void GraphicsGraph::GradientArrow(i_math::pos2di &pt,DWORD col1,DWORD col2,BOOL bInv)
{
	Gdiplus::Rect rc(pt.x+0,pt.y+-1,13,14);
	CPoint buf[]=
	{
		CPoint(pt.x+0,pt.y+3),
		CPoint(pt.x+6,pt.y+3),
		CPoint(pt.x+6,pt.y+-1),
		CPoint(pt.x+13,pt.y+6),
		CPoint(pt.x+6,pt.y+13),
		CPoint(pt.x+6,pt.y+10),
		CPoint(pt.x+0,pt.y+10),
		CPoint(pt.x+0,pt.y+3),
	};
	if (bInv)
	{
		for (int i=0;i<ARRAY_SIZE(buf);i++)
			buf[i].x=pt.x+pt.x+14-buf[i].x;
	}

	Gdiplus::LinearGradientBrush br(rc,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}

void GraphicsGraph::GradientCheck(i_math::pos2di &pt,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rc(pt.x+0,pt.y+-1,13,14);
	CPoint buf[]=
	{
		CPoint(pt.x+2,pt.y+4),
		CPoint(pt.x+0,pt.y+6),
		CPoint(pt.x+4,pt.y+10),
		CPoint(pt.x+12,pt.y+2),
		CPoint(pt.x+10,pt.y+0),
		CPoint(pt.x+4,pt.y+6),
		CPoint(pt.x+2,pt.y+4),
	};
	for (int i=0;i<ARRAY_SIZE(buf);i++)
	{
		buf[i].x=buf[i].x+3;
		buf[i].y=buf[i].y+3;
	}

// 	if (bInv)
// 	{
// 		for (int i=0;i<ARRAY_SIZE(buf);i++)
// 			buf[i].x=pt.x+pt.x+14-buf[i].x;
// 	}

	Gdiplus::LinearGradientBrush br(rc,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}

void GraphicsGraph::GradientCross(i_math::pos2di &pt,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rc(pt.x+0,pt.y+-1,13,16);
	CPoint buf[]=
	{
		CPoint(pt.x+2,pt.y+4),
		CPoint(pt.x+0,pt.y+6),
		CPoint(pt.x+4,pt.y+10),
		CPoint(pt.x+0,pt.y+14),
		CPoint(pt.x+2,pt.y+16),
		CPoint(pt.x+6,pt.y+12),
		CPoint(pt.x+10,pt.y+16),
		CPoint(pt.x+12,pt.y+14),
		CPoint(pt.x+8,pt.y+10),
		CPoint(pt.x+12,pt.y+6),
		CPoint(pt.x+10,pt.y+4),
		CPoint(pt.x+6,pt.y+8),
		CPoint(pt.x+2,pt.y+4),
	};
	for (int i=0;i<ARRAY_SIZE(buf);i++)
	{
		buf[i].x=buf[i].x+2;
		buf[i].y=buf[i].y-2;
	}

	// 	if (bInv)
	// 	{
	// 		for (int i=0;i<ARRAY_SIZE(buf);i++)
	// 			buf[i].x=pt.x+pt.x+14-buf[i].x;
	// 	}

	Gdiplus::LinearGradientBrush br(rc,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}


void GraphicsGraph::GradientPyrimid(i_math::recti &rc,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());
	CPoint buf[]=
	{
		CPoint((rc.Left()+rc.Right())/2,rc.Top()),
		CPoint(rc.Left(),rc.Bottom()),
		CPoint(rc.Right(),rc.Bottom()),
		CPoint((rc.Left()+rc.Right())/2,rc.Top()),
	};

	Gdiplus::LinearGradientBrush br(rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

}

void GraphicsGraph::GradientPyrimidInv(i_math::recti &rc,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());
	CPoint buf[]=
	{
		CPoint((rc.Left()+rc.Right())/2,rc.Bottom()),
		CPoint(rc.Left(),rc.Top()),
		CPoint(rc.Right(),rc.Top()),
		CPoint((rc.Left()+rc.Right())/2,rc.Bottom()),
	};

	Gdiplus::LinearGradientBrush br(rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeHorizontal);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}

void GraphicsGraph::GradientPie(i_math::recti &rc,DWORD col1,DWORD col2,float start,float sweep)
{
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());
	Gdiplus::LinearGradientBrush br((Gdiplus::Rect&)rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeForwardDiagonal);
	_grph->FillPie(&br,rcBr,start,sweep);
}

void GraphicsGraph::FramePie(i_math::recti &rc_,DWORD col,int width,DWORD alpha,BOOL bDash)
{
	Gdiplus::Rect rc(rc_.Left(),rc_.Top(),rc_.getWidth(),rc_.getHeight());
	Gdiplus::Pen pen(Gdiplus::Color(ColorRGBA(col,(BYTE)alpha)),(float)width);
	if (bDash)
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		pen.SetDashCap(Gdiplus::DashCapRound);
	}
	_grph->DrawPie(&pen,rc,0,360);


}

void GraphicsGraph::GradientFlash(i_math::pos2di &pt,DWORD col1,DWORD col2)
{
	Gdiplus::Rect rc(pt.x+0,pt.y+0,13,14);
	CPoint buf[]=
	{
		CPoint(pt.x+5,pt.y+0),
		CPoint(pt.x+11,pt.y+0),
		CPoint(pt.x+8,pt.y+4),
		CPoint(pt.x+13,pt.y+4),
		CPoint(pt.x+6,pt.y+14),
		CPoint(pt.x+7,pt.y+7),
		CPoint(pt.x+2,pt.y+7),
	};

	Gdiplus::LinearGradientBrush br(rc,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeVertical);

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));

	Gdiplus::Pen pen(Gdiplus::Color(0,0,0),1);
	_grph->DrawPolygon(&pen,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}


void GraphicsGraph::_DrawConnect(i_math::pos2di *cps,DWORD col,DWORD flag)
{
	_grph->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	Gdiplus::Pen pen(Gdiplus::Color(GetRValue(col),GetGValue(col),GetBValue(col)),2);

	if (flag&DRAWCONNECT_DASH)
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		pen.SetDashCap(Gdiplus::DashCapRound);
	}

	float w=6,h=4;
	if (_scale.x<1.0f)
	{
		w*=_scale.x;
		h*=_scale.x;
	}
	Gdiplus::AdjustableArrowCap arr(w,h);

	if (!(flag&DRAWCONNECT_NOCAP))
		pen.SetCustomEndCap(&arr);
	if (flag&DRAWCONNECT_BIDIRECTIONAL)
		pen.SetCustomStartCap(&arr);

	_grph->DrawBeziers(&pen,(const Gdiplus::Point*)cps,4);
}


void GraphicsGraph::DrawConnectH(i_math::pos2di &ptSrc,i_math::pos2di &ptDest,DWORD col,DWORD flag)
{
	i_math::pos2di cps[4];
	int ext=80;
	if (flag&DRAWCONNECT_INV)
		ext=-ext;

	cps[1]=cps[0]=ptSrc;
	cps[1].x+=ext;
	cps[3]=cps[2]=ptDest;
	cps[2].x-=ext;

	_DrawConnect(cps,col,flag);
}

void GraphicsGraph::DrawConnectV(i_math::pos2di &ptSrc,i_math::pos2di &ptDest,DWORD col,DWORD flag)
{
	i_math::pos2di cps[4];
	int ext=40;
	if (flag&DRAWCONNECT_INV)
		ext=-ext;

	cps[1]=cps[0]=ptSrc;
	cps[1].y-=ext;
	cps[3]=cps[2]=ptDest;
	cps[2].y+=ext;

	_DrawConnect(cps,col,flag);
}


void GraphicsGraph::DrawConnect(i_math::pos2di &ptSrc,i_math::pos2di &ptSrcCtrl,i_math::pos2di &ptDest,i_math::pos2di &ptDestCtrl,DWORD col,DWORD flag)
{
	i_math::pos2di cps[4];
	cps[0]=ptSrc;
	cps[1]=ptSrcCtrl;
	cps[2]=ptDest;
	cps[3]=ptDestCtrl;
	_DrawConnect(cps,col,flag);
}


void GraphicsGraph::DrawRoundRect(i_math::recti &rc_,DWORD col1,DWORD col2)
{
	i_math::recti rc=rc_;
	Gdiplus::GraphicsPath gp;

	Gdiplus::REAL radius=((float)rc.getHeight()-1.0f)/2.0f;
//	gp.StartFigure();
	gp.AddLine(rc.Left(),rc.Top(),rc.Right()-1,rc.Top());
	gp.AddArc((float)rc.Right()-1-radius,(float)rc.Top(),radius*2,radius*2,(float)-90,(float)180);
	gp.AddLine(rc.Right(),rc.Bottom()-1,rc.Left(),rc.Bottom()-1);
	gp.AddArc((float)rc.Left()-radius,(float)rc.Top(),radius*2,radius*2,(float)90,(float)180);

	rc.inflate(rc.getHeight()/2,0,rc.getHeight()/2,0);
	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());

	Gdiplus::LinearGradientBrush br(rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeVertical);

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	_grph->FillPath(&br,&gp);
}


void GraphicsGraph::DrawRoundRectF(i_math::rectf &rc_,DWORD col1,DWORD col2)
{
	i_math::rectf rc=rc_;
	Gdiplus::GraphicsPath gp;

	Gdiplus::REAL radius=((float)rc.getHeight()-1.0f)/2.0f;
	//	gp.StartFigure();
	gp.AddLine(rc.Left(),rc.Top(),rc.Right()-1,rc.Top());
	gp.AddArc((float)rc.Right()-1-radius,(float)rc.Top(),radius*2,radius*2,(float)-90,(float)180);
	gp.AddLine(rc.Right(),rc.Bottom()-1,rc.Left(),rc.Bottom()-1);
	gp.AddArc((float)rc.Left()-radius,(float)rc.Top(),radius*2,radius*2,(float)90,(float)180);

	rc.inflate(rc.getHeight()/2,0,rc.getHeight()/2,0);
	Gdiplus::RectF rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());

	Gdiplus::LinearGradientBrush br(rcBr,ColorRGB(col1),ColorRGB(col2),
		Gdiplus::LinearGradientModeVertical);

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	_grph->FillPath(&br,&gp);
}

void MakeRoundCornerPath(i_math::recti& rc, Gdiplus::REAL r, Gdiplus::GraphicsPath& gp)
{
	gp.AddLine((float)rc.Left()+r,(float)rc.Top(),(float)rc.Right()-r-1,(float)rc.Top());
	gp.AddArc((float)rc.Right()-1-2*r,(float)rc.Top(),r*2,r*2,(float)-90,(float)90);
	gp.AddLine((float)rc.Right()-1,(float)rc.Top()+r,(float)rc.Right()-1,(float)rc.Bottom()-1-r);
	gp.AddArc((float)rc.Right()-1-2*r,(float)rc.Bottom()-1-2*r,r*2,r*2,(float)0,(float)90);
	gp.AddLine((float)rc.Right()-1-r,(float)rc.Bottom()-1,(float)rc.Left()+r,(float)rc.Bottom()-1);
	gp.AddArc((float)rc.Left(),(float)rc.Bottom()-1-2*r,r*2,r*2,(float)90,(float)90);
	gp.AddLine((float)rc.Left(),(float)rc.Bottom()-1-2*r,(float)rc.Left(),(float)rc.Top()+2*r);
	gp.AddArc((float)rc.Left(),(float)rc.Top(),r*2,r*2,(float)180,(float)90);
}

void GraphicsGraph::DrawRoundCornerRect(i_math::recti &rc_,int radiusCorner,DWORD col1,DWORD col2,DWORD alpha)
{
	i_math::recti rc=rc_;
	Gdiplus::GraphicsPath gp;

	Gdiplus::REAL r=(Gdiplus::REAL)radiusCorner;
	MakeRoundCornerPath(rc,r,gp);

	Gdiplus::Rect rcBr(rc.Left(),rc.Top(),rc.getWidth(),rc.getHeight());

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	Gdiplus::LinearGradientBrush br(rcBr, ColorRGBA(col1, (BYTE)alpha), ColorRGBA(col2, (BYTE)alpha),
		Gdiplus::LinearGradientModeVertical);
	_grph->FillPath(&br,&gp);
}

void GraphicsGraph::FrameRoundCornerRect(i_math::recti &rc_,int radiusCorner,DWORD col,int width,DWORD alpha,BOOL bDash)
{
	i_math::recti rc=rc_;
	Gdiplus::GraphicsPath gp;

	Gdiplus::REAL r=(Gdiplus::REAL)radiusCorner;
	MakeRoundCornerPath(rc,r,gp);

	_grph->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	Gdiplus::Pen pen(Gdiplus::Color(ColorRGBA(col,(BYTE)alpha)),(float)width);
	if (bDash)
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		pen.SetDashCap(Gdiplus::DashCapRound);
	}
	_grph->DrawPath(&pen,&gp);
}


i_math::size2di GraphicsGraph::MessureText(const char *str)
{
	return MessureText(str,10000);
}

i_math::size2di GraphicsGraph::MessureText(const char *str,DWORD w)
{
	std::vector<WORD>buf;
	DWORD len=strlen(str);
	buf.resize(len);
	DWORD nWideCodes=MultiByteToWideChar(936,0,str,len,(WCHAR*)buf.data(),len*2);
	buf.resize(nWideCodes);

	Gdiplus::SizeF sz;
	_grph->MeasureString((WCHAR*)buf.data(),nWideCodes,_font,Gdiplus::SizeF((float)w,10000.0f),&Gdiplus::StringFormat(),&sz);

	return i_math::size2di((int)sz.Width,(int)sz.Height);

}


void GraphicsGraph::DrawText(const char *str,i_math::rectf &rc,DWORD align,BOOL bYInverse/* = FALSE*/)
{
	std::vector<WORD>buf;
	DWORD len=strlen(str);
	buf.resize(len);
	DWORD nWideCodes=MultiByteToWideChar(936,0,str,len,(WCHAR*)buf.data(),len*2);
	buf.resize(nWideCodes);

	Gdiplus::SolidBrush br(Gdiplus::Color(0,0,0));
	Gdiplus::StringFormat fmt;
	fmt.SetAlignment((Gdiplus::StringAlignment)align);

	i_math::rectf rcAdj;
	rcAdj.UpperLeftCorner.x = rc.UpperLeftCorner.x;
	rcAdj.UpperLeftCorner.y = - rc.LowerRightCorner.y;
	rcAdj.LowerRightCorner.x = rc.LowerRightCorner.x;
	rcAdj.LowerRightCorner.y = - rc.UpperLeftCorner.y;

	Gdiplus::RectF rcStr;	

	Gdiplus::Matrix matOld;
	_grph->GetTransform(&matOld);

	if(bYInverse)
	{
		rcStr.X = (float)rcAdj.Left(); rcStr.Y = (float)rcAdj.Top();
		rcStr.Width = (float)rcAdj.getWidth(); rcStr.Height = (float)rcAdj.getHeight();
		_grph->ScaleTransform(1.0f,-1.0f);
	}
	else
	{	
		rcStr.X = (float)rc.Left(); rcStr.Y = (float)rc.Top();
		rcStr.Width = (float)rc.getWidth(); rcStr.Height = (float)rc.getHeight();
	}

	_grph->DrawString((WCHAR*)buf.data(),nWideCodes,_font,rcStr,&fmt,&br);

	_grph->SetTransform(&matOld);

}

void GraphicsGraph::DrawText(const char *str,i_math::recti &rc,DWORD align,BOOL bYInverse,DWORD col)
{
	std::vector<WORD>buf;
	DWORD len=strlen(str);
	buf.resize(len);
	DWORD nWideCodes=MultiByteToWideChar(936,0,str,len,(WCHAR*)buf.data(),len*2);
	buf.resize(nWideCodes);

	Gdiplus::SolidBrush br(ColorRGB(col));
	Gdiplus::StringFormat fmt;
	fmt.SetAlignment((Gdiplus::StringAlignment)align);

	i_math::recti rcAdj;
	rcAdj.UpperLeftCorner.x = rc.UpperLeftCorner.x;
	rcAdj.UpperLeftCorner.y = - rc.LowerRightCorner.y;
	rcAdj.LowerRightCorner.x = rc.LowerRightCorner.x;
	rcAdj.LowerRightCorner.y = - rc.UpperLeftCorner.y;

	Gdiplus::RectF rcStr;	
	
	Gdiplus::Matrix matOld;
	_grph->GetTransform(&matOld);

	if(bYInverse)
	{
		rcStr.X = (float)rcAdj.Left(); rcStr.Y = (float)rcAdj.Top();
		rcStr.Width = (float)rcAdj.getWidth(); rcStr.Height = (float)rcAdj.getHeight();
		_grph->ScaleTransform(1.0f,-1.0f);
	}
	else
	{	
		rcStr.X = (float)rc.Left(); rcStr.Y = (float)rc.Top();
		rcStr.Width = (float)rc.getWidth(); rcStr.Height = (float)rc.getHeight();
	}

	_grph->DrawString((WCHAR*)buf.data(),nWideCodes,_font,rcStr,&fmt,&br);
		
	_grph->SetTransform(&matOld);

}

void GraphicsGraph::FillSolidRect(i_math::recti &rc_,DWORD col,DWORD alpha)
{
	Gdiplus::SolidBrush br(ColorRGBA(col,(BYTE)alpha));
	Gdiplus::Rect rc(rc_.Left(),rc_.Top(),rc_.getWidth(),rc_.getHeight());
	_grph->FillRectangle(&br,(const Gdiplus::Rect&)rc);
}

void GraphicsGraph::FillSolidRectF( i_math::rectf &rc_, DWORD col, DWORD alpha )
{
	Gdiplus::SolidBrush br( ColorRGBA( col, (BYTE)alpha ) );
	Gdiplus::RectF rc( rc_.Left(), rc_.Top(), rc_.getWidth(), rc_.getHeight() );
	_grph->FillRectangle( &br, (const Gdiplus::RectF&)rc );
}

void GraphicsGraph::FillHatchRect(i_math::recti &rc_,DWORD colFg,DWORD colBg,int style,DWORD alpha)
{
	Gdiplus::HatchBrush br((Gdiplus::HatchStyle)style,ColorRGBA(colFg,alpha),ColorRGBA(colBg,alpha));
	Gdiplus::Rect rc(rc_.Left(),rc_.Top(),rc_.getWidth(),rc_.getHeight());
	_grph->FillRectangle(&br,(const Gdiplus::Rect&)rc);
}


void GraphicsGraph::DrawImageData(i_math::rectf &rc_,DWORD*data,DWORD w,DWORD h)
{
	Gdiplus::Bitmap bmp(w,h,w*4, PixelFormat32bppARGB,(BYTE*)data);

	Gdiplus::RectF rc( rc_.Left(), rc_.Top(), rc_.getWidth(), rc_.getHeight() );

	_grph->DrawImage(&bmp,rc);


}


void GraphicsGraph::DrawMore(i_math::pos2di &pt)
{
	CPoint buf[]=
	{
		CPoint(pt.x+1,pt.y+1),
		CPoint(pt.x+9,pt.y+1),
		CPoint(pt.x+5,pt.y+5),
	};

	Gdiplus::SolidBrush br(Gdiplus::Color(0,0,0));

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
}

void GraphicsGraph::DrawMore(i_math::pos2df &pt)
{
	Gdiplus::PointF buf[]=
	{
		Gdiplus::PointF( pt.x+1, pt.y+1 ),
		Gdiplus::PointF( pt.x+9, pt.y+1 ),
		Gdiplus::PointF( pt.x+5, pt.y+5 ),	
	};

	Gdiplus::SolidBrush br(Gdiplus::Color(0,0,0));
	
	_grph->FillPolygon(&br,(const Gdiplus::PointF*)buf,ARRAY_SIZE(buf));
}


void GraphicsGraph::DrawShrink(i_math::pos2di &pt)
{
	CPoint buf[]=
	{
		CPoint(pt.x+1,pt.y+1),
		CPoint(pt.x+11,pt.y+1),
		CPoint(pt.x+6,pt.y+6),
	};
	CPoint buf2[]=
	{
		CPoint(pt.x+1,pt.y+7),
		CPoint(pt.x+11,pt.y+7),
		CPoint(pt.x+11,pt.y+8),
		CPoint(pt.x+1,pt.y+8),
	};
	Gdiplus::SolidBrush br(Gdiplus::Color(0,0,0));

	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf,ARRAY_SIZE(buf));
	_grph->FillPolygon(&br,(const Gdiplus::Point*)buf2,ARRAY_SIZE(buf2));
}
void GraphicsGraph::DrawFrameRect(i_math::recti &rc,DWORD col,int width,DWORD alpha,BOOL bDash)
{
	Gdiplus::Point p0(rc.UpperLeftCorner.x,rc.UpperLeftCorner.y);
	Gdiplus::Point p1(rc.LowerRightCorner.x,rc.UpperLeftCorner.y);
	Gdiplus::Point p2(rc.LowerRightCorner.x,rc.LowerRightCorner.y);
	Gdiplus::Point p3(rc.UpperLeftCorner.x,rc.LowerRightCorner.y);

	Gdiplus::Point points[5] = {p0,p1,p2,p3,p0};
	
	Gdiplus::Pen pen(Gdiplus::Color(ColorRGBA(col,(BYTE)alpha)),(float)width);

	if (bDash)
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		pen.SetDashCap(Gdiplus::DashCapRound);
	}

	_grph->DrawPolygon(&pen,points,5);
}

void GraphicsGraph::FillTri(i_math::pos2di * pos,DWORD col,DWORD alpha/* = 255*/)
{
	Gdiplus::Point p0(pos[0].x,pos[0].y);
	Gdiplus::Point p1(pos[1].x,pos[1].y);
	Gdiplus::Point p2(pos[2].x,pos[2].y);

	Gdiplus::Point points[4] = {p0,p1,p2,p0};
	
	Gdiplus::SolidBrush br(ColorRGBA(col,(BYTE)alpha));
	_grph->FillPolygon(&br,points,4);
} 

void GraphicsGraph::FillTriF(i_math::pos2df * pos,DWORD col,DWORD alpha)
{
	Gdiplus::PointF p0(pos[0].x,pos[0].y);
	Gdiplus::PointF p1(pos[1].x,pos[1].y);
	Gdiplus::PointF p2(pos[2].x,pos[2].y);

	Gdiplus::PointF points[4] = {p0,p1,p2,p0};

	Gdiplus::SolidBrush br(ColorRGBA(col,(BYTE)alpha));
	_grph->FillPolygon(&br,points,4);
}


void GraphicsGraph::FillCircle(i_math::pos2di& center,int r,DWORD col0,DWORD col1,DWORD alpha/* = 255*/)
{
	Gdiplus::Rect rc(center.x-r,center.y - r,2*r,2*r);
	Gdiplus::LinearGradientBrush br(rc,ColorRGBA(col0,alpha),ColorRGBA(col1,alpha), Gdiplus::LinearGradientModeVertical);
	_grph->FillEllipse(&br,rc);
}

void GraphicsGraph::FrameCircle(i_math::pos2di &center,int r,DWORD col,int width,DWORD alpha,BOOL bDash)
{
	Gdiplus::Rect rc(center.x-r,center.y - r,2*r,2*r);
	Gdiplus::Pen pen(Gdiplus::Color(ColorRGBA(col,(BYTE)alpha)),(float)width);
	if (bDash)
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot);
		pen.SetDashCap(Gdiplus::DashCapRound);
	}
	_grph->DrawEllipse(&pen,rc);
}


void GraphicsGraph::Transform(i_math::pos2df &off,i_math::pos2df &scale0,BOOL bYInverse/* = FALSE*/)
{
	i_math::pos2df scale=scale0;
	if(bYInverse)
		scale.y=-scale.y;

	Transform(off,scale);
}

void GraphicsGraph::GetTranform(i_math::pos2df &off,i_math::pos2df &scale)
{
	off=_off;
	scale=_scale;
}

void GraphicsGraph::Transform(i_math::pos2df &off,i_math::pos2df &scale)
{
	_grph->ResetTransform();

	_off=off;
	_scale=scale;
	_grph->ScaleTransform(_scale.x,_scale.y);
	_grph->TranslateTransform(_off.x,_off.y, Gdiplus::MatrixOrderAppend);
}



// add by yuyang for gg

void GraphicsGraph::DrawLine( DWORD crColor, float fWidth, i_math::pos2di &ptFrom, i_math::pos2di &ptTo )
{
	Gdiplus::Pen pen( crColor, fWidth );

	Gdiplus::Point pt1( ptFrom.x, ptFrom.y );
	Gdiplus::Point pt2( ptTo.x, ptTo.y );
	_grph->DrawLine( &pen, pt1, pt2 );
}

void GraphicsGraph::DrawLine( DWORD crColor, float fWidth, i_math::pos2df &ptFrom, i_math::pos2df &ptTo )
{
	Gdiplus::Pen pen( crColor, fWidth );
	Gdiplus::PointF pt1( ptFrom.x, ptFrom.y );
	Gdiplus::PointF pt2( ptTo.x, ptTo.y );
	_grph->DrawLine( &pen, pt1, pt2 );
}

void GraphicsGraph::SetSmoothingMode(Gdiplus::SmoothingMode mode )
{
	_grph->SetSmoothingMode( mode );
}

void GraphicsGraph::ResetTransform()
{
	_grph->ResetTransform();
}

void GraphicsGraph::DrawConnectLine( i_math::pos2df &ptSrc, i_math::pos2df &ptDest, DWORD col, float width, DWORD flag )
{
	_grph->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias );

	Gdiplus::Pen pen( Gdiplus::Color( GetRValue( col ), GetGValue( col ), GetBValue( col ) ), width );

	if ( flag & DRAWCONNECT_DASH )
	{
		pen.SetDashStyle(Gdiplus::DashStyleDot );
		pen.SetDashCap(Gdiplus::DashCapRound );
	}

	float w = 2,h = 2;
	if ( _scale.x < 1.0f )
	{
		w *= _scale.x;
		h *= _scale.x;
	}
	Gdiplus::AdjustableArrowCap arr( w, h );

	if (!(flag&DRAWCONNECT_NOCAP))
		pen.SetCustomEndCap( &arr );
	if ( flag & DRAWCONNECT_BIDIRECTIONAL )
		pen.SetCustomStartCap( &arr );

	Gdiplus::PointF pt1( ptSrc.x, ptSrc.y );
	Gdiplus::PointF pt2( ptDest.x, ptDest.y );
	_grph->DrawLine( &pen, pt1, pt2 );

}
// end