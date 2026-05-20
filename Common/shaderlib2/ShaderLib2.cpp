/********************************************************************
	created:	2011/9/28   12:29
	file path:	e:\IxEngine\Common\shaderlib2
	author:		chenxi
	
	purpose:	ShaderLib 2ºËÐÄÊý¾Ý
*********************************************************************/
#include "stdh.h"

#include "ShaderLib2.h"

#include "DataPacket/DataPacket.h"

#include "stringparser/stringparser.h"

//////////////////////////////////////////////////////////////////////////
//CShaderVar

#define SHADERVAR_CURVER 1
void CShaderVar::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERVAR_CURVER;
	DP_WriteVar(dp,_stage);
	DP_WriteVar(dp,_type);
	DP_WriteVar(dp,_sem);
	DP_WriteVar(dp,_nm);
	DP_WriteVar(dp,_pos);
}

void CShaderVar::Load(CDataPacket &dp)
{
	dp.Data_NextDword();
	DP_ReadVar(dp,_stage);
	DP_ReadVar(dp,_type);
	DP_ReadVar(dp,_sem);
	DP_ReadVar(dp,_nm);
	DP_ReadVar(dp,_pos);

}


//////////////////////////////////////////////////////////////////////////
//CShaderFormula
#define SHADERFORMULA_CURVER 1
void CShaderFormula::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERFORMULA_CURVER;
	DP_WriteVar(dp,_output);
	DP_WriteVectorN(dp,_inputs);
	dp.Data_WriteStringSH(_code);
	DP_WriteVar(dp,_platform);
}

void CShaderFormula::Load(CDataPacket &dp)
{
	dp.Data_NextDword();

	DP_ReadVar(dp,_output);
	DP_ReadVectorN(dp,_inputs);
	dp.Data_ReadStringSH(_code);
	DP_ReadVar(dp,_platform);
}

//////////////////////////////////////////////////////////////////////////
//CShaderFeature
#define SHADERFEATURE_CURVER 1
void CShaderFeature::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERFEATURE_CURVER;

	dp.Data_WriteString(_nm);

	dp.Data_NextDword()=_formulas.size();

	for (int i=0;i<_formulas.size();i++)
		_formulas[i]->Save(dp);
}

void CShaderFeature::Load(CDataPacket &dp)
{
	Clear();

	dp.Data_NextDword();
	dp.Data_ReadString(_nm);

	DWORD sz=dp.Data_NextDword();
	_formulas.resize(sz);
	for (int i=0;i<sz;i++)
	{
		_formulas[i]=Class_New2(CShaderFormula);
		_formulas[i]->Load(dp);
	}
}

//////////////////////////////////////////////////////////////////////////
//CShaderParamBuffer
#define SHADERPARAMBUFFER_CURVER 1
void CShaderParamBuffer::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERPARAMBUFFER_CURVER;
	DP_WriteVectorN(dp,_params);
}

void CShaderParamBuffer::Load(CDataPacket &dp)
{
	dp.Data_NextDword();
	DP_ReadVectorN(dp,_params);
}


//////////////////////////////////////////////////////////////////////////
//CShaderParamPool
#define SHADERPARAMPOOL_CURVER 1
void CShaderParamPool::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERPARAMPOOL_CURVER;

	dp.Data_NextDword()=_bufs.size();
	for (int i=0;i<_bufs.size();i++)
		_bufs[i]->Save(dp);
}

void CShaderParamPool::Load(CDataPacket &dp)
{
	Clear();
	dp.Data_NextDword();
	_bufs.resize(dp.Data_NextDword());
	for (int i=0;i<_bufs.size();i++)
	{
		_bufs[i]=Class_New2(CShaderParamBuffer);
		_bufs[i]->Load(dp);
	}
}  

//////////////////////////////////////////////////////////////////////////
//CShaderLib2

void CShaderLib2::Zero()
{
	_heightVS=400.0f;
	_heightPS=400.0f;
}

void CShaderLib2::Clear()
{
	_features.clear();
	for (int i=0;i<_vars.size();i++)
	{
		Safe_Class_Delete(_vars[i]);
	}
	_vars.clear();
	_features.clear();
	_base.Clear();
	Zero();
}


#define SHADERLIB2_CURVER 1
void CShaderLib2::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=SHADERLIB2_CURVER;
	dp.Data_WriteString(_nm);

	dp.Data_NextDword()=_vars.size();
	for (int i=0;i<_vars.size();i++)
		_vars[i]->Save(dp);

	dp.Data_NextDword()=_features.size();
	for (int i=0;i<_features.size();i++)
		dp.Data_WriteString(_features[i]);

	_base.Save(dp);

	DP_WriteVar(dp,_heightVS);
	DP_WriteVar(dp,_heightPS);

}

void CShaderLib2::Load(CDataPacket &dp)
{
	Clear();

	dp.Data_NextDword();
	dp.Data_ReadString(_nm);

	_vars.resize(dp.Data_NextDword());
	for (int i=0;i<_vars.size();i++)
	{
		_vars[i]=Class_New2(CShaderVar);
		_vars[i]->Load(dp);
	}

	_features.resize(dp.Data_NextDword());
	for (int i=0;i<_features.size();i++)
		dp.Data_ReadString(_features[i]);
	_base.Load(dp);

	DP_ReadVar(dp,_heightVS);
	DP_ReadVar(dp,_heightPS);

}
