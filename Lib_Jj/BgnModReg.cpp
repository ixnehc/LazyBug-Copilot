/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnModReg.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgn_ModReg
BIND_BGN_CLASS(CBgn_ModReg,CBgp_ModReg);


void CBgn_ModReg::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ModReg*pad=_GetPad<CBgp_ModReg>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		DWORD*reg=bhv->FindReg(pad->nm);
		if (reg)
		{
			switch(pad->mode)
			{
				case 0:
					*reg+=pad->vRef;
					break;
				case 1:
					*reg-=pad->vRef;
					break;
				case 2:
					*reg=pad->vRef;
					break;
			}
		}
		else
		{
			LOG_DUMP_1P("CBgn_ModReg",Log_Error,"无法找到寄存器(%s)!",StrLib_GetStr(pad->nm));
		}
	}

	_VerifyStbName(1,"结束");
	outputs.Add(1,_link);
	_SetResult(A_Ok);
}

