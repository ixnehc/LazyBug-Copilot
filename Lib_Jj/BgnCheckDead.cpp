/********************************************************************
	created:	2018/09/02 
	author:		cxi
	
	purpose:	 检查自己是否死亡
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordBuff.h"

#include "BgnCheckDead.h"

//////////////////////////////////////////////////////////////////////////
//CBgp_CheckDead
void CBgp_CheckDead::FillDesc(std::string &s,FillDescAssist *assist)
{
	if (_nmLo==StringID_Invalid)
		s="检测自己是否死亡";
	else
		FormatString(s,"检测游戏对象[%s]是否死亡",assist->GetStr(_nmLo));
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckDead,CBgp_CheckDead);

void CBgn_CheckDead::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckDead*pad=_GetPad<CBgp_CheckDead>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (level)
	{
		if (pad->_nmLo)
		{
			LevelObjID id;
			if (_GetID(pad->_nmLo,BehaviorMemType_ObjID,id))
			{
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				lo=LevelUtil_GetAliveLo(level,id);
			}
		}
	}


	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if (LevelUtil_CheckDead(lo))
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}

