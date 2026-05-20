#pragma once

#include <vector>
#include <map>
#include <deque>
#include "SLDefines.h"

#include "fvfex/fvfex_type.h"



struct SLTemplate;
struct SLFeature;

class CSLComposer
{
public:
	CSLComposer()
	{
		_template=NULL;		
		_bHLSL=TRUE;
	}
	void Clear();
	void SetLang(BOOL bHLSL)	{		_bHLSL=bHLSL;	}
	void ClearFeature();
	void AddFeature(SLFeature *f);
	void SetTemplate(SLTemplate* t);
	void PreCheck(std::vector<std::string>&errors,std::vector<std::string>&warnings,
		std::string &templatepath,std::vector<std::string>&featurepaths);
	void ComposeFX(std::string &result,std::vector<ShaderCap>&caps);
	BOOL ComposeGL(std::string &resultVS,std::string &resultPS,std::string *srcRaw,std::string &err);
protected:
	void _Compose(std::string &sEP,std::string &sVI,std::string &sPI,std::string &sVS,std::string &sPS,std::string &sGlobal,std::string &sTec,std::vector<ShaderCap>&caps,FVFEx &fvfAttributes);
	BOOL _CheckDependency(const char *var1,const char *var2);
	BOOL _CheckDependencyInExpr(ExprNode*en,const char *var1,const char *var2);
	BOOL _CheckDependencyInVar(ShaderVar &var,const char *var1,const char *var2);
	BOOL _SearchVarInExpr(ExprNode *en,const char *varname);
	int _GetMaxVarArraySize(const char *varname);
	ShaderVar* _FindPriorVar(const char *varname);
	ShaderVar* _FindBaseVar(const char *varname);//find in _template
	ShaderVar* _FindFeatureEffectParam(const char *varname);//find in _features
	const char *_GetExprMainVar(ExprNode *en);// for example: a=b+c, a is the main var

	BOOL _Collapse(ExprNode *en);//return whether any error occurs
	BOOL _IsExprNodeValueOf(ExprNode *en,int v);//whether the node has an arbitory value of v
	void ExprNodeToString(ExprNode *en,std::string &s,const char *tpTarget="");

	void _EnumReferenceVar(const char *varSeed,std::vector<int>&varsFlag,std::deque<std::string>&varsRefer);

	void _CollectStates(std::vector<ShaderVar>&statevars,std::vector<std::string>&statevarfeatures);
	void _ResolveShaderVer(std::string &ver,BOOL bVs);
	std::vector<SLFeature *>_features;
	SLTemplate *_template;

	void _BuildPriorVarTable();
	std::map<std::string,ShaderVar *>_priorvars;
	std::map<std::string,std::string>_priorfeatures;

	//append a comment to declare which feature this prior var is belong to
	void _AppendPriorFeature(std::string &s,const char *varname);

	BOOL _bHLSL;
};

