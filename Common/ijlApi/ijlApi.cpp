/********************************************************************
	created:	2007/2/5   11:05
	filename: 	e:\IxEngine\Common\ijlApi\ijlApi.cpp
	author:		cxi
	
	purpose:	IJL(Intel Jpeg Lib) calling interfaces
*********************************************************************/
#include "stdh.h"
#include "ijlApi.h"

#include "ijl.h"
#include <vector>
#include <string>


BOOL IJL_Decode565(WORD *dest,DWORD &w,DWORD &h,BYTE *src,DWORD szSrc)
{
	static JPEG_CORE_PROPERTIES jcp;
	static std::vector<BYTE> buf24;

	BOOL bRet=FALSE;

	memset(&jcp,0,sizeof(JPEG_CORE_PROPERTIES));
	if(IJL_OK != ijlInit(&jcp))
		return FALSE;

	jcp.JPGBytes = src;

	jcp.JPGSizeBytes = szSrc;
	if(IJL_OK != ijlRead(&jcp,IJL_JBUFF_READPARAMS))
		goto _final;

	switch(jcp.JPGChannels)
	{
	case 1:
		{
			jcp.JPGColor = IJL_G;
			jcp.DIBChannels = 3;
			jcp.DIBColor = IJL_BGR;
		}
		break;
	case 3:
		{
			jcp.JPGColor = IJL_YCBCR;
			jcp.DIBChannels = 3;
			jcp.DIBColor = IJL_BGR;
		}
		break;
	default:
		goto _final;
	}

	jcp.DIBWidth = jcp.JPGWidth;
	jcp.DIBHeight = jcp.JPGHeight;

	jcp.DIBPadBytes = IJL_DIB_PAD_BYTES(jcp.DIBWidth,jcp.DIBChannels);

	if (!dest)
	{
		w=jcp.DIBWidth;
		h=jcp.DIBHeight;
		bRet=TRUE;
		goto _final;
	}

	DWORD pitch24=(jcp.DIBWidth * jcp.DIBChannels + jcp.DIBPadBytes);
	buf24.resize(pitch24*jcp.DIBHeight);

	jcp.DIBBytes = buf24.data();


	if(IJL_OK != ijlRead(&jcp,IJL_JBUFF_READWHOLEIMAGE))
		goto _final;

	if (TRUE)
	{
		//convert 24bit to 16bit
		BYTE *p,*p0;
		p=p0=buf24.data();
		WORD *q=dest;
		for (int j=0;j<jcp.DIBHeight;j++)
		{
			p=p0;
			for (int i=0;i<jcp.DIBWidth;i++)
			{
				*q=((p[2]&0xf8)<<8) | ((p[1]&0xfc)<<3) | ((p[0]&0xf8)>>3) ;
				p+=3;
				q++;
			}
			p0+=pitch24;
		}
	}

	bRet=TRUE;

_final:
	ijlFree(&jcp);
	return bRet;
}


BOOL IJL_Encode565(BYTE *dest,DWORD &szDest,WORD *src,DWORD w,DWORD h,int quality)
{
	static JPEG_CORE_PROPERTIES jcp;
	static std::vector<BYTE>buf24;

	BOOL bRet=FALSE;

	memset(&jcp,0,sizeof(JPEG_CORE_PROPERTIES));
	if(IJL_OK != ijlInit(&jcp))
		return FALSE;

	jcp.DIBWidth=w;
	jcp.DIBHeight=h;
	jcp.DIBChannels=3;
	jcp.DIBColor = IJL_BGR;
	jcp.DIBPadBytes = IJL_DIB_PAD_BYTES(jcp.DIBWidth,jcp.DIBChannels);

	DWORD pitch24=(jcp.DIBWidth * jcp.DIBChannels + jcp.DIBPadBytes);
	buf24.resize(pitch24*jcp.DIBHeight);
	jcp.DIBBytes = buf24.data();

	//convert 16bit to 24bit
	BYTE *p,*p0;
	p=p0=buf24.data();
	WORD *q=src;
	for (int j=0;j<jcp.DIBHeight;j++)
	{
		p=p0;
		for (int i=0;i<jcp.DIBWidth;i++)
		{
			p[0]=((*q)&0x1f)<<3;
			p[1]=((*q)&0x7e0)>>3;
			p[2]=((*q)&0xf800)>>8;
			p+=3;
			q++;
		}
		p0+=pitch24;
	}

	
	//Now write to jpg data
	jcp.JPGBytes=dest;
	jcp.JPGSizeBytes=szDest;
	jcp.JPGChannels=3;
	jcp.JPGColor=IJL_YCBCR;
	jcp.JPGSubsampling=IJL_NONE;
	jcp.JPGWidth=jcp.DIBWidth;
	jcp.JPGHeight=jcp.DIBHeight;
	jcp.jquality=quality;

	if (IJL_OK != ijlWrite(&jcp,IJL_JBUFF_WRITEWHOLEIMAGE))
		goto _final;

	szDest=jcp.JPGSizeBytes;
	bRet=TRUE;

_final:
	ijlFree(&jcp);

	return bRet;

}
BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel, const char * name)
{
	std::vector<BYTE> buf;
	JPEG_CORE_PROPERTIES  properties;
	std::string filename=name;
	memset(&properties,0,sizeof(properties));

	if(IJL_OK!=ijlInit(&properties)) 
		return FALSE;
	properties.JPGFile=filename.c_str();
	int ret;
	ret=ijlRead(&properties,IJL_JFILE_READPARAMS);
	if(ret!=IJL_OK)
	 goto _final;

    w=properties.JPGWidth;
	h=properties.JPGHeight;
	channel=properties.JPGChannels;
	
	return TRUE;

_final: 
	ijlFree(&properties);
	return FALSE;
}


BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel,BYTE *data,DWORD szData)
{
	std::vector<BYTE> buf;
	JPEG_CORE_PROPERTIES  properties;
	memset(&properties,0,sizeof(properties));

	if(IJL_OK!=ijlInit(&properties)) 
		return FALSE;
	properties.JPGBytes=data;
	properties.JPGSizeBytes=szData;
	int ret;
	ret=ijlRead(&properties,IJL_JBUFF_READPARAMS);
	if(ret!=IJL_OK)
		goto _final;

	w=properties.JPGWidth;
	h=properties.JPGHeight;
	channel=properties.JPGChannels;

	return TRUE;

_final: 
	ijlFree(&properties);
	return FALSE;
}
