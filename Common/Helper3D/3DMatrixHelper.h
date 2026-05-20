//--------------------------------------------------------------------------------------
// File: D3D9Enum.h
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>

#include <vector>

#include "..\D3DBase\D3DBaseType.h"

struct WorldMatrixEnv
{
	D3DXMATRIX m_matBase;//The first mat supplied to modify the whole entity's PRS
	D3DXMATRIX m_matFrame;//Current frame matrix(including all the parent mat & m_matBase)
	std::vector<D3DXMATRIX> m_aWorkingMats;//only for used during the HT3D_SetWorldMatrix hook processing
	BOOL m_bBoneMatrix;
	void StartWorking()
	{
		m_bBoneMatrix=FALSE;
		m_aWorkingMats.clear();
	}
};

class C3DHelper_Matrix
{
public:
	C3DHelper_Matrix()
	{
	}

	BOOL IsProjMatOrtho(D3DXMATRIX &mat);

	void ResetMatrixTranslation(D3DXMATRIX &mat);//reset the translation part of the matrix to (0,0,0)

	void TransformBoundaryCube(BoundaryCube &bc,D3DXMATRIX &mat);

	void InverseTransformHitTestProbe(HitTestProbe &probe,D3DXMATRIX &mat);

	void BuildHitTestProbe(HitTestProbe &probe,int x,int y,D3DViewport &viewport,D3DXMATRIX &matView,D3DXMATRIX &matProj);

	void Calc2DPointFrom3DPoint(std::vector<D3DXVECTOR2> &aPoints,std::vector<D3DVECTOR> &a3DPoints,D3DViewport &viewport,D3DXMATRIX &matView,D3DXMATRIX &matProj);

	void CameraPosFromMatView(D3DXVECTOR3 &posCamera,D3DXMATRIX &matView);


};

