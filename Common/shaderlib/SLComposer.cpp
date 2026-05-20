/********************************************************************
	created:	2006/9/23   11:33
	filename: 	d:\IxEngine\Proj_MFeatureEditor\slcomposer.cpp
	author:		cxi
	
	purpose:	a combiner to generate a composed fx file(string)
*********************************************************************/
#include "stdh.h"
#pragma warning (disable:4786)
#pragma warning (disable:4311)
#pragma warning (disable:4244)
#include "SLComposer.h"
#include "SLHolder.h"

#include "commondefines/general_stl.h"

#include "Log/LogFile.h"
#include "stringparser/stringparser.h"
#include "timer/timer.h"

#include <assert.h>

#include <map>


void CSLComposer::Clear()
{
	ClearFeature();
	if (_template)
		_template->CleanAndDelete();
	_template=NULL;
	_bHLSL=TRUE;
}
void CSLComposer::ClearFeature()
{
	for (int i=0;i<_features.size();i++)
		_features[i]->CleanAndDelete();
	_features.clear();
}
void CSLComposer::AddFeature(SLFeature *f)
{
	if (!f)
		return;
	_features.push_back(f);
}
void CSLComposer::SetTemplate(SLTemplate*t)
{
	if (!t)
		return;
	if (_template)
		_template->CleanAndDelete();
	_template=t;
}

//return TRUE if found
BOOL CSLComposer::_SearchVarInExpr(ExprNode *en,const char *varname)
{
	if (en->type==ExprNode::Operand_Var)
	{
		if (en->varname==varname)
			return TRUE;
	}
	if (en->left)
	{
		if (_SearchVarInExpr(en->left,varname))
			return TRUE;
	}
	if (en->right)
	{
		if (_SearchVarInExpr(en->right,varname))
			return TRUE;
	}

	return FALSE;
}

//check whether var2 is dependent on var1 in en 
BOOL CSLComposer::_CheckDependencyInExpr(ExprNode*en,const char *var1,const char *var2)
{
	if (en->type!=ExprNode::Operator)
		return FALSE;
	if (en->op!=O_ASSIGN)
		return FALSE;
	if (!en->left)
		return FALSE;
	if (en->left->type!=ExprNode::Operand_Var)
		return FALSE;
	if (en->left->varname!=var2)
		return FALSE;

	if (!_SearchVarInExpr(en,var1))
		return FALSE;

	return TRUE;
}

//check whether var2 is dependent on var1 in var
BOOL CSLComposer::_CheckDependencyInVar(ShaderVar &var,const char *var1,const char *var2)
{
	if (var.name!=var2)
		return FALSE;

	if(var.vInit)
	{
		if (_SearchVarInExpr(var.vInit,var1))
			return TRUE;
	}
	std::string s;
	s=s+" "+var1+" ";//each id in assign is seperated by " ",add " "to the both side of var1 to make the search more accurate

	if (-1!=var.assign.find(s.c_str()))//in assign,each symbol should be seperated by " "
		return TRUE;

	return FALSE;
}


//check whether var2 is dependent on var1
BOOL CSLComposer::_CheckDependency(const char *var1,const char *var2)
{
	if (TRUE)
	{
		ShaderVar *var;
		var=_FindPriorVar(var2);
		assert(var);
		if (_CheckDependencyInVar(*var,var1,var2))
			return TRUE;
	}

	for (int i=0;i<_template->exprVS.size();i++)
	{
		if (_CheckDependencyInExpr(_template->exprVS[i],var1,var2))
			return TRUE;
	}
	for (int i=0;i<_template->exprPS.size();i++)
	{
		if (_CheckDependencyInExpr(_template->exprPS[i],var1,var2))
			return TRUE;
	}

	return FALSE;
}

int CSLComposer::_GetMaxVarArraySize(const char *varname)
{
	int szArray=0;
	ShaderVar *ret=NULL;
	for (int i=0;i<_template->vars.size();i++)
	{
		if (_template->vars[i].name==varname)
		{
			if (_template->vars[i].szArray>szArray)
				szArray=_template->vars[i].szArray;
		}
	}

	for (int i=0;i<_features.size();i++)
	{
		for (int j=0;j<_features[i]->vars.size();j++)
		{
			if (_features[i]->vars[j].name==varname)
			{
				if (_features[i]->vars[j].szArray>szArray)
					szArray=_features[i]->vars[j].szArray;
			}
		}
	}

	return szArray;
}

void CSLComposer::_BuildPriorVarTable()
{
	_priorvars.clear();
	_priorfeatures.clear();

	for (int i=0;i<_template->vars.size();i++)
	{
		std::map<std::string,ShaderVar *>::iterator it;
		it=_priorvars.find(_template->vars[i].name);

		if (it==_priorvars.end())
		{
			_priorvars[_template->vars[i].name]=&_template->vars[i];
			_priorfeatures[_template->vars[i].name]="<Default>";
		}
		else
		{
			if (_template->vars[i].priority>(*it).second->priority)
			{
				(*it).second=&_template->vars[i];
				_priorfeatures[_template->vars[i].name]="<Default>";
			}
		}
	}

	for (int i=0;i<_features.size();i++)
	{
		for (int j=0;j<_features[i]->vars.size();j++)
		{
			std::map<std::string,ShaderVar *>::iterator it;
			it=_priorvars.find(_features[i]->vars[j].name);

			if (it==_priorvars.end())
			{
				_priorvars[_features[i]->vars[j].name]=&_features[i]->vars[j];
				_priorfeatures[_features[i]->vars[j].name]=_features[i]->fc.ToName();
			}
			else
			{
				if (_features[i]->vars[j].priority>(*it).second->priority)
				{
					(*it).second=&_features[i]->vars[j];
					_priorfeatures[_features[i]->vars[j].name]=_features[i]->fc.ToName();
				}
			}
		}
	}
}


//Find the var that has the most priority
ShaderVar *CSLComposer::_FindPriorVar(const char *varname)
{
	std::map<std::string,ShaderVar *>::iterator it;
	it=_priorvars.find(std::string(varname));
	if (it==_priorvars.end())
		return NULL;
	return (*it).second;
}

//find in _template
ShaderVar* CSLComposer::_FindBaseVar(const char *varname)
{
	for (int i=0;i<_template->vars.size();i++)
	{
		if (_template->vars[i].name==varname)
			return&_template->vars[i];
	}
	return NULL;
}

ShaderVar* CSLComposer::_FindFeatureEffectParam(const char *varname)
{
	for (int j=0;j<_features.size();j++)
	{
		SLFeature *feature=_features[j];
		for (int i=0;i<feature->vars.size();i++)
		{
			if (feature->vars[i].category!=SVC_EffectParam)
				continue;
			if (feature->vars[i].name==varname)
				return &feature->vars[i];
		}
	}
	return NULL;
}



BOOL CSLComposer::_IsExprNodeValueOf(ExprNode *en,int v)
{
	if (en->type==ExprNode::Operand_Number)
	{
		if (en->varvalue==v)
			return TRUE;
		return FALSE;
	}

	if (en->type==ExprNode::Operand_Var)
	{
		ShaderVar *var;
		var=_FindPriorVar(en->varname.c_str());
		if (!var)//couldn't resolve the variable by this name
			return FALSE;

		//only these 2 types of var could be used in collapsing
		if (!((var->category==SVC_VsFactor)||(var->category==SVC_PsFactor)))
			return FALSE;

		if (var->vInit)
			return _IsExprNodeValueOf(var->vInit,v);
	}

	return FALSE;//not a value
	
}

void _ConvertToValue(ExprNode *en,int v)
{
	en->left->CleanAndDelete();
	en->right->CleanAndDelete();
	en->left=NULL;
	en->right=NULL;
	en->type=ExprNode::Operand_Number;
	en->varvalue=v;
}

void _ConvertToSub(ExprNode *en,BOOL bKeepLeft)//keep left or keep right
{
	ExprNode *t,**enRemove,**enKeep;

	if (!bKeepLeft)
	{
		enRemove=&en->left;
		enKeep=&en->right;
	}
	else
	{
		enRemove=&en->right;
		enKeep=&en->left;
	}
	//remove enRemove
	(*enRemove)->CleanAndDelete();
	(*enRemove)=NULL;
	//back up original enKeep
	t=(*enKeep);
	//copy the content from enKeep
	(ExprCode&)(*en)=(ExprCode&)*t;
	//move the ptrs
	en->left=t->left;
	en->right=t->right;
	t->left=NULL;
	t->right=NULL;
	//remove the original enKeep
	t->CleanAndDelete();

}


BOOL CSLComposer::_Collapse(ExprNode *en)
{
	if (en->type==ExprNode::Operand_Number)
		return TRUE;

	if (en->type==ExprNode::Operand_Var)
	{
		ShaderVar *var;
		var=_FindPriorVar(en->varname.c_str());
		if (!var)
		{
			LogFile::Prompt("Failed to resolve variable \"%s\"!",en->varname.c_str());
			return FALSE;
		}
		if (var->vInit)
		{
			if (var->vInit==en)
			{
				LogFile::Prompt("Illegal initial value for\"%s\"!",en->varname.c_str());
				return FALSE;
			}

			return _Collapse(var->vInit);
		}
		return TRUE;
	}


	if (en->type==ExprNode::Operator)
	{
		if (en->left)
		{
			if (!_Collapse(en->left))
				return FALSE;
		}
		if (en->right)
		{
			if (!_Collapse(en->right))
				return FALSE;
		}

		if (en->op==O_ADD)
		{
			assert(en->left&&en->right);
			BOOL bLeftZero,bRightZero;
			bLeftZero=_IsExprNodeValueOf(en->left,0);
			bRightZero=_IsExprNodeValueOf(en->right,0);
			if (bLeftZero&&bRightZero)
			{
				_ConvertToValue(en,0);
				return TRUE;
			}
			if ((!bLeftZero)&&(!bRightZero))//could do nothing
				return TRUE;

			_ConvertToSub(en,!bLeftZero);
			return TRUE;
		}
		if (en->op==O_MULTIPLY)
		{
			assert(en->left&&en->right);
			BOOL bLeftOne,bRightOne,bLeftZero,bRightZero;
			bLeftOne=_IsExprNodeValueOf(en->left,1);
			bRightOne=_IsExprNodeValueOf(en->right,1);
			bLeftZero=_IsExprNodeValueOf(en->left,0);
			bRightZero=_IsExprNodeValueOf(en->right,0);

			if (bLeftZero||bRightZero)
			{
				_ConvertToValue(en,0);
				return TRUE;
			}

			if (bLeftOne&&bRightOne)
			{
				_ConvertToValue(en,1);
				return TRUE;
			}

			if ((!bLeftOne)&&(!bRightOne))//could do nothing
				return TRUE;
			_ConvertToSub(en,!bLeftOne);
			return TRUE;
		}
	}

	return TRUE;
}

void CSLComposer::_EnumReferenceVar(const char *varSeed,std::vector<int>&varsFlag,std::deque<std::string>&varsRefer)
{
	for (int i=0;i<_template->vars.size();i++)
	{
		if (varsFlag[i]==1)
			continue;

		if (!_CheckDependency(_template->vars[i].name.c_str(),varSeed))
			continue;

		varsFlag[i]=1;
		_EnumReferenceVar(_template->vars[i].name.c_str(),varsFlag,varsRefer);
	}

	varsRefer.push_back(std::string(varSeed));
}

void MakeConvertedValue(const char *tpTarget,int v,std::string &s)
{
	FormatString(s,"%d",v);
	if (strcmp(tpTarget,"float")==0)
		FormatString(s,"%f",(float)v);
	if (strcmp(tpTarget,"vec2")==0)
		FormatString(s,"vec2(%f,%f)",(float)v,(float)v);
	if (strcmp(tpTarget,"vec3")==0)
		FormatString(s,"vec3(%f,%f,%f)",(float)v,(float)v,(float)v);
	if (strcmp(tpTarget,"vec4")==0)
		FormatString(s,"vec4(%f,%f,%f,%f)",(float)v,(float)v,(float)v,(float)v);
	if (strcmp(tpTarget,"ivec2")==0)
		FormatString(s,"ivec2(%d,%d)",v,v);
	if (strcmp(tpTarget,"ivec3")==0)
		FormatString(s,"ivec3(%d,%d,%d)",v,v,v);
	if (strcmp(tpTarget,"ivec4")==0)
		FormatString(s,"ivec4(%d,%d,%d,%d)",v,v,v,v);
}

void CSLComposer::ExprNodeToString(ExprNode *en,std::string &s,const char *tpTarget)
{
	s="";
	if (en->type==ExprNode::Operand_Number)
	{
		if (_bHLSL)
			FormatString(s,"%d",en->varvalue);
		else
			MakeConvertedValue(tpTarget,en->varvalue,s);
		return;
	}
	if (en->type==ExprNode::Operand_Var)
	{
		if (_bHLSL)
		{
			ShaderVar *var;
			var=_FindBaseVar(en->varname.c_str());
			if (var)//some prefix
			{
				if (var->category==SVC_VsIn)
					s+="vi.";
				if (var->category==SVC_PsIn)
					s+="pi.";
			}
		}
		s+=en->varname;
		return;
	}

	if (en->type==ExprNode::Operator)
	{
		std::string sLeft,sRight;
		ExprNodeToString(en->left,sLeft);
		ExprNodeToString(en->right,sRight);
		switch(en->op)
		{
			case O_ADD:
				s=std::string("(")+sLeft+"+"+sRight+")";
				break;
			case O_MULTIPLY:
				s=std::string("(")+sLeft+"*"+sRight+")";
				break;
			case O_ASSIGN:
				s=sLeft+"="+sRight;
				break;
			default:
				assert(FALSE);
		}
	}
}

// for example: a=b+c, a is the main var
const char *CSLComposer::_GetExprMainVar(ExprNode *en)
{
	if (en->type!=ExprNode::Operator)
		return "";
	if (en->op!=O_ASSIGN)
		return "";
	if (!en->left)
		return "";
	if (en->left->type!=ExprNode::Operand_Var)
		return "";
	return en->left->varname.c_str();
}


void CSLComposer::_CollectStates(std::vector<ShaderVar>&statevars,std::vector<std::string>&statevarfeatures)
{
	for (int i=0;i<_features.size();i++)
	{
		for (int j=0;j<_features[i]->vars.size();j++)
		{
			ShaderVar &var=_features[i]->vars[j];

			if (var.category!=SVC_StateVar)
				continue;

			//see if already existing
			int k;
			for (k=0;k<statevars.size();k++)
			{
				if ((statevars[k].name==var.name)&&(statevars[k].szArray==var.szArray))
					break;
			}
			if (k>=statevars.size())//not existing,just add it
			{
				statevars.push_back(var);
				statevarfeatures.push_back(_features[i]->fc.ToName());
			}
			else
			{
				if (var.priority>statevars[k].priority)//compare priority
				{
					statevars[k]=var;//replace
					statevarfeatures[k]=_features[i]->fc.ToName();//replace
				}
			}
		}
	}
}

void CSLComposer::_ResolveShaderVer(std::string &ver,BOOL bVs)
{
	if (bVs)
		ver=_template->vs_ver;
	else
		ver=_template->ps_ver;
	std::string verFeature;
	for (int i=0;i<_features.size();i++)
	{
		if (bVs)
		{
			verFeature=_features[i]->vs_ver;
			if (verFeature=="")
				verFeature="vs_1_1";//at least vs_1_1,if overriding some factor var
		}
		else
		{
			verFeature=_features[i]->ps_ver;
			if (verFeature=="")
				verFeature="ps_1_1";//at least ps_1_1,if overriding some factor var
		}
		if (verFeature>ver)
		{
			for (int j=0;j<_features[i]->vars.size();j++)
			{
				if (_features[i]->vars[j].category==SVC_None)
				{
					ShaderVar *var=_FindBaseVar(_features[i]->vars[j].name.c_str());
					if (var)
					{
						if ((bVs&&(var->category==SVC_VsFactor))||
							((!bVs)&&((var->category==SVC_PsOut)||(var->category==SVC_PsFactor))))
						{
							ver=verFeature;
							break;
						}
					}
				}
			}
		}
	}
}

void CSLComposer::PreCheck(std::vector<std::string>&errors,std::vector<std::string>&warnings,
							   std::string &templatepath,std::vector<std::string>&featurepaths)
{
	std::string sErr,sWarning;

	//firstly check whether each var in features residents in template
	if (TRUE)	
	{
		std::map<std::string,std::string>mapFeatureNames;
		std::map<std::string,int>mapVarPriority;
		BOOL bBaseFeature=FALSE;
		for (int i=0;i<_features.size();i++)
		{
			SLFeature *feature=_features[i];
			if (feature->name=="base")
				bBaseFeature=TRUE;
			if (feature->name=="")
			{
				FormatString(sErr,"No feature name defined in feature file \"%s\"!",
									featurepaths[i].c_str());
				errors.push_back(sErr);
			}
			else
			{
				if (!FeatureCode::CheckValidName(feature->name.c_str()))
				{
					FormatString(sErr,"Undefined feature name(\"%s\" defined in feature file \"%s\"!",
						feature->name.c_str(),featurepaths[i].c_str());
					errors.push_back(sErr);
				}
				else
				{
					if (mapFeatureNames.end()!=mapFeatureNames.find(feature->name))
					{
						FormatString(sErr,"Duplicated feature name (\"%s\") found in feature file \"%s\"!",
							feature->name.c_str(),featurepaths[i].c_str());
						errors.push_back(sErr);
					}
					else
						mapFeatureNames[feature->name]=featurepaths[i];

				}
			}

			for (int k=0;k<feature->vars.size();k++)
			{
				ShaderVar &var=feature->vars[k];

				if ((var.category!=SVC_StateVar)&&(var.category!=SVC_EffectParam))
				{
					ShaderVar *varBase;
					if (!(varBase=_FindBaseVar(var.name.c_str())))
					{
						FormatString(sErr,"Undefined variable \"%s\" found in feature file \"%s\"!",
								var.name.c_str(),featurepaths[i].c_str());
						errors.push_back(sErr);
					}
					else
					{
						if ((varBase->category==SVC_PsIn)||(varBase->category==SVC_VsIn))
						{
							FormatString(sErr,"VsIn or PsIn variable \"%s\" overridden in feature file \"%s\"!",
								var.name.c_str(),featurepaths[i].c_str());
							errors.push_back(sErr);
						}
						else
						{
							if ((varBase->category==SVC_PsFactor)||(varBase->category==SVC_VsFactor)
								||(varBase->category==SVC_PsOut))
							{
								std::map<std::string,int>::iterator it;
								it=mapVarPriority.find(var.name);

								if (it==mapVarPriority.end())
									mapVarPriority[var.name]=var.priority;
								else
								{
									if ((*it).second==var.priority)
									{
										FormatString(sWarning,"Priority conflict(%d) found in variable \"%s\" in feature file \"%s\"!",
											var.priority,var.name.c_str(),featurepaths[i].c_str());
										warnings.push_back(sWarning);
									}
									else
									{
										if ((*it).second<var.priority)
											(*it).second=var.priority;
									}
								}
							}
						}
					}
				}
			}
		}

		if (!bBaseFeature)
		{
			FormatString(sErr,"\"base\" feature missed!!");
			errors.push_back(sErr);
		}
	}

	//now check whether each feature group in template contains any invalid feature name
	//and check whether each feature is referenced more than once in the feature groups
	if (TRUE)
	{
		std::vector<std::string>temp;
		std::vector<int>refcount;//for each feature
		refcount.resize(_features.size());
		memset(&(refcount[0]),0,sizeof(refcount[0])*refcount.size());
		for (int i=0;i<_template->featuregroups.size();i++)
		{
			SplitStringBy(",",_template->featuregroups[i],&temp);
			for (int j=0;j<temp.size();j++)
			{
				int k;
				for (k=0;k<_features.size();k++)
				{
					if (temp[j]==_features[k]->name)
					{
						refcount[k]++;//add 1 ref count for that feature
						break;
					}
				}
				if (k>=_features.size())
				{
					FormatString(sErr,"Unknown feature name\"%s\" found in feature group in \"%s\"!",
						temp[j].c_str(),templatepath.c_str());
					errors.push_back(sErr);
				}
			}
		}

		for (int i=0;i<refcount.size();i++)
		{
			if (refcount[i]>1)
			{
				FormatString(sErr,"feature \"%s\" is referenced more than once by feature groups in \"%s\"!",
					_features[i]->name.c_str(),templatepath.c_str());
				errors.push_back(sErr);
			}
		}
	}

}


//append a comment to declare which feature this prior var is belong to
void CSLComposer::_AppendPriorFeature(std::string &s,const char *varname)
{
	std::string ss=_priorfeatures[std::string(varname)];
	if (ss!="")
		AppendFmtString(s,"//%s",ss.c_str());
}

void CSLComposer::_Compose(std::string &sEP,std::string &sVI,std::string &sPI,std::string &sVS,std::string &sPS,std::string &sGlobal,std::string &sTec,std::vector<ShaderCap>&capsRet,FVFEx &fvfAttributes)
{
	_BuildPriorVarTable();

	//Now collapse
	if (TRUE)
	{
		for (int i=0;i<_template->exprVS.size();i++)
			_Collapse(_template->exprVS[i]);
		for (int i=0;i<_template->exprPS.size();i++)
			_Collapse(_template->exprPS[i]);
	}

	for (int i=0;i<_template->vars.size();i++)
	{
		ShaderVar *var;
		var=_FindPriorVar(_template->vars[i].name.c_str());
		if (var->vInit)
			_Collapse(var->vInit);
	}


	//enumerate all the referenced variables
	std::deque<std::string>varsRefer;
	std::vector<int>varsReferFlag;
	varsReferFlag.resize(_template->vars.size());
	memset(varsReferFlag.data(),0,sizeof(varsReferFlag[0])*varsReferFlag.size());
	for (int i=0;i<_template->vars.size();i++)
	{
		ShaderVar *var=&_template->vars[i];
		if (var->name=="pi_pos")
		{
			varsReferFlag[i]=1;
			continue;
		}

		if (var->category==SVC_PsOut)
		{
			ShaderVar *varPrior=_FindPriorVar(var->name.c_str());
			assert(varPrior);
			if(varPrior->assign!="")
				varsReferFlag[i]=1;
		}
	}
	if (_template->bVS)
		_EnumReferenceVar("pi_pos",varsReferFlag,varsRefer);
	if (_template->bPsOut)
	{
		for (int i=0;i<_template->vars.size();i++)
		{
			ShaderVar *var=&_template->vars[i];

			if (var->category==SVC_PsOut)
			{
				ShaderVar *varPrior=_FindPriorVar(var->name.c_str());
				assert(varPrior);
				if(varPrior->assign!="")
					_EnumReferenceVar(var->name.c_str(),varsReferFlag,varsRefer);
			}
		}
	}

	//Check each effect param whether they are referenced in each feature's global and its state vars
	if (TRUE)
	{
		std::vector<std::string>epnames;
		for (int i=0;i<_template->vars.size();i++)
		{
			if (_template->vars[i].category!=SVC_EffectParam)
				continue;
			epnames.push_back(_template->vars[i].name);
		}

		for (int i=0;i<epnames.size();i++)
		{
			std::string name=epnames[i];

			int idx;
			VEC_FIND(varsRefer,name,idx);
			if (idx!=-1)
				continue;//already referenced

			//each state value or global's id is seperated by " ",
			//add " "to the both side of name to make the search more accurate
			std::string name2;
			name2=" ";
			name2+=name+" ";

			for (int j=0;j<_features.size();j++)
			{
				if (-1!=_features[j]->global.find(name2.c_str()))//use the expanded name to search
				{
					varsRefer.push_front(name);
					break;
				}
				int k;
				for (k=0;k<_features[j]->vars.size();k++)
				{
					ShaderVar &var=_features[j]->vars[k];
					if (var.category!=SVC_StateVar)
						continue;
					if (-1!=var.statevalue.find(name2.c_str()))//use the expanded name to search
						break;
				}
				if (k<_features[j]->vars.size())//Found in some state var
				{
					varsRefer.push_front(name);
					break;
				}
			}
		}
	}

	//feature中定义的的EffectParam,我们认为肯定会被引用到
	for (int j=0;j<_features.size();j++)
	{
		SLFeature *feature=_features[j];
		for (int i=0;i<feature->vars.size();i++)
		{
			if (feature->vars[i].category!=SVC_EffectParam)
				continue;
			UNIQUE_VEC_ADD(varsRefer,feature->vars[i].name);
		}
	}


	//Now compose the fx txt string
	std::string sVar,sExpr;

	//Effect Params
	for (int i=0;i<varsRefer.size();i++)
	{
		ShaderVar *varBase;
		varBase=_FindBaseVar(varsRefer[i].c_str());
		if (!varBase)
			varBase=_FindFeatureEffectParam(varsRefer[i].c_str());

		assert(varBase);
		if (varBase->category!=SVC_EffectParam)
			continue;

		if (_bHLSL)
		{
			sVar=varBase->type+" "+varBase->name;
			int szArray=_GetMaxVarArraySize(varsRefer[i].c_str());
			if (szArray>0)
				AppendFmtString(sVar,"[%d]",szArray);
			if (varBase->sementic!="")
				AppendFmtString(sVar,":%s",varBase->sementic.c_str());
		}
		else
		{
			sVar="uniform ";
			sVar=sVar+varBase->type+" "+varBase->name;
			int szArray=_GetMaxVarArraySize(varsRefer[i].c_str());
			if (szArray>0)
				AppendFmtString(sVar,"[%d]",szArray);
		}

		sVar+=";";
		sVar+="\r\n";

		sEP+=sVar;
	}

	//VsIn
	fvfAttributes=0;
	if (_template->bVS)
	{
		for (int i=0;i<varsRefer.size();i++)
		{
			ShaderVar *varBase;
			varBase=_FindBaseVar(varsRefer[i].c_str());
			if (!varBase)
				continue;
			if (varBase->category!=SVC_VsIn)
				continue;

			if (!_bHLSL)
			{
				extern FVFEx fvfFromName(const char *nm);
				fvfAttributes|=fvfFromName(varBase->name.c_str());
			}

			if (_bHLSL)
			{
				sVar=varBase->type+" "+varBase->name;
				if (varBase->sementic!="")
					AppendFmtString(sVar,":%s",varBase->sementic.c_str());
				sVar+=";\r\n";
				sVI+="	"+sVar;
			}
			else
			{
				sVar="attribute ";
				sVar=sVar+varBase->type+" "+varBase->name;
				sVar+=";\r\n";
				sVI+=sVar;
			}
		}
		if (_bHLSL)
			sVI=std::string("struct VsIn\r\n{\r\n")+sVI+"};\r\n";
	}

	//PsIn
	if (_template->bPS)
	{
		std::map<std::string,int>mapSemCount;
		for (int i=0;i<varsRefer.size();i++)
		{
			ShaderVar *varBase;
			varBase=_FindBaseVar(varsRefer[i].c_str());
			if (!varBase)
				continue;
			if (varBase->category!=SVC_PsIn)
				continue;

			if (_bHLSL)
			{
				sVar=varBase->type+" "+varBase->name;
				if (varBase->sementic!="")
				{
					std::string sem=varBase->sementic;
					RemoveTailNumber(sem);

					std::map<std::string,int>::iterator it;
					it=mapSemCount.find(sem);
					if (it==mapSemCount.end())
						mapSemCount[sem]=0;
					else
						mapSemCount[sem]++;
					if (mapSemCount[sem]>0)
						AppendFmtString(sVar,":%s%d",sem.c_str(),mapSemCount[sem]);
					else
						AppendFmtString(sVar,":%s0",sem.c_str());
				}
				sVar+=";\r\n";
				sPI+="	"+sVar;
			}
			else
			{
				sVar="varying ";
				sVar=sVar+varBase->type+" "+varBase->name;
				sVar+=";\r\n";
				sPI+=sVar;
			}
		}		
		if (_bHLSL)
			sPI=std::string("struct PsIn\r\n{\r\n")+sPI+"};\r\n";
	}

	//VShader
	if (_template->bVS)
	{
		//the vars
		for (int i=0;i<varsRefer.size();i++)
		{
			ShaderVar *varBase,*varPrior;
			varBase=_FindBaseVar(varsRefer[i].c_str());
			if (!varBase)
				continue;
			if (varBase->category!=SVC_VsFactor)
				continue;

			varPrior=_FindPriorVar(varsRefer[i].c_str());
			assert(varPrior);

			if (varPrior->assign=="")
			{
				sVar=varBase->type+" "+varBase->name;
				if (varPrior->vInit)
				{
					std::string sInit;
					ExprNodeToString(varPrior->vInit,sInit,varBase->type.c_str());
					sVar=sVar+"="+sInit;
				}
				sVar+=";";
				_AppendPriorFeature(sVar,varsRefer[i].c_str());
				sVar+="\r\n";
			}
			else
			{
				sVar=varBase->type+" "+varBase->name+";";
				_AppendPriorFeature(sVar,varsRefer[i].c_str());
				sVar+="\r\n";
				sVar+=std::string("	{\r\n		")+varPrior->assign+"\r\n	}\r\n";
			}
			sVS=sVS+"\r\n	"+sVar;
		}		

		sVS+="\r\n	//Expressions\r\n";

		//The exprs
		for (int i=0;i<_template->exprVS.size();i++)
		{
			sExpr=_GetExprMainVar(_template->exprVS[i]);
			int idx;
			VEC_FIND(varsRefer,sExpr,idx);
			if (idx==-1)
				continue;//this expression is not referenced

			ExprNodeToString(_template->exprVS[i],sExpr);
			sVS=sVS+"	"+sExpr+";\r\n";
		}

		if (_bHLSL)
		{
			sVS=sVS+"	return pi;\r\n";

			sVS=std::string("PsIn VShader(VsIn vi)\r\n{\r\n	PsIn pi;\r\n")+sVS+"}\r\n";
		}
		else
		{
			sVS=sVS+"	gl_Position=pi_pos;\r\n";
			sVS=std::string("void VShader()\r\n{\r\n")+sVS+"}//EndOfVShader\r\n";
		}
	}


	//PShader
	if (_template->bPS)
	{
		std::string sOut,sFinal;
		if (_bHLSL)
			sFinal="\r\n	//Final Color Output\r\n	PsOut po;\r\n";

		int nOut=0;

		//the PsFactor
		for (int i=0;i<varsRefer.size();i++)
		{
			ShaderVar *varBase,*varPrior;
			varBase=_FindBaseVar(varsRefer[i].c_str());
			if (!varBase)
				continue;
			if (varBase->category!=SVC_PsFactor)
				continue;

			varPrior=_FindPriorVar(varsRefer[i].c_str());
			assert(varPrior);

			if (varPrior->assign=="")
			{
				sVar=varBase->type+" "+varBase->name;
				if (varPrior->vInit)
				{
					std::string sInit;
					ExprNodeToString(varPrior->vInit,sInit,varBase->type.c_str());
					sVar=sVar+"="+sInit;
				}
				sVar+=";";
				_AppendPriorFeature(sVar,varsRefer[i].c_str());
				sVar+="\r\n";
			}
			else
			{
				sVar=varBase->type+" "+varBase->name+";";
				_AppendPriorFeature(sVar,varsRefer[i].c_str());
				sVar+="\r\n";
				sVar+=std::string("	{\r\n		")+varPrior->assign+"\r\n	}\r\n";
			}
			sPS=sPS+"\r\n	"+sVar;
		}

		sPS=sPS+"\r\n	//Output Params";

		//the PsOut
		for (int i=0;i<varsRefer.size();i++)
		{
			ShaderVar *varBase,*varPrior;
			varBase=_FindBaseVar(varsRefer[i].c_str());
			if (!varBase)
				continue;
			if (varBase->category!=SVC_PsOut)
				continue;

			varPrior=_FindPriorVar(varsRefer[i].c_str());
			assert(varPrior);

			if (varPrior->assign=="")
				continue;

			if (_bHLSL)
			{
				for (int j=0;j<4;j++)
				{
					std::string s;
					FormatString(s,"po_color%d",j);
					if (s==varPrior->name)
					{
						AppendFmtString(sFinal,"	po.colors[%d]=po_color%d;\r\n",j,j);
						nOut=j+1;
					}
				}
			}

			sVar=varBase->type+" "+varBase->name+";";
			_AppendPriorFeature(sVar,varsRefer[i].c_str());
			sVar+="\r\n";
			sVar+=std::string("	{\r\n		")+varPrior->assign+"\r\n	}\r\n";
			sPS=sPS+"\r\n	"+sVar;
		}

		if (_bHLSL)
			sFinal+="	return po;\r\n";
		else
			sFinal="\r\n	gl_FragColor=po_color0;\r\n";

		if (_bHLSL)
		{
			FormatString(sOut,"struct PsOut\r\n{\r\n	float4 colors[%d]:COLOR0;\r\n};\r\n\r\n",nOut);

			sPS=sOut+"PsOut PShader(PsIn pi)\r\n{\r\n"+sPS+sFinal+	"	\r\n}\r\n";
		}
		else
			sPS=std::string("void PShader()\r\n{\r\n")+sPS+sFinal+	"	\r\n}//EndOfPShader\r\n";
	}

	//the Globals
	if (TRUE)
	{
		sGlobal="";
		for (int i=0;i<_features.size();i++)
		{
			if (_features[i]->global!="")
			{
				sGlobal+=std::string("////")+_features[i]->name+"\r\n";
				sGlobal+=_features[i]->global+"\r\n";
				sGlobal+="//\r\n";
			}
		}
	}

	//the tecnique/pass block
	if (_bHLSL)
	{
		std::vector<ShaderVar>statevars;
		std::vector<std::string>statevarfeatures;
		_CollectStates(statevars,statevarfeatures);

		for (int i=0;i<statevars.size();i++)
		{
			sVar=statevars[i].name;
			if (statevars[i].szArray>=0)
				AppendFmtString(sVar,"[%d]",statevars[i].szArray);
			sVar+="=";
			sVar+=statevars[i].statevalue;
			sVar+=";";
			sVar+="//"+statevarfeatures[i];

			sVar+="\r\n";

			sTec=sTec+"	"+sVar;
		}

		if (TRUE)//VS,PS versions
		{
			std::string verVS,verPS,t;
			if (_template->bVS)
				_ResolveShaderVer(verVS,TRUE);
			if (_template->bPS)
				_ResolveShaderVer(verPS,FALSE);

			//使两个版本号一致
			if (TRUE)
			{
				std::string ver;
				if (verVS!="")
					ver=verVS.c_str()+3;
				if (verPS!="")
				{
					std::string s=verPS.c_str()+3;
					if (s>ver)
						ver=s;
				}
				if (verVS!="")
				{
					if (ver=="1_4")
						verVS="vs_1_1";
					else
						verVS=std::string("vs_")+ver;
				}
				if (verPS!="")
					verPS=std::string("ps_")+ver;

			}

			t="	VertexShader=NULL;\r\n";
			if (_template->bVS)
			{
				if (verVS!="")
				{
					t="";
					t+="	VertexShader=compile "+verVS+" VShader();\r\n";
				}
			}
			sTec+=t;
			t="	PixelShader=NULL;\r\n";
			if (_template->bPS)
			{
				if (verPS!="")
				{
					t="";
					t+="	PixelShader=compile "+verPS+" PShader();\r\n";
				}
			}
			sTec+=t;
		}

		sTec=std::string("technique tec \r\n{ \r\n pass p0 \r\n{\r\n")+sTec+"}\r\n}\r\n";
	}
	else
		sTec="";


	//Now the caps
	if (_bHLSL)
	{
		std::vector<PriorShaderCap> caps;
		for (int i=0;i<_features.size();i++)
			for (int j=0;j<_features[i]->caps.size();j++)
			{
				PriorShaderCap &cap=_features[i]->caps[j];

				int k;
				for (k=0;k<caps.size();k++)
				{
					if (caps[k].code==cap.code)
					{
						if (caps[k].priority<cap.priority)
							caps[k]=cap;
						break;
					}
				}

				if (k>=caps.size())//Not found
					caps.push_back(cap);
			}

			capsRet.resize(caps.size());
			for (int i=0;i<caps.size();i++)
				capsRet[i]=(ShaderCap&)caps[i];
	}

}


void CSLComposer::ComposeFX(std::string &result,std::vector<ShaderCap>&capsRet)
{
	assert(_bHLSL);
	std::string sEP,sVI,sPI,sVS,sPS,sGlobal,sTec;
	FVFEx fvfAttributes;
	_Compose(sEP,sVI,sPI,sVS,sPS,sGlobal,sTec,capsRet,fvfAttributes);

	result=sEP+"\r\n"+sVI+"\r\n"+sPI+"\r\n"+sGlobal+"\r\n"+sVS+"\r\n"+sPS+"\r\n"+sTec;
	// 	LogFile::Prompt("%d,%d,%d,%d,%d",(int)(t2-t1),(int)(t3-t2),(int)(t4-t3),(int)(t5-t4),(int)(t6-t5));
}


BOOL CommentShaderFunc(std::vector<std::string> &lines,const char *sStart,const char *sEnd,std::string &sErr)
{
	//找到PixelShader的函数体,并把它注释掉
	int iStart=-1,iEnd=-1;
	for (int i=0;i<lines.size();i++)
	{
		if (lines[i].find(sEnd)!=-1)
		{
			iEnd=i;
			continue;
		}
		if (lines[i].find(sStart)!=-1)
			iStart=i;
	}
	if ((iStart==-1)||(iEnd==-1)||(iEnd<iStart))
	{
		FormatString(sErr,"Cannot find legal shader function body:(%s/%s pair)!",sStart,sEnd);
		return FALSE;
	}
	std::string sSlash="//";
	for (int i=iStart;i<=iEnd;i++)
		lines[i]=sSlash+lines[i];
	return TRUE;
}

BOOL ParseSamplerBlock(std::vector<std::string> &lines,std::string &nmSampler,int &iStart,int &iEnd)
{
	iStart=iEnd=-1;
	std::string ss;
	for (int i=0;i<lines.size();i++)
	{
		ss=lines[i];
		RemoveHeadBlank(ss);
		if (ss.find("sampler")==0)
		{
			iStart=i;
			std::string s;
			BOOL bSealed=FALSE;
			for (int ii=i;ii<lines.size();ii++)
			{
				if (lines[ii].find("}")!=-1)
					bSealed=TRUE;
				if (bSealed)
				{
					if (lines[ii].find(";")!=-1)
					{
						iEnd=ii;
						break;
					}
				}
			}
			break;
		}
	}

	if ((iStart!=-1)&&(iEnd!=-1))
	{
		std::string s;
		for (int i=iStart;i<=iEnd;i++)
			s+=lines[i];

		RemoveHeadBlank(s);

		if (memcmp(s.c_str(),"sampler",strlen("sampler"))==0)
		{
			char *p=(char *)s.c_str();
			p+=strlen("sampler");

			std::string nm;
			while(*p)
			{
				char c=*p;
				p++;

				if (c=='=')
					break;
				nm+=c;
			}
			RemoveHeadBlank(nm);
			RemoveTailBlank(nm);

			if (!nm.empty())
			{
				nmSampler=nm;
				return TRUE;
			}
		}
	}

	return FALSE;
}

void LegalizeSampler(std::vector<std::string> &lines)
{
	std::string nm;
	int iStart,iEnd;
	std::string sSlash="//";
	while(ParseSamplerBlock(lines,nm,iStart,iEnd))
	{
		FormatString(lines[iStart],"uniform sampler2D %s;",nm.c_str());
		for (int i=iStart+1;i<=iEnd;i++)
			lines[i]=sSlash+lines[i];
	}

}

//bUnifiedSrc指明src是不是包含所有shader代码(VertexShader/PixelShader),
BOOL PrepareVShader(BOOL bUnifiedSrc,std::string &src,std::string &srcResult,std::string &sErr)
{
	//分成一行一行
	std::vector<std::string> lines;
	SplitStringBy("\r\n",src,&lines);

	LegalizeSampler(lines);

	//找到PixelShader的函数体,并把它注释掉
	if (bUnifiedSrc)
	{
		if (FALSE==CommentShaderFunc(lines,"PShader","EndOfPShader",sErr))
			return FALSE;
	}

	LinkStringBy("\r\n",srcResult,&lines);

	srcResult=ReplaceString(srcResult.c_str(),"VShader","main");
	return TRUE;
}

BOOL PreparePShader(BOOL bUnifiedSrc,std::string &src,std::string &srcResult,std::string &sErr)
{
	//分成一行一行
	std::vector<std::string> lines;
	SplitStringBy("\r\n",src,&lines);

	LegalizeSampler(lines);

	//找到VertexShader的函数体,并把它注释掉
	if (bUnifiedSrc)
	{
		if (FALSE==CommentShaderFunc(lines,"VShader","EndOfVShader",sErr))
			return FALSE;
	}

	//注释掉attribute的变量
	if (TRUE)
	{
		std::string sSlash="//";
		for (int i=0;i<lines.size();i++)
		{
			if (lines[i].find("attribute")!=-1)
				lines[i]=sSlash+lines[i];
		}
	}

	LinkStringBy("\r\n",srcResult,&lines);

	srcResult=ReplaceString(srcResult.c_str(),"PShader","main");

	return TRUE;
}

//srcRaw里返回原始的source code,用来进行错误定位
BOOL CSLComposer::ComposeGL(std::string &resultVS,std::string &resultPS,std::string *srcRaw,std::string &err)
{
	assert(!_bHLSL);

	std::string sEP,sVI,sPI,sVS,sPS,sGlobal,sTec;
	FVFEx fvfAttributes;
	std::vector<ShaderCap>capsRet;
	_Compose(sEP,sVI,sPI,sVS,sPS,sGlobal,sTec,capsRet,fvfAttributes);

	resultVS="";
	resultPS="";

	std::string sWorking;
	sWorking=sEP+"\r\n"+sVI+"\r\n"+sGlobal+"\r\n"+sPI+"\r\n"+sVS;
	if (FALSE==PrepareVShader(FALSE,sWorking,resultVS,err))
	{
		if (srcRaw)
			*srcRaw=sWorking;
		return FALSE;
	}

	err="";
	sWorking=sEP+"\r\n"+sPI+"\r\n"+sGlobal+"\r\n"+sPS;
	if (FALSE==PreparePShader(FALSE,sWorking,resultPS,err))
	{
		if (srcRaw)
			*srcRaw=sWorking;
		return FALSE;
	}

	return TRUE;
}


//std::string sOut;
//std::string sFinal;
//if (TRUE)
//{
//	AppendFmtString(sFinal,"\r\n	//Output\r\n");
//	AppendFmtString(sFinal,"	PsOut po;\r\n");
//
//	int nOut=0;
//	for (int i=0;i<_template->vars.size();i++)
//	{
//		ShaderVar *var=&_template->vars[i];
//		if ((var->category==SVC_PsFactor)&&(var->sementic!=""))
//		{
//			int idx=0;
//			char elem[32];
//			if (TRUE)
//			{
//				int len=var->sementic.length();
//				assert(len>0);
//				if (isdigit(var->sementic.c_str()[len-1]))
//				{
//					idx=var->sementic.c_str()[len-1]-'0';
//					len--;
//				}
//
//				memcpy(elem,var->sementic.c_str(),len);
//				elem[len]=0; 
//			}
//
//			if (idx+1>nOut)
//				nOut=idx+1;
//
//			AppendFmtString(sFinal,"	po.colors[%d].%s=%s;\r\n",idx,elem,var->name.c_str());
//		}
//	}
//
//	AppendFmtString(sFinal,"	return po;\r\n");
//
//	AppendFmtString(sOut,"struct PsOut\r\n{\r\n");
//	AppendFmtString(sOut,"	float4 colors[%d]:COLOR0;\r\n",nOut);
//	AppendFmtString(sOut,"};\r\n");
//
//}
