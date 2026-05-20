//--------------------------------------------------------------------------------------
// File: D3D9Enum.cpp
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "..\D3DBASE\D3DBaseType.h"
#include "3DHelper.h"

FVFExInfo g_aFVFList[]=
{
	FVFExInfo(FVFEX_XYZ0,				D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_POSITION,  0),
	FVFExInfo(FVFEX_XYZ1,				D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_POSITION,  1),
	FVFExInfo(FVFEX_XYZ2,				 D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_POSITION,  2),
	FVFExInfo(FVFEX_XYZ3,				 D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_POSITION,  3),
	FVFExInfo(FVFEX_XYZRHW,		 D3DDECLTYPE_FLOAT4 ,			16,		D3DDECLUSAGE_POSITIONT,  0),
	FVFExInfo(FVFEX_XYZW,         D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_POSITIONT,  0),
	//	FVFExInfo(FVFEX_WEIGHT0,		D3DDECLTYPE_FLOAT1,				4,			D3DDECLUSAGE_BLENDWEIGHT,  0),
	FVFExInfo(FVFEX_WEIGHT01,		 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_BLENDWEIGHT,  0),
	FVFExInfo(FVFEX_WEIGHT012,		D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_BLENDWEIGHT,  0),
	FVFExInfo(FVFEX_WEIGHT0123,		D3DDECLTYPE_FLOAT3,				12,			D3DDECLUSAGE_BLENDWEIGHT,  0),
	FVFExInfo(FVFEX_BONEINDICE,		D3DDECLTYPE_D3DCOLOR,				4,			D3DDECLUSAGE_BLENDINDICES,  0),
	FVFExInfo(FVFEX_NORMAL0,		 D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_NORMAL,  0),
	FVFExInfo(FVFEX_NORMAL1,     D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_NORMAL,  1),
	FVFExInfo(FVFEX_NORMAL2,		 D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_NORMAL,  2),
	FVFExInfo(FVFEX_NORMAL3,		 D3DDECLTYPE_FLOAT3 ,			12,		D3DDECLUSAGE_NORMAL,  3),
	FVFExInfo(FVFEX_PSIZE,         D3DDECLTYPE_FLOAT1 ,			4,			D3DDECLUSAGE_PSIZE,  0),
	FVFExInfo(FVFEX_DIFFUSE,		 D3DDECLTYPE_D3DCOLOR,		4,			D3DDECLUSAGE_COLOR,  0),
	FVFExInfo(FVFEX_SPECULAR,		 D3DDECLTYPE_D3DCOLOR,		4,			D3DDECLUSAGE_COLOR,  1),
	FVFExInfo(FVFEX_FLAG_TEX0,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  0),
	FVFExInfo(FVFEX_FLAG_TEX1,	 D3DDECLTYPE_FLOAT2,			8,			D3DDECLUSAGE_TEXCOORD,  1),
	FVFExInfo(FVFEX_FLAG_TEX2,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  2),
	FVFExInfo(FVFEX_FLAG_TEX3,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  3),
	FVFExInfo(FVFEX_FLAG_TEX4,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  4),
	FVFExInfo(FVFEX_FLAG_TEX5,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  5),
	FVFExInfo(FVFEX_FLAG_TEX6,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  6),
	FVFExInfo(FVFEX_FLAG_TEX7,	D3DDECLTYPE_FLOAT2,				8,			D3DDECLUSAGE_TEXCOORD,  7),
	FVFExInfo(FVFEX_FLAG_VOX0,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  0),
	FVFExInfo(FVFEX_FLAG_VOX1,	D3DDECLTYPE_FLOAT3,				12,		D3DDECLUSAGE_TEXCOORD,  1),
	FVFExInfo(FVFEX_FLAG_VOX2,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  2),
	FVFExInfo(FVFEX_FLAG_VOX3,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  3),
	FVFExInfo(FVFEX_FLAG_VOX4,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  4),
	FVFExInfo(FVFEX_FLAG_VOX5,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  5),
	FVFExInfo(FVFEX_FLAG_VOX6,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  6),
	FVFExInfo(FVFEX_FLAG_VOX7,	 D3DDECLTYPE_FLOAT3,			12,		D3DDECLUSAGE_TEXCOORD,  7),
	FVFExInfo(FVFEX_FLAG_LIX0,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  0),
	FVFExInfo(FVFEX_FLAG_LIX1,   D3DDECLTYPE_FLOAT1,				4,			D3DDECLUSAGE_TEXCOORD,  1),
	FVFExInfo(FVFEX_FLAG_LIX2,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  2),
	FVFExInfo(FVFEX_FLAG_LIX3,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  3),
	FVFExInfo(FVFEX_FLAG_LIX4,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  4),
	FVFExInfo(FVFEX_FLAG_LIX5,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  5),
	FVFExInfo(FVFEX_FLAG_LIX6,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  6),
	FVFExInfo(FVFEX_FLAG_LIX7,	 D3DDECLTYPE_FLOAT1,			4,			D3DDECLUSAGE_TEXCOORD,  7),
};

int g_SizeOfFVFList=sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);




void C3DHelper_Vertex::Reset(BOOL bKeepFVF)
{
	if (!bKeepFVF)
		m_FVF=HELPERFVF_DEFAULT;
	ClearVertice();
}
void C3DHelper_Vertex::ClearVertice()
{
	m_aHelpVertice.clear();
	m_aIndice16.clear();
	m_aIndice32.clear();

	m_bConvertedToIndice=FALSE;
}

//vertice format should be the default fvf(use SetFVF() to set the default FVF)
//pMatrix is the matrix the vertice will multiply before adding them to the internal buffer,
BOOL C3DHelper_Vertex::AddVertice(BYTE *pVerticeData,int nVerticeCount,D3DMATRIX *pMatrix)
{
	if (m_bConvertedToIndice)
		return FALSE;
	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return FALSE;
	if (nVerticeCount<=0)
		return FALSE;

	if (!pMatrix)
	{
		int szTotal;
		szTotal=szFVF*nVerticeCount;
		int szOrg;
		szOrg=m_aHelpVertice.size();
		m_aHelpVertice.resize(szOrg+szTotal);
		memcpy(&m_aHelpVertice[szOrg],pVerticeData,szTotal);
		return TRUE;
	}
	else
	{
		//Add one by one
		BYTE *p;
		p=pVerticeData;

		int i;
		for (i=0;i<nVerticeCount;i++)
		{
			D3DVECTOR *q;
			q=XYZFromFVFVertex(p,m_FVF);
			if (q)
			{
				D3DXVECTOR4 v(q->x,q->y,q->z,1);
				D3DXVECTOR4 v2;
				D3DXMATRIX mat(*pMatrix);
				D3DXVec4Transform(&v2,(const D3DXVECTOR4*)&v,&mat);
				q->x=v2.x;q->y=v2.y;q->z=v2.z;
			}
			q=NormalFromFVFVertex(p,m_FVF);
			if (q)
			{
				D3DXVECTOR4 v(q->x,q->y,q->z,1);
				D3DXVECTOR4 v2;
				D3DXMATRIX mat(*pMatrix);
				D3DXVec4Transform(&v2,(const D3DXVECTOR4*)&v,&mat);
				q->x=v2.x;q->y=v2.y;q->z=v2.z;
			}

			//add to the buffer
			if (TRUE)
			{
				int szTotal;
				szTotal=szFVF;
				int szOrg;
				szOrg=m_aHelpVertice.size();
				m_aHelpVertice.resize(szOrg+szTotal);
				memcpy(&m_aHelpVertice[szOrg],p,szTotal);
			}
		}
	}

	return TRUE;
}

void C3DHelper_Vertex::SetFVF(FVFEx dwFVF)//Set default fvf,the initial value is HELPERFVF_DEFAULT.Should be a valid fvf,otherwise this function takes no effect
{
	m_FVF=dwFVF;
}
FVFEx C3DHelper_Vertex::GetFVF()
{
	return m_FVF;
}

D3DFVF C3DHelper_Vertex::D3DFVFFromFVF(FVFEx fvfEx)
{
	return 0;
}

int C3DHelper_Vertex::CalcTextCoordSize(FVFEx fvf,int nSets)//calculate the size of the texture coord data ,in byte.nSet indicate how many set of the coord should be calculated,if 0,all of the coord should be calculated
{
	int i;
	int count;
	count=0;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (nSets==0)
			break;
		int delta;
		delta=0;
		if ((fvf&FVFEX_FLAG_VOX(i))==FVFEX_FLAG_VOX(i))
			delta=3;
		else
			if ((fvf&FVFEX_FLAG_LIX(i))==FVFEX_FLAG_LIX(i))
				delta=1;
			else
				if ((fvf&FVFEX_FLAG_TEX(i))==FVFEX_FLAG_TEX(i))
					delta=2;

		if (delta>0)
			nSets--;

		count+=delta;
	}

	return count*sizeof(float);
}

BOOL C3DHelper_Vertex::CheckFVF(FVFEx fvf)
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


DWORD C3DHelper_Vertex::SizeOfFVF(FVFEx fvfTotal)
{
	if (!CheckFVF(fvfTotal))
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

D3DVECTOR *C3DHelper_Vertex::XYZFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (D3DVECTOR *)PointerFromFVFVertex(pVertex,fvf,FVFEX_XYZ0);
}

float *C3DHelper_Vertex::XYZRHWFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (float *)PointerFromFVFVertex(pVertex,fvf,FVFEX_XYZRHW);
}

float *C3DHelper_Vertex::XYZWFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (float *)PointerFromFVFVertex(pVertex,fvf,FVFEX_XYZW);
}

float *C3DHelper_Vertex::WeightFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (float *)PointerFromFVFVertex(pVertex,fvf,FVFEX_WEIGHT0);
}

DWORD *C3DHelper_Vertex::BoneIndicesFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (DWORD*)PointerFromFVFVertex(pVertex,fvf,FVFEX_BONEINDICE);
}



D3DVECTOR *C3DHelper_Vertex::NormalFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (D3DVECTOR *)PointerFromFVFVertex(pVertex,fvf,FVFEX_NORMAL0);
}

float *C3DHelper_Vertex::PSizeFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (float *)PointerFromFVFVertex(pVertex,fvf,FVFEX_PSIZE);
}

D3DCOLOR *C3DHelper_Vertex::DiffuseFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (D3DCOLOR*)PointerFromFVFVertex(pVertex,fvf,FVFEX_DIFFUSE);
}

D3DCOLOR *C3DHelper_Vertex::SpecularFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (D3DCOLOR*)PointerFromFVFVertex(pVertex,fvf,FVFEX_SPECULAR);
}

float *C3DHelper_Vertex::TexCoordFromFVFVertex(BYTE *pVertex,FVFEx fvf)
{
	return (float *)PointerFromFVFVertex(pVertex,fvf,FVFEX_FLAG_TEX0);
}

BYTE *C3DHelper_Vertex::PointerFromFVFVertex(BYTE *pVertex,FVFEx fvfVertex,FVFEx fvfToSearch)
{
	if (!CheckFVF(fvfVertex))
		return NULL;
	if (!CheckFVF(fvfToSearch))
		return NULL;
	BYTE *pReturn;
	pReturn=pVertex;
	int i;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		FVFEx fvf;
		fvf=g_aFVFList[i].m_fvf;
		if ((fvf&fvfToSearch)==fvf)
		{
			if ((fvf&fvfVertex)!=fvf)
				return NULL;
			return pReturn;
		}
		else
		{
			if ((fvf&fvfVertex)==fvf)
				pReturn+=g_aFVFList[i].m_size;
		}
	}

	return NULL;//Not in it

	return NULL;

}


FVFEx C3DHelper_Vertex::GetFirstXYZ(FVFEx fvf)
{
	FVFEx a[]=
	{
		FVFEX_XYZ0,
		FVFEX_XYZ1,
		FVFEX_XYZ2,
		FVFEX_XYZ3,
	};
	int i;
	for (i=0;i<sizeof(a)/sizeof(FVFEx);i++)
	{
		if (fvf&a[i])
			return a[i];
	}
	return FVFEX_NULL;
}
FVFEx C3DHelper_Vertex::GetFirstNormal(FVFEx fvf)
{
	FVFEx a[]=
	{
		FVFEX_NORMAL0,
		FVFEX_NORMAL1,
		FVFEX_NORMAL2,
		FVFEX_NORMAL3,
	};
	int i;
	for (i=0;i<sizeof(a)/sizeof(FVFEx);i++)
	{
		if (fvf&a[i])
			return a[i];
	}
	return FVFEX_NULL;

}
FVFEx C3DHelper_Vertex::GetFirstTex(FVFEx fvf)
{
	
	int i;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (fvf&FVFEX_FLAG_TEX(i))
		{
			if ((fvf&FVFEX_FLAG_VOX(i))==FVFEX_FLAG_VOX(i))
				continue;
			if ((fvf&FVFEX_FLAG_LIX(i))==FVFEX_FLAG_VOX(i))
				continue;
			return FVFEX_FLAG_TEX(i);
		}
	}
	return FVFEX_NULL;

}
FVFEx C3DHelper_Vertex::GetFirstVox(FVFEx fvf)
{
	int i;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (fvf&FVFEX_FLAG_VOX(i))
			return FVFEX_FLAG_VOX(i);
	}
	return FVFEX_NULL;

}
FVFEx C3DHelper_Vertex::GetFirstLix(FVFEx fvf)
{
	int i;
	for (i=0;i<FVFEX_TEX_MAXSET;i++)
	{
		if (fvf&FVFEX_FLAG_LIX(i))
			return FVFEX_FLAG_LIX(i);
	}
	return FVFEX_NULL;

}

FVFEx C3DHelper_Vertex::GetFirstFVF(FVFEx fvf)
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

int C3DHelper_Vertex::D3DVERTEXELEMENT9FromFVF(D3DVERTEXELEMENT9 *pElements,FVFEx fvfTotal,int iStream)
{
	if (fvfTotal==FVFEX_NULL)
		return FVFEX_NULL;

	FVFEx fvfTemp;
	fvfTemp=fvfTotal;

	DWORD count;
	count=0;
	int i;
	for (i=0;i<sizeof(g_aFVFList)/sizeof(g_aFVFList[0]);i++)
	{
		if ((fvfTotal&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
		{
			FVFEx fvf;
			fvf=g_aFVFList[i].m_fvf;

			D3DVERTEXELEMENT9 *p;
			p=&(pElements[count]);
			count++;

			BYTE buffer[256];
			p->Stream=iStream;
			p->Offset=PointerFromFVFVertex((BYTE *)buffer,fvfTemp,fvf)-(BYTE *)buffer;
			p->Method=D3DDECLMETHOD_DEFAULT;
			p->Type=g_aFVFList[i].m_type;
			p->Usage=g_aFVFList[i].m_usage;
			p->UsageIndex=g_aFVFList[i].m_usageindex;

			fvfTotal&=(~fvf);
			if (fvfTotal==FVFEX_NULL)
				return count;
		}

	}

	return count;


}

BOOL C3DHelper_Vertex::CheckFVFRefConsistency(FVFEx fvf1,FVFEx fvf2)
{
	int i;
	for (i=0;i<5;i++)
	{
		while(1)
		{
			FVFEx v1,v2;
			switch(i)
			{
			case 0:
				v1=GetFirstXYZ(fvf1);
				v2=GetFirstXYZ(fvf2);
				break;
			case 1:
				v1=GetFirstNormal(fvf1);
				v2=GetFirstNormal(fvf2);
				break;
			case 2:
				v1=GetFirstTex(fvf1);
				v2=GetFirstTex(fvf2);
				break;
			case 3:
				v1=GetFirstVox(fvf1);
				v2=GetFirstVox(fvf2);
				break;
			case 4:
				v1=GetFirstLix(fvf1);
				v2=GetFirstLix(fvf2);
				break;
			default:
				ASSERT(FALSE);
			}
			if ((v1!=FVFEX_NULL)&&	(v2!=FVFEX_NULL))
			{
				fvf1&=~v1;
				fvf2&=~v2;
				continue;
			}
			if ((v1==FVFEX_NULL)&&(v2==FVFEX_NULL))
				break;
			return FALSE;//Not consistency
		}
	}

	if ((fvf1!=FVFEX_NULL)||(fvf2!=FVFEX_NULL))
		return FALSE;

	return TRUE;

}

//fvfSub is part of fvf,this function will find its corresponding part within fvfRefTarget
FVFEx C3DHelper_Vertex::GetRefCorrespondingFVF(FVFEx fvf,FVFEx fvfRefTarget,FVFEx fvfSub)
{

	FVFEx fvf1,fvf2,fvfNew;
	fvfNew=FVFEX_NULL;

	int i;
	for (i=0;i<5;i++)
	{
		while(1)
		{
			switch(i)
			{
			case 0:
				fvf1=GetFirstXYZ(fvf);
				fvf2=GetFirstXYZ(fvfRefTarget);
				break;
			case 1:
				fvf1=GetFirstNormal(fvf);
				fvf2=GetFirstNormal(fvfRefTarget);
				break;
			case 2:
				fvf1=GetFirstTex(fvf);
				fvf2=GetFirstTex(fvfRefTarget);
				break;
			case 3:
				fvf1=GetFirstVox(fvf);
				fvf2=GetFirstVox(fvfRefTarget);
				break;
			case 4:
				fvf1=GetFirstLix(fvf);
				fvf2=GetFirstLix(fvfRefTarget);
				break;
			default:
				ASSERT(FALSE);
			}

			if (fvf1==FVFEX_NULL)
			{
				ASSERT(fvf2==FVFEX_NULL);
				break;
			}
			if (fvfSub&fvf1)
				fvfNew|=fvf2;

			fvf&=~fvf1;
			fvfRefTarget&=~fvf2;
		}
	}

	return fvfNew;

}

void C3DHelper_Vertex::CopyFVFData(DWORD nVertice,void *pDest,FVFEx fvfDest,void *pSrc,FVFEx fvfSrc)
{

	int i,n;
	n=g_SizeOfFVFList;

	DWORD szDest,szSrc;
	szDest=SizeOfFVF(fvfDest);
	szSrc=SizeOfFVF(fvfSrc);
	FVFEx fvfSrc0;
	fvfSrc0=fvfSrc;

	for (i=0;i<n;i++)
	{
		if ((fvfSrc&g_aFVFList[i].m_fvf)==g_aFVFList[i].m_fvf)
		{
			BYTE *p,*q;
			p=PointerFromFVFVertex((BYTE*)pDest,fvfDest,g_aFVFList[i].m_fvf);
			q=PointerFromFVFVertex((BYTE*)pSrc,fvfSrc0,g_aFVFList[i].m_fvf);

			int k;
			for (k=0;k<nVertice;k++)
			{
				memcpy(p,q,g_aFVFList[i].m_size);
				p+=szDest;
				q+=szSrc;
			}

			fvfSrc&=~g_aFVFList[i].m_fvf;

			if (fvfSrc==FVFEX_NULL)
				return;
		}
	}
}

void C3DHelper_Vertex::CopyFVFDataByStride(DWORD nVertice,FVFEx fvfSrc,void *pDest,DWORD nStrideDest,void *pSrc,DWORD nStrideSrc)
{
	DWORD szSrc;
	szSrc=SizeOfFVF(fvfSrc);
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




//if pVerticeData is NULL,return the buffer size(in vertex count) to contain the vertice data,if not,return the vertice count
int C3DHelper_Vertex::GetVertice(BYTE *pVerticeData)
{
	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return 0;

	if (!pVerticeData)
		return m_aHelpVertice.size()/szFVF;
	else
	{
		memcpy(pVerticeData,&m_aHelpVertice[0],m_aHelpVertice.size());
		return m_aHelpVertice.size()/szFVF;
	}
}

//return the vertice count
//the return pointer should not be kept for later use
int C3DHelper_Vertex::GetVerticePointer(BYTE *&pVerticeData)
{
	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return 0;

	if (m_aHelpVertice.size()<=0)
		return 0;

	pVerticeData=&m_aHelpVertice[0];
	return m_aHelpVertice.size()/szFVF;
	
}





//if the vertex format(FVF) doesnot contain normal slot,return falsebuild the normal perpandical to the face of each vertex.So ,if the vertex count is not a multiply of 3,return false
BOOL C3DHelper_Vertex::BuildNormal()
{
	if (m_bConvertedToIndice)
		return FALSE;

	if (TRUE)//See whether fvf contain a normal slot
	{
		BYTE buffer[256];
		if (!NormalFromFVFVertex(buffer,m_FVF))
			return FALSE;
		if (!XYZFromFVFVertex(buffer,m_FVF))
			return FALSE;
	}

	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return FALSE;


	int nVertice;
	nVertice=m_aHelpVertice.size()/szFVF;

	if (nVertice%3!=0)
		return FALSE;

	int nFaces;
	nFaces=nVertice/3;

	int i;
	for (i=0;i<nFaces;i++)
	{
		D3DXVECTOR3 *p1,*p2,*p3;
		p1=(D3DXVECTOR3*)XYZFromFVFVertex(&m_aHelpVertice[(i*3+0)*szFVF],m_FVF);
		p2=(D3DXVECTOR3*)XYZFromFVFVertex(&m_aHelpVertice[(i*3+1)*szFVF],m_FVF);
		p3=(D3DXVECTOR3*)XYZFromFVFVertex(&m_aHelpVertice[(i*3+2)*szFVF],m_FVF);

		D3DXVECTOR3 v1,v2,vNormal,vNormal2;
		v1=*p3-*p1;
		v2=*p2-*p1;

		D3DXVec3Cross(&vNormal,&v2,&v1);
		D3DXVec3Normalize(&vNormal2,&vNormal);

		p1=(D3DXVECTOR3*)NormalFromFVFVertex(&m_aHelpVertice[(i*3+0)*szFVF],m_FVF);
		p2=(D3DXVECTOR3*)NormalFromFVFVertex(&m_aHelpVertice[(i*3+1)*szFVF],m_FVF);
		p3=(D3DXVECTOR3*)NormalFromFVFVertex(&m_aHelpVertice[(i*3+2)*szFVF],m_FVF);

		*p1=vNormal2;
		*p2=vNormal2;
		*p3=vNormal2;
	}

	return TRUE;
}

//nStridePos & nStrideNormal are both in byte
//nIndice should be multiple of 3
BOOL C3DHelper_Vertex::BuildNormalSmooth(BYTE *pPos,DWORD nStridePos,BYTE *pNormal,DWORD nStrideNormal,DWORD nVertex,WORD *pIndice,DWORD nIndice)
{
	int i;

	if (TRUE)//First clear all the normals to (0.0,0.0,0.0)
	{
		for (i=0;i<nVertex;i++)
		{
			D3DXVECTOR3 *q;
			q=(D3DXVECTOR3*)(pNormal+i*nStrideNormal);

			q->x=q->y=q->z=0.0;
		}
	}

	for (i=0;i<nIndice;i+=3)
	{
		D3DXVECTOR3 *p1,*p2,*p3;
		p1=(D3DXVECTOR3*)(pPos+pIndice[i]*nStridePos);
		p2=(D3DXVECTOR3*)(pPos+pIndice[i+1]*nStridePos);
		p3=(D3DXVECTOR3*)(pPos+pIndice[i+2]*nStridePos);

		D3DXVECTOR3 vNormal;
		if (TRUE)
		{
			D3DXVECTOR3 v1,v2,vNormal2;
			v1=*p3-*p1;
			v2=*p2-*p1; 

			D3DXVec3Cross(&vNormal2,&v2,&v1);
			D3DXVec3Normalize(&vNormal,&vNormal2);
		}

		D3DXVECTOR3 *q1,*q2,*q3;
		q1=(D3DXVECTOR3*)(pNormal+pIndice[i]*nStrideNormal);
		q2=(D3DXVECTOR3*)(pNormal+pIndice[i+1]*nStrideNormal);
		q3=(D3DXVECTOR3*)(pNormal+pIndice[i+2]*nStrideNormal);

		(*q1)+=vNormal;
		(*q2)+=vNormal;
		(*q3)+=vNormal;

	}

	for (i=0;i<nVertex;i++)
	{
		D3DXVECTOR3 *q;
		q=(D3DXVECTOR3*)(pNormal+i*nStrideNormal);

		D3DXVec3Normalize(q,q);
	}

	return TRUE;

}



//Use a default (suitable) direction/distance to look at the target vertices
BOOL C3DHelper_Vertex::CalcLookAtCamera(D3DMATRIX &matView,D3DMATRIX &matProj,int wCameraView,int hCameraView)
{
	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return FALSE;

	int nVertice;
	nVertice=m_aHelpVertice.size()/szFVF;

	if (nVertice<=0)
		return FALSE;

	//First calculate a boundary sphere
	D3DXVECTOR3 posCenter;
	float radius;
	if (TRUE)
	{
		BYTE *p;
		p=&m_aHelpVertice[0];
		p=(BYTE*)XYZFromFVFVertex(p,m_FVF);
		if (!p)
			return FALSE;

		if (D3D_OK!=D3DXComputeBoundingSphere((const D3DXVECTOR3*)p,nVertice,szFVF,&posCenter,&radius))
			return FALSE;
	}

	float dist;
	dist=radius*2.0;

	D3DXVECTOR3 posEye;
	D3DXVECTOR3 posUp(0,1,0);

	posEye=posCenter;
	posEye.z-=dist;
	posEye.y-=dist;
	posEye.x-=dist;

	D3DXMatrixLookAtLH((D3DXMATRIX*)&matView,&posEye,&posCenter,&posUp);

	float fov,aspect,zn,zf;
	fov=1.00;
	aspect=(float)wCameraView/(float)hCameraView;

	zn=radius;
	zf=2*dist;

	D3DXMatrixOrthoLH((D3DXMATRIX*)&matProj,radius*4,radius*4/aspect,zn,zf);
//	D3DXMatrixPerspectiveFovLH((D3DXMATRIX*)&matProj,fov,aspect,zn,zf);

	return TRUE;
}

BOOL C3DHelper_Vertex::CalcLightPosDir(D3DVECTOR &posLight,D3DVECTOR &dirLight,int iSet)//Use a light at default (suitable) pos/dir to enlighten the target vertices
{
	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return FALSE;

	int nVertice;
	nVertice=m_aHelpVertice.size()/szFVF;

	if (nVertice<=0)
		return FALSE;

	//First calculate a boundary sphere
	D3DXVECTOR3 posCenter;
	float radius;
	if (TRUE)
	{
		BYTE *p;
		p=&m_aHelpVertice[0];
		p=(BYTE*)XYZFromFVFVertex(p,m_FVF);
		if (!p)
			return FALSE;

		if (D3D_OK!=D3DXComputeBoundingSphere((const D3DXVECTOR3*)p,nVertice,szFVF,&posCenter,&radius))
			return FALSE;
	}

	D3DXVECTOR3 pos,dir;
	pos=posCenter;

	pos.z-=radius*2;
	pos.x-=radius*2;
	pos.y-=radius*2;

	dir=posCenter-pos;

	D3DXVec3Normalize((D3DXVECTOR3*)&dirLight,&dir);

	posLight=(D3DVECTOR)pos;

	return TRUE;
}


BOOL C3DHelper_Vertex::GenSampleCube(float rate,D3DCOLOR color)//Generate a cube(in HELPERFVF_DEFAULT)
{
	Reset(TRUE);

	DWORD szFVF;
	szFVF=SizeOfFVF(m_FVF);
	if (szFVF==0)
		return FALSE;

	if (TRUE)//See whether fvf contains a normal/xyz slot
	{
		BYTE buffer[256];
		if (!NormalFromFVFVertex(buffer,m_FVF))
			return FALSE;
		if (!XYZFromFVFVertex(buffer,m_FVF))
			return FALSE;
	}

	//The 8 corners of the cube
	//	1.000000,1.000000,-1.000000,//0
	//	-1.000000,1.000000,-1.000000,//1
	//	-1.000000,1.000000,1.000000, //2
	//	1.000000,1.000000,1.000000,//3
	//	1.000000,-1.000000,-1.000000,//4
	//	-1.000000,-1.000000,-1.000000,//5
	//	-1.000000,-1.000000,1.000000,//6
	//	1.000000,-1.000000,1.000000,//7

	//Vertice
	float temp[36*3]=
	{
		1.000000,1.000000,-1.000000,//0
		-1.000000,1.000000,-1.000000,//1
		-1.000000,1.000000,1.000000, //2

		1.000000,1.000000,-1.000000,//0
		-1.000000,1.000000,1.000000, //2
		1.000000,1.000000,1.000000,//3

		1.000000,1.000000,-1.000000,//0
		1.000000,-1.000000,-1.000000,//4
		-1.000000,-1.000000,-1.000000,//5

		1.000000,1.000000,-1.000000,//0
		-1.000000,-1.000000,-1.000000,//5
		-1.000000,1.000000,-1.000000,//1

		-1.000000,1.000000,-1.000000,//1
		-1.000000,-1.000000,-1.000000,//5
		-1.000000,-1.000000,1.000000,//6

		-1.000000,1.000000,-1.000000,//1
		-1.000000,-1.000000,1.000000,//6
		-1.000000,1.000000,1.000000,//2

		-1.000000,1.000000,1.000000,//2
		-1.000000,-1.000000,1.000000,//6
		1.000000,-1.000000,1.000000,//7

		-1.000000,1.000000,1.000000,//2
		1.000000,-1.000000,1.000000,//7
		1.000000,1.000000,1.000000,//3

		1.000000,1.000000,1.000000,//3
		1.000000,-1.000000,1.000000,//7
		1.000000,-1.000000,-1.000000,//4

		1.000000,1.000000,1.000000,//3
		1.000000,-1.000000,-1.000000,//4
		1.000000,1.000000,-1.000000,//0

		1.000000,-1.000000,-1.000000,//4
		1.000000,-1.000000,1.000000,//7
		-1.000000,-1.000000,1.000000,//6

		1.000000,-1.000000,-1.000000,//4
		-1.000000,-1.000000,1.000000,//6
		-1.000000,-1.000000,-1.000000,//5
	};

	m_aHelpVertice.resize(36*szFVF);
	memset(&m_aHelpVertice[0],0,36*szFVF);//Clear

	BYTE *p;
	p=&m_aHelpVertice[0];

	int i;
	for (i=0;i<36;i++)
	{
		D3DVECTOR *q;
		q=XYZFromFVFVertex(p,m_FVF);

		q->x=temp[i*3+0]*rate;q->y=temp[i*3+1]*rate;q->z=temp[i*3+2]*rate;

		D3DCOLOR *r;
		r=DiffuseFromFVFVertex(p,m_FVF);
		if (r)
			*r=color;

		r=SpecularFromFVFVertex(p,m_FVF);
		if (r)
			*r=color;

		p+=szFVF;
	}

	if (FALSE==BuildNormal())
	{
		Reset(TRUE);
		return FALSE;
	}

	return TRUE;
}


BOOL C3DHelper_Vertex::ConvertToIndexed()//Combine the same vertice and build a index list
{
	return FALSE;//Not implemented yet.
}
int C3DHelper_Vertex::GetVerticeIndice16(BYTE *pVerticeIndiceData)//if pVerticeIndiceData is NULL,return the buffer size(in vertex count) to contain all the data,otherwise,return the indice count(16 bit index value)
{
	return 0;//Not implemented yet.
}
int C3DHelper_Vertex::GetVerticeIndice32(BYTE *pVerticeIndiceData)//if pVerticeIndiceData is NULL,return the buffer size(in vertex count) to contain all the data,otherwise,return the indice count(32 bit index value)
{
	return 0;//Not implemented yet.
}

void C3DHelper_Vertex::BoundaryBoxFromBoundaryCube(BoundaryBox *q,BoundaryCube *p)
{
	q->SetEmpty();
	if (p->m_bEmpty)
		return;

	int i;
	for (i=0;i<sizeof(p->m_aCubeCorners)/sizeof(p->m_aCubeCorners[0]);i++)
		q->Add(p->m_aCubeCorners[i]);
}

void C3DHelper_Vertex::BoundaryCubeFromBoundaryBox(BoundaryCube *p,BoundaryBox *q)
{
	if (q->Empty())
		p->m_bEmpty=TRUE;
	else
	{
		p->m_bEmpty=FALSE;
		p->m_aCubeCorners[0]=(D3DVECTOR)(D3DXVECTOR3(q->m_max.x,q->m_max.y,q->m_min.z));//1,1,0
		p->m_aCubeCorners[1]=(D3DVECTOR)(D3DXVECTOR3(q->m_min.x,q->m_max.y,q->m_min.z));//0,1,0
		p->m_aCubeCorners[2]=(D3DVECTOR)(D3DXVECTOR3(q->m_min.x,q->m_max.y,q->m_max.z));//0,1,1
		p->m_aCubeCorners[3]=(D3DVECTOR)(D3DXVECTOR3(q->m_max.x,q->m_max.y,q->m_max.z));//1,1,1
		p->m_aCubeCorners[4]=(D3DVECTOR)(D3DXVECTOR3(q->m_max.x,q->m_min.y,q->m_min.z));//1,0,0
		p->m_aCubeCorners[5]=(D3DVECTOR)(D3DXVECTOR3(q->m_min.x,q->m_min.y,q->m_min.z));//0,0,0
		p->m_aCubeCorners[6]=(D3DVECTOR)(D3DXVECTOR3(q->m_min.x,q->m_min.y,q->m_max.z));//0,0,1
		p->m_aCubeCorners[7]=(D3DVECTOR)(D3DXVECTOR3(q->m_max.x,q->m_min.y,q->m_max.z));//1,0,1
																								//	1.000000,1.000000,-1.000000,//0
																								//	-1.000000,1.000000,-1.000000,//1
																								//	-1.000000,1.000000,1.000000, //2
																								//	1.000000,1.000000,1.000000,//3
																								//	1.000000,-1.000000,-1.000000,//4
																								//	-1.000000,-1.000000,-1.000000,//5
																								//	-1.000000,-1.000000,1.000000,//6
																								//	1.000000,-1.000000,1.000000,//7

	}
}


//return the indice count,the returned pointer should not be released by the caller
DWORD C3DHelper_Vertex::GetBoundaryCubeFaces(WORD *&pIndice)
{
	static WORD s_aCubeFaceIndice[36]=
	{
		0,1,2,
		0,2,3,
		0,4,5,
		0,5,1,
		1,5,6,
		1,6,2,
		2,6,7,
		2,7,3,
		3,7,4,
		3,4,0,
		4,7,6,
		4,6,5,
	};

	pIndice=(WORD*)s_aCubeFaceIndice;

	return 36;
}