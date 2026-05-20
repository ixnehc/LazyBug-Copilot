/********************************************************************
	created:	2007/2/6   12:56
	filename: 	e:\IxEngine\Proj_GuiLib\TexWnd.cpp
	author:		cxi
	
	purpose:	a wnd to display a texture
*********************************************************************/

#include "stdh.h"
#include ".\GuiLib.h"

#include <vector>
#include <string>


#include "interface/interface.h"

#include "RenderSystem/IRenderSystem.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "resource.h"


#include "ximage.h"
#include "CxImageWnd.h"

#include "ImageBase.h"

void CxImageWnd::SetBlank()
{
	ImageFromBMP(IDB_IMAGEBLANKMARK,_image);
}

void CxImageWnd::SetError()
{
	ImageFromBMP(IDB_IMAGEERRORMARK,_image);
}

void CxImageWnd::SetImage(const char * namefile)
{
	_image->Load(fromMBCS(namefile));
	Invalidate();
}
CxImageWnd::CxImageWnd()
{
	_image=new CxImage(CXIMAGE_FORMAT_BMP);
	SetBlank();
}

CxImageWnd::~CxImageWnd()
{
	SAFE_DELETE(_image);
}


BEGIN_MESSAGE_MAP(CxImageWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()

void CxImageWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CWnd::OnPaint() for painting messages


	CxImage * image = _image;

	i_math::recti rcImage,rcClient;
	GetClientRect((LPRECT)&rcClient);

	dc.FillSolidRect((LPRECT)&rcClient,RGB(192,192,192));//static control好像不会刷新背景,所以我们在这里手工重画一下背景
 
//	rcClient.getCenter();
	rcImage=rcClient.arrangeCenter(image->GetWidth(),image->GetHeight());

	image->Draw(dc.m_hDC,(const RECT &)rcImage);
}

