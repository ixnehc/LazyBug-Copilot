
#pragma once

#include "IResource.h"

#include "fvfex/fvfex_type.h"
#include "shaderlib/SLDefines.h"

struct DtrPieceData
{
	i_math::vector3df *verticesShp;
	DWORD szVerticesShp;
	WORD *indicesShp;
	DWORD szIndicesShp;
	BYTE tpData;//0: hk, 1: px
	BYTE *data;
	DWORD szData;
	DWORD tpBuildHk;
	i_math::vector4df offHk;
};

class IVertexBuffer;
class IIndexBuffer;
struct DtrPieceInfo;
class IShader;
class IDtr;

class IDtrPieces
{
public:
	INTERFACE_REFCOUNT;
	virtual IDtr *GetOwner()=0;
	virtual i_math::aabbox3df &GetAABB()=0;
	virtual DWORD GetPartCount()=0;
	virtual DWORD GetPieceCount()=0;
	virtual BOOL GetPieceData(DWORD idx,DtrPieceData &data)=0;
	virtual BOOL GetPieceBase(DWORD idx,i_math::vector3df &posBase)=0;
	virtual IDtrPieces *GetSub(DWORD idx)=0;//返回的指针不带引用计数
	virtual IDtrPieces *ObtainSub(DWORD idx)=0;//返回的指针带一个引用计数

	//mask表示哪些piece要绘制,目前最多支持绘制32个pieces
	virtual BOOL Draw(IShader *shader,i_math::matrix43f *mats,DWORD cMats,DWORD iPart,DWORD mask)=0;
};

class IDtr:public IResource
{
public:
	virtual IDtrPieces *ObtainRoot()=0;//返回的指针带一个引用计数
};

class IDtrMgr:public IResourceMgr
{
public:
};
