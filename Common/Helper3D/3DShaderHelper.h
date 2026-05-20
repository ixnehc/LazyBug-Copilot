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
#include "..\D3D9BASE\D3D9ShaderType.h"

class CD3D9Base;
class CShaderTypeDecider
{
public:
	void SetAvailable(CD3D9Base *pD3D9Base,ShaderType shadertypes);
	CD3D9Base *GetD3D9Base();
	BOOL Expect(ShaderType ShaderTypePair);//return whether at least one of the expections could be met
	BOOL Expect(std::vector<ShaderType>*pShaderTypePairs);//return whether at least one of the expections could be met

	ShaderType GetDecided();
private:
	CD3D9Base *m_pD3D9Base;
	ShaderType m_available;
	std::vector<ShaderType> m_aPrioritied;

	void CullVecByVec(std::vector<ShaderType>*pVec1,std::vector<ShaderType>*pVec2);//remove the element from pVec1,if it does not exist in pVec2
};



class C3DHelper_Shader
{
public:
	C3DHelper_Shader()
	{
	}

	ShaderType GetFirstShaderType(ShaderType st);
	ShaderType CullFirstShaderType(ShaderType& st);

};

