/********************************************************************
	created:	2015/07/03 
	author:		cxi
	
	purpose:	 调用Agent的函数
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnAGA_Call.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnAGA_Call
BIND_BGN_CLASS(CBgnAGA_Call,CBgpAGA_Call);


void CBgnAGA_Call::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAGA_Call*pad=_GetPad<CBgpAGA_Call>();

	_OutputOk(outputs,1,"结束");
}



