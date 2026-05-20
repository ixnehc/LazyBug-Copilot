/********************************************************************
	created:	2016/01/02 
	author:		cxi
	
	purpose:	 函数
*********************************************************************/
#include "stdh.h"
#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/Behavior.h"

#include "BgnMath.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Func

BIND_BGN_CLASS(CBgn_CalcFace,CBgp_CalcFace);
void CBgn_CalcFace::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CalcFace*pad=_GetPad<CBgp_CalcFace>();
	if (pad)
	{
		if ((pad->varSrcPos!=StringID_Invalid)&&(pad->varTargetPos!=StringID_Invalid)&&
			(pad->result!=StringID_Invalid))
		{
			LevelPos posSrc,posTarget;
			if (_GetPos(pad->varSrcPos,posSrc))
			{
				if (_GetPos(pad->varSrcPos,posTarget))
				{
					LevelPos dir=posTarget-posSrc;
					LevelFace face=LevelFaceFromDir(dir);
					if (_SetFloat(pad->result,face))
					{
						_OutputOk(outputs,1,"成功");
						return;
					}
				}
			}
		}
	}
	_OutputFail(outputs,2,"失败");
}
