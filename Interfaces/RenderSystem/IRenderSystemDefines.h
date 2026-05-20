#pragma once

#include "../extlib/dx9sdk/Include/d3d9types.h"


class IFileSystem;

//////////////////////////////////////////////////////////////////////////
//RenderSyste Init
class CConfig;
struct RenderSystemInit
{
	RenderSystemInit()
	{
		pFS=NULL;
		pFS2=NULL;
		pFS3=NULL;
		bMonitorResChange=FALSE;
		bLoadShaderCache=FALSE;
		cfg=NULL;
	}
	IFileSystem *pFS;//为主线程服务
	IFileSystem *pFS2;//为资源读取线程服务
	IFileSystem *pFS3;//为音效线程服务
	std::string pathEffect;
	std::string pathRes;
	std::string pathFont;
	std::string pathShaderCache;
	std::string pathFontCache;
	std::string pathSheet;
	std::string pathRecords;
	std::string pathBehaviorGraph;

	CConfig *cfg;
	BOOL bMonitorResChange;
	BOOL bLoadShaderCache;
};

enum RenderSystemPath
{
	Path_Effect,
	Path_Res,
	Path_Font,
	Path_ShaderCache,
	Path_FontCache,
	Path_Sheet,
	Path_Records,
	Path_BehaviorGraph,
	Path_Max,
};



//////////////////////////////////////////////////////////////////////////
//Render system result code
#define Result_RenderSystemBase 0x2000
enum RenderSystemResult
{
	DeviceErr_NODIRECT3D=Result_RenderSystemBase,
	DeviceErr_NOCOMPATIBLEDEVICES,
	DeviceErr_MEDIANOTFOUND,
	DeviceErr_NONZEROREFCOUNT,
	DeviceErr_CREATINGDEVICE,
	DeviceErr_RESETTINGDEVICE,
	DeviceErr_CREATINGDEVICEOBJECTS,
	DeviceErr_RESETTINGDEVICEOBJECTS,
	DeviceErr_INCORRECTVERSION,
	DeviceErr_INITVBPOOL,
	DeviceErr_INITSHADERMANAGER,
	DeviceErr_INITTEXTUREMANAGER,
	DeviceErr_INITRTMANAGER,
	DeviceErr_SWITCHEDTOREF,

	TexFileErr_CannotOpenFile,
	TexFileErr_CannotInitD3D,
	TexFileErr_FailToLoadImage,
	TexFileErr_FailToConvertTexture,
	TexFileErr_FailToSave,
	TexFileErr_NotConsistency,

	ResFileErr_CannotOpenFile,

	RenderSystemResult_Force_Dword=0xffffffff,
};


//////////////////////////////////////////////////////////////////////////
//Device
struct DeviceConfig
{
	DeviceConfig(HWND hWnd)
	{
		m_hWnd=hWnd;
		m_bPostPSBlending=FALSE;
		m_bRefDevice=FALSE;
		m_bFullScreen=FALSE;
		m_bCopySwap=FALSE;
		m_Width=0;
		m_Height=0;
		m_bpp=32;
	}
	HWND m_hWnd;
	BOOL m_bPostPSBlending;
	BOOL m_bRefDevice;
	BOOL m_bFullScreen;
	BOOL m_bCopySwap;
	DWORD m_bpp;
	int m_Width,m_Height;
};


struct DeviceCap
{
	DeviceCap()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL bDSTex_D24S8;
	BOOL bDSTex_D16;
	BOOL bNullRT;
};

//synchronized with D3DCLEAR_XXXX
enum ClearBufferFlag
{
	ClearBuffer_RT=1,
	ClearBuffer_Depth=2,
	ClearBuffer_Stencil=4,
	ClearBuffer_All=7,
	ClearBuffer_DS=6,
	ClearBuffer_RTDepth=3,
};



//////////////////////////////////////////////////////////////////////////
//For Device State cache
#define INVALID_RS_VALUE             D3DRS_FORCE_DWORD
#define INVALID_TSS_VALUE            D3DTSS_FORCE_DWORD
#define INVALID_SS_VALUE             D3DSAMP_FORCE_DWORD

#define MAX_RENDERSTATE_NUM          210 
#define MAX_TEXTURESTAGE_NUM         8   
#define MAX_SAMPLESTAGE_NUM          8 
#define MAX_TEXTURESTAGESTATE_NUM    33
#define MAX_SAMPLESTATE_NUM          14



//////////////////////////////////////////////////////////////////////////
//probe

#define HITPROBE_DefaultLength (200.0f)
class HitProbe:public i_math::line3df
{
};



//////////////////////////////////////////////////////////////////////////
//UVAtlas 
struct UVAtlasArg
{
	UVAtlasArg()
	{
		chUV=-1;
		w=h=256;
		gutter=2.0f;
		stretch=1.0f;
	}
	int chUV;//the new added uv should be put into which channel,if -1,automatically decided
	DWORD w,h;
	float gutter;
	float stretch;
};


