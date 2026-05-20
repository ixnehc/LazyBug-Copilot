#pragma once

#include "SLDefines.h"
#include "SLHolder.h"


class CShaderLib
{
public:
	CShaderLib()
	{
		_template=NULL;
	}
	void Clean();
	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);
	void SetTemplate(SLTemplate* t);
	void AddFeature(SLFeature* f);
	void AddUniFeature(std::string &name,std::string &sFX);
	const char *GetName()	{		return _name.c_str();	}
	void SetName(const char *name) 	{		_name=name;	}
	SLTemplate*GetTemplate()	{		return _template;	}
	DWORD GetFeatureCount()	{		return _features.size();	}
	SLFeature *GetFeature(DWORD idx)
	{
		if (idx>=_features.size())
			return NULL;
		return _features[idx];
	}
	DWORD GetUniFeatureCount()	{		return _unifeatures.size();	}
	BOOL GetUniFeature(DWORD idx,std::string &name,std::string &sFX);
	const char *GetUniFeatureCode(DWORD idx);
	BOOL GetUniFeatureName(DWORD idx,std::string &name);
	int FindUniFeature(const char *name);//return -1 if cannot find,NOTE: the finding is case sensitive

	//get all support feature code
	FeatureCode GetFeatureCode()	{		return _fc;	}
	FeatureCode GetBaseFeatureCode();
	FeatureCode GetNonBaseFeatureCode();

	BOOL CheckFeatureConflict(FeatureCode fc);

protected:
	std::string _name;
	SLTemplate* _template;
	std::vector<SLFeature*> _features;
	std::vector<FeatureCode> _featuregroups;//feature conflict groups
	FeatureCode _fc;//all support feature codes
	std::vector<std::string> _unifeatures;//the unique features.
};