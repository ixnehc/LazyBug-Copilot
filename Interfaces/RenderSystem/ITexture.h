
#pragma once

#include "IResource.h"


//////////////////////////////////////////////////////////////////////////
//Texture
#define TEXFILE_OLDVER1 1
#define TEXFILE_CURVER 2

#define TEXTUREFILE_SUFFIX "tex"

#define TEXINFOFLAG_RTCREATE 1 //this texture is created at run time
#define TEXINFOFLAG_RENDERTARGET 2 //this texture could be a render target
#define TEXINFOFLAG_WRITABLE 4

#define TEXTYPE_NONE 0
#define TEXTYPE_2D 2
#define TEXTYPE_CUBIC 4
#define TEXTYPE_3D 8 //Volume


#define MAX_TEXTURE 8
struct TexInfo
{
	TexInfo()
	{
		Zero();
	}
	void Zero()
	{
		width=256;
		height=256;
		depth=0;
		miplevel=1;
		type=TEXTYPE_2D;
		fmt=33;//D3DFMT_X8B8G8R8;//A D3DFORMAT value
		fmtbit=32;
		filterAutoGenMipmap=2;//D3DTEXF_LINEAR;
	}
	void MakeDefault()
	{
		Zero();
		width=height=depth=0;
		miplevel=0;
		type=TEXTYPE_NONE;
		fmt=0;//D3DFMT_UNKNOWN
		fmtbit=0;
	}
	WORD type;//TEXTYPE_XXXX
	WORD width;
	WORD height;
	WORD depth;
	WORD miplevel;//if 0,this texture is an autogen-mipmap texture
	DWORD fmt;//A D3DFORMAT value
	DWORD fmtbit;//in bit
	DWORD filterAutoGenMipmap;//only valid when an autogen-mipmap texture
};


class ITexture;

struct TexStretchArg
{
	TexStretchArg()
	{
		iFaceSrc=0;
		iFaceDest=0;
		iLvlSrc=0;
		iLvlDest=0;
		rcSrc.set(0,0,0,0);
		rcDest.set(0,0,0,0);
		filter=3;//D3DX_FILTER_LINEAR;
	}
	DWORD iFaceSrc;
	DWORD iFaceDest;
	DWORD iLvlSrc;
	DWORD iLvlDest;
	i_math::recti rcSrc;//if Zero Rect, use the full rect
	i_math::recti rcDest;//if Zero Rect,use the full rect
	int filter;// a D3DX_FILTER value
};

enum TexLockFlag
{
	TexLock_ReadWrite=0,
	TexLock_ReadOnly,
	TexLock_WriteOnly,//will discard all the existing content
};


//None-Standard texture format
#define D3DFMT_ATI2N ((D3DFORMAT) MAKEFOURCC('A', 'T', 'I', '2'))



struct TexInfo;
struct TexData;

class ITexture:public IResource
{
public:
	virtual const TexInfo *GetInfo()=0;
	virtual DWORD GetType()=0;
	virtual DWORD GetFlag()=0;
	virtual DWORD GetWidth()=0;
	virtual DWORD GetHeight()=0;
	virtual DWORD GetDepth()=0;//for volume texture only
	virtual DWORD GetMipLevel()=0;//if 0,this texture is a autogen-mipmap texture
	virtual DWORD GetFormat()=0;//return a D3DFORMAT value
	virtual void* GetTex()=0;//return a XDirect3DBaseTexture ptr
	virtual void* GetSurf(DWORD iLevel=0,DWORD iFace=0)=0;//return a XDirect3DSurface ptr
	virtual BOOL Stretch(ITexture *texSrc,TexStretchArg &arg)=0;
	virtual BOOL Fill(DWORD color)=0;
	virtual BOOL Filter(int filter=4)=0;//generate mipmap level,4 is D3DX_FILTER_TRIANGLE
	virtual BOOL DumpData(TexData*&data)=0;//the pointer returned should NOT be kept for later using
	//Dump data in bmp format for a certain frame
	//the pointer returned should NOT be kept for later using
	virtual BOOL DumpTga(TexData*&data)=0;
	virtual BOOL DumpTga(const char *fn)=0;
	virtual BOOL DumpJpg(TexData *&data)=0;
	virtual BOOL DumpJpg(const char *fn)=0;

	//lock to modify,
	//iLevel is the level of the mipmap
	//for cubic texture,iFace is ranged within [0,5]
	//for volume texture, iFace is ranged within [0,depth-1]
	//NOTE:once you lock a texture,you should NOT lock it again before you unlock it
	virtual void *Lock(DWORD &pitch,TexLockFlag flag,DWORD iLevel=0,DWORD iFace=0)=0;
	virtual void *Lock(i_math::recti &rc,DWORD &pitch,TexLockFlag flag,DWORD iLevel=0,DWORD iFace=0)=0;
	virtual void UnLock()=0;

	//return a temply ptr to a buffer containing the single pixel data
	virtual void *GetPixel(int x,int y,DWORD iLevel=0,DWORD iFace=0)=0;
	virtual DWORD GetPixelColor(int x,int y,DWORD iLevel=0,DWORD iFace=0)=0;
};


class ITextureMgr:public IResourceMgr
{
public:

};

//R: Render Target
class IRTextureMgr:public IResourceMgr
{
public:
	//by now ,not supports volume textures
	virtual ITexture *Create(const TexInfo &ti)=0;
	//for level:pass 0 to create a autogenmipmap texture,pass 0xffffffff to create full level of mipmap
	virtual ITexture *Create(DWORD w,DWORD h,DWORD fmt,DWORD level)=0;
	virtual ITexture *CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level)=0;
};

//W: Writable
struct TexData;
class IWTextureMgr:public IResourceMgr
{
public:
	//if bForce is TRUE, this funtion should fail if requirement cannot be met,
	//otherwise this function will try to adjust the requirement and try to find a match one
	//by now ,not supports volume textures
	virtual ITexture *Create(const TexInfo &ti)=0;
	//for level:pass 0 to create a autogenmipmap texture,pass 0xffff to create full level of mipmap
	virtual ITexture *Create(DWORD w,DWORD h,DWORD fmt,DWORD level=1)=0;
	virtual ITexture *CreateCube(DWORD w,DWORD h,DWORD fmt,DWORD level=1)=0;
	virtual ITexture *Create(TexData &data)=0;
};
