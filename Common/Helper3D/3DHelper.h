//--------------------------------------------------------------------------------------
// File: D3D9Enum.h
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once
#ifndef D3D9_3DHELPER_H
#define D3D9_3DHELPER_H

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <vector>

#include "..\D3DBase\D3DBaseType.h"

#define HELPERFVF_DEFAULT (D3DFVF_DIFFUSE|D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

class C3DHelper_Vertex
{
	std::vector<BYTE> m_aHelpVertice;
	std::vector<WORD>m_aIndice16;
	std::vector<DWORD>m_aIndice32;
	FVFEx m_FVF;
	BOOL m_bConvertedToIndice;
public:
	C3DHelper_Vertex()
	{
		Reset();
	}
	void Reset(BOOL bKeepFVF=FALSE);
	void ClearVertice();
	BOOL AddVertice(BYTE *pVerticeData,int nVerticeCount,D3DMATRIX *pMatrix=NULL);//if dwFVF is 0,keep the original fvf;
	int GetVertice(BYTE *pVerticeData);//if pVerticeData is NULL,return the buffer size(in vertex count) to contain the vertice data,if not,return the vertice count
	int GetVerticePointer(BYTE *&pVerticeData);//return the vertice count
	BOOL ConvertToIndexed();//Combine the same vertice and build a index list
	int GetVerticeIndice16(BYTE *pVerticeIndiceData);//if pVerticeIndiceData is NULL,return the buffer size(in byte) to contain all the data,otherwise,return the indice count(16 bit index value)
	int GetVerticeIndice32(BYTE *pVerticeIndiceData);//if pVerticeIndiceData is NULL,return the buffer size(in byte) to contain all the data,otherwise,return the indice count(32 bit index value)

	void SetFVF(FVFEx dwFVF);//Set default fvf,the initial value is HELPERFVF_DEFAULT.Should be a valid fvf,otherwise this function takes no effect
	FVFEx GetFVF();//Set default fvf,the initial value is HELPERFVF_DEFAULT
	BOOL CheckFVF(FVFEx fvf);
	DWORD SizeOfFVF(FVFEx fvf);
	D3DFVF D3DFVFFromFVF(FVFEx fvf);
	D3DVECTOR *XYZFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	float *XYZRHWFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	float *XYZWFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	float *WeightFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	D3DVECTOR *NormalFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	float *PSizeFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	D3DCOLOR *DiffuseFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	D3DCOLOR *SpecularFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	float *TexCoordFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	DWORD *BoneIndicesFromFVFVertex(BYTE *pVertex,FVFEx fvf);
	BYTE *PointerFromFVFVertex(BYTE *pVertex,FVFEx fvfVertex,FVFEx fvfToSearch);
	FVFEx GetFirstFVF(FVFEx fvf);
	FVFEx GetFirstXYZ(FVFEx fvf);
	FVFEx GetFirstNormal(FVFEx fvf);
	FVFEx GetFirstTex(FVFEx fvf);
	FVFEx GetFirstVox(FVFEx fvf);
	FVFEx GetFirstLix(FVFEx fvf);
	BOOL CheckFVFRefConsistency(FVFEx fvf1,FVFEx fvf2);//whether the 2 fvf could referencing each other
	//fvfSub is part of fvf,this function will find its corresponding part within fvfRefTarget
	FVFEx GetRefCorrespondingFVF(FVFEx fvf,FVFEx fvfRefTarget,FVFEx fvfSub);
	int D3DVERTEXELEMENT9FromFVF(D3DVERTEXELEMENT9 *pElements,FVFEx fvf,int iStream);
	void CopyFVFData(DWORD nVertice,void *pDest,FVFEx fvfDest,void *pSrc,FVFEx fvfSrc);
	void CopyFVFDataByStride(DWORD nVertice,FVFEx fvfSrc,void *pDest,DWORD nStrideDest,void *pSrc,DWORD nStrideSrc);

	BOOL GenSampleCube(float rate=1.0,D3DCOLOR color=0xffffffff);//Generate a cube(in set fvf)
	BOOL CalcLookAtCamera(D3DMATRIX &matView,D3DMATRIX &matProj,int wCameraView,int hCameraView);//Use a default (suitable) direction/distance to look at the target vertices
	BOOL CalcLightPosDir(D3DVECTOR &pos,D3DVECTOR &dir,int iSet=0);//Use a light at default (suitable) pos/dir to enlighten the target vertices

	BOOL BuildNormal();//if the vertex format(FVF) doesnot contain normal slot,return false;build the normal perpandical to the face of each vertex.So ,if the vertex count is not a multiply of 3,return false
	BOOL BuildNormalSmooth(BYTE *pPos,DWORD nStridePos,BYTE *pNormal,DWORD nStrideNormal,DWORD nVertex,WORD *pIndice,DWORD nIndice);

	void BoundaryCubeFromBoundaryBox(BoundaryCube *p,BoundaryBox *q);
	void BoundaryBoxFromBoundaryCube(BoundaryBox *q,BoundaryCube *p);
	DWORD GetBoundaryCubeFaces(WORD *&pIndice);//return the indice count,the returned pointer should not be released by the caller

private:
	int CalcTextCoordSize(FVFEx fvf,int nSets=-1);//calculate the size of the texture coord data ,in byte.nSet indicate how many set of the coord should be calculated,if -1,all of the coord should be calculated
};

#endif
