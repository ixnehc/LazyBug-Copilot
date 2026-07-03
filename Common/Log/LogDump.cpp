/********************************************************************
	created:	5:12:2008   14:59
	filename: 	d:\IxEngine\Common\Log\LogDump.cpp
	author:		chenxi
	
	purpose:	log dump system
*********************************************************************/

#include "stdh.h"
#include "LogDump.h"

#include "stdio.h"

#include <string>

LogHandler g_handlerLog;

void RegisterLogHandler(LogHandler &handler)
{
	g_handlerLog=handler;
}

char g_dumpbuff2[2048];
void LogDump(const char *category,LogType type,int line,const char *file,const char *content,...)
{
	extern LogHandler g_handlerLog;
	if (g_handlerLog)
	{
		Log t;
		t.type=type;
		char buffer[128];
		sprintf(buffer,"Line %d in %s",line,file);
		t.place=buffer;
		t.category=category;

		va_list args;
		va_start(args,content);
		int nSize = _vsnprintf(g_dumpbuff2,sizeof(g_dumpbuff2),content,args);
		va_end(args);
		t.content=g_dumpbuff2;
		g_handlerLog(&t);
	}
}
