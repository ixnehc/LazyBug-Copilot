/********************************************************************
	created:	27:7:2006   20:28
	file path:	d:\IxEngine\Common\Log
	file base:	LogFile
	file ext:	cpp
	author:		cxi
	
	purpose:	log file used to dump information(to file or to screen)
*********************************************************************/
#include "stdh.h"
#include "LogFile.h"

#include "stdio.h"

#include "../stringparser/stringparser.h"

#include <string>
#include <tchar.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LogFile::LogFile()
{
	_bValidate = FALSE;
}

LogFile::LogFile(const char *fnLogFile)
{
	_bValidate = FALSE;

	_Create(fnLogFile);
}

BOOL LogFile::Create(const char *fnLogFile)
{
	if (_bValidate)
		return FALSE;

	return _Create(fnLogFile);
}

LogFile::~LogFile()
{
	_bValidate = FALSE;
	if (_hFileLog)
		CloseHandle(_hFileLog);
	DeleteCriticalSection(&_CriticalSetion);
}

BOOL LogFile::_Create(const char *lpszFileName)
{
	if (!lpszFileName[0])
		return FALSE;
	std::string path=lpszFileName;
	if (FALSE)
	{
#if defined(UNICODE) || defined(_UNICODE)
		wchar_t buffer[512];
#else
		char buffer[512];
#endif
		GetModuleFileName(NULL,buffer,512);
		path = toMBCS(buffer);
		path=path.substr(0,path.rfind('\\'));
		path+="\\log";
		CreateDirectory(fromMBCS(path.c_str()),NULL);
		path+="\\";
		std::string name;
		name=lpszFileName;
		name=name.substr(0,name.rfind('.'));
		path+=name;
		path+=".txt";
	}

	_hFileLog= CreateFile(fromMBCS(path.c_str()),GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	CloseHandle(_hFileLog);
	InitializeCriticalSection (&_CriticalSetion);

	_hFileLog= CreateFile(fromMBCS(path.c_str()),GENERIC_WRITE|GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	_bValidate = (_hFileLog!= NULL);
	return _bValidate;
}

BOOL LogFile::_Write(const char* lpszContent,BOOL bAddReturn)
{
	if(!_bValidate)
		return FALSE;
	EnterCriticalSection(&_CriticalSetion);
	if(_hFileLog )
	{
		DWORD dwNumberWrite;
		SetFilePointer(_hFileLog,0,NULL,FILE_END);
		WriteFile(_hFileLog,lpszContent,(DWORD)strlen(lpszContent),&dwNumberWrite,NULL);
		if(bAddReturn)
			WriteFile(_hFileLog,"\r\n",(DWORD)strlen("\r\n"),&dwNumberWrite,NULL);
	}
	LeaveCriticalSection(&_CriticalSetion);
	return (_hFileLog != NULL);
}
#include <stdio.h>

void LogFile::Dump(const char *formatstring,...)
{
	thread_local static char g_dumpbuff[16384];

	va_list args;

	SYSTEMTIME time;
	GetSystemTime(&time);
	sprintf(g_dumpbuff,"[%02d/%02d,%02d:%02d:%02d:%05d]- ",
				time.wMonth,time.wDay,(time.wHour+8)%24,time.wMinute,time.wSecond,time.wMilliseconds);
	DWORD len;
	len=strlen(g_dumpbuff);
	
	va_start(args,formatstring);
	int nSize = _vsnprintf(g_dumpbuff+len,sizeof(g_dumpbuff)-len, formatstring,args);
	va_end(args);
	_Write(g_dumpbuff,TRUE);
}

void LogFile::Prompt(const char *formatstring,...)
{
// 	va_list args;
// 
// 	va_start(args,formatstring);
// 	int nSize = _vsnprintf(g_dumpbuff,sizeof(g_dumpbuff), formatstring,args);
// 	va_end(args);
// 	MessageBox(NULL, fromMBCS(g_dumpbuff), _T("Log"),MB_OK);
}
 
BOOL LogFile::PromptOkCancel(const char *formatstring,...) 
{
// 	va_list args;
// 
// 	va_start(args,formatstring);
// 	int nSize = _vsnprintf(g_dumpbuff,sizeof(g_dumpbuff), formatstring,args);
// 	va_end(args);
// 	return (IDOK==MessageBox(NULL, fromMBCS(g_dumpbuff), _T("Log"),MB_OKCANCEL));
	return FALSE;
}



BOOL LogFile::IsValidate()
{
	return _bValidate;
}

