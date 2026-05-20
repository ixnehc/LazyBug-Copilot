
#pragma once

#include "IResource.h"


#define MAX_SURFACE_FRAMECOUNT 32

#define SURFINFOFLAG_RENDERTARGET 1 
#define SURFINFOFLAG_DEPTHSTENCILBUFFER 2 

#define SurfHandle_DisableDS (SurfHandle((ISurface*)1))

#define D3DFMT_FOURCC_NULL ((D3DFORMAT)MAKEFOURCC('N','U','L','L'))

struct SurfInfo
{
	WORD width;
	WORD height;
	WORD flag;
	DWORD fmt;
};

class ISurface;
class ITexture;
struct SurfHandle
{
	SurfHandle()
	{
		Zero();
	}
	void Zero()
	{
		tex=NULL;
		surf=NULL;
	}
	BOOL IsEmpty()
	{
		return (tex==NULL)&&(surf==NULL);
	}
	SurfHandle(ITexture *tex_,DWORD iFace_=0)
	{
		Set(tex_,iFace_);
	}
	SurfHandle(ISurface *surf_)
	{
		Set(surf_);
	}
	void Set(ITexture *tex_,DWORD iFace_=0)
	{
		tex=tex_;
		iFace=iFace_;
		surf=NULL;
	}
	void Set(ISurface *surf_,DWORD iFrame_=0)
	{
		surf=surf_;
		tex=NULL;
		iFace=0;
	}
	ITexture *tex;
	ISurface *surf;
	DWORD iFace;
};

class ISurface:public IResource
{
public:
	virtual DWORD GetWidth()=0;
	virtual DWORD GetHeight()=0;
	virtual DWORD GetFormat()=0;//return a D3DFORMAT value
	virtual DWORD GetFlag()=0;
	virtual void* GetSurf()=0;//return a XDirect3DSurface ptr
};

class ISurfaceMgr:public IResourceMgr
{
public:
	virtual ISurface *CreateRT(DWORD w,DWORD h,DWORD fmt,BOOL bForce=TRUE)=0;
	virtual ISurface *CreateDS(DWORD w,DWORD h,DWORD fmt,BOOL bForce=TRUE)=0;
};

