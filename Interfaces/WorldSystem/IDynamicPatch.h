/********************************************************************
	created:	18:2:2009   9:00
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IDynamicPatch.h
	author:		chenxi
	
	purpose:	dynamic patch interface
*********************************************************************/

#pragma once

#define MAX_PATCH_VERTEX 8192*2
#define MAX_PATCH_INDEX (MAX_PATCH_VERTEX*3)

#include "vertexfmt/vertexfmt.h"
#include "RenderSystem/IRenderSystem.h"
#include "WorldSystem/IAssetRendererDefines.h"

class IAssetRenderer;
struct DynamicPatchContext
{
	ITexture *texScrnDepth;//不带引用计数
	AnimTick tFrame;
	i_math::vector3df posCamera;
	i_math::vector3df xaxisCamera;//归一化了
	i_math::vector3df yaxisCamera;//归一化了
	i_math::vector3df zaxisCamera;//归一化了
	float n,f;//近截面,远截面
	BOOL bEditMode;
};

struct ShaderCode;
//这个接口由CRagentPatches的使用者实现,并传给CRagentPatches
class IDynamicPatch
{
public:
	virtual BOOL IsOpaque()=0;
	virtual BOOL GetAabb(i_math::aabbox3df &aabb,AnimTick t)=0;
	virtual BOOL GetSortPos(i_math::vector3df &pos,AnimTick t)=0;

	virtual FVFEx GetFVF()=0;

	virtual GlobalLightingMethod GetGlm()=0;

	virtual BOOL NeedDynLocalLight()	{		return FALSE;	}
	virtual BOOL CastingShadow()	{		return FALSE;	}

	virtual DWORD GetPatchCount(){ return 1;}

	virtual BOOL BindShader(IShader *shader,DWORD iPatch,DynamicPatchContext &context)=0;
	virtual BOOL ModifyFC(ShaderCode &fc,DWORD iPatch)=0;
	virtual BOOL BindShdwShader(IShader *shader,DWORD iPatch,DynamicPatchContext &context)	{		return FALSE;	}
	virtual BOOL ModifyShdwFC(ShaderCode &fc,DWORD iPatch)	{		return FALSE;	}


	//vb/ib的buffer大小分别为MAX_PATCH_VERTEX/MAX_PATCH_INDEX个
	//nVB/nIB里返回实际填入的vertex,index个数
	virtual BOOL FillVB( BYTE *bufVB,DWORD &nVB,WORD*bufIB,DWORD &nIB,DWORD iPatch,DynamicPatchContext &context)=0;

	virtual BOOL IsWireframe()	{		return FALSE;	}
	virtual BOOL NeedGlow()	{		return FALSE;	}
	virtual BOOL IsScreenSpace()	{		return FALSE;	}
};
