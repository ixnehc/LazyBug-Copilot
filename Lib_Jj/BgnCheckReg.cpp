/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckReg.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckReg,CBgp_CheckReg);

void CBgn_CheckReg::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckReg*pad=_GetPad<CBgp_CheckReg>();

	CBehavior*bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		DWORD*reg=bhv->FindReg(pad->nm);
		if (reg)
		{
			int v=(int)*reg;
			BOOL b=FALSE;
			switch(pad->op)
			{
				case CBgp_CheckReg::EQ:
					b=(v==pad->vRef);break;
				case CBgp_CheckReg::NE:
					b=(v!=pad->vRef);break;
				case CBgp_CheckReg::GE:
					b=(v>=pad->vRef);break;
				case CBgp_CheckReg::GT:
					b=(v>pad->vRef);break;
				case CBgp_CheckReg::LE:
					b=(v<=pad->vRef);break;
				case CBgp_CheckReg::LT:
					b=(v<pad->vRef);break;
			}
			if (b)
			{
				_VerifyStbName(1,"是");
				outputs.Add(1,_link);
				_SetResult(A_Ok);
				return;
			}
		}
	}

	_VerifyStbName(2,"否");
	outputs.Add(2,_link);
	_SetResult(A_Ok);
}

