/********************************************************************
	created:	2007/3/7   12:14
	filename: 	e:\IxEngine\Common\fvfex\fvfex.cpp
	author:		cxi
	
	purpose:	fvfex management functions
*********************************************************************/
#include "stdh.h"
#include "fvfex.h"

#pragma warning(disable:4018)

//////////////////////////////////////////////////////////////////////////
//D3D9
  
//copyed from <d3d9types.h>
enum D3DDECLTYPEX
{
	D3DDECLTYPEX_FLOAT1    =  0,  // 1D float expanded to (value, 0., 0., 1.)
	D3DDECLTYPEX_FLOAT2    =  1,  // 2D float expanded to (value, value, 0., 1.)
	D3DDECLTYPEX_FLOAT3    =  2,  // 3D float expanded to (value, value, value, 1.)
	D3DDECLTYPEX_FLOAT4    =  3,  // 4D float
	D3DDECLTYPEX_D3DCOLOR  =  4,  // 4D packed unsigned bytes mapped to 0. to 1. range
	// Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
	D3DDECLTYPEX_UBYTE4    =  5,  // 4D unsigned byte
	D3DDECLTYPEX_SHORT2    =  6,  // 2D signed short expanded to (value, value, 0., 1.)
	D3DDECLTYPEX_SHORT4    =  7,  // 4D signed short

	// The following types are valid only with vertex shaders >= 2.0


	D3DDECLTYPEX_UBYTE4N   =  8,  // Each of 4 bytes is normalized by dividing to 255.0
	D3DDECLTYPEX_SHORT2N   =  9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
	D3DDECLTYPEX_SHORT4N   = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
	D3DDECLTYPEX_USHORT2N  = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
	D3DDECLTYPEX_USHORT4N  = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
	D3DDECLTYPEX_UDEC3     = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
	D3DDECLTYPEX_DEC3N     = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
	D3DDECLTYPEX_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
	D3DDECLTYPEX_FLOAT16_4 = 16,  // Four 16-bit floating point values
	D3DDECLTYPEX_UNUSED    = 17,  // When the type field in a decl is unused.
};

enum D3DDECLUSAGEX
{
	D3DDECLUSAGEX_POSITION = 0,
	D3DDECLUSAGEX_BLENDWEIGHT,   // 1
	D3DDECLUSAGEX_BLENDINDICES,  // 2
	D3DDECLUSAGEX_NORMAL,        // 3
	D3DDECLUSAGEX_PSIZE,         // 4
	D3DDECLUSAGEX_TEXCOORD,      // 5
	D3DDECLUSAGEX_TANGENT,       // 6
	D3DDECLUSAGEX_BINORMAL,      // 7
	D3DDECLUSAGEX_TESSFACTOR,    // 8
	D3DDECLUSAGEX_POSITIONT,     // 9
	D3DDECLUSAGEX_COLOR,         // 10
	D3DDECLUSAGEX_FOG,           // 11
	D3DDECLUSAGEX_DEPTH,         // 12
	D3DDECLUSAGEX_SAMPLE,        // 13
};


typedef enum DXGI_FORMAT_EX
{
	DXGI_FORMAT_EX_UNKNOWN                      = 0,
	DXGI_FORMAT_EX_R32G32B32A32_TYPELESS        = 1,
	DXGI_FORMAT_EX_R32G32B32A32_FLOAT           = 2,
	DXGI_FORMAT_EX_R32G32B32A32_UINT            = 3,
	DXGI_FORMAT_EX_R32G32B32A32_SINT            = 4,
	DXGI_FORMAT_EX_R32G32B32_TYPELESS           = 5,
	DXGI_FORMAT_EX_R32G32B32_FLOAT              = 6,
	DXGI_FORMAT_EX_R32G32B32_UINT               = 7,
	DXGI_FORMAT_EX_R32G32B32_SINT               = 8,
	DXGI_FORMAT_EX_R16G16B16A16_TYPELESS        = 9,
	DXGI_FORMAT_EX_R16G16B16A16_FLOAT           = 10,
	DXGI_FORMAT_EX_R16G16B16A16_UNORM           = 11,
	DXGI_FORMAT_EX_R16G16B16A16_UINT            = 12,
	DXGI_FORMAT_EX_R16G16B16A16_SNORM           = 13,
	DXGI_FORMAT_EX_R16G16B16A16_SINT            = 14,
	DXGI_FORMAT_EX_R32G32_TYPELESS              = 15,
	DXGI_FORMAT_EX_R32G32_FLOAT                 = 16,
	DXGI_FORMAT_EX_R32G32_UINT                  = 17,
	DXGI_FORMAT_EX_R32G32_SINT                  = 18,
	DXGI_FORMAT_EX_R32G8X24_TYPELESS            = 19,
	DXGI_FORMAT_EX_D32_FLOAT_S8X24_UINT         = 20,
	DXGI_FORMAT_EX_R32_FLOAT_X8X24_TYPELESS     = 21,
	DXGI_FORMAT_EX_X32_TYPELESS_G8X24_UINT      = 22,
	DXGI_FORMAT_EX_R10G10B10A2_TYPELESS         = 23,
	DXGI_FORMAT_EX_R10G10B10A2_UNORM            = 24,
	DXGI_FORMAT_EX_R10G10B10A2_UINT             = 25,
	DXGI_FORMAT_EX_R11G11B10_FLOAT              = 26,
	DXGI_FORMAT_EX_R8G8B8A8_TYPELESS            = 27,
	DXGI_FORMAT_EX_R8G8B8A8_UNORM               = 28,
	DXGI_FORMAT_EX_R8G8B8A8_UNORM_SRGB          = 29,
	DXGI_FORMAT_EX_R8G8B8A8_UINT                = 30,
	DXGI_FORMAT_EX_R8G8B8A8_SNORM               = 31,
	DXGI_FORMAT_EX_R8G8B8A8_SINT                = 32,
	DXGI_FORMAT_EX_R16G16_TYPELESS              = 33,
	DXGI_FORMAT_EX_R16G16_FLOAT                 = 34,
	DXGI_FORMAT_EX_R16G16_UNORM                 = 35,
	DXGI_FORMAT_EX_R16G16_UINT                  = 36,
	DXGI_FORMAT_EX_R16G16_SNORM                 = 37,
	DXGI_FORMAT_EX_R16G16_SINT                  = 38,
	DXGI_FORMAT_EX_R32_TYPELESS                 = 39,
	DXGI_FORMAT_EX_D32_FLOAT                    = 40,
	DXGI_FORMAT_EX_R32_FLOAT                    = 41,
	DXGI_FORMAT_EX_R32_UINT                     = 42,
	DXGI_FORMAT_EX_R32_SINT                     = 43,
	DXGI_FORMAT_EX_R24G8_TYPELESS               = 44,
	DXGI_FORMAT_EX_D24_UNORM_S8_UINT            = 45,
	DXGI_FORMAT_EX_R24_UNORM_X8_TYPELESS        = 46,
	DXGI_FORMAT_EX_X24_TYPELESS_G8_UINT         = 47,
	DXGI_FORMAT_EX_R8G8_TYPELESS                = 48,
	DXGI_FORMAT_EX_R8G8_UNORM                   = 49,
	DXGI_FORMAT_EX_R8G8_UINT                    = 50,
	DXGI_FORMAT_EX_R8G8_SNORM                   = 51,
	DXGI_FORMAT_EX_R8G8_SINT                    = 52,
	DXGI_FORMAT_EX_R16_TYPELESS                 = 53,
	DXGI_FORMAT_EX_R16_FLOAT                    = 54,
	DXGI_FORMAT_EX_D16_UNORM                    = 55,
	DXGI_FORMAT_EX_R16_UNORM                    = 56,
	DXGI_FORMAT_EX_R16_UINT                     = 57,
	DXGI_FORMAT_EX_R16_SNORM                    = 58,
	DXGI_FORMAT_EX_R16_SINT                     = 59,
	DXGI_FORMAT_EX_R8_TYPELESS                  = 60,
	DXGI_FORMAT_EX_R8_UNORM                     = 61,
	DXGI_FORMAT_EX_R8_UINT                      = 62,
	DXGI_FORMAT_EX_R8_SNORM                     = 63,
	DXGI_FORMAT_EX_R8_SINT                      = 64,
	DXGI_FORMAT_EX_A8_UNORM                     = 65,
	DXGI_FORMAT_EX_R1_UNORM                     = 66,
	DXGI_FORMAT_EX_R9G9B9E5_SHAREDEXP           = 67,
	DXGI_FORMAT_EX_R8G8_B8G8_UNORM              = 68,
	DXGI_FORMAT_EX_G8R8_G8B8_UNORM              = 69,
	DXGI_FORMAT_EX_BC1_TYPELESS                 = 70,
	DXGI_FORMAT_EX_BC1_UNORM                    = 71,
	DXGI_FORMAT_EX_BC1_UNORM_SRGB               = 72,
	DXGI_FORMAT_EX_BC2_TYPELESS                 = 73,
	DXGI_FORMAT_EX_BC2_UNORM                    = 74,
	DXGI_FORMAT_EX_BC2_UNORM_SRGB               = 75,
	DXGI_FORMAT_EX_BC3_TYPELESS                 = 76,
	DXGI_FORMAT_EX_BC3_UNORM                    = 77,
	DXGI_FORMAT_EX_BC3_UNORM_SRGB               = 78,
	DXGI_FORMAT_EX_BC4_TYPELESS                 = 79,
	DXGI_FORMAT_EX_BC4_UNORM                    = 80,
	DXGI_FORMAT_EX_BC4_SNORM                    = 81,
	DXGI_FORMAT_EX_BC5_TYPELESS                 = 82,
	DXGI_FORMAT_EX_BC5_UNORM                    = 83,
	DXGI_FORMAT_EX_BC5_SNORM                    = 84,
	DXGI_FORMAT_EX_B5G6R5_UNORM                 = 85,
	DXGI_FORMAT_EX_B5G5R5A1_UNORM               = 86,
	DXGI_FORMAT_EX_B8G8R8A8_UNORM               = 87,
	DXGI_FORMAT_EX_B8G8R8X8_UNORM               = 88,
	DXGI_FORMAT_EX_R10G10B10_XR_BIAS_A2_UNORM   = 89,
	DXGI_FORMAT_EX_B8G8R8A8_TYPELESS            = 90,
	DXGI_FORMAT_EX_B8G8R8A8_UNORM_SRGB          = 91,
	DXGI_FORMAT_EX_B8G8R8X8_TYPELESS            = 92,
	DXGI_FORMAT_EX_B8G8R8X8_UNORM_SRGB          = 93,
	DXGI_FORMAT_EX_BC6H_TYPELESS                = 94,
	DXGI_FORMAT_EX_BC6H_UF16                    = 95,
	DXGI_FORMAT_EX_BC6H_SF16                    = 96,
	DXGI_FORMAT_EX_BC7_TYPELESS                 = 97,
	DXGI_FORMAT_EX_BC7_UNORM                    = 98,
	DXGI_FORMAT_EX_BC7_UNORM_SRGB               = 99,
	DXGI_FORMAT_EX_FORCE_UINT                   = 0xffffffffUL 
} DXGI_FORMAT_EX, *LPDXGI_FORMAT_EX;


struct D3DVERTEXELEMENT9_x
{
	WORD    Stream;     // Stream index
	WORD    Offset;     // Offset in the stream in bytes
	BYTE    Type;       // Data type
	BYTE    Method;     // Processing method
	BYTE    Usage;      // Semantics
	BYTE    UsageIndex; // Semantic index
};

struct D3D11_INPUT_ELEMENT_DESC_x
{
	const char *SemanticName;
	DWORD SemanticIndex;
	DWORD Format;
	DWORD InputSlot;
	DWORD AlignedByteOffset;
	DWORD InputSlotClass;
	DWORD InstanceDataStepRate;
};

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C



FVFExInfo g_aFVFList[]=
{
	FVFExInfo(FVFEX_XYZ0,						D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_POSITION,  0,"POSITION0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_pos",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_XYZ1,						D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_POSITION,  1,"POSITION1",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_pos2",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_XYZRHW,				D3DDECLTYPEX_FLOAT4 ,			16,		D3DDECLUSAGEX_POSITIONT,  0,"",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_posRHW",4,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_XYZW0,					D3DDECLTYPEX_FLOAT4 ,			16,		D3DDECLUSAGEX_POSITION,  0,"",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_xyzw",4,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_XYZW1,					D3DDECLTYPEX_FLOAT4 ,			16,		D3DDECLUSAGEX_POSITION,  1,"",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_xyzw2",4,GL_FLOAT,FALSE),
	//	FVFExInfo(FVFEX_WEIGHT0,			D3DDECLTYPEX_FLOAT1,			4,			D3DDECLUSAGEX_BLENDWEIGHT,  0),
	FVFExInfo(FVFEX_NORMAL0,				D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_NORMAL,  0,"NORMAL0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_normal",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_NORMAL1,				D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_NORMAL,  1,"NORMAL1",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_normal2",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_BINORMAL,			D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_BINORMAL,  0,"BINORMAL0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_binormal",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_TANGENT,				D3DDECLTYPEX_FLOAT3 ,			12,		D3DDECLUSAGEX_TANGENT,  0,"TANGENT0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_tangent",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_PSIZE,					D3DDECLTYPEX_FLOAT1 ,			4,			D3DDECLUSAGEX_PSIZE,  0,	"PSIZE0",DXGI_FORMAT_EX_R32_FLOAT,"vi_psize",1,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_DIFFUSE,				D3DDECLTYPEX_D3DCOLOR,		4,			D3DDECLUSAGEX_COLOR,  0,"COLOR0",DXGI_FORMAT_EX_R8G8B8A8_UNORM,"vi_dif",4,GL_UNSIGNED_BYTE,TRUE),
	FVFExInfo(FVFEX_SPECULAR,			D3DDECLTYPEX_D3DCOLOR,		4,			D3DDECLUSAGEX_COLOR,  1,"COLOR1",DXGI_FORMAT_EX_R8G8B8A8_UNORM,"vi_spec",4,GL_UNSIGNED_BYTE,TRUE),

	FVFExInfo(FVFEX_WEIGHT01,			D3DDECLTYPEX_FLOAT1,			4,			D3DDECLUSAGEX_BLENDWEIGHT,  0,"BLENDWEIGHT0",DXGI_FORMAT_EX_R32_FLOAT,"vi_weightx2",1,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_WEIGHT012,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_BLENDWEIGHT,  0,"BLENDWEIGHT0",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_weightx3",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_WEIGHT0123,		D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_BLENDWEIGHT,  0,"BLENDWEIGHT0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_weightx4",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_BONEINDICE0,		D3DDECLTYPEX_UBYTE4,		4,			D3DDECLUSAGEX_BLENDINDICES,  0,"BLENDINDICES0",DXGI_FORMAT_EX_R8G8B8A8_UINT,"vi_blendindice",4,GL_UNSIGNED_BYTE,FALSE),
	FVFExInfo(FVFEX_BONEINDICE1,		D3DDECLTYPEX_UBYTE4,		4,			D3DDECLUSAGEX_BLENDINDICES,  0,"BLENDINDICES0",DXGI_FORMAT_EX_R8G8B8A8_UINT,"vi_blendindice2",4,GL_UNSIGNED_BYTE,FALSE),

	FVFExInfo(FVFEX_TRRN_POS,			D3DDECLTYPEX_SHORT4,			8,			D3DDECLUSAGEX_POSITION,  0,"POSITION0",DXGI_FORMAT_EX_R16G16B16A16_SINT,"vi_posTrrn",4,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX00,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  0,"TEXCOORD0",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn0",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX01,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  1,"TEXCOORD1",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn1",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX02,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  2,"TEXCOORD2",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn2",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX03,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  3,"TEXCOORD3",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn3",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX04,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  4,"TEXCOORD4",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn4",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_TEX05,		D3DDECLTYPEX_SHORT2,			4,			D3DDECLUSAGEX_TEXCOORD,  5,"TEXCOORD5",DXGI_FORMAT_EX_R16G16_SINT,"vi_uvTrrn5",2,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_TRRN_NORMAL,	D3DDECLTYPEX_D3DCOLOR,		4,			D3DDECLUSAGEX_NORMAL,  0,"NORMAL0",DXGI_FORMAT_EX_R8G8B8A8_UNORM,"vi_nmlTrrn",4,GL_UNSIGNED_BYTE,TRUE),

	FVFExInfo(FVFEX_FLAG_TEX0,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  0,"TEXCOORD0",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv0",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX1,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  1,"TEXCOORD1",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv1",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX2,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  2,"TEXCOORD2",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv2",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX3,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  3,"TEXCOORD3",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv3",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX4,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  4,"TEXCOORD4",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv4",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX5,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  5,"TEXCOORD5",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv5",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX6,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  6,"TEXCOORD6",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv6",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_TEX7,			D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  7,"TEXCOORD7",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uv7",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX0,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  0,"TEXCOORD0",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw0",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX1,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  1,"TEXCOORD1",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw1",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX2,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  2,"TEXCOORD2",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw2",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX3,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  3,"TEXCOORD3",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw3",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX4,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  4,"TEXCOORD4",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw4",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX5,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  5,"TEXCOORD5",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw5",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX6,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  6,"TEXCOORD6",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw6",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_VOX7,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  7,"TEXCOORD7",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_uvw7",3,GL_FLOAT,FALSE),

	FVFExInfo(FVFEX_FLAG_QUX0,			D3DDECLTYPEX_FLOAT4,			16,		D3DDECLUSAGEX_TEXCOORD,  0,"TEXCOORD0",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_qux0",4,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_QUX1,			D3DDECLTYPEX_FLOAT4,			16,		D3DDECLUSAGEX_TEXCOORD,  1,"TEXCOORD1",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_qux1",4,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_QUX2,			D3DDECLTYPEX_FLOAT4,			16,		D3DDECLUSAGEX_TEXCOORD,  2,"TEXCOORD2",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_qux2",4,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_FLAG_QUX3,			D3DDECLTYPEX_FLOAT4,			16,		D3DDECLUSAGEX_TEXCOORD,  3,"TEXCOORD3",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_qux3",4,GL_FLOAT,FALSE),

	FVFExInfo(FVFEX_COMPACT_POS,	D3DDECLTYPEX_SHORT4,			8,			D3DDECLUSAGEX_POSITION,		0,"POSITION0",DXGI_FORMAT_EX_R16G16B16A16_SINT,"vi_posC",4,GL_SHORT,FALSE),
	FVFExInfo(FVFEX_COMPACT_NORMAL,	D3DDECLTYPEX_D3DCOLOR,			4,			D3DDECLUSAGEX_NORMAL,  0,"NORMAL0",DXGI_FORMAT_EX_R8G8B8A8_UINT,"vi_normalC",4,GL_UNSIGNED_BYTE,TRUE),
	FVFExInfo(FVFEX_COMPACT_BINORMAL,	D3DDECLTYPEX_D3DCOLOR,			4,			D3DDECLUSAGEX_BINORMAL,  0,"BINORMAL0",DXGI_FORMAT_EX_R8G8B8A8_UINT,"vi_binormalC",4,GL_UNSIGNED_BYTE,TRUE),
	FVFExInfo(FVFEX_VEGE_TEX,				D3DDECLTYPEX_FLOAT2,			8,			D3DDECLUSAGEX_TEXCOORD,  0,"TEXCOORD0",DXGI_FORMAT_EX_R32G32_FLOAT,"vi_uvVege",2,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_VEGE_COL,				D3DDECLTYPEX_D3DCOLOR,		4,			D3DDECLUSAGEX_COLOR,  0,"COLOR0",DXGI_FORMAT_EX_R8G8B8A8_UNORM,"vi_colVege",4,GL_UNSIGNED_BYTE,TRUE),
	FVFExInfo(FVFEX_VEGE_GEOM,			D3DDECLTYPEX_FLOAT3,			12,		D3DDECLUSAGEX_TEXCOORD,  1,"TEXCOORD1",DXGI_FORMAT_EX_R32G32B32_FLOAT,"vi_geomVege",3,GL_FLOAT,FALSE),
	FVFExInfo(FVFEX_VEGE_PARAM,		D3DDECLTYPEX_D3DCOLOR,		4,			D3DDECLUSAGEX_COLOR,  1,"COLOR1",DXGI_FORMAT_EX_R8G8B8A8_UNORM,"vi_parVege",4,GL_UNSIGNED_BYTE,TRUE),

	FVFExInfo(FVFEX_DIFFUSE_F,				D3DDECLTYPEX_FLOAT4,		16,			D3DDECLUSAGEX_COLOR,  0,"COLOR0",DXGI_FORMAT_EX_R32G32B32A32_FLOAT,"vi_colF",4,GL_FLOAT,FALSE),

};

int g_SizeOfFVFList=sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);

FVFEx fvfFromName(const char *nm)
{
	for (int i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		if (strcmp(g_aFVFList[i].m_nmGL,nm)==0)
			return g_aFVFList[i].m_fvf;
	}
	return 0;
}


//check validity of fvf
BOOL fvfCheck(FVFEx fvf)
{
	if (fvf==FVFEX_NULL)
		return FALSE;
	int count;
	int i;
	count=0;
	for (i=0;i<FVFEX_WEIGHT_MAXSET;i++)
	{
		if (fvf&FVFEX_WEIGHT(i))
			count++;
	}
	if (count>1)
		return FALSE;

	return TRUE;
}


DWORD fvfSize(FVFEx fvfTotal)
{
	if (!fvfCheck(fvfTotal))
		return 0;
	int i;
	int count;
	count=0;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		FVFEx fvf;
		fvf=g_aFVFList[i].m_fvf;
		if ((fvf&fvfTotal)==fvf)
			count+=g_aFVFList[i].m_size;
		fvfTotal&=(~fvf);
		if (fvfTotal==FVFEX_NULL)
			return count;
	}

	return count;
}


int fvfOffset(FVFEx fvfVertex,FVFEx fvfToSearch)
{
	if (!fvfCheck(fvfVertex))
		return -1;
	if (!fvfCheck(fvfToSearch))
		return -1;

	int i,n;
	n=0;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		FVFEx fvf;
		fvf=g_aFVFList[i].m_fvf;
		if ((fvf&fvfToSearch)==fvf)
		{
			if ((fvf&fvfVertex)!=fvf)
				return -1;
			return n;
		}
		else
		{
			if ((fvf&fvfVertex)==fvf)
				n+=g_aFVFList[i].m_size;
		}
	}

	return -1;//Not in it
}


FVFEx fvfFirstPos(FVFEx fvf)
{
	FVFEx a[]=
	{
		FVFEX_XYZ0,
		FVFEX_XYZ1,
	};
	int i;
	for (i=0;i<sizeof(a)/sizeof(FVFEx);i++)
	{
		if (fvf&a[i])
			return a[i];
	}
	return FVFEX_NULL;
}
FVFEx fvfFirstNormal(FVFEx fvf)
{
	FVFEx a[]=
	{
		FVFEX_NORMAL0,
		FVFEX_NORMAL1,
	};
	int i;
	for (i=0;i<sizeof(a)/sizeof(FVFEx);i++)
	{
		if (fvf&a[i])
			return a[i];
	}
	return FVFEX_NULL;

}
FVFEx fvfFirstTex(FVFEx fvf)
{
	
	int i;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (fvf&FVFEX_FLAG_TEX(i))
			return FVFEX_FLAG_TEX(i);
	}
	return FVFEX_NULL;

}
FVFEx fvfFirstVox(FVFEx fvf)
{
	int i;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (fvf&FVFEX_FLAG_VOX(i))
			return FVFEX_FLAG_VOX(i);
	}
	return FVFEX_NULL;

}
//FVFEx fvfFirstLix(FVFEx fvf)
//{
//	int i;
//	for (i=0;i<FVFEX_TEX_MAXSET;i++)
//	{
//		if (fvf&FVFEX_FLAG_LIX(i))
//			return FVFEX_FLAG_LIX(i);
//	}
//	return FVFEX_NULL;
//
//}

FVFEx fvfFirst(FVFEx fvf)
{
	if (fvf==FVFEX_NULL)
		return FVFEX_NULL;
	int i;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		if ((fvf&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
			return g_aFVFList[i].m_fvf;
	}

	return FVFEX_NULL;

}

int fvfToD3DVERTEXELEMENT9(FVFEx fvfTotal,FVFEx fvf,D3DVERTEXELEMENT9_x *pElements,int iStream)
{
	if (fvfTotal==FVFEX_NULL)
		return 0;
	if (fvf==FVFEX_NULL)
		return 0;

	DWORD count;
	count=0;
	int i;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		if ((fvf&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
		{
			FVFEx fvfElement;
			fvfElement=g_aFVFList[i].m_fvf;

			if ((fvfElement&fvfTotal)==fvfElement)//if this element exists in fvfTotal
			{
				D3DVERTEXELEMENT9_x *p;
				p=&(pElements[count]);
				count++;

				p->Stream=iStream;
				p->Offset=fvfOffset(fvfTotal,fvfElement);//for each element in fvf,find its offset in fvfTotal
				p->Method=0;//D3DDECLMETHOD_DEFAULT;
				p->Type=g_aFVFList[i].m_type;
				p->Usage=g_aFVFList[i].m_usage;
				p->UsageIndex=g_aFVFList[i].m_usageindex;
			}

			fvf&=(~fvfElement);//cull out this element from fvf
			if (fvf==FVFEX_NULL)
				return count;
		}

	}

	return count;


}

int fvfToD3DVERTEXELEMENT11(FVFEx fvfTotal,FVFEx fvf,D3D11_INPUT_ELEMENT_DESC_x*pElements,int iSlot)
{
	if (fvfTotal==FVFEX_NULL)
		return 0;
	if (fvf==FVFEX_NULL)
		return 0;

	DWORD count;
	count=0;
	int i;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		if ((fvf&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
		{
			FVFEx fvfElement;
			fvfElement=g_aFVFList[i].m_fvf;

			if ((fvfElement&fvfTotal)==fvfElement)//if this element exists in fvfTotal
			{
				D3D11_INPUT_ELEMENT_DESC_x *p;
				p=&(pElements[count]);
				count++;

				p->SemanticName=g_aFVFList[i].m_sem;
				p->SemanticIndex=g_aFVFList[i].m_usageindex;
				p->AlignedByteOffset=fvfOffset(fvfTotal,fvfElement);//for each element in fvf,find its offset in fvfTotal
				p->Format=g_aFVFList[i].m_fmt;
				p->InputSlot=iSlot;

				p->InputSlotClass=0;//D3D10_INPUT_PER_VERTEX_DATA;
				p->InstanceDataStepRate=0;

			}

			fvf&=(~fvfElement);//cull out this element from fvf
			if (fvf==FVFEX_NULL)
				return count;
		}

	}

	return count;
}


//copy all the copyable fvf from source buffer to dest buffer
void fvfCopy(DWORD nVertice,void *pDest,FVFEx fvfDest,void *pSrc,FVFEx fvfSrc)
{
	BOOL bRet=TRUE;

	int i,n;
	n=g_SizeOfFVFList;

	DWORD szDest,szSrc;
	szDest=fvfSize(fvfDest);
	szSrc=fvfSize(fvfSrc);

	FVFEx fvfDest0,fvfSrc0;
	fvfDest0=fvfDest;
	fvfSrc0=fvfSrc;

	for (i=0;i<n;i++)
	{
		if (((fvfSrc&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)&&
			((fvfDest&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf))
		{
			BYTE *p,*q;
			p=(BYTE*)pDest+fvfOffset(fvfDest0,g_aFVFList[i].m_fvf);
			q=(BYTE*)pSrc+fvfOffset(fvfSrc0,g_aFVFList[i].m_fvf);

			int k;
			for (k=0;k<nVertice;k++)
			{
				memcpy(p,q,g_aFVFList[i].m_size);
				p+=szDest;
				q+=szSrc;
			}

			fvfSrc&=~g_aFVFList[i].m_fvf;
			fvfDest&=~g_aFVFList[i].m_fvf;

			if ((fvfSrc==FVFEX_NULL)||(fvfDest==FVFEX_NULL))
				break;
		}
	}
}

void fvfCopyByStride(DWORD nVertice,FVFEx fvfSrc,void *pDest,DWORD nStrideDest,void *pSrc,DWORD nStrideSrc)
{
	DWORD szSrc;
	szSrc=fvfSize(fvfSrc);
	if (szSrc<=0)
		return;
	BYTE *p,*q;
	p=(BYTE*)pDest;
	q=(BYTE*)pSrc;

	int k;
	for (k=0;k<nVertice;k++)
	{
		memcpy(p,q,szSrc);
		p+=nStrideDest;
		q+=nStrideSrc;
	}
}

#include "../math/vector3d.h"
#include "../math/quaternion.h"

//By now interpolate FVFEX_XYZ0,FVFEX_NORMAL0,
//not interpolated elements are simply copied from pSrc1
void fvfInterpolate(DWORD nVertice,FVFEx fvf,void *pDest,void *pSrc1,void *pSrc2,float r)
{
	DWORD dwStride;	
	dwStride=fvfSize(fvf);
	memcpy(pDest,pSrc1,dwStride*nVertice);

	int off;
	off=fvfOffset(fvf,FVFEX_XYZ0);
	if (-1!=off)
	{
		i_math::vector3df *p,*q1,*q2;
		p=(i_math::vector3df *)((BYTE*)pDest+off);
		q1=(i_math::vector3df *)((BYTE*)pSrc1+off);
		q2=(i_math::vector3df *)((BYTE*)pSrc2+off);

		for (int i=0;i<nVertice;i++)
		{
			(*p)=(*q2).getInterpolated(*q1,r);

			((BYTE*&)p)+=dwStride;
			((BYTE*&)q1)+=dwStride;
			((BYTE*&)q2)+=dwStride;
		}
	}

	off=fvfOffset(fvf,FVFEX_NORMAL0);
	if (-1!=off)
	{
		i_math::vector3df *p,*q1,*q2;
		p=(i_math::vector3df *)((BYTE*)pDest+off);
		q1=(i_math::vector3df *)((BYTE*)pSrc1+off);
		q2=(i_math::vector3df *)((BYTE*)pSrc2+off);

		for (int i=0;i<nVertice;i++)
		{
			i_math::quatf qt,qtZero;
			qt.from2Vector(*q1,*q2);
			qt.slerp(qtZero,qt,r);
			(*p)=qt*(*q1);

			((BYTE*&)p)+=dwStride;
			((BYTE*&)q1)+=dwStride;
			((BYTE*&)q2)+=dwStride;
		}
	}

}


