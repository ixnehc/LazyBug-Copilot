/********************************************************************
	created:	2007/3/5   14:23
	filename: 	e:\IxEngine\Common\progress\progress.cpp
	author:		cxi
	
	purpose:	progress
*********************************************************************/
#include "stdh.h"

#include "progress.h"


BOOL CProgress::SetTitle(const char *title)
{
	if (this)
	if (_settitle)
		return _settitle(title);
	return FALSE;
}

BOOL CProgress::SetProgress(const char *desc,int cur,int full)
{
	if (this)
	if (_setprogess)
		return _setprogess(desc,cur,full);
	return FALSE;
}

BOOL CProgress::SetBegin(const char *name)
{
	if (this)
	if (_setbegin)
		return _setbegin(name);
	return FALSE;
}
BOOL CProgress::SetEnd()
{
	if (this)
	if(_setend)
		return _setend();
	return FALSE;
}


void CProgress::ClearHandlers()
{
	_setprogess=NULL;
	_settitle=NULL;
	_setbegin=NULL;
	_setend=NULL;
}
void CProgress::SetHandler(PrgHandler_SetTitle &handler)
{
	_settitle=handler;
}
void CProgress::SetHandler(PrgHandler_SetProgess &handler)
{
	_setprogess=handler;
}

void CProgress::SetBeginHanlder(PrgHandler_SetBegin &handler)
{
	_setbegin=handler;
}
void CProgress::SetEndHanlder(PrgHandler_SetEnd &handler)
{
	_setend=handler;
}
