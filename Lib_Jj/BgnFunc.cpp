/********************************************************************
	created:	2016/01/02 
	author:		cxi
	
	purpose:	 函数
*********************************************************************/
#include "stdh.h"
#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/Behavior.h"

#include "BgnFunc.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Func

BIND_BGN_CLASS(CBgn_Func,CBgp_Func);
void CBgn_Func::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Func*pad=_GetPad<CBgp_Func>();
	if (pad)
	{
		if (pad->_nm!=StringID_Invalid)
			outputs.Add(0,_thrd);
	}
	_SetResult(A_Ok);
}

////////////////////////////////////////////////////////////////////////
//CBgn_Call
BIND_BGN_CLASS(CBgn_Call,CBgp_Call);
void CBgn_Call::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Call*pad=_GetPad<CBgp_Call>();
	if (pad)
	{
		if (pad->_call._padFunc)
		{
			outputs.idRelay=pad->_call._padFunc->GetID();
			outputs.thrdRelay=_thrd;
			outputs.thrdRelay.keyRewind=0;
			outputs.thrdRelay.idNode=_id;

			if (TRUE)
			{
				std::unordered_map<StringID,BhvVal*>::iterator it;
				for (it=pad->_call._paramsRT.begin();it!=pad->_call._paramsRT.end();it++)
				{
					StringID nm=(*it).first;
					BhvVal *value=(*it).second;

					if (value->nmRef==StringID_BhvValInvalidRef)
						_call._paramsRT[nm]=value;//不是引用
					else
					{
						//Resolve引用
						BhvVal *valueNew=NULL;
						BhvValDeclare *declare=NULL;
						_FindValue(value->nmRef,valueNew,declare);

						if (!valueNew)
						{
							if (declare)
							{
								_values.resize(_values.size()+1);
								valueNew=&_values[_values.size()-1];
								declare->AssignDefault(*valueNew);
							}
							else
							{
								_values.resize(_values.size()+1);
								valueNew=&_values[_values.size()-1];
								valueNew->CopyFrom(*value);
								valueNew->nmRef=StringID_BhvValInvalidRef;

								if (nm>LevelSimpleVarName_Max)
								{
									CBehaviorMem *mem=_GetMem();
									if (mem)
									{
										if (!mem->FillBehaviorValue(value->nmRef,*valueNew))
										{
											_values.pop_back();
											valueNew=NULL;
										}
									}
								}
								else
								{
									//Simple Mem
									LevelSimpleMem *memSimple=_GetSimpleMem();
									if (memSimple)
									{
										if (!memSimple->FillBehaviorValue(value->nmRef,*valueNew))
										{
											_values.pop_back();
											valueNew=NULL;
										}
									}
								}
							}
						}

						if (valueNew)
							_call._paramsRT[nm]=valueNew;
					}
				}
			}
			outputs.thrdRelay.call=&_call;
			return;
		}
	}
	_OutputOk(outputs,1,"成功");
}

void CBgn_Call::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_OutputOk(outputs,1,"成功");
}

void CBgn_Call::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_OutputFail(outputs,2,"失败");
}

void CBgn_Call::Break(BGNOutputs &outputs)
{
	CBgp_Call*pad=_GetPad<CBgp_Call>();
	if (pad)
	{
		if (pad->_call._padFunc)
		{
			BgnThread thrd=_thrd;
			thrd.keyRewind=0;
			thrd.idNode=_id;
			thrd.call=NULL;

			outputs.thrdsBreak.push_back(thrd);
		}
	}
}

