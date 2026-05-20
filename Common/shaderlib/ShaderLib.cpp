/********************************************************************
	created:	2006/10/8   11:14
	filename: 	e:\IxEngine\Common\CShaderLib\CShaderLib.cpp
	author:		cxi
	
	purpose:	shader lib
*********************************************************************/

#include "stdh.h"

#include "ShaderLib.h"

#include "../DataPacket/DataPacket.h"

#include "../stringparser/stringparser.h"



void CShaderLib::Clean()
{
	_name="";
	if(_template)
		_template->CleanAndDelete();
	_template=NULL;

	for (int i=0;i<_features.size();i++)
		_features[i]->CleanAndDelete();
	_features.clear();

	_featuregroups.clear();

	_fc.Clear();

	_unifeatures.clear();
}
void CShaderLib::Save(CDataPacket &dp)
{
	dp.Data_WriteString(_name);
	if (_template)
	{
		dp.Data_NextInt()=1;
		_template->Save(dp);
	}
	else
		dp.Data_NextInt()=0;

	dp.Data_NextDword()=_features.size();
	for (int i=0;i<_features.size();i++)
		_features[i]->Save(dp);

	DP_WriteVector(dp,_featuregroups);

	dp.Data_WriteData(&_fc,sizeof(_fc));

	dp.Data_NextDword()=_unifeatures.size();
	for (int i=0;i<_unifeatures.size();i++)
		dp.Data_WriteString(_unifeatures[i]);
}

void CShaderLib::Load(CDataPacket &dp)
{
	Clean();

	dp.Data_ReadString(_name);

	if (dp.Data_NextInt())
	{
		_template=new SLTemplate;
		_template->Load(dp);
	}

	_features.resize(dp.Data_NextDword());
	for (int i=0;i<_features.size();i++)
	{
		_features[i]=new SLFeature;
		_features[i]->Load(dp);
	}

	DP_ReadVector(dp,_featuregroups);

	dp.Data_ReadData(&_fc,sizeof(_fc));

	_unifeatures.resize(dp.Data_NextDword());
	for (int i=0;i<_unifeatures.size();i++)
		dp.Data_ReadString(_unifeatures[i]);
}

void CShaderLib::SetTemplate(SLTemplate* t)
{
	_template=t;

	FeatureCode fc;
	for (int i=0;i<_template->featuregroups.size();i++)
	{
		fc.FromName(_template->featuregroups[i].c_str());
		if (!fc.IsEmpty())
			_featuregroups.push_back(fc);
	}

}

void CShaderLib::AddFeature(SLFeature* f)
{
	_features.push_back(f);
	_fc.Add(f->fc);
}

FeatureCode CShaderLib::GetBaseFeatureCode()
{
	FeatureCode fc;
	for (int i=0;i<_features.size();i++)
	{
		if (_features[i]->flag&FF_Base)
			fc.Add(_features[i]->fc);
	}
	return fc;
}
FeatureCode CShaderLib::GetNonBaseFeatureCode()
{
	FeatureCode fc;
	for (int i=0;i<_features.size();i++)
	{
		if (!(_features[i]->flag&FF_Base))
			fc.Add(_features[i]->fc);
	}
	return fc;
}


//return whether there is conflicting features in fc
BOOL CShaderLib::CheckFeatureConflict(FeatureCode fc)
{
	FeatureCode t;
	for (int i=0;i<_featuregroups.size();i++)
	{
		if (fc.JudgeConflict(_featuregroups[i]))
			return TRUE;
	}
	return FALSE;
}


void CShaderLib::AddUniFeature(std::string &name,std::string &sFX)
{
	_unifeatures.resize(_unifeatures.size()+1);
	_unifeatures[_unifeatures.size()-1]=name+"|"+sFX;
}

const char *CShaderLib::GetUniFeatureCode(DWORD idx)
{
	if (idx>=_unifeatures.size())
		return "";
	const char *p=_unifeatures[idx].c_str();
	while(*p)
	{
		if ((*p)=='|')
			return p+1;
		p++;
	}

	return _unifeatures[idx].c_str();
}


BOOL CShaderLib::GetUniFeature(DWORD idx,std::string &name,std::string &sFX)
{
	name="";
	sFX="";
	if (idx>=_unifeatures.size())
		return FALSE;

	sFX=_unifeatures[idx];
	SeperateStringBy("|",sFX,name);
	return TRUE;
}

BOOL CShaderLib::GetUniFeatureName(DWORD idx,std::string &name)
{
	name="";
	if (idx>=_unifeatures.size())
		return FALSE;

	std::string s=_unifeatures[idx];
	SeperateStringBy("|",s,name);
	return TRUE;
}


//return -1 if cannot find,NOTE: the finding is case sensitive
int CShaderLib::FindUniFeature(const char *name)
{
	std::string s,s2;
	for (int i=0;i<_unifeatures.size();i++)
	{
		s2=_unifeatures[i];
		SeperateStringBy("|",s2,s);
		if (s==name)
			return i;
	}
	return -1;
}
