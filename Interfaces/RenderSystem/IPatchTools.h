
#pragma once

#include "fvfex/fvfex_type.h"

#include "IVertexBuffer.h"

//////////////////////////////////////////////////////////////////////////
//Patch Geom
typedef DWORD PGPatchHandle;
typedef WORD PGSurfHandle;

#define MAX_PG_PATCH_GROUP ((1<<14)-1) //we use 14 bit to record a group value

struct PGStat //Statistics
{
	DWORD nSurf;
	DWORD nSubSurf;
	DWORD MaxSubSurfInSurf;
	DWORD VBMemActual;
	DWORD VBMemUsed;
	DWORD VBMemAlloc;
	DWORD IBMemActual;
	DWORD IBMemUsed;
	DWORD IBMemAlloc;
	DWORD PhysMemUsed;
	DWORD PhysMemAlloc;
};

//PatchQueue
typedef void* PQPatch;

class IRenderer;

class IPatchGeom
{
public:
	INTERFACE_REFCOUNT;

	//bCleanSurface indicate whether the surfaces should be cleaned,if FALSE,only the 
	//content(the patches) in surfaces will be cleaned
	//NOTE: the internal mem pool's architecture will NOT be modified
	virtual void Reset(BOOL bCleanSurface=FALSE)=0;

	//PatchPossibleSize is used to define the internal mem pool's architecture
	//in PatchPossibleSize you could transfer all the sizes that a patch could possibly be,
	//the size include both the vertex and index data,in byte
	virtual void Reset(std::vector<DWORD>*PatchPossibleSize,BOOL bCleanSurface=FALSE)=0;

	//maxvtx is in vertex,maxidx is in index
	//bMultipleVB indicates whether more than 1 VB could be used to contain all
	//the vertex/index if their count exceeds maxvtx/maxidx.if FALSE,the internal vb will be
	//re-allocated with a big enough size ,instead of aqcuiring more vb
	//NOTE that if bMultipleVB is FALSE,you could at most add 64k vertex into this surf
	//dpt=4: D3DPT_TRIANGLELIST
	virtual PGSurfHandle AddSurf(FVFEx fvf,DWORD maxvtx,DWORD maxidx,
											BOOL bMultipleVB=TRUE,DWORD dpt=4)=0;

	virtual BOOL RemoveSurf(PGSurfHandle surf)=0;
	//group should be within MAX_PG_PATCH_GROUP
	virtual PGPatchHandle AddPatch(PGSurfHandle surf,void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group)=0;//return 0 if fail
	virtual BOOL RemovePatch(PGPatchHandle patch)=0;
	virtual BOOL RemoveAllPatches(PGSurfHandle surf)=0;//clean all the patches in the surf
	//IMPORTANT:the patch passed in is not garranteed valid after calling this function,
	//you need replace it with the returned one
	virtual PGPatchHandle UpdatePatch(PGPatchHandle patch,void *vertice,DWORD nVtx,WORD *indice,DWORD nIdx,DWORD group)=0;
	//IMPORTANT:the patch passed in is not garranteed valid after calling this function,
	//you need update it with the returned handle
	virtual PGPatchHandle AssignPatch(PGPatchHandle patch,PGSurfHandle surf)=0;
	//set the group for the patch
	virtual BOOL SetPatchGroup(PGPatchHandle patch,DWORD group)=0;

	virtual DWORD GetVBCount(PGSurfHandle surf)=0;
	virtual VBHandles GetVB(PGSurfHandle surf,DWORD idx,DWORD &ps,DWORD &pc,DWORD group)=0;
	virtual FVFEx GetFVF(PGSurfHandle surf)=0;

	virtual BOOL DrawSurf(IRenderer *rdr,PGSurfHandle surf,DWORD group=0,BOOL bWireframe=FALSE)=0;

	virtual void CollectStats(PGStat &stat)=0;


};

struct PQDrawArg
{
	PQDrawArg()
	{
		primstart=-1;
		primcount=-1;
	}
	int primstart;
	int primcount;
};

class IShader;
class IPatchQueue
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Reset(FVFEx fvf,DWORD maxvertex=0xffff,
												DWORD maxindice=0xffff*3)=0;//if maxindice is 0,do not use index buffer

	virtual FVFEx GetFVF()=0;

	virtual PQPatch AddPatch(void *vertice,DWORD nVtx,
							WORD *indice,DWORD nIdx,DWORD primtype=4)=0;//return 0 if fail,
																													//4 is D3DPT_TRIANGLELIST
	virtual void RemovePatch(PQPatch patch)=0;
	virtual void RemoveAllPatches()=0;//clean all the patches
	//IMPORTANT:the patch passed in is not garranteed valid after calling this function,
	//you need replace it with the returned one
	virtual PQPatch UpdatePatch(PQPatch patch,void *vertice,DWORD nVtx,
								WORD *indice,DWORD nIdx,DWORD primtype=4)=0;

	virtual BOOL BindPatch(IShader *shader,PQPatch pa)=0;

};

class IShader;
struct VBBindArg;

class IPatchBuilder
{
public:
	enum Result{
		Bad,
		Overflow,
		Success,		
	};

	virtual BOOL Begin(const FVFEx & fvf) = 0;
	
	virtual BOOL Append(DWORD nVtx,DWORD nIB,const void * pVBData,const WORD * pIBData) = 0;

	virtual BOOL Append(DWORD nVtx,DWORD nIB,const WORD * pIBData,void *&pVtxArray) = 0;
	
	virtual Result GetResult() = 0;

	virtual void Clear() = 0;

	virtual void End(void) = 0;
	
	virtual BOOL BindPatch(IShader * shader,VBBindArg * arg = NULL) = 0;
};



