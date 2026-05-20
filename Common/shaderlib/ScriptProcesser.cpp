// ScriptParser.cpp: implementation of the CScriptParser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdh.h"
#include "ScriptProcesser.h"

#include "SLHolder.h"

#include <assert.h>

#pragma warning (disable:4786)


CScriptProcesser g_ScriptProcesser;
CScriptProcesser* g_pScriptProcesser=&g_ScriptProcesser;

/////////////////////////////////////////////////////////////////////////
//Action Functions

int AF_AcceptNoneVarCategory(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetCurSVC(SVC_None);
	return TRUE;
}

int AF_AcceptEPVarCategory(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetCurSVC(SVC_EffectParam);
	return TRUE;
}

int AF_AcceptStateVarCategory(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetCurSVC(SVC_StateVar);
	return TRUE;
}



int AF_AcceptVarCategory(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	switch(wcRecent.m_WordCode)
	{
		case C_RVertexShader:
			pProcesser->SetCurSVC(SVC_VsFactor);
			pProcesser->MarkVsPresence();
			break;
		case C_RPixelShader:
			pProcesser->SetCurSVC(SVC_PsFactor);
			pProcesser->MarkPsPresence();
			break;
		case C_RVertexIn:
			pProcesser->SetCurSVC(SVC_VsIn);
			pProcesser->MarkVsInPresence();
			break;
		case C_RPixelIn:
			pProcesser->SetCurSVC(SVC_PsIn);
			pProcesser->MarkPsInPresence();
			break;
		case C_RPixelOut:
			pProcesser->SetCurSVC(SVC_PsOut);
			pProcesser->MarkPsOutPresence();
			break;
		case C_REffectParam:
			pProcesser->SetCurSVC(SVC_EffectParam);
			pProcesser->MarkEPPresence();
			break;
		default:
			assert(FALSE);
			return FALSE;
	}
	return TRUE;
}

int AF_AcceptVarType(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (wcRecent.m_WordCode==C_Rtype)
	{
		pProcesser->SetCurVarType(wcRecent.m_String.c_str());
		return TRUE;
	}
	assert(FALSE);
	return FALSE;
}

int AF_AcceptVar(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (wcRecent.m_WordCode!=C_ID)
	{
		pParser->AddError(Err_InvalidVariableName,wcRecent.m_pos);
		return FALSE;
	}
	if (pProcesser->CheckVarDupe(wcRecent.m_String.c_str(),pProcesser->GetCurSVC()))
	{
		pParser->AddError(Err_VariableRedefined,wcRecent.m_pos);
		return FALSE;
	}
	pProcesser->AddVar(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptVarArraySize(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (wcRecent.m_WordCode==C_Number)
	{
		pProcesser->SetRecentVarArraySize(wcRecent.m_Value);
		return TRUE;
	}
	pParser->AddError(Err_InvalidArraySize,wcRecent.m_pos);
	return FALSE;
}

int AF_AcceptInitExpr(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (pProcesser->AcceptRecentVarInit())
		return TRUE;

	pParser->AddError(Err_InvalidInitExpression,wcRecent.m_pos);
	return FALSE;
}

int AF_AcceptCap(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (!pProcesser->AddCap(wcRecent.m_String.c_str()))
	{
		pParser->AddError(Err_InvalidCapName,wcRecent.m_pos);
		return FALSE;
	}
	return TRUE;
}

int AF_AcceptCapValue(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptRecentCapValue(wcRecent.m_Value);
	return TRUE;
}

int AF_SetDefaultCapPriority(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetRecentCapPriority(ShaderCapPriority_Default);
	return TRUE;
}

int AF_SetCapPriority(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetRecentCapPriority(wcRecent.m_Value);
	return TRUE;
}

int AF_AcceptVarSementic(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (wcRecent.m_WordCode==C_ID)
	{
		if (pProcesser->SetRecentVarSementic(wcRecent.m_String.c_str()))
			return TRUE;
	}

	pParser->AddError(Err_UnknownSymbol,wcRecent.m_pos);
	return FALSE;
}

int AF_AcceptVarAssign(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AppendRecentVarAssign(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptVarState(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AppendRecentVarState(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AccumExprCode(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (pProcesser->AccumExprCode(wcRecent))
		return TRUE;

	pParser->AddError(Err_ExpressionSyntaxError,wcRecent.m_pos);
	return FALSE;
}


int AF_AcceptExpr(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptExpr();
	return TRUE;
}

int AF_AcceptDefaultPriority(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->SetRecentVarPriority(ShaderVarPriority_Default);
	return TRUE;
}

int AF_AcceptPriority(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	if (wcRecent.m_WordCode==C_Number)
	{
		pProcesser->SetRecentVarPriority(wcRecent.m_Value);
		return TRUE;
	}

	pParser->AddError(Err_InvalidPriority,wcRecent.m_pos);
	return FALSE;
}

int AF_AcceptFeature(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	assert(wcRecent.m_WordCode==C_ID);

	pProcesser->AcceptFeature(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptFeatureFlag(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	assert(wcRecent.m_WordCode==C_ID);

	if(FALSE==pProcesser->AcceptFeatureFlag(wcRecent.m_String.c_str()))
	{
		pParser->AddError(Err_InvalidFeatureFlagName,wcRecent.m_pos);
		return FALSE;
	}
	return TRUE;
}


int AF_AcceptGlobal(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{

	pProcesser->AppendGlobal(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptVsVer(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptVsVer(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptPsVer(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptPsVer(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptFeatureGroupName(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptFeatureGroupName(wcRecent.m_String.c_str());
	return TRUE;
}

int AF_AcceptGroupFeature(CScriptParser *pParser,CScriptProcesser*pProcesser,CWordCode &wcRecent)
{
	pProcesser->AcceptGroupFeature(wcRecent.m_String.c_str());
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CScriptProcesser

int GetNeedOperateNum(int nOperator)
{
	switch (nOperator)
	{
	case O_LOGIC_NOT:
	case O_BITWISE_NOT:
	case O_NEGATIVE:
		return 1;
	default:
		return 2;
	}
}

BOOL IsByRight(int nOperatorID)
{
	switch( nOperatorID)
	{
	case O_EQUAL:
	case O_COMMA:
		return TRUE;
	default:
		return FALSE;
	}
}

int GetOperatorPriority(int nOperator)
{
	switch (nOperator)
	{
	case O_ARRAY:
		return 12;
	case O_LOGIC_NOT:
	case O_BITWISE_NOT:
	case O_NEGATIVE:
		return 11;
	case O_MULTIPLY:
	case O_DIVIDE:
	case O_REMAINDER:
		return 10;
	case O_ADD:
	case O_SUBTRACT:
		return 9;
	case O_LEFT_SHIFT:
	case O_RIGHT_SHIFT:
		return 8;
	case O_LESS_THAN:
	case O_LESS_THAN_OR_EQUAL:
	case O_GREATER_THAN:
	case O_GREATER_THAN_OR_EQUAL:
	case O_EQUAL:
		return 7;
	case O_NOT_EQUAL:
		return 6;
	case O_BITWISE_AND:
		return 5;
	case O_BITWISE_XOR:
		return 4;
	case O_BITWISE_OR:
		return 3;
	case O_LOGIC_AND:
		return 2;
	case O_LOGIC_OR:
		return 1;
	case O_ASSIGN:
		return 0;
	case O_COMMA:
		return -1;
	default:
		assert(FALSE);
	}

	return -1;
}



void CScriptProcesser::Clean()
{
	_cursvc=SVC_None;
	_bEP=_bVsIn=_bPsIn=_bPsOut=_bVS=_bPS=FALSE;

	_curtype="";
	_featurename="";
	_featureglobal="";
	_vs_ver="";
	_ps_ver="";
	_exprcodes.clear();

	for (int i=0;i<_vars.size();i++)
		_vars[i].Clean();
	_vars.clear();

	_caps.clear();

	for (int i=0;i<_exprVS.size();i++)
		_exprVS[i]->CleanAndDelete();
	for (int i=0;i<_exprPS.size();i++)
		_exprPS[i]->CleanAndDelete();

	_exprVS.clear();
	_exprPS.clear();

	_featuregroups.clear();

	_featureflag=FF_None;
}


void CScriptProcesser::AddVar(const char *name)
{
	_vars.resize(_vars.size()+1);
	ShaderVar *var=&_vars[_vars.size()-1];

	var->category=_cursvc;
	var->type=_curtype;
#pragma message("Need validate effect param var")
	var->name=name;
}

BOOL CScriptProcesser::SetRecentVarSementic(const char *sementic)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];


	var->sementic=sementic;

	return TRUE;
}

void CScriptProcesser::SetRecentVarAnnotation(const char *annotation)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];

	var->annotation=annotation;
}

#pragma message("Should make the init var of effect param a string that appending it")
BOOL CScriptProcesser::AcceptRecentVarInit()
{
	ExprNode *node;
	node=CalcExpr();
	if (!node)
		return FALSE;
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];
	var->vInit=node;
	return TRUE;
}

void CScriptProcesser::SetRecentVarArraySize(DWORD sz)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];
	var->szArray=sz;
}

void CScriptProcesser::SetRecentVarPriority(int priority)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];
	var->priority=priority;
}


void CScriptProcesser::AppendRecentVarAssign(const char *assign)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];
	if (var->assign=="")
		var->assign+=" ";//a seperator space
	var->assign+=assign;
	var->assign+=" ";//a seperator space
}

void CScriptProcesser::AppendRecentVarState(const char *state)
{
	assert(_vars.size()>0);

	ShaderVar *var=&_vars[_vars.size()-1];
	if (var->statevalue=="")
		var->statevalue+=" ";//a seperator space
	var->statevalue+=state;
	var->statevalue+=" ";//a seperator space
}

BOOL CScriptProcesser::AddCap(const char *name)
{
	PriorShaderCap cap;
	if (FALSE==cap.SetCode(name))
		return FALSE;
	_caps.push_back(cap);
	return TRUE;
}
void CScriptProcesser::AcceptRecentCapValue(int v)
{
	_caps[_caps.size()-1].SetValue(v);
}
void CScriptProcesser::SetRecentCapPriority(int priority)
{
	_caps[_caps.size()-1].priority=priority;
}



void CScriptProcesser::AcceptFeature(const char *feature)
{
	_featurename=feature;
	_featureflag=FF_None;
}

BOOL CScriptProcesser::AcceptFeatureFlag(const char *flagname)
{
	extern FeatureFlag FeatureFlagFromName(const char * name);
	
	FeatureFlag ff=FeatureFlagFromName(flagname);
	if (ff==FF_None)
		return FALSE;
	
	(DWORD&)_featureflag|=(DWORD)ff;

	return TRUE;
}


void CScriptProcesser::AppendGlobal(const char *global)
{
	if (_featureglobal=="")
		_featureglobal+=" ";//a seperator space
	_featureglobal+=global;
	_featureglobal+=" ";//a seperator space
}



BOOL CScriptProcesser::CheckVarDupe(const char *varname,ShaderVarCategory svc)
{
	for (int i=0;i<_vars.size();i++)
	{
		if (_vars[i].name==varname)
		{
			if ((_vars[i].category==SVC_StateVar)&&(svc==SVC_StateVar))
				continue;//A state var could have a same name with another state var
			return TRUE;
		}
	}
	return FALSE;
}

extern int g_CodeOperatorMapTable[];

BOOL CScriptProcesser::AccumExprCode(CWordCode wc)
{
	ExprCode c;
	if ((wc.m_WordCode>=C_Comma)&&(wc.m_WordCode<=C_GT))
	{
		c.type=ExprCode::Operator;
		c.op=g_CodeOperatorMapTable[wc.m_WordCode-C_Comma];
		_exprcodes.push_back(c);
		return TRUE;
	}
	if (wc.m_WordCode==C_ID)
	{
		c.type=ExprCode::Operand_Var;
		c.varname=wc.m_String.c_str();
		_exprcodes.push_back(c);
		return TRUE;
	}
	if (wc.m_WordCode==C_Number)
	{
		c.type=ExprCode::Operand_Number;
		c.varvalue=wc.m_Value;
		_exprcodes.push_back(c);
		return TRUE;
	}

	return FALSE;
}

ExprNode *ExprNodeFromCode(ExprCode *ec)
{
	if (ec->type==ExprCode::Node)
		return ec->node;
	ExprNode *node=new ExprNode;
	(ExprCode&)(*node)=*ec;
	return node;
}

void BuildExprNode(int op,ExprCode &ec,ExprCode *l,ExprCode *r)
{
	ExprNode *node=new ExprNode;

	node->type=ExprCode::Operator;
	node->op=op;

	if (l)
		node->left=ExprNodeFromCode(l);
	if (r)
		node->right=ExprNodeFromCode(r);

	ec.type=ExprCode::Node;
	ec.node=node;
}


ExprNode *CScriptProcesser::CalcExpr()
{
	int i;
	ExprCode stackOperand[200];//big enough
	ExprCode stackOperator[200];//big enough
	ExprCode *topOperand=stackOperand;
	ExprCode *topOperator=stackOperator;

	for (i=0;i<_exprcodes.size();i++)
	{
		ExprCode &ec=_exprcodes[i];

		switch(ec.type)
		{
		case ExprCode::Operand_Var:
		case ExprCode::Operand_Number:
			{
				*topOperand++=ec;
				break;
			}
		case ExprCode::Operator:
			{
				if (topOperator==stackOperator||ec.op==O_LEFT_BRACE)
					*topOperator++=ec;
				else if (ec.op==O_RIGHT_BRACE)//calculate the expression between braces,pop up left brace
				{
					ExprCode n;
					ExprCode *o=topOperator-1;
					ExprCode *r=topOperand-1;				
					while(1)
					{
						if(o->op==O_LEFT_BRACE)
							break;
						if (o<stackOperator)
							return FALSE;
						if (TRUE)
						{
							ExprCode nR,nL;
							nR=*r;
							if (GetNeedOperateNum(o->op)==2)
							{
								r--;
								nL=*r;

								BuildExprNode(o->op,n,&nL,&nR);
							}
							else
								BuildExprNode(o->op,n,NULL,&nR);
							*r=n;
						}
						o--;
					}
					o--;//pop up left brace
					topOperand=r+1;
					topOperator=o+1;
				}
				else
				{
					ExprCode *o=topOperator-1;
					ExprCode *r=topOperand-1;
					//there are higher priority than mine in stack, calculate them,still find a operatoer whose priority is less than mine
					while (o>=stackOperator&& o->op!=O_LEFT_BRACE && 
						(!IsByRight(ec.op) && GetOperatorPriority(ec.op) == GetOperatorPriority(o->op)
						|| GetOperatorPriority(ec.op) < GetOperatorPriority(o->op) )	)
					{
						ExprCode nR,nL,n;
						nR=*r;
						if (GetNeedOperateNum(o->op)==2)
						{
							r--;
							nL=*r;
							BuildExprNode(o->op,n,&nL,&nR);
						}
						else
							BuildExprNode(o->op,n,NULL,&nR);

						*r=n;
						o--;
					}
					topOperand=r+1;
					topOperator=o+1;
					//push to stack
					*topOperator++=ec;
				}
				break;
			}
		default:
			assert(FALSE);
		}
	}
	ExprCode *o=topOperator-1;
	ExprCode *r=topOperand-1;
	while(o>=stackOperator)
	{
		ExprCode nR,nL,n;
		nR=*r;
		if (GetNeedOperateNum(o->op)==2)
		{
			r--;
			nL=*r;
			BuildExprNode(o->op,n,&nL,&nR);
		}
		else
			BuildExprNode(o->op,n,NULL,&nR);
		*r=n;
		o--;
	}

	ExprNode*node=ExprNodeFromCode(r);
	_exprcodes.clear();

	return node;
}

BOOL CScriptProcesser::AcceptExpr()
{	
	ExprNode *node;
	node=CalcExpr();
	if (!node)
		return FALSE;

	if (_cursvc==SVC_VsFactor)
		_exprVS.push_back(node);
	else
	{
		if (_cursvc==SVC_PsFactor)
			_exprPS.push_back(node);
		else
		{
			assert(FALSE);
			delete node;
			return FALSE;
		}
	}

	return TRUE;
}

void CScriptProcesser::AcceptVsVer(const char *ver)
{
	_vs_ver=ver;
}
void CScriptProcesser::AcceptPsVer(const char *ver)
{
	_ps_ver=ver;
}


BOOL CScriptProcesser::AcceptFeatureGroupName(const char *group)
{
	_featuregroups.resize(_featuregroups.size()+1);
//	_featuregroups[_featuregroups.size()-1]=group;
	return TRUE;
}

BOOL CScriptProcesser::AcceptGroupFeature(const char *feature)
{
	std::string &s=_featuregroups[_featuregroups.size()-1];
	if (s!="")
		s+=",";
	s+=feature;

	return TRUE;
}

SLTemplate*CScriptProcesser::FetchTemplate()
{
	SLTemplate *holder=new SLTemplate;


	holder->bVS=_bVS;
	holder->bPS=_bPS;
	holder->bVsIn=_bVsIn;
	holder->bPsIn=_bPsIn;
	holder->bPsOut=_bPsOut;
	holder->bEP=_bEP;

	holder->exprVS=_exprVS;
	holder->exprPS=_exprPS;
	holder->vs_ver=_vs_ver;
	holder->ps_ver=_ps_ver;
	holder->vars=_vars;
	holder->featuregroups=_featuregroups;
	_exprVS.clear();
	_exprPS.clear();
	_vars.clear();

	Clean();

	return holder;
}

void CScriptProcesser::FetchFeature(SLFeature *holder)
{
	holder->fc.FromName(_featurename.c_str());
	holder->name=_featurename;
	holder->global=_featureglobal;
	holder->vs_ver=_vs_ver;
	holder->ps_ver=_ps_ver;

	holder->vars=_vars;
	holder->caps=_caps;
	holder->flag=_featureflag;

	_vars.clear();
	Clean();
}

SLFeature *CScriptProcesser::FetchFeature()
{
	SLFeature *holder=new SLFeature;
	FetchFeature(holder);
	return holder;
}
