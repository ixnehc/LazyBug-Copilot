#pragma once

#include "SLDefines.h"

#include "SLFeature.h"



struct SLTemplate
{
	void Clean();
	void CleanAndDelete();
	SLTemplate *Clone();
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	BOOL bEP,bVsIn,bPsIn,bPsOut,bVS,bPS;
	std::vector<ShaderVar>vars;
	std::vector<ExprNode*>exprVS;
	std::vector<ExprNode*>exprPS;

	std::string vs_ver;
	std::string ps_ver;

	std::vector<std::string>featuregroups;//feature conflict group

};

struct SLFeature
{
	SLFeature()
	{
		flag=FF_None;
	}
	SLFeature *Clone();
	void Clean();
	void CleanAndDelete();
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	void CopyFrom(SLFeature &src);
	FeatureCode fc;
	std::string name;
	FeatureFlag flag;
	std::string vs_ver;
	std::string ps_ver;
	std::string global;
	std::vector<ShaderVar>vars;
	std::vector<PriorShaderCap>caps;

	void AdjustString(SLTemplate *t,BOOL bHLSL);

};