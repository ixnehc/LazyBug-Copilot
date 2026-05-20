/********************************************************************
	created:	2011/4/21   10:43
	file path:	e:\IxEngine\Common\procedure
	author:		chenxi
	
	purpose:	procedure
*********************************************************************/
#include "stdh.h"

#include "../class/class.h"
#include "procedure.h"

#include "../log/LogDump.h"

void CProcedure::Destroy()
{
	_OnDestroy();

	if(_cur)
		_cur->Destroy();
	Safe_Class_Delete(_cur);
}

void CProcedure::Start()	
{		
	_Start();

}


void CProcedure::_Start()
{
	_locNext=NEXTLOC_START;
	_bEnd=FALSE;
	_tStart=_GetCurTime();
	_OnStart();
	_Main();
}

void CProcedure::_End()
{
	_OnEnd();
	_bEnd=TRUE;
	_locNext=NEXTLOC_END;
}

void CProcedure::Update()
{
	if (_cur)
	{
		if (!_cur->_bEnd)
			_cur->Update();
		if (_cur->_bEnd)
		{
			_cur->Destroy();
			Safe_Class_Delete(_cur);
		}
	}

	if (!_cur)
		_Main();

	_OnUpdate();//更新自己

	DWORD gd=_GetGuardDur();
	if (gd>0)
	{
		if (_GetCurTime()>(DWORD)(_tStart+gd))
		{
			LOG_DUMP_1P("Procedure",Log_Error,"Procedure(%s)执行时间过长,被强制中断!",GetClass()->GetName());
			_End();
		}
	}
}
