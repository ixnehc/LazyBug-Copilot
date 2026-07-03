#pragma once

#include "../fastdelegate/FastDelegate.h"



class LogFile  
{
public:
	LogFile();
	LogFile(const char *fnLogFile);
	virtual ~LogFile();

	BOOL Create(const char *fnLogFile);

	static void Prompt(const char *formatstring,...);
	static BOOL PromptOkCancel(const char *formatstring,...);
	void Dump(const char *formatstring,...);
	BOOL IsValidate();
protected:
	BOOL _Create(const char *lpszFileName);
	BOOL _Write(const char *lpszContent,BOOL bAddReturn = TRUE);
	HANDLE _hFileLog;
	BOOL _bValidate;
	CRITICAL_SECTION _CriticalSetion;
};

