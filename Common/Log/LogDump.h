#pragma once

#include "../fastdelegate/FastDelegate.h"


enum LogType
{
	Log_Error,
	Log_Warning,
	Log_Notify,
};

struct Log
{
	void Dump(const char *formatstring,...);
	LogType type;
	const char *content;
	const char *category;
	const char *place;
};


#define LOG_DUMP_ENABLED 

#ifdef LOG_DUMP_ENABLED
	#define LOG_DUMP(sys,type,content) 		LogDump(sys,type,__LINE__,__FILE__,content)
	#define LOG_DUMP_1P(sys,type,content,p0) 	LogDump(sys,type,__LINE__,__FILE__,content,p0)
	#define LOG_DUMP_2P(sys,type,content,p0,p1)	LogDump(sys,type,__LINE__,__FILE__,content,p0,p1)
	#define LOG_DUMP_3P(sys,type,content,p0,p1,p2)	LogDump(sys,type,__LINE__,__FILE__,content,p0,p1,p2)
	#define LOG_DUMP_4P(sys,type,content,p0,p1,p2,p3)	LogDump(sys,type,__LINE__,__FILE__,content,p0,p1,p2,p3)
	#define LOG_DUMP_5P(sys,type,content,p0,p1,p2,p3,p4)		LogDump(sys,type,__LINE__,__FILE__,content,p0,p1,p2,p3,p4)
	#define LOG_DUMP_6P(sys,type,content,p0,p1,p2,p3,p4,p5)	LogDump(sys,type,__LINE__,__FILE__,content,p0,p1,p2,p3,p4,p5)
#else
	#define LOG_DUMP(sys,name,content)
	#define LOG_DUMP_1P(sys,name,content,p0)
	#define LOG_DUMP_2P(sys,name,content,p0,p1)
	#define LOG_DUMP_3P(sys,name,content,p0,p1,p2)
	#define LOG_DUMP_4P(sys,name,content,p0,p1,p2,p3)
	#define LOG_DUMP_5P(sys,name,content,p0,p1,p2,p3,p4)
	#define LOG_DUMP_6P(sys,name,content,p0,p1,p2,p3,p4,p5)
#endif


struct LogHandler:public fastdelegate::FastDelegate1<Log*>
{
};

extern void LogDump(const char *category,LogType type,int line,const char *file,const char *content,...);

extern LogHandler g_handlerLog;
extern void RegisterLogHandler(LogHandler &handler);
