/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelUtil.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckVar.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckVar

BIND_BGN_CLASS(CBgn_CheckVar,CBgp_CheckVar);

void CBgn_CheckVar::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckVar*pad=_GetPad<CBgp_CheckVar>();

	CBehavior*bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		short n;
		if (_GetNumber(pad->nm,n))
		{
			int v=(int)n;
			BOOL b=FALSE;
			switch(pad->op)
			{
				case CBgp_CheckVar::EQ:
					b=(v==pad->vRef);break;
				case CBgp_CheckVar::NE:
					b=(v!=pad->vRef);break;
				case CBgp_CheckVar::GE:
					b=(v>=pad->vRef);break;
				case CBgp_CheckVar::GT:
					b=(v>pad->vRef);break;
				case CBgp_CheckVar::LE:
					b=(v<=pad->vRef);break;
				case CBgp_CheckVar::LT:
					b=(v<pad->vRef);break;
			}
			if (b)
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
}


////////////////////////////////////////////////////////////////////////
//CBgn_CompareBool_Obsolete

BIND_BGN_CLASS(CBgn_CompareBool_Obsolete,CBgp_CompareBool_Obsolete);

void CBgn_CompareBool_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CompareBool_Obsolete*pad=_GetPad<CBgp_CompareBool_Obsolete>();

	BOOL b1=_GetBPR(pad->bpr1);
	BOOL b2=_GetBPR(pad->bpr2);

	BOOL b=FALSE;
	switch(pad->op)
	{
	case CBgp_CheckVar::EQ:
		b=(b1==b2);break;
	case CBgp_CheckVar::NE:
		b=(b1!=b2);break;
	}
	if (b)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}




////////////////////////////////////////////////////////////////////////
//CBgn_CompareInt_Obsolete

BIND_BGN_CLASS(CBgn_CompareInt_Obsolete,CBgp_CompareInt_Obsolete);

void CBgn_CompareInt_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CompareInt_Obsolete*pad=_GetPad<CBgp_CompareInt_Obsolete>();

	int v1=_GetBPR(pad->bpr1);
	int v2=_GetBPR(pad->bpr2);

	BOOL b=FALSE;
	switch(pad->op)
	{
		case CBgp_CompareInt_Obsolete::EQ:
			b=(v1==v2);break;
		case CBgp_CompareInt_Obsolete::NE:
			b=(v1!=v2);break;
		case CBgp_CompareInt_Obsolete::GE:
			b=(v1>=v2);break;
		case CBgp_CompareInt_Obsolete::LE:
			b=(v1<=v2);break;
		case CBgp_CompareInt_Obsolete::GT:
			b=(v1>v2);break;
		case CBgp_CompareInt_Obsolete::LT:
			b=(v1<v2);break;
	}
	if (b)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}



////////////////////////////////////////////////////////////////////////
//CBgn_CompareNumber

BIND_BGN_CLASS(CBgn_CompareNumber,CBgp_CompareNumber);

void CBgn_CompareNumber::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CompareNumber*pad=_GetPad<CBgp_CompareNumber>();

	BOOL b=FALSE;
	switch(pad->tpNum)
	{
		case CBgp_CompareNumber::Boolean:
		{
			switch(pad->op)
			{
				case CBgp_CompareNumber::EQ:
					b=(pad->b1==pad->b2);break;
				case CBgp_CompareNumber::NE:
					b=(pad->b1!=pad->b2);break;
				case CBgp_CompareNumber::GE:
					b=(pad->b1>=pad->b2);break;
				case CBgp_CompareNumber::LE:
					b=(pad->b1<=pad->b2);break;
				case CBgp_CompareNumber::GT:
					b=(pad->b1>pad->b2);break;
				case CBgp_CompareNumber::LT:
					b=(pad->b1<pad->b2);break;
			}
			break;
		}
		case CBgp_CompareNumber::Int:
		{
			switch(pad->op)
			{
				case CBgp_CompareNumber::EQ:
					b=(pad->i1==pad->i2);break;
				case CBgp_CompareNumber::NE:
					b=(pad->i1!=pad->i2);break;
				case CBgp_CompareNumber::GE:
					b=(pad->i1>=pad->i2);break;
				case CBgp_CompareNumber::LE:
					b=(pad->i1<=pad->i2);break;
				case CBgp_CompareNumber::GT:
					b=(pad->i1>pad->i2);break;
				case CBgp_CompareNumber::LT:
					b=(pad->i1<pad->i2);break;
			}
			break;
		}
		case CBgp_CompareNumber::Float:
		{
			switch(pad->op)
			{
				case CBgp_CompareNumber::EQ:
					b=(pad->f1==pad->f2);break;
				case CBgp_CompareNumber::NE:
					b=(pad->f1!=pad->f2);break;
				case CBgp_CompareNumber::GE:
					b=(pad->f1>=pad->f2);break;
				case CBgp_CompareNumber::LE:
					b=(pad->f1<=pad->f2);break;
				case CBgp_CompareNumber::GT:
					b=(pad->f1>pad->f2);break;
				case CBgp_CompareNumber::LT:
					b=(pad->f1<pad->f2);break;
			}
			break;
		}
	}

	if (b)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}

////////////////////////////////////////////////////////////////////////
//CBgn_CompareStringID

BIND_BGN_CLASS(CBgn_CompareStringID,CBgp_CompareStringID);

void CBgn_CompareStringID::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CompareStringID*pad=_GetPad<CBgp_CompareStringID>();

	BOOL b=FALSE;
	switch(pad->op)
	{
		case CBgp_CompareNumber::EQ:
			b=(pad->v1==pad->v2);break;
		case CBgp_CompareNumber::NE:
			b=(pad->v1!=pad->v2);break;
	}

	if (b)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}

//////////////////////////////////////////////////////////////////////////
//CBgn_CheckVar_ID
BIND_BGN_CLASS(CBgn_CheckVar_ID,CBgp_CheckVar_ID);

void CBgn_CheckVar_ID::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckVar_ID*pad=_GetPad<CBgp_CheckVar_ID>();

	BehaviorMemType tpMem;
	StringID nm=StringID_Invalid;

	if (pad->type==0)
	{
		nm=pad->nm;
		tpMem=BehaviorMemType_ObjID;
	}
	if (pad->type==1)
	{
		nm=pad->nmBuff;
		tpMem=BehaviorMemType_BuffRecord;
	}
	if (pad->type==2)
	{
		nm=pad->nmSkill;
		tpMem=BehaviorMemType_SkillRecord;
	}

	if (pad->type==3)
	{
		nm=pad->nmItem;
		tpMem=BehaviorMemType_ItemRecord;
	}

	if (pad->type==4)
		nm=pad->nmObj;

	if (nm!=StringID_Invalid)
	{
		if (pad->type==4)
		{
			CBehaviorMem *mem=_GetMem();
			if (mem)
			{
				if (mem->GetObj(nm))
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
		else
		{
			DWORD id=0;
			if (_GetID(nm,tpMem,id))
			{
				if (id!=0)
				{
					LevelBehaviorContext *ctx=_GetCtx();

					if(pad->type==0)
					{
						if (!LevelUtil_GetAliveLo(ctx->level,id))
							id=0;
					}
				}

				if (id!=0)
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}
	_OutputFail(outputs,2,"否");
}
