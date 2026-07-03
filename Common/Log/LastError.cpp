/********************************************************************
	created:	27:7:2006   20:28
	file path:	d:\IxEngine\Common\Log
	file base:	LastError
	file ext:	cpp
	author:		cxi
	
	purpose:	last error recorder(in registry)
*********************************************************************/
#include "stdh.h"
#include "LastError.h"

#include <string>

#include "../Registry/Registry.h"
#include <stdio.h>

CCurrentUserRegistry g_regLastError("Ix Software","Ix Engine");


const char *GetLastErrString()
{
	static std::string s;
	s=g_regLastError.ReadString("LastError","String");
	return s.c_str();
}

void ClearLastErrString()
{
	SetLastErrString("");
}

void SetLastErrString(LPCSTR strMsg, ... )
{
	CHAR strBuffer[512];

	if (TRUE)
	{
		va_list args;
		va_start(args, strMsg);
		_vsnprintf( strBuffer, 512, strMsg, args );
		strBuffer[511] = '\0';
		va_end(args);
	}

	g_regLastError.WriteString("LastError","String",strBuffer);
}

void SetLastErrCode(DWORD err)
{
	g_regLastError.WriteInt("LastError","Code",(int)err);
}
DWORD GetLastErrCode()
{
	int v;
	v=g_regLastError.ReadInt("LastError","Code");
	return (DWORD)v;
}
