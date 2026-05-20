//--------------------------------------------------------------------------------------
// File: D3D9Enum.cpp
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "..\D3DBASE\D3DBaseType.h"
#include "3DMatrixHelper.h"

BOOL C3DHelper_Matrix::IsProjMatOrtho(D3DXMATRIX &mat)
{
	if (FLOAT_EQUAL(mat._44,1.0))
		return TRUE;

	return FALSE;
}


//reset the translation part of the matrix to (0,0,0)
void C3DHelper_Matrix::ResetMatrixTranslation(D3DXMATRIX &mat)
{
	D3DXVECTOR3 vecScale;
	D3DXQUATERNION qtnRotate;
	D3DXVECTOR3 vecTranslate;

	D3DXMatrixDecompose(&vecScale,&qtnRotate,&vecTranslate,&mat);

	qtnRotate.w=-qtnRotate.w;
	D3DXMatrixTransformation(&mat,NULL,NULL,&vecScale,NULL,&qtnRotate,NULL);
}

void C3DHelper_Matrix::BuildHitTestProbe(HitTestProbe &probe,int x,int y,D3DViewport &viewport,D3DXMATRIX &matView,D3DXMATRIX &matProj)
{
	D3DXVECTOR3 vecProjected0,vecProjected1;
	vecProjected0.x=(((float)x-(float)viewport.X)-(float)viewport.Width/2.0)/(float)viewport.Width*2.0;
	vecProjected0.y=(((float)viewport.Y+(float)viewport.Height-(float)y)-(float)viewport.Height/2.0)/(float)viewport.Height*2.0;
	vecProjected0.z=0.0;

	vecProjected1=vecProjected0;
	vecProjected1.z=1.0;

	if (IsProjMatOrtho(matProj))
	{
		probe.m_vOrg=vecProjected0;
		probe.m_vDir=vecProjected1-vecProjected0;

		D3DXMATRIX mat;
		D3DXMatrixMultiply(&mat,&matView,&matProj);

		InverseTransformHitTestProbe(probe,mat);
	}
	else
	{
		probe.m_vOrg=D3DXVECTOR3(0,0,0);
		probe.m_vDir.x=vecProjected0.x/matProj._11;
		probe.m_vDir.y=vecProjected0.y/matProj._22;
		probe.m_vDir.z=1.0;

		InverseTransformHitTestProbe(probe,matView);
	}
}


void C3DHelper_Matrix::InverseTransformHitTestProbe(HitTestProbe &probe,D3DXMATRIX &mat)
{
	D3DXMATRIX matInverse;
	if (NULL==D3DXMatrixInverse(&matInverse,NULL,&mat))
	{
		ASSERT(FALSE);
		return;
	}

	D3DXVECTOR3 vSrc,vDest;
	vSrc=(D3DXVECTOR3)probe.m_vOrg;
	vDest=(D3DXVECTOR3)probe.m_vDir;
	vDest=vSrc+vDest;

	D3DXVECTOR4 temp;

	D3DXVec3Transform(&temp,&vSrc,&matInverse);
	vSrc.x=temp.x;
	vSrc.y=temp.y;
	vSrc.z=temp.z;

	D3DXVec3Transform(&temp,&vDest,&matInverse);
	vDest.x=temp.x;
	vDest.y=temp.y;
	vDest.z=temp.z;

	probe.m_vOrg=(D3DVECTOR)vSrc;
	probe.m_vDir=(D3DVECTOR)(vDest-vSrc);
}


void C3DHelper_Matrix::TransformBoundaryCube(BoundaryCube &bc,D3DXMATRIX &mat)
{
	if (bc.m_bEmpty)
		return;

	int i;
	for (i=0;i<sizeof(bc.m_aCubeCorners)/sizeof(bc.m_aCubeCorners[0]);i++)
	{
		D3DXVECTOR4 t;
		D3DXVec3Transform(&t,(D3DXVECTOR3*)&bc.m_aCubeCorners[i],&mat);
		bc.m_aCubeCorners[i].x=t.x;
		bc.m_aCubeCorners[i].y=t.y;
		bc.m_aCubeCorners[i].z=t.z;
	}
}

void C3DHelper_Matrix::Calc2DPointFrom3DPoint(std::vector<D3DXVECTOR2> &aPoints,std::vector<D3DVECTOR> &a3DPoints,D3DViewport &viewport,D3DXMATRIX &matView,D3DXMATRIX &matProj)
{
	int sz;
	sz=a3DPoints.size();
	aPoints.resize(sz);

	D3DXMATRIX mat;
	mat=matView*matProj;

	int i;
	for (i=0;i<sz;i++)
	{
		D3DXVECTOR4 v;

		D3DXVec3Transform(&v,(D3DXVECTOR3*)&a3DPoints[i],&mat);

		v.x/=v.w;
		v.y/=v.w;
		v.z/=v.w;
//		D3DXVec4Normalize(&v,&v);

		aPoints[i].x=(float)viewport.X+(v.x-(-1.0))/2.0*(float)viewport.Width;
		aPoints[i].y=(float)viewport.Y+(1.0-v.y)/2.0*(float)viewport.Height;
	}


}


void C3DHelper_Matrix::CameraPosFromMatView(D3DXVECTOR3 &posCamera,D3DXMATRIX &matView)
{
	D3DXMATRIX mat;
	D3DXMatrixInverse(&mat,NULL,&matView);
	posCamera.x=mat._41;
	posCamera.y=mat._42;
	posCamera.z=mat._43;
}
