/********************************************************************
	created:	2006/9/6   11:29
	filename: 	e:\IxEngine\Proj_GuiLib\CommonCtrlBase.cpp
	author:		cxi
	
	purpose:	useful functions for CxImage
*********************************************************************/
#include "stdh.h"

#include "ImageBase.h"
#include "ximage.h"

#include "RenderSystem/IUtilRS.h"

#include "RenderSystem/IRenderSystem.h"


#include "RenderSystem/ITexture.h"


#include <vector>
#include <string>

#include "stringparser/stringparser.h"
#include "resdata/TexData.h"


CxImage *ImageFromTexData(IRenderSystem*pRS,TexData *td,CxImage *container)
{
	CxImage *ret=NULL;

	ITexture *tex=pRS->GetWTexMgr2()->Create(*td);
	if (!tex)
		return NULL;
	TexData *tdTga;
	if (tex->DumpTga(tdTga))
	{
		if (!container)
			ret=new CxImage;
		else
			ret=container;

		if (FALSE==ret->Decode((BYTE*)&tdTga->data[0],tdTga->data.size(),CXIMAGE_FORMAT_TGA))
		{
			if (ret!=container)
			{
				SAFE_DELETE(ret);
			}
			else
				ret=NULL;
		}
	}
	SAFE_RELEASE(tex);

	return ret;
}

CxImage *ImageFromBMP(UINT idBitmap,CxImage *container)
{
	HINSTANCE hInst= AfxFindResourceHandle(MAKEINTRESOURCE(idBitmap), RT_BITMAP);
	HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(idBitmap), RT_BITMAP);

	if (!container)
		container=new CxImage;

	container->LoadResource(hResource,CXIMAGE_FORMAT_BMP,hInst);

	return container;
}

CxImage *ImageFromTGA(UINT idTga,CxImage *container)
{
	HINSTANCE hInst = AfxFindResourceHandle(MAKEINTRESOURCE(idTga), _T("TGA"));
	HRSRC hResource = ::FindResource(hInst, MAKEINTRESOURCE(idTga), _T("TGA"));

	if (!container)
		container=new CxImage;

	container->LoadResource(hResource,CXIMAGE_FORMAT_TGA,hInst);

	return container;
}

CxImage *ImageFromTex(ITexture*tex,CxImage *container)
{
	CxImage *t=container;
	if (!t)
		t=new CxImage;

	TexData *td;
	if (tex->DumpTga(td))
	{
		if (t->Decode((BYTE*)&td->data[0],td->data.size(),CXIMAGE_FORMAT_TGA))
			return t;
	}
	if (!container)
		delete t;
	return NULL;
}


void SaveImage(DWORD *col,DWORD w,DWORD h,const char *fn)
{
	CxImage img;
	img.Create(w,h,24);

	DWORD *p=col;
	for (int j=0;j<h;j++)
	for (int i=0;i<w;i++)
	{
		img.SetPixelColor(i,h-1-j,*p);
		p++;
	}

	img.Save(fromMBCS(fn), CXIMAGE_FORMAT_BMP);
}

DWORD LoadImage(DWORD *col,DWORD &w,DWORD &h,const char *fn)
{
	CxImage img;

	img.Load(fromMBCS(fn),CXIMAGE_FORMAT_BMP);
	w=img.GetWidth();
	h=img.GetHeight();
	if (col)
	{
		DWORD *p=col;
		for (int j=0;j<h;j++)
		for (int i=0;i<w;i++)
		{
			*p=*(DWORD*)&img.GetPixelColor(i,h-1-j);
			p++;
		}
	}
	return w*h*sizeof(DWORD);

}

DWORD LoadImageTga(DWORD *col,DWORD &w,DWORD &h,const char *fn)
{
	CxImage img;

	img.Load(fromMBCS(fn),CXIMAGE_FORMAT_TGA);
	w=img.GetWidth();
	h=img.GetHeight();
	if (col)
	{
		DWORD *p=col;
		for (int j=0;j<h;j++)
			for (int i=0;i<w;i++)
			{
				*p=*(DWORD*)&img.GetPixelColor(i,h-1-j);
				p++;
			}
	}
	return w*h*sizeof(DWORD);

}


void SaveAliasePattern(DWORD base)
{
	CxImage imgWork;
	imgWork.Create(base*3,base*3,24);

	for (int j=0;j<16;j++)
	for (int i=0;i<32;i++)
	{
		DWORD idx=j*32+i;

		//Build a rough bitmap
		for (int k=0;k<9;k++)
		{
			i_math::pos2di pt;
			pt.x=k%3;
			pt.y=k/3;

			i_math::recti rc;
			rc.set(pt.x*base,pt.y*base,pt.x*base+base,pt.y*base+base);

			DWORD v;
			if ((1<<k)&idx)
				memset(&v,0xff,sizeof(v));
			else
				memset(&v,0x0,sizeof(v));

			for (int ii=rc.Left();ii<rc.Right();ii++)
			for (int jj=rc.Top();jj<rc.Bottom();jj++)
				imgWork.SetPixelColor(ii,imgWork.GetHeight()-1-jj,v);
		}

		std::string path;
		FormatString(path,"E:\\temp\\pttn_a\\%02d_%02d.bmp",i,j);

		imgWork.Save(fromMBCS(path.c_str()),CXIMAGE_FORMAT_BMP);

	}
}



GuiLib_Api void LoadAliasePattern(DWORD base,const char *fn)
{
	CxImage img;
	img.Create(base*16*2,base*16,24);

	CxImage imgWork;
	imgWork.Create(base*3,base*3,24);

	for (int j=0;j<16;j++)
	for (int i=0;i<32;i++)
	{
		std::string path;
		FormatString(path,"E:\\temp\\pttn_b\\%02d_%02d.bmp",i,j);

		imgWork.Load(fromMBCS(path.c_str()),CXIMAGE_FORMAT_BMP);

		DWORD idx=j*32+i;
		DWORD x=0,y=0;
		if (TRUE)//arrange it
		{
			if (idx&2)
				x+=16;
			if (idx&8)
				x+=8;
			if (idx&16)
				x+=4;
			if (!(idx&1))
				x+=2;
			if (!(idx&4))
				x+=1;

			if (idx&32)
				y+=8;
			if (idx&128)
				y+=4;
			if (!(idx&64))
				y+=2;
			if (!(idx&256))
				y+=1;
		}

		i_math::pos2di ptSrc,ptDest;
		ptDest.set(x*base,y*base);
		ptSrc.set(base,base);//the center part

		for (int ii=0;ii<base;ii++)
		for (int jj=0;jj<base;jj++)
		{
			RGBQUAD v;
			v=imgWork.GetPixelColor(ptSrc.x+ii,imgWork.GetHeight()-1-(ptSrc.y+jj),false);
			img.SetPixelColor(ptDest.x+ii,img.GetHeight()-1-(ptDest.y+jj),FORCE_TYPE(DWORD,v));
		}
	}
	img.Save(fromMBCS(fn),CXIMAGE_FORMAT_BMP);
}
