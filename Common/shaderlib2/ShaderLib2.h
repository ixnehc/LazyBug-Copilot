#pragma once

#include "class/class.h"
#include "strlib/strlibdefines.h"

#include "SLDefines.h"

class CShaderVar
{
public:
	DEFINE_CLASS(CShaderVar);
	CShaderVar()
	{
		_stage=SVS_None;
		_type=SVT_None;
		_sem=SVSem_NONE;
		_nm=StringID_Invalid;
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

public:
	ShaderVarStage _stage;
	ShaderVarType _type;
	ShaderVarSem _sem;
	StringID _nm;
	i_math::pos2d_sh _pos;

};

class CShaderFormula
{
public:
	DEFINE_CLASS(CShaderFormula);

	CShaderFormula()
	{
		Zero();
	}
	~CShaderFormula()
	{
		Clear();
	}

	void Zero()
	{
		_output=StringID_Invalid;
		_platform=ShaderPlatform_None;
	}
	void Clear()
	{
		_inputs.clear();
		_code.clear();
		Zero();
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);


public:
	struct RefVar
	{
		StringID nm;
		ShaderVarType tp;
	};
	StringID _output;
	std::vector<RefVar> _inputs;

	std::string _code;

	ShaderPlatform _platform;
};

class CShaderFeature
{
public:
	DEFINE_CLASS(CShaderFeature);
	CShaderFeature()
	{
		Zero();
	}
	~CShaderFeature()
	{
		Clear();
	}
	void Zero()
	{
	}
	void Clear()
	{
		for (int i=0;i<_formulas.size();i++)
		{
			Safe_Class_Delete(_formulas[i]);
		}
		_formulas.clear();
		Zero();
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);


public:
	std::string _nm;

	std::vector<CShaderFormula*>_formulas;

};

class CShaderParamBuffer
{
public:
	DEFINE_CLASS(CShaderParamBuffer);
	CShaderParamBuffer()
	{

	}
	~CShaderParamBuffer()
	{
		Clear();
	}
	void Clear()
	{
		_params.clear();
	}
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

protected:
	std::vector<StringID>_params;
};

class CShaderParamPool
{
public:
	CShaderParamPool()
	{
		
	}
	~CShaderParamPool()
	{
		Clear();
	}
	void Clear()
	{
		for (int i=0;i<_bufs.size();i++)
		{
			Safe_Class_Delete(_bufs[i]);
		}
		_bufs.clear();
	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

protected:
	std::vector<CShaderParamBuffer*> _bufs;

};

class CShaderLib2
{
public:
	DEFINE_CLASS(CShaderLib2)
	CShaderLib2()
	{
		Zero();
	}
	~CShaderLib2()
	{
		Clear();
	}
	void Zero();
	void Clear();
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

public://当成protected
	std::string _nm;
	std::vector<CShaderVar *>_vars;
	std::vector<std::string> _features;
	CShaderFeature _base;

	//一些用来绘制的参数
	float _heightVS;
	float _heightPS;
};

