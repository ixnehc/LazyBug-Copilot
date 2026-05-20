
#pragma once

#include "IResource.h"

#include "anim/animdefines.h"

#include "shaderlib/SLDefines.h"


struct MtrlLod;

struct ShaderCode;
struct MtrlDemand;
struct MtrlCap;
struct ShaderState;
class IShader;
class IMtrl:public IResource
{
public:
	virtual DWORD GetLodCount()=0;
	virtual MtrlLod*GetLod(DWORD iLod)=0;
	virtual MtrlDemand*GetLodDemand(DWORD iLod)=0;
	virtual int ResolveLod(MtrlCap &cap)=0;//根据cap决定可以是用哪个lod进行绘制,如果没有lod满足要求,返回-1

	virtual BOOL GetShaderState(DWORD iLod,ShaderState &state)=0;

	//Note: a mtrl is taken as opaque when the base layors has its blend
	//mode as opaque
	virtual BOOL IsOpaque(DWORD iLod)=0;

	//Note: a mtrl is taken as opaque when the base layors has its alphatest 
	//mode set as AlphaTest_Standard
	virtual BOOL IsAlphaTest(DWORD iLod)=0;

	//是否完全遮盖,return TRUE when IsOpaque()&&(!IsAlphaTest())
	virtual BOOL IsCover(DWORD iLod)=0;

	virtual BOOL Is2Sided(DWORD iLod)=0;

	virtual BOOL IsWarp(DWORD iLod)=0;
	virtual BOOL IsWarpML(DWORD iLod)=0;


	virtual ShaderCode &GetShaderCode(DWORD iLod)=0;
	virtual int GetShaderLib(DWORD iLod)=0;//如果失败,返回-1


	virtual ShaderState *GetState(DWORD iLod)=0;

	virtual void BindEP(IShader *shader,DWORD iLod,AnimTick t=ANIMTICK_INFINITE)=0;
	virtual void BindState(IShader *shader,DWORD iLod)=0;
};

struct MtrlData;
class IMtrlMgr:public IResourceMgr
{
public:
	virtual IMtrl *Create(MtrlData *data,const char *pathOverride)=0;//注意,这个path是为了ResolveRefPath()的需要
};
