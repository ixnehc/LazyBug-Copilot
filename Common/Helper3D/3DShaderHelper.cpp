//--------------------------------------------------------------------------------------
// File: D3D9Enum.cpp
//
// Enumerates D3D adapters, devices, modes, etc.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "..\D3DBASE\D3DBaseType.h"
#include "3DShaderHelper.h"

ShaderType g_aShaderTypeList[]=
{
	ShaderType_Specular_1111,
	ShaderType_FixedFunctionHW,
	ShaderType_FixedFunctionSW,
};


//////////////////////////////////////////////////////////////////////////
//CShaderTypeDecider
CD3D9Base *CShaderTypeDecider::GetD3D9Base()
{
	return m_pD3D9Base;
}

void CShaderTypeDecider::SetAvailable(CD3D9Base *pD3D9Base,ShaderType shadertypes)
{
	m_pD3D9Base=pD3D9Base;
	m_available=shadertypes;
	ASSERT(m_available!=0);
}

//remove the element from pVec1,if it does not exist in pVec2
void CShaderTypeDecider::CullVecByVec(std::vector<ShaderType>*pVec1,std::vector<ShaderType>*pVec2)
{
	int i;
	i=0;
	while(i<pVec1->size())
	{
		ShaderType t;
		t=(*pVec1)[i];
		int j;
		for (j=0;j<pVec2->size();j++)
		{
			if (t==(*pVec2)[j])
				break;
		}
		if (j>=pVec2->size())//Not found
		{
			pVec1->erase(pVec1->begin()+i);
			continue;
		}
		i++;
	}
}

//return whether the expections could be met
BOOL CShaderTypeDecider::Expect(ShaderType ShaderTypePair)
{
	std::vector<ShaderType>vec;
	vec.push_back(ShaderTypePair);
	return Expect(&vec);
}


//return whether at least one of the expections could be met
BOOL CShaderTypeDecider::Expect(std::vector<ShaderType>*pShaderTypePairs)
{
	C3DHelper_Shader helper;
	int i,sz;
	sz=pShaderTypePairs->size();
	std::vector<ShaderType>aTemp;
	for (i=0;i<sz;i++)
	{
		ShaderType t;
		t=(*pShaderTypePairs)[i];
		if (TRUE)//First check whether the expected shadertype is in available shader list
		{
			t&=m_available;
			if (t==ShaderType_None)
				continue;
		}

		if (TRUE)//Split to single shadertypes(onle 1 shader type in a ShaderType)
		{
			while(t!=ShaderType_None)
			{
				ShaderType v;
				v=helper.GetFirstShaderType(t);
				t&=~v;
				aTemp.push_back(v);
			}
		}
	}


	if (TRUE)//Intersect aTemp & m_aPrioritied
	{
		if (m_aPrioritied.size()<=0)//Not set yet,just copy it
		{
			m_aPrioritied.resize(aTemp.size());
			memcpy(&(m_aPrioritied[0]),&(aTemp[0]),aTemp.size()*sizeof(ShaderType));
		}
		else
		{
			CullVecByVec(&aTemp,&m_aPrioritied);
			if (aTemp.size()<=0)
				return FALSE;
			CullVecByVec(&m_aPrioritied,&aTemp);
			ASSERT(m_aPrioritied.size()>0);
		}
	}

	return TRUE;
}


ShaderType CShaderTypeDecider::GetDecided()
{
	if (m_aPrioritied.size()>0)
		return m_aPrioritied[0];

	C3DHelper_Shader helper;
	return helper.GetFirstShaderType(m_available);
}


//////////////////////////////////////////////////////////////////////////
//C3DHelper_Shader
ShaderType C3DHelper_Shader::GetFirstShaderType(ShaderType st)
{
	int i;
	for (i=0;i<sizeof(g_aShaderTypeList)/sizeof(g_aShaderTypeList[0]);i++)
	{
		if (st&g_aShaderTypeList[i])
			return g_aShaderTypeList[i];
	}

	return ShaderType_None;
}

ShaderType C3DHelper_Shader::CullFirstShaderType(ShaderType& st)
{
	ShaderType t;
	t=GetFirstShaderType(st);

	if (t!=ShaderType_None)
		st&=(~t);//

	return t;
}
